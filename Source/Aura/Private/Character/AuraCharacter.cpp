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

	/*Movement configuration*/
	UCharacterMovementComponent* AuraMovementComponent = GetCharacterMovement();
	check(AuraMovementComponent);
	AuraMovementComponent->bOrientRotationToMovement = true;
	AuraMovementComponent->RotationRate = FRotator(0.0f, 400.f, 0.f);
	AuraMovementComponent->bConstrainToPlane = true; 
	AuraMovementComponent->bSnapToPlaneAtStart = true; 

	/*Fixed Camera*/
	bUseControllerRotationPitch = false;
	bUseControllerRotationRoll = false;
	bUseControllerRotationYaw = false;


}

void AAuraCharacter::PossessedBy(AController* NewController)
{
	Super::PossessedBy(NewController);

	//Server Side
	InitAbilityActorInfo();

}

void AAuraCharacter::OnRep_PlayerState()
{
	Super::OnRep_PlayerState();

	//Client Side
	InitAbilityActorInfo();
}

void AAuraCharacter::InitAbilityActorInfo()
{
	AAuraPlayerState* AuraPlayerState = GetPlayerState<AAuraPlayerState>();
	check(AuraPlayerState);
	AuraPlayerState->GetAbilitySystemComponent()->InitAbilityActorInfo(AuraPlayerState, this);
	AbilitySystemComponent = AuraPlayerState->GetAbilitySystemComponent();
	AttributeSet = AuraPlayerState->GetAttributeSet();


	// Run only on owning client, the HUD is only relevant to owning client
	if (AAuraPlayerController* AuraPlayerController = GetController<AAuraPlayerController>())
	{
		//HUD is only valid if is locally controlled 
		if (AAuraHUD* AuraHUD = AuraPlayerController->GetHUD<AAuraHUD>())
		{
			AuraHUD->InitOverlayWidget(AuraPlayerController, AuraPlayerState, AbilitySystemComponent, AttributeSet);
		}
	}
}
