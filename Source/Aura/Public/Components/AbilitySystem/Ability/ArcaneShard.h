// @Copyright HaolunYuan

#pragma once

#include "CoreMinimal.h"
#include "Components/AbilitySystem/Ability/AuraDamageGameplayAbility.h"
#include "ArcaneShard.generated.h"

/**
 * UArcaneShard
 *
 * Damage gameplay ability that describes Aura's arcane shard spell for UI and progression systems.
 *
 * The ability inherits its damage payload authoring from UAuraDamageGameplayAbility and specializes the
 * player-facing rich-text descriptions used by spell menu widgets. It exists so the spell can expose
 * level-scaled damage, mana cost, cooldown, and shard-count information without duplicating this text
 * construction in UI code or Blueprints.
 *
 * Important functions:
 *   - GetDescription() - Builds the current-level tooltip shown for the learned spell.
 *   - GetNextLevelDescription() - Builds the preview tooltip for the next spell level.
 */
UCLASS()
class AURA_API UArcaneShard : public UAuraDamageGameplayAbility
{
	GENERATED_BODY()

public:
	/* UAuraGameplayAbility begins */
	/* UI Description begins */

	/**
	 * @brief Builds the current-level rich-text tooltip shown by the spell menu.
	 *
	 * Reads the level-scaled damage, mana cost, cooldown, and shard count from the ability's authored
	 * data, then formats them for Aura's rich-text spell UI.
	 *
	 * @param Level Ability level to describe.
	 *
	 * @return Rich-text description for the current spell level.
	 */
	virtual FString GetDescription(int32 Level) const override;

	/**
	 * @brief Builds the preview tooltip for what the next spell level will change.
	 *
	 * Uses the same level-scaled data as GetDescription(), but labels the text as a next-level preview
	 * so the spell menu can show progression before the player commits an upgrade.
	 *
	 * @param Level Ability level to preview.
	 *
	 * @return Rich-text description for the next spell level preview.
	 */
	virtual FString GetNextLevelDescription(int32 Level) const override;

	/* UI Description ends */
	/* UAuraGameplayAbility ends */

	// Spell tuning variables
	/** Maximum number of arcane shards that can be described and spawned at high ability levels. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	int32 MaxNumShards = 11;
};
