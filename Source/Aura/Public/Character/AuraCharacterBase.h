// @Copyright HaolunYuan  https://github.com/AAAiee/Aura.git

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "AuraCharacterBase.generated.h"

UCLASS(Abstract)
class AURA_API AAuraCharacterBase : public ACharacter
{
	GENERATED_BODY()

public:

	/*Constructor*/
	AAuraCharacterBase();

protected:
	/*ACharacter::Begin*/
	virtual void BeginPlay() override;
	/*AChahracter::End*/


protected:

	/*Combat Related Components Begins*/
	UPROPERTY(EditAnywhere, Category = Combat)
	TObjectPtr<USkeletalMeshComponent> Weapon; //TObjectPtr(Lazy Loading and Tracking)
	/*Combat Related Components Ends*/
};
