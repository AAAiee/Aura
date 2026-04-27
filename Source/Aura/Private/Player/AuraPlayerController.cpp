// @Copyright HaolunYuan

#include "Player/AuraPlayerController.h"

#include "AbilitySystemBlueprintLibrary.h"
#include "Components/AbilitySystem/AuraAbilitySystemComponent.h"
#include "Components/Player/AutoMoveComponent.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "GameFramework/Character.h"
#include "Input/AuraInputComponent.h"
#include "Interaction/Highlightable.h"
#include "UI/HUD/AuraHUD.h"
#include "UI/WidgetComponent/DamageWidgetComponent.h"

AAuraPlayerController::AAuraPlayerController()
{
	// Replicate the controller so Server RPCs (e.g., AutoMoveComponent) can execute on the server.
	bReplicates = true;
	AutoMoveComponent = CreateDefaultSubobject<UAutoMoveComponent>(TEXT("AutoMoveComponent"));
}

void AAuraPlayerController::ToggleAttributeMenuRequested()
{
	AAuraHUD* AuraHUD = GetAuraHUD();
	if (!AuraHUD)
	{
		return;
	}

	if (AuraHUD->IsAttributeMenuOnScreen())
	{
		AuraHUD->CloseAttributeMenu();
	}
	else
	{
		AuraHUD->ShowAttributeMenu();
	}
}

void AAuraPlayerController::Client_ShowDamageNumber_Implementation(float DamageAmount, ACharacter* TargetCharacter, bool bIsBlockedHit, bool bIsCriticalHit)
{
	if (IsValid(TargetCharacter) && IsValid(DamageTextComponentClass) && IsLocalController())
	{
		// We create a short-lived widget component per hit so combat text can exist in world space
		// without adding a permanently attached component to every character blueprint.
		UDamageWidgetComponent* DamageTextWidgetComponent = NewObject<UDamageWidgetComponent>(TargetCharacter, DamageTextComponentClass);
		DamageTextWidgetComponent->RegisterComponent();
		DamageTextWidgetComponent->AttachToComponent(TargetCharacter->GetRootComponent(), FAttachmentTransformRules::KeepRelativeTransform);

		// Detach immediately after placement so the text can animate independently instead of being
		// dragged around by any later root-motion or ragdoll movement on the target.
		DamageTextWidgetComponent->DetachFromComponent(FDetachmentTransformRules::KeepWorldTransform);

		// Blocked/critical state is server-authored effect-context metadata; the widget only decides
		// how to present those resolved combat facts.
		DamageTextWidgetComponent->SetDamageText(DamageAmount, bIsBlockedHit, bIsCriticalHit);
	}
}

void AAuraPlayerController::BeginPlay()
{
	Super::BeginPlay();

	InitializeInputContext();
	InitializeMouseCursorMode();
}

void AAuraPlayerController::PlayerTick(float DeltaTime)
{
	Super::PlayerTick(DeltaTime);

	if (IsLocalController())
	{
		CursorTrace();
	}
}

void AAuraPlayerController::SetupInputComponent()
{
	Super::SetupInputComponent();

	UEnhancedInputComponent* EnhancedInputComponent = CastChecked<UEnhancedInputComponent>(InputComponent);
	UAuraInputComponent* AuraEnhancedInputComponent = CastChecked<UAuraInputComponent>(EnhancedInputComponent);

	BindNativeInputActions(AuraEnhancedInputComponent);

	check(InputConfigs);
	AuraEnhancedInputComponent->BindAbilityActions(InputConfigs, this, &AAuraPlayerController::AbilityInputTagPressed, &AAuraPlayerController::AbilityInputTagReleased, &AAuraPlayerController::AbilityInputTagHeld);
}

void AAuraPlayerController::InitializeInputContext() const
{
	check(AuraContext);

	UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(GetLocalPlayer());
	if (Subsystem)
	{
		Subsystem->AddMappingContext(AuraContext, 0);
	}
}

