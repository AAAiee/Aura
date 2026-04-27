// @Copyright HaolunYuan


#include "Components/AbilitySystem/AuraAbilitySystemGlobals.h"
#include "AuraAbilityTypes.h"

FGameplayEffectContext* UAuraAbilitySystemGlobals::AllocGameplayEffectContext() const
{
	// Allocate Aura's context at the GAS factory layer so abilities, projectiles, ExecCalcs, and
	// AttributeSets can share metadata through the standard effect-spec path.
	return new FAuraGameplayEffectContext();
}
