// @Copyright HaolunYuan

#include "Player/AuraPlayerController.h"

#include "AbilitySystemBlueprintLibrary.h"
#include "Aura/Aura.h"
#include "AuraGameTagManager.h"
#include "Character/AuraCharacter.h"
#include "Components/AbilitySystem/AuraAbilitySystemComponent.h"
#include "Components/DecalComponent.h"
#include "Components/Player/AutoMoveComponent.h"
#include "Effect/MagicCircle.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "GameFramework/Character.h"
#include "GameFramework/SpringArmComponent.h"
#include "Input/AuraInputComponent.h"
#include "Interaction/EnemyInterface.h"
#include "Interaction/Highlightable.h"
#include "NiagaraFunctionLibrary.h"
#include "InventoryManagement/Component/InvSS_InventoryComponent.h"
#include "UI/HUD/AuraHUD.h"
#include "UI/WidgetComponent/DamageWidgetComponent.h"

AAuraPlayerController::AAuraPlayerController()
{
	// Replicate the controller so Server RPCs (e.g., AutoMoveComponent) can execute on the server.
	bReplicates = true;
	AutoMoveComponent = CreateDefaultSubobject<UAutoMoveComponent>(TEXT("AutoMoveComponent"));
}

/* Player-Facing Presentation : ToggleAttributeMenuRequested() ShowMagicCircle() HideMagicCircle() Client_ShowDamageNumber() *****************************/

void AAuraPlayerController::ToggleAttributeMenuRequested()
{
	if (!CachedAuraHUD)
	{
		return;
	}

	if (AutoMoveComponent->IsAutoMoving())
	{
		AutoMoveComponent->RequestCancelAutoMove();
	}

	if (CachedAuraHUD->IsAttributeMenuOnScreen())
	{
		CachedAuraHUD->CloseAttributeMenu();
	}
	else
	{
		CachedAuraHUD->ShowAttributeMenu();
	}
}

void AAuraPlayerController::ShowMagicCircle(UMaterialInterface* InMaterial)
{
	if (!IsValid(MagicCircle))
	{
		MagicCircle = GetWorld()->SpawnActor<AMagicCircle>(MagicCircleClass);
		if (InMaterial != nullptr)
		{
			MagicCircle->MagicCircleDecal->SetMaterial(0, InMaterial);
		}
	}
}

void AAuraPlayerController::HideMagicCircle()
{
	if (IsValid(MagicCircle))
	{
		MagicCircle->Destroy();
		MagicCircle = nullptr;
	}
}

void AAuraPlayerController::ToggleInventoryMenu()
{
	GetInventoryComponent()->ToggleInventoryMenu();
}

UInvSS_InventoryComponent* AAuraPlayerController::GetInventoryComponent()
{
	if (!InventoryComponent.IsValid())
	{
		InventoryComponent = FindComponentByClass<UInvSS_InventoryComponent>();
	}

	check(InventoryComponent.IsValid())
	return InventoryComponent.Get();
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

/* Controller Setup And Ticking : PlayerTick() BeginPlay() SetupInputComponent() OnPossess() MakePlayerFacingCameraLookAtDirection() *****************************/

void AAuraPlayerController::PlayerTick(float DeltaTime)
{
	Super::PlayerTick(DeltaTime);

	if (IsLocalController())
	{
		CursorTrace();
		UpdateMagicCirclePosition();
	}
}

void AAuraPlayerController::BeginPlay()
{
	Super::BeginPlay();

	// Pipeline:
	// 1. Install the local input mapping context.
	// 2. Configure mouse cursor behavior for game/UI play.
	// 3. Align the possessed Aura character to the fixed combat camera direction.
	InitializeInputContext();
	InitializeMouseCursorMode();

	if (!MakePlayerFacingCameraLookAtDirection())
	{
		return;
	}
	
	InventoryComponent = FindComponentByClass<UInvSS_InventoryComponent>();
	checkf(InventoryComponent.IsValid(), TEXT("InventoryComponent is not found on %s."), *GetName());
}

void AAuraPlayerController::SetupInputComponent()
{
	Super::SetupInputComponent();

	// Pipeline:
	// 1. Resolve the Enhanced Input component into Aura's typed input component.
	// 2. Bind native movement, UI, and targeting actions.
	// 3. Bind Gameplay Ability input tags to pressed/released handlers.
	UEnhancedInputComponent* EnhancedInputComponent = CastChecked<UEnhancedInputComponent>(InputComponent);
	UAuraInputComponent* AuraEnhancedInputComponent = CastChecked<UAuraInputComponent>(EnhancedInputComponent);

	BindNativeInputActions(AuraEnhancedInputComponent);

	check(InputConfigs);
	AuraEnhancedInputComponent->BindAbilityActions(
		InputConfigs,
		this,
		&AAuraPlayerController::AbilityInputTagTriggered,
		&AAuraPlayerController::AbilityInputTagEnded,
		&AAuraPlayerController::AbilityInputTagEnded);
}

void AAuraPlayerController::OnPossess(APawn* InPawn)
{
	Super::OnPossess(InPawn);
}

bool AAuraPlayerController::MakePlayerFacingCameraLookAtDirection()
{
	CachedAuraHUD = Cast<AAuraHUD>(GetHUD());
	if (AAuraCharacter* AuraCharacter = Cast<AAuraCharacter>(GetPawn()))
	{
		USpringArmComponent* SpringArm = AuraCharacter->FindComponentByClass<USpringArmComponent>();
		if (!SpringArm)
		{
			return false;
		}

		const float CameraPitch = -40.f;
		const float SpawnYaw = AuraCharacter->GetActorRotation().Yaw;

		const FRotator CameraRot(CameraPitch, SpawnYaw, 0.f);

		SetControlRotation(CameraRot);

		SpringArm->bUsePawnControlRotation = false;
		SpringArm->bInheritPitch = false;
		SpringArm->bInheritYaw = false;
		SpringArm->bInheritRoll = false;

		SpringArm->SetUsingAbsoluteRotation(true);
		SpringArm->SetWorldRotation(CameraRot);
	}

	return true;
}

/* Input Setup : InitializeInputContext() InitializeMouseCursorMode() BindNativeInputActions() *****************************/

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
	AuraEnhancedInputComponent->BindAction(ToggleInventoryMenuAction, ETriggerEvent::Started, this, &AAuraPlayerController::OnToggleInventoryMenu);
}