void AAuraPlayerController::InitializeMouseCursorMode()
{
	bShowMouseCursor = true;
	DefaultMouseCursor = EMouseCursor::Hand;

	FInputModeGameAndUI InputModeData;
	InputModeData.SetLockMouseToViewportBehavior(EMouseLockMode::DoNotLock);
	InputModeData.SetHideCursorDuringCapture(false);
	SetInputMode(InputModeData);
}

void AAuraPlayerController::BindNativeInputActions(UAuraInputComponent* AuraEnhancedInputComponent)
{
	checkf(KeyboardMovementAction, TEXT("KeyboardMoveAction is not set in PlayerController"));
	checkf(ClickToMoveAction, TEXT("ClickToMoveAction is not set in PlayerController"));
	checkf(ToggleAttributeMenuAction, TEXT("ToggleAttributeMenuAction is not set in PlayerController"));
	checkf(MoveToCursorAction, TEXT("MoveToCursorAction is not set in PlayerController"));
	checkf(AttackHelpAction, TEXT("AttackHelpAction is not set in PlayerController"));

	AuraEnhancedInputComponent->BindAction(KeyboardMovementAction, ETriggerEvent::Triggered, this, &AAuraPlayerController::Move);
	AuraEnhancedInputComponent->BindAction(ClickToMoveAction, ETriggerEvent::Triggered, this, &AAuraPlayerController::OnClickMove);
	AuraEnhancedInputComponent->BindAction(ToggleAttributeMenuAction, ETriggerEvent::Started, this, &AAuraPlayerController::OnToggleAttributeMenu);
	AuraEnhancedInputComponent->BindAction(MoveToCursorAction, ETriggerEvent::Triggered, this, &AAuraPlayerController::OnMoveToCursor);
	AuraEnhancedInputComponent->BindAction(AttackHelpAction, ETriggerEvent::Started, this, &AAuraPlayerController::OnAttackHelpPressed);
	AuraEnhancedInputComponent->BindAction(AttackHelpAction, ETriggerEvent::Completed, this, &AAuraPlayerController::OnAttackHelpReleased);
}

AAuraHUD* AAuraPlayerController::GetAuraHUD() const
{
	return Cast<AAuraHUD>(GetHUD());
}

void AAuraPlayerController::OnToggleAttributeMenu(const FInputActionValue& ActionValues)
{
	ToggleAttributeMenuRequested();
}

void AAuraPlayerController::Move(const FInputActionValue& ActionValues)
{
	CancelAutoMoveIfActive();

	const FVector2D MoveVector = ActionValues.Get<FVector2D>();
	const FRotator ControllerRotation = GetControlRotation();
	const FRotator YawRotation(0.f, ControllerRotation.Yaw, 0.f);
	const FVector PlayerForwardDirection = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::X);
	const FVector PlayerRightDirection = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::Y);

	if (APawn* ControlledPawn = GetPawn())
	{
		ControlledPawn->AddMovementInput(PlayerForwardDirection, MoveVector.Y);
		ControlledPawn->AddMovementInput(PlayerRightDirection, MoveVector.X);
	}
}

void AAuraPlayerController::OnClickMove(const FInputActionValue& ActionValues)
{
	check(AutoMoveComponent);

	FVector MoveTargetLocation = FVector::ZeroVector;
	if (TryGetCachedMoveTargetLocation(MoveTargetLocation))
	{
		AutoMoveComponent->RequestToMoveToLocation(MoveTargetLocation);
	}
}

void AAuraPlayerController::OnMoveToCursor(const FInputActionValue& ActionValues)
{
	check(AutoMoveComponent);

	FVector MoveTargetLocation = FVector::ZeroVector;
	if (TryGetCachedMoveTargetLocation(MoveTargetLocation))
	{
		AutoMoveComponent->MoveDirectlyToLocation(MoveTargetLocation);
	}
}

void AAuraPlayerController::OnAttackHelpPressed(const FInputActionValue& ActionValues)
{
	bIsAttackHelpKeyPressed = true;
}

