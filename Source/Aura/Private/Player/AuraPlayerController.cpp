// @Copyright HaolunYuan


#include "Player/AuraPlayerController.h"

#include "EnhancedInputSubsystems.h"
#include "EnhancedInputComponent.h"
#include "Interaction/Highlightable.h"

#include "Components/Player/AutoMoveComponent.h"
#include "UI/HUD/AuraHUD.h"


AAuraPlayerController::AAuraPlayerController()
{
	// Replicate the controller so Server RPCs (e.g., AutoMoveComponent) can execute on the server.
	bReplicates = true; 

	AutoMoveComponent = CreateDefaultSubobject<UAutoMoveComponent>(TEXT("AutoMoveComponent"));
}

void AAuraPlayerController::ToggleAttributeMenuRequested()
{
	// The HUD owns the actual widget instance, so the controller only asks it to toggle visibility.
	AAuraHUD* AuraHUD = Cast<AAuraHUD>(GetHUD());
	if (!AuraHUD)
	{
		return;
	}

	const bool bIsMenuOpen = AuraHUD->IsAttributeMenuOnScreen();
	if (bIsMenuOpen)
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

	/*Enhanced Input — register the mapping context on the local player's input subsystem.
	 * Priority 0 = lowest; higher-priority contexts can override these bindings later. */
	UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(GetLocalPlayer());
	if (Subsystem) // Subsystem is null on dedicated servers (no local player)
	{
		Subsystem->AddMappingContext(AuraContext, 0);
	}

	/*Cursor Setup — show a hand cursor for the top-down ARPG feel*/
	bShowMouseCursor = true;
	DefaultMouseCursor = EMouseCursor::Hand;

	/*Input Mode — allow both game input and UI interaction.
	 * DoNotLock: cursor can leave the viewport (useful for windowed mode).
	 * HideCursorDuringCapture=false: keep cursor visible when clicking. */
	FInputModeGameAndUI InputModeData;
	InputModeData.SetLockMouseToViewportBehavior(EMouseLockMode::DoNotLock);
	InputModeData.SetHideCursorDuringCapture(false);
	SetInputMode(InputModeData);
}

void AAuraPlayerController::PlayerTick(float DeltaTime)
{
	Super::PlayerTick(DeltaTime);

	// Cursor trace only on the local client — remote players don't need it
	if (IsLocalController())
	{
		CursorTrace();
	}
}

void AAuraPlayerController::SetupInputComponent()
{
	Super::SetupInputComponent();

	UEnhancedInputComponent* EnhancedInputComponent = CastChecked<UEnhancedInputComponent>(InputComponent);
	checkf(KeyboardMovementAction,TEXT("KeyboardMoveAction is not set in PlayerController"));
	checkf(MouseClickAction, TEXT("MouseClickAction is not set in PlayerController"));
	checkf(ToggleAttributeMenuAction, TEXT("ToggleAttributeMenuAction is not set in PlayerController"));

	/*Bind input actions — Triggered fires every frame the key is held (for smooth movement)*/
	EnhancedInputComponent->BindAction(KeyboardMovementAction, ETriggerEvent::Triggered, this, &AAuraPlayerController::Move);
	EnhancedInputComponent->BindAction(MouseClickAction, ETriggerEvent::Triggered, this, &AAuraPlayerController::OnClickMove);
	EnhancedInputComponent->BindAction(ToggleAttributeMenuAction, ETriggerEvent::Started, this, &AAuraPlayerController::OnToggleAttributeMenu); 
}

/**
 * WASD Movement — camera-relative direction.
 * We extract the controller's Yaw rotation to determine "forward" and "right"
 * relative to the camera, then apply the input vector as movement.
 * Also cancels any active auto-move so WASD takes priority.
 */
void AAuraPlayerController::Move(const FInputActionValue& ActionValues)
{
	if (AutoMoveComponent && AutoMoveComponent->IsAutoMoving())
	{
		AutoMoveComponent->RequestCancelAutoMove();
	}

	FVector2D MoveVector = ActionValues.Get<FVector2D>();

	const FRotator ControllerRotation = GetControlRotation();
	const FRotator YawRotation(0, ControllerRotation.Yaw, 0);
	const FVector PlayerForwardDirection = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::X);
	const FVector PlayerRightDirection = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::Y);

	if (APawn* ControlledPawn = GetPawn())
	{
		ControlledPawn->AddMovementInput(PlayerForwardDirection, MoveVector.Y);
		ControlledPawn->AddMovementInput(PlayerRightDirection, MoveVector.X);
	}
}

/** Click-to-Move — sends the cached cursor location to the AutoMoveComponent. */
void AAuraPlayerController::OnClickMove(const FInputActionValue& ActionValues)
{
	if (!bHasCachedMoveTargetLocation)
	{
		return;
	}

	check(AutoMoveComponent);
	AutoMoveComponent->RequestToMoveToLocation(CachedMoveTargetLocation);
}

void AAuraPlayerController::OnToggleAttributeMenu(const FInputActionValue& ActionValues)
{
	// Keep the input callback tiny — the shared toggle helper is the real source of truth.
	ToggleAttributeMenuRequested();
}


/**
 * Cursor Trace — runs every tick on the local client.
 *
 * 1. Line trace under the cursor on ECC_Visibility channel.
 * 2. Cache the hit location for click-to-move.
 * 3. Check if the hit actor implements IHighlightable.
 * 4. Compare CurrentHighlightable vs LastHighlightable and transition:
 *
 *    | Last  | Current | Action                              |
 *    |-------|---------|-------------------------------------|
 *    | null  | valid   | Highlight current                   |
 *    | valid | null    | Unhighlight last                    |
 *    | valid | valid   | If different: unhighlight last,     |
 *    |       |  ≠ last |   highlight current                 |
 *    | valid | valid   | Same actor — do nothing             |
 *    | null  | null    | Do nothing                          |
 */
void AAuraPlayerController::CursorTrace()
{
	FHitResult CursorHitResult;
	GetHitResultUnderCursor(ECollisionChannel::ECC_Visibility, false, CursorHitResult);

	LastHighlightable = CurrentHighlightable;

	if (CursorHitResult.bBlockingHit)
	{
		CachedMoveTargetLocation = CursorHitResult.ImpactPoint; 
		bHasCachedMoveTargetLocation = true;

		AActor* HitActor = CursorHitResult.GetActor();
		if (HitActor && HitActor->Implements<UHighlightable>())
		{
			CurrentHighlightable = HitActor;
		}
		else
		{
			CurrentHighlightable = nullptr;
		}
	}
	else
	{
		bHasCachedMoveTargetLocation = false;
		CurrentHighlightable = nullptr;
	}

	// State machine — transition highlights based on the table above
	if (CurrentHighlightable)
	{
		if (LastHighlightable == nullptr)
		{
			CurrentHighlightable->HighLightActor();
		}
		else
		{
			if (CurrentHighlightable != LastHighlightable)
			{
				LastHighlightable->UnhighLightActor();
				CurrentHighlightable->HighLightActor();
			}
		}
	}
	else
	{
		if (LastHighlightable)
		{
			LastHighlightable->UnhighLightActor();
		}
	}
}