/* UI Menu Flow : OnToggleAttributeMenu() ToggleInventoryMenu() IsAnyMenuOnScreen() *****************************/

void AAuraPlayerController::OnToggleAttributeMenu(const FInputActionValue& ActionValues)
{
	ToggleAttributeMenuRequested();
}

void AAuraPlayerController::OnToggleInventoryMenu(const FInputActionValue& ActionValues)
{
	ToggleInventoryMenu();
}

bool AAuraPlayerController::IsAnyMenuOnScreen() const
{
	if (!CachedAuraHUD)
	{
		return false;
	}

	// Floating menus consume attention and cursor intent, so movement commands pause while either
	// menu is visible. The HUD owns the actual widget lifetime; the controller only asks for state.
	return CachedAuraHUD->IsSpellMenuOnScreen() || CachedAuraHUD->IsAttributeMenuOnScreen();
}

/* Movement Commands : Move() OnClickMove() OnMoveToCursor() CancelAutoMoveIfActive() TryGetCachedMoveTargetLocation() *****************************/

void AAuraPlayerController::Move(const FInputActionValue& ActionValues)
{
	if (GetAuraASC() && GetAuraASC()->HasMatchingGameplayTag(FAuraGameTagManager::Get().PLayer_BlockInputPressed))
	{
		return;
	}
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
	if (!AutoMoveComponent)
	{
		return;
	}
	if (IsAnyMenuOnScreen())
	{
		return;
	}
	if (GetAuraASC() && GetAuraASC()->HasMatchingGameplayTag(FAuraGameTagManager::Get().PLayer_BlockInputPressed))
	{
		return;
	}

	FVector MoveTargetLocation = FVector::ZeroVector;
	if (TryGetCachedMoveTargetLocation(MoveTargetLocation))
	{
		AutoMoveComponent->RequestToMoveToLocation(MoveTargetLocation);
		UNiagaraFunctionLibrary::SpawnSystemAtLocation(this, OnClickMoveNiagaraSystem, MoveTargetLocation);
	}
}

