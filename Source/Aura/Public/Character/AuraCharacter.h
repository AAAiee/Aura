// @Copyright HaolunYuan

#pragma once

#include "CoreMinimal.h"
#include "Character/AuraCharacterBase.h"
#include "AuraCharacter.generated.h"


/**
 * 
 */
UCLASS()
class AURA_API AAuraCharacter : public AAuraCharacterBase
{
	GENERATED_BODY()


public:
	AAuraCharacter(); 

	//~ ACharacters Begins 
	virtual void PossessedBy(AController* NewController) override;
	//~ ACharacters Ends

	/** PlayerState Replication Notification Callback */
	virtual void OnRep_PlayerState() override;
private:

	/*Initialize the ASC and AttributeSet of the Character, called in both PossessedBy and OnRep_PlayerState to cover both Server and Client cases*/
	void InitAbilityActorInfo();

};
