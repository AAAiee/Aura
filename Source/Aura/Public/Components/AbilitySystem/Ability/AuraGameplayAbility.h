// @Copyright HaolunYuan

#pragma once

#include "CoreMinimal.h"
#include "Abilities/GameplayAbility.h"
#include "AuraGameplayAbility.generated.h"

/**
 * Base Aura gameplay ability carrying the startup input/ability tag authored by designers.
 */
UCLASS()
class AURA_API UAuraGameplayAbility : public UGameplayAbility
{
	GENERATED_BODY()

public:
	/** Startup input/status tag used when the ASC grants tutorial/default abilities. */
	UPROPERTY(EditDefaultsOnly, Category = StartUpProperties)
	FGameplayTag StartupGameTag;

	/** Current-level description text for ability menus. Child abilities can include damage/cost/cooldown data. */
	virtual FString GetDescription(int32 Level) const;

	/** Next-level preview text for ability menus. */
	virtual FString GetNextLevelDescription(int32 Level) const;

	/** Static locked-state text used before the player has met the ability's level requirement. */
	static FString GetLockedDescription(int32 Level);

protected:
	/** Reads the absolute mana cost from the ability's cost GameplayEffect at a requested level. */
	float GetManaCost(float InLevel = 1.f) const;

	/** Reads the cooldown duration from the ability's cooldown GameplayEffect at a requested level. */
	float GetCooldown(float InLevel = 1.f) const;
};
