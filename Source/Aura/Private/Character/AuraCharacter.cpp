// @Copyright HaolunYuan


#include "Character/AuraCharacter.h"

#include "GameFramework/CharacterMovementComponent.h"

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
