// @Copyright HaolunYuan

#pragma once

#include "CoreMinimal.h"
#include "Components/AbilitySystem/Ability/AuraDamageGameplayAbility.h"
#include "AuraMeleeAttack.generated.h"

/**
 * Marker damage ability used for melee attack blueprints that share the generic damage payload flow.
 */
UCLASS()
class AURA_API UAuraMeleeAttack : public UAuraDamageGameplayAbility
{
	GENERATED_BODY()
};
