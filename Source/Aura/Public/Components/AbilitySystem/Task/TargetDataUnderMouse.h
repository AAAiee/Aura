// @Copyright HaolunYuan

#pragma once

#include "CoreMinimal.h"
#include "Abilities/Tasks/AbilityTask.h"
#include "TargetDataUnderMouse.generated.h"

/**
 * 
 */
UCLASS()
class AURA_API UTargetDataUnderMouse : public UAbilityTask
{
	GENERATED_BODY()

public:

	UFUNCTION(BlueprintCallable, Category = "Ability|Tasks", meta = (DisplayName = "Get Target Data Under Mouse", HidePin = "OwningAbility", DefaultToSelf = "OwningAbility"))
	static UTargetDataUnderMouse* GetTargetDataUnderMouse(UGameplayAbility* OwningAbility);

protected:


private:

	
};
