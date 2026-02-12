// @Copyright HaolunYuan


#include "Player/AuraPlayerController.h"

/*Input Related Begin*/
#include "EnhancedInputSubsystems.h"
#include "EnhancedInputComponent.h"
#include "Interaction/Highlightable.h"
/*Input Related End*/

/*AutoMove Component Support Begin*/
#include "Components/Player/AutoMoveComponent.h"
/*AutoMove Component Support End*/


AAuraPlayerController::AAuraPlayerController()
{
	bReplicates = true; 

	AutoMoveComponent = CreateDefaultSubobject<UAutoMoveComponent>(TEXT("AutoMoveComponent"));

}

void AAuraPlayerController::BeginPlay()
{
	Super::BeginPlay();

	if (!IsLocalController()) return;

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
	if (IsLocalController())
	{
		CursorTrace();
	}

}

void AAuraPlayerController::SetupInputComponent()
{
	Super::SetupInputComponent();

	UEnhancedInputComponent* EnhancedInputComponent = CastChecked<UEnhancedInputComponent>(InputComponent);
	check(KeyboardMovementAction);
	check(MouseClickAction);

	/*Bind Input Actions*/
	EnhancedInputComponent->BindAction(KeyboardMovementAction, ETriggerEvent::Triggered, this, &AAuraPlayerController::Move);
	EnhancedInputComponent->BindAction(MouseClickAction, ETriggerEvent::Triggered, this, &AAuraPlayerController::OnClickMove);
}

void AAuraPlayerController::Move(const FInputActionValue& ActionValues)
{
	if (AutoMoveComponent && AutoMoveComponent->IsAutoMoving())
	{
		AutoMoveComponent->RequestCancelAutoMove();
	}

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


void AAuraPlayerController::OnClickMove(const FInputActionValue& ActionValues)
{
	/*If there is no valid target, do nothing*/
	if (!bHasCachedMoveTargetLocation)
	{
		return;
	}

	/*Filtering goes here*/
	check(AutoMoveComponent);
	AutoMoveComponent->RequestToMoveToLocation(CachedMoveTargetLocation);
}

void AAuraPlayerController::CursorTrace()
{
	FHitResult CursorHitResult;
	GetHitResultUnderCursor(ECollisionChannel::ECC_Visibility, false, CursorHitResult);

	LastHighlightable = CurrentHighlightable;

	if (CursorHitResult.bBlockingHit)
	{
		/*Cache cursor's position for click-to-move function*/
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





