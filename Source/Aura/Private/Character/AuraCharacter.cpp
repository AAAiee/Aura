// @Copyright HaolunYuan


#include "Character/AuraCharacter.h"

#include "GameFramework/CharacterMovementComponent.h"
#include "Player/AuraPlayerState.h"

#include "Components/AbilitySystem/AuraAbilitySystemComponent.h"
#include "Components/AbilitySystem/AuraAttributeSet.h"

#include "Player/AuraPlayerController.h"
#include "UI/HUD/AuraHUD.h"

AAuraCharacter::AAuraCharacter()
{
	/*Movement Configuration ¡ª top-down ARPG style:
	 * bOrientRotationToMovement: character faces the direction of movement automatically.
	 * bConstrainToPlane + bSnapToPlaneAtStart: keeps movement on a flat plane (no flying). */
	UCharacterMovementComponent* AuraMovementComponent = GetCharacterMovement();
	check(AuraMovementComponent);
	AuraMovementComponent->bOrientRotationToMovement = true;
	AuraMovementComponent->RotationRate = FRotator(0.0f, 400.f, 0.f);
	AuraMovementComponent->bConstrainToPlane = true; 
	AuraMovementComponent->bSnapToPlaneAtStart = true; 

	/*Fixed Camera ¡ª disable controller rotation so the camera stays fixed (top-down view).
	 * The character rotates via bOrientRotationToMovement instead. */
	bUseControllerRotationPitch = false;
	bUseControllerRotationRoll = false;
	bUseControllerRotationYaw = false;
}

void AAuraCharacter::PossessedBy(AController* NewController)
{
	Super::PossessedBy(NewController);

	// Server-side: PlayerState is already valid at this point
	InitAbilityActorInfo();

	// Server-side: apply the GE that initializes primary attributes (Strength, Intellect, etc).
	// This will replicate to clients and trigger UI updates.
	InitDefaultAttributes();

}

void AAuraCharacter::OnRep_PlayerState()
{
	Super::OnRep_PlayerState();

	// Client-side: PlayerState just replicated, safe to read ASC/AS from it now
	InitAbilityActorInfo();
}

int32 AAuraCharacter::GetPlayerLevel() const
{
	AAuraPlayerState* AuraPlayerState = GetPlayerState<AAuraPlayerState>();
	check(AuraPlayerState);
	return AuraPlayerState->GetPlayerLevel();
}

/**
 * Initialization flow ¡ª ORDER MATTERS:
 *
 * Step 1: Tell the ASC who owns it (PlayerState) and who the physical avatar is (this pawn).
 * Step 2: Cache ASC and AttributeSet on the base class so other systems can find them.
 * Step 3: Initialize the HUD ¡ú creates OverlayWidget ¡ú WidgetController subscribes to ASC delegates.
 * Step 4: AbilityActorInfoSet() binds OnGameplayEffectAppliedDelegateToSelf.
 *         This MUST come after Step 3, otherwise the ASC broadcasts effect events
 *         before the WidgetController's lambda is registered (0 listeners).
 */
void AAuraCharacter::InitAbilityActorInfo()
{
	// Step 1: Initialize ASC with Owner=PlayerState, Avatar=this Pawn
	AAuraPlayerState* AuraPlayerState = GetPlayerState<AAuraPlayerState>();
	check(AuraPlayerState);
	AuraPlayerState->GetAbilitySystemComponent()->InitAbilityActorInfo(AuraPlayerState, this);

	// Step 2: Cache GAS references on the base class
	AbilitySystemComponent = AuraPlayerState->GetAbilitySystemComponent();
	AttributeSet = AuraPlayerState->GetAttributeSet();

	// Step 3: HUD initialization ¡ª only on the owning client (HUD is local-only)
	if (AAuraPlayerController* AuraPlayerController = GetController<AAuraPlayerController>())
	{
		if (AAuraHUD* AuraHUD = AuraPlayerController->GetHUD<AAuraHUD>())
		{
			AuraHUD->InitOverlayWidget(AuraPlayerController, AuraPlayerState, AbilitySystemComponent, AttributeSet);
		}
	}

	// Step 4: Bind ASC effect-applied delegate AFTER UI listeners are registered
	Cast<UAuraAbilitySystemComponent>(AuraPlayerState->GetAbilitySystemComponent())->AbilityActorInfoSet();
}
