// @Copyright HaolunYuan

#pragma once

#include "CoreMinimal.h"
#include "Character/AuraCharacterBase.h"
#include "Interaction//Highlightable.h"
#include "AuraEnemy.generated.h"

/**
 * 
 */
UCLASS()
class AURA_API AAuraEnemy : public AAuraCharacterBase, public IHighlightable
{
	GENERATED_BODY()

public:
	AAuraEnemy();


	/*IHighlightable Interface Begin */
	virtual void HighLightActor() override;
	virtual void UnhighLightActor() override;
	/*IHighlightable Interface End */
	

protected:
	virtual void Tick(float DeltaTime) override;
	virtual void BeginPlay() override;
private:

	/*Highlight Related Begin*/
	bool bIsHighlighted = false;
	/*Highlight Related Ends*/

};
