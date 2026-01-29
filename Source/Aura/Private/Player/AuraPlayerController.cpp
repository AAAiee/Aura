// @Copyright HaolunYuan


#include "Player/AuraPlayerController.h"

/*Input Related Begin*/
#include "EnhancedInputSubsystems.h"
#include "EnhancedInputComponent.h"
#include "Interaction/Highlightable.h"
/*Input Related End*/


AAuraPlayerController::AAuraPlayerController()
{
	bReplicates = true; 

}

void AAuraPlayerController::BeginPlay()
{
	Super::BeginPlay();
	check(AuraContext);

	/*Enhanced System Configuration*/
	UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(GetLocalPlayer());
	check(Subsystem);
	Subsystem->AddMappingContext(AuraContext, 0);

	/*Cursor Configuration*/ 
	bShowMouseCursor = true;
	DefaultMouseCursor = EMouseCursor::Hand;

	/*Input Mode Configuration*/
	FInputModeGameAndUI InputModeData;
	InputModeData.SetLockMouseToViewportBehavior(EMouseLockMode::DoNotLock);
	InputModeData.SetHideCursorDuringCapture(false);
	SetInputMode(InputModeData);
}

void AAuraPlayerController::PlayerTick(float DeltaTime)
{

	Super::PlayerTick(DeltaTime);

	CursorTrace();
}

void AAuraPlayerController::SetupInputComponent()
{
	Super::SetupInputComponent();

	UEnhancedInputComponent* EnhancedInputComponent = CastChecked<UEnhancedInputComponent>(InputComponent);
	check(Movement);

	/*Bind Input Actions*/
	EnhancedInputComponent->BindAction(Movement, ETriggerEvent::Triggered, this, &AAuraPlayerController::Move);
}

void AAuraPlayerController::Move(const FInputActionValue& ActionValues)
{
	FVector2D MoveVector = ActionValues.Get<FVector2D>();

	/*Figure out where the user is looking at, then apply velocity*/
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

void AAuraPlayerController::CursorTrace()
{
	FHitResult CursorHitResult;
	GetHitResultUnderCursor(ECollisionChannel::ECC_Visibility, false, CursorHitResult);

	if (CursorHitResult.bBlockingHit == false) return;

	LastHighlightable = CurrentHighlightable; 
	CurrentHighlightable = CursorHitResult.GetActor();

	/**
	 * Now there is a few scenarios
	 * 1. CurHighlightable != null && LastHighlightable == null : Just started highlighting the current one
	 * 2. CurHighlightable == null && LastHighlightable != null : Just stopped highlighting the last one
	 * 3. Both are valid && CurHighlightable != LastHighlightable : Switched highlight from last to current
	 * 4. Both are valid && CurHighlightable == LastHighlightable : Do nothing
	 * 5. Both are null : Do nothing
	 */

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
			else
			{
				// Do nothing
			}
		}
	}
	else // CurrentHighlightable is null
	{
		if (LastHighlightable)
		{
			LastHighlightable->UnhighLightActor();
		}
		else
		{
			// Do nothing
		}
	}


}