void AAuraPlayerController::OnMoveToCursor(const FInputActionValue& ActionValues)
{
	if (!AutoMoveComponent)
	{
		return;
	}
	if (IsAnyMenuOnScreen())
	{
		return;
	}
	if (GetAuraASC() && GetAuraASC()->HasMatchingGameplayTag(FAuraGameTagManager::Get().PLayer_BlockInputPressed))
	{
		return;
	}

	FVector MoveTargetLocation = FVector::ZeroVector;
	if (TryGetCachedMoveTargetLocation(MoveTargetLocation))
	{
		AutoMoveComponent->MoveDirectlyToLocation(MoveTargetLocation);
	}
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

/* Cursor Targeting : CursorTrace() UpdateCachedCursorHitResult() UpdateCurrentHighlightable() ApplyHighlightStateTransition() UpdateMagicCirclePosition() *****************************/

void AAuraPlayerController::CursorTrace()
{
	if (GetAuraASC() && GetAuraASC()->HasMatchingGameplayTag(FAuraGameTagManager::Get().PLayer_BlockCursorTrace))
	{
		if (LastHighlightable)
		{
			LastHighlightable->UnhighLightActor();
		}
		if (CurrentHighlightable)
		{
			CurrentHighlightable->UnhighLightActor();
		}
		LastHighlightable = nullptr;
		CurrentHighlightable = nullptr;
		return;
	}

	// Pipeline:
	// 1. Trace under the cursor using the channel required by the current targeting mode.
	// 2. Cache the hit result so movement and ability targeting consume the same point.
	// 3. Update highlight state only when the magic circle is not taking over cursor targeting.
	FHitResult CursorHitResult;
	const ECollisionChannel TraceChannel = IsValid(MagicCircle) ? ECC_ExcludeEnemiesAndPlayers : ECC_Visibility;
	GetHitResultUnderCursor(TraceChannel, false, CursorHitResult);
	UpdateCachedCursorHitResult(CursorHitResult);

	if (!IsValid(MagicCircle))
	{
		LastHighlightable = CurrentHighlightable;
		UpdateCurrentHighlightable(CursorHitResult);
		ApplyHighlightStateTransition();
	}

	if (CachedCursorHitResult.bBlockingHit && CachedCursorHitResult.GetActor()->Implements<UEnemyInterface>())
	{
		bIsTargeting = true;
	}
	else
	{
		bIsTargeting = false;
	}
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

void AAuraPlayerController::ApplyHighlightStateTransition() const
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

void AAuraPlayerController::UpdateMagicCirclePosition() const
{
	if (IsValid(MagicCircle))
	{
		MagicCircle->SetActorLocation(CachedCursorHitResult.ImpactPoint);
	}
}

/* Ability Input : OnAttackHelpPressed() OnAttackHelpReleased() GetAuraASC() AbilityInputTagTriggered() AbilityInputTagEnded() CouldLaunchGameplayAbility() *****************************/

void AAuraPlayerController::OnAttackHelpPressed(const FInputActionValue& ActionValues)
{
	bIsAttackHelpKeyPressed = true;
}

void AAuraPlayerController::OnAttackHelpReleased(const FInputActionValue& ActionValues)
{
	bIsAttackHelpKeyPressed = false;
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

void AAuraPlayerController::AbilityInputTagTriggered(FGameplayTag InputTag)
{
	// Pipeline:
	// 1. Validate the tag and resolve the current Aura ASC.
	// 2. Respect block-input tags before forwarding player intent.
	// 3. Consume repeated Triggered events until the matching release/end event arrives.
	if (!ensureMsgf(InputTag.IsValid(), TEXT("AbilityInputTagTriggered received an invalid input tag.")))
	{
		return;
	}

	UAuraAbilitySystemComponent* AuraASC = GetAuraASC();
	if (!ensureMsgf(AuraASC, TEXT("AbilityInputTagTriggered could not resolve Aura ASC for [%s]."), *InputTag.ToString()))
	{
		return;
	}

	if (AuraASC->HasMatchingGameplayTag(FAuraGameTagManager::Get().PLayer_BlockInputPressed))
	{
		return;
	}

	if (ConsumedTriggeredInputTags.Contains(InputTag))
	{
		return;
	}

	const bool bCanActivateInactiveAbility = CouldLaunchGameplayAbility();

	// Mark consumed before forwarding. The ASC may satisfy WaitInputPress and end the ability
	// immediately, and later Triggered events from the same hold must not reactivate it.
	ConsumedTriggeredInputTags.Add(InputTag);
	const bool bHandledByAbilitySystem = AuraASC->AbilityInputTagTriggered(InputTag, bCanActivateInactiveAbility);
	if (!bHandledByAbilitySystem)
	{
		ConsumedTriggeredInputTags.Remove(InputTag);
		return;
	}

	CancelAutoMoveIfActive();
}

void AAuraPlayerController::AbilityInputTagEnded(FGameplayTag InputTag)
{
	if (!ensureMsgf(InputTag.IsValid(), TEXT("AbilityInputTagEnded received an invalid input tag.")))
	{
		return;
	}

	ConsumedTriggeredInputTags.Remove(InputTag);

	UAuraAbilitySystemComponent* AuraASC = GetAuraASC();
	if (!ensureMsgf(AuraASC, TEXT("AbilityInputTagEnded could not resolve Aura ASC for [%s]."), *InputTag.ToString()))
	{
		return;
	}

	if (AuraASC->HasMatchingGameplayTag(FAuraGameTagManager::Get().PLayer_BlockInputReleased))
	{
		return;
	}

	AuraASC->AbilityInputTagReleased(InputTag);
}

bool AAuraPlayerController::CouldLaunchGameplayAbility() const
{
	return bIsTargeting || bIsAttackHelpKeyPressed;
}
