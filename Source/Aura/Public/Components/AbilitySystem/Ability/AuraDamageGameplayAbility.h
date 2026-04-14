// @Copyright HaolunYuan

#pragma once

#include "CoreMinimal.h"
#include "Components/AbilitySystem/Ability/AuraGameplayAbility.h"
#include "AuraDamageGameplayAbility.generated.h"

/**
 * Shared base class for abilities that expose a scalable damage value to projectile and
 * execution code.
 */
UCLASS()
class AURA_API UAuraDamageGameplayAbility : public UAuraGameplayAbility
{
	GENERATED_BODY()


public:
	// Designers author the base damage as a scalable float so one ability asset can scale cleanly by level.
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = Damage)
	FScalableFloat Damage;
};