void AAuraPlayerController::OnAttackHelpReleased(const FInputActionValue& ActionValues)
{
	bIsAttackHelpKeyPressed = false;
}

void AAuraPlayerController::CancelAutoMoveIfActive() const
{
	if (AutoMoveComponent && (AutoMoveComponent->IsAutoMoving() || AutoMoveComponent->HasPendingPathRequest()))
	{
		AutoMoveComponent->RequestCancelAutoMove();
	}
}

bool AAuraPlayerController::TryGetCachedMoveTargetLocation(FVector& OutMoveTargetLocation) const
{
	if (!CachedCursorHitResult.bBlockingHit)
	{
		return false;
	}

	if (bIsTargeting || bIsAttackHelpKeyPressed)
	{
		return false;
	}

	OutMoveTargetLocation = CachedCursorHitResult.ImpactPoint;
	return true;
}

void AAuraPlayerController::CursorTrace()
{
	FHitResult CursorHitResult;
	GetHitResultUnderCursor(ECollisionChannel::ECC_Visibility, false, CursorHitResult);

	UpdateCachedCursorHitResult(CursorHitResult);

	LastHighlightable = CurrentHighlightable;
	UpdateCurrentHighlightable(CursorHitResult);
	ApplyHighlightStateTransition();

	bIsTargeting = CurrentHighlightable != nullptr;
}

void AAuraPlayerController::UpdateCachedCursorHitResult(const FHitResult& CursorHitResult)
{
	CachedCursorHitResult = CursorHitResult;
}

void AAuraPlayerController::UpdateCurrentHighlightable(const FHitResult& CursorHitResult)
{
	CurrentHighlightable = nullptr;

	if (!CursorHitResult.bBlockingHit)
	{
		return;
	}

	AActor* HitActor = CursorHitResult.GetActor();
	if (HitActor && HitActor->Implements<UHighlightable>())
	{
		CurrentHighlightable = HitActor;
	}
}

void AAuraPlayerController::ApplyHighlightStateTransition()
{
	if (LastHighlightable == CurrentHighlightable)
	{
		return;
	}

	if (LastHighlightable)
	{
		LastHighlightable->UnhighLightActor();
	}

	if (CurrentHighlightable)
	{
		CurrentHighlightable->HighLightActor();
	}
}

UAuraAbilitySystemComponent* AAuraPlayerController::GetAuraASC()
{
	if (CachedASC)
	{
		return CachedASC;
	}

	if (APawn* ControlledPawn = GetPawn())
	{
		CachedASC = Cast<UAuraAbilitySystemComponent>(UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(ControlledPawn));
	}

	return CachedASC;
}

void AAuraPlayerController::AbilityInputTagPressed(FGameplayTag InputTag)
{
	ForwardAbilityInputTag(InputTag, &UAuraAbilitySystemComponent::AbilityInputTagPressed);
}

void AAuraPlayerController::AbilityInputTagReleased(FGameplayTag InputTag)
{
	ForwardAbilityInputTag(InputTag, &UAuraAbilitySystemComponent::AbilityInputTagReleased);
}

void AAuraPlayerController::AbilityInputTagHeld(FGameplayTag InputTag)
{
	ForwardAbilityInputTag(InputTag, &UAuraAbilitySystemComponent::AbilityInputTagHeld);
}

void AAuraPlayerController::ForwardAbilityInputTag(FGameplayTag InputTag, void (UAuraAbilitySystemComponent::*InputHandler)(FGameplayTag))
{
	if (!CouldLaunchGameplayAbility())
	{
		return;
	}

	CancelAutoMoveIfActive();

	if (UAuraAbilitySystemComponent* AuraASC = GetAuraASC())
	{
		(AuraASC->*InputHandler)(InputTag);
	}
}

bool AAuraPlayerController::CouldLaunchGameplayAbility() const
{
	return bIsTargeting || bIsAttackHelpKeyPressed;
}
