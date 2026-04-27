// @Copyright HaolunYuan

#pragma once

#include "CoreMinimal.h"
#include "AbilitySystemGlobals.h"
#include "AuraAbilitySystemGlobals.generated.h"

/**
 * Project-wide GAS hook point.
 *
 * Config/DefaultGame.ini points AbilitySystemGlobalsClassName at this class so every new
 * GameplayEffect context is Aura's custom type. That keeps combat-result metadata available
 * through the normal GAS spec flow instead of bolting separate replicated state onto actors.
 */
UCLASS()
class AURA_API UAuraAbilitySystemGlobals : public UAbilitySystemGlobals
{
	GENERATED_BODY()

protected:
	// GAS calls this whenever it needs a fresh effect context for a new outgoing spec.
	virtual FGameplayEffectContext* AllocGameplayEffectContext() const override;
};
