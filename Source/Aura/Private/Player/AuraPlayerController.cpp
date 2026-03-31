// @Copyright HaolunYuan
#include "Player/AuraPlayerController.h"
#include "AbilitySystemBlueprintLibrary.h"
#include "Components/AbilitySystem/AuraAbilitySystemComponent.h"
#include "Components/Player/AutoMoveComponent.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "Input/AuraInputComponent.h"
#include "Interaction/Highlightable.h"
#include "UI/HUD/AuraHUD.h"
AAuraPlayerController::AAuraPlayerController()
{
	// Replicate the controller so Server RPCs (e.g., AutoMoveComponent) can execute on the server.
	bReplicates = true;
	AutoMoveComponent = CreateDefaultSubobject<UAutoMoveComponent>(TEXT("AutoMoveComponent"));
}
void AAuraPlayerController::ToggleAttributeMenuRequested()
{
	AAuraHUD* AuraHUD = Cast<AAuraHUD>(GetHUD());
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
void AAuraPlayerController::BeginPlay()
{
	Super::BeginPlay();
	check(AuraContext);
	UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(GetLocalPlayer());
	if (Subsystem)
	{
		Subsystem->AddMappingContext(AuraContext, 0);
	}
	bShowMouseCursor = true;
	DefaultMouseCursor = EMouseCursor::Hand;
	FInputModeGameAndUI InputModeData;
	InputModeData.SetLockMouseToViewportBehavior(EMouseLockMode::DoNotLock);
	InputModeData.SetHideCursorDuringCapture(false);
	SetInputMode(InputModeData);
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
	checkf(KeyboardMovementAction, TEXT("KeyboardMoveAction is not set in PlayerController"));
	checkf(ClickToMoveAction, TEXT("ClickToMoveAction is not set in PlayerController"));
	checkf(ToggleAttributeMenuAction, TEXT("ToggleAttributeMenuAction is not set in PlayerController"));
	checkf(MoveToCursorAction, TEXT("MoveToCursorAction is not set in PlayerController"));
	AuraEnhancedInputComponent->BindAction(KeyboardMovementAction, ETriggerEvent::Triggered, this, &AAuraPlayerController::Move);
	AuraEnhancedInputComponent->BindAction(ClickToMoveAction, ETriggerEvent::Triggered, this, &AAuraPlayerController::OnClickMove);
	AuraEnhancedInputComponent->BindAction(ToggleAttributeMenuAction, ETriggerEvent::Started, this, &AAuraPlayerController::OnToggleAttributeMenu);
	AuraEnhancedInputComponent->BindAction(MoveToCursorAction, ETriggerEvent::Triggered, this, &AAuraPlayerController::OnMoveToCursor);
	check(InputConfigs);
	AuraEnhancedInputComponent->BindAbilityActions(InputConfigs, this, &AAuraPlayerController::AbilityInputTagPressed, &AAuraPlayerController::AbilityInputTagReleased, &AAuraPlayerController::AbilityInputTagHeld);
}
void AAuraPlayerController::Move(const FInputActionValue& ActionValues)
{
	if (AutoMoveComponent && (AutoMoveComponent->IsAutoMoving() || AutoMoveComponent->HasPendingPathRequest()))
	{
		AutoMoveComponent->RequestCancelAutoMove();
	}
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
	if (!bHasCachedMoveTargetLocation)
	{
		return;
	}
	check(AutoMoveComponent);
	if(!CurrentHighlightable)
	{
		AutoMoveComponent->RequestToMoveToLocation(CachedMoveTargetLocation);
	}
}
void AAuraPlayerController::OnMoveToCursor(const FInputActionValue& ActionValues)
{
	if (!bHasCachedMoveTargetLocation)
	{
		return;
	}
	check(AutoMoveComponent);

	if (!CurrentHighlightable)
	{
		AutoMoveComponent->MoveDirectlyToLocation(CachedMoveTargetLocation);
	}
}
void AAuraPlayerController::OnToggleAttributeMenu(const FInputActionValue& ActionValues)
{
	ToggleAttributeMenuRequested();
}
void AAuraPlayerController::CursorTrace()
{
	FHitResult CursorHitResult;
	GetHitResultUnderCursor(ECollisionChannel::ECC_Visibility, false, CursorHitResult);
	LastHighlightable = CurrentHighlightable;
	CurrentHighlightable = nullptr;
	if (CursorHitResult.bBlockingHit)
	{
		CachedMoveTargetLocation = CursorHitResult.ImpactPoint;
		bHasCachedMoveTargetLocation = true;
		AActor* HitActor = CursorHitResult.GetActor();
		if (HitActor && HitActor->Implements<UHighlightable>())
		{
			CurrentHighlightable = HitActor;
		}
	}
	else
	{
		bHasCachedMoveTargetLocation = false;
	}
	if (LastHighlightable != CurrentHighlightable)
	{
		if (LastHighlightable)
		{
			LastHighlightable->UnhighLightActor();
		}
		if (CurrentHighlightable)
		{
			CurrentHighlightable->HighLightActor();
		}
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
		return CachedASC;
	}
	return nullptr;
}
void AAuraPlayerController::AbilityInputTagPressed(FGameplayTag InputTag)
{ 
	if (CurrentHighlightable)
	{
		if (GetAuraASC())
		{
			GetAuraASC()->AbilityInputTagPressed(InputTag);
		}
	}
}
void AAuraPlayerController::AbilityInputTagReleased(FGameplayTag InputTag)
{
	if (CurrentHighlightable)
	{
		if (GetAuraASC())
		{
			GetAuraASC()->AbilityInputTagReleased(InputTag);
		}
	}
}
void AAuraPlayerController::AbilityInputTagHeld(FGameplayTag InputTag)
{
	if (CurrentHighlightable)
	{
		if (GetAuraASC())
		{
			GetAuraASC()->AbilityInputTagHeld(InputTag);
		}
	}
}