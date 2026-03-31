// @Copyright HaolunYuan


#include "Components/AbilitySystem/Task/TargetDataUnderMouse.h"

UTargetDataUnderMouse* UTargetDataUnderMouse::GetTargetDataUnderMouse(UGameplayAbility* OwningAbility)
{
	UTargetDataUnderMouse* MyObj = NewAbilityTask<UTargetDataUnderMouse>(OwningAbility); 
	return MyObj;

}
