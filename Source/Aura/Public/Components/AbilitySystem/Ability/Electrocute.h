// @Copyright HaolunYuan

#pragma once

#include "CoreMinimal.h"
#include "Components/AbilitySystem/Ability/AuraBeamSpell.h"
#include "Electrocute.generated.h"

/**
 * UElectrocute
 *
 * Beam spell ability that describes Aura's lightning chain attack for UI and progression systems.
 *
 * The ability inherits targeting and chain-target support from UAuraBeamSpell, then specializes the
 * player-facing tooltip text for the spell menu. It reports level-scaled damage, resource cost, cooldown,
 * and additional target count so UI can explain how the beam grows as the ability levels up.
 *
 * Important functions:
 *   - GetDescription() - Builds the current-level tooltip for the learned lightning beam.
 *   - GetNextLevelDescription() - Builds the preview tooltip for the next lightning beam level.
 */
UCLASS()
class AURA_API UElectrocute : public UAuraBeamSpell
{
	GENERATED_BODY()

public:
	/* UAuraGameplayAbility begins */
	/* UI Description begins */

	/**
	 * @brief Builds the current-level rich-text tooltip shown by the spell menu.
	 *
	 * Reads level-scaled damage, mana cost, cooldown, and chain target count, then formats the spell text
	 * to describe either a single-target beam or a chaining beam.
	 *
	 * @param Level Ability level to describe.
	 *
	 * @return Rich-text description for the current spell level.
	 */
	virtual FString GetDescription(int32 Level) const override;

	/**
	 * @brief Builds the preview tooltip for what the next spell level will change.
	 *
	 * Uses the requested preview level to show how the beam's damage, cost, cooldown, and chain count will
	 * appear after an upgrade.
	 *
	 * @param Level Ability level to preview.
	 *
	 * @return Rich-text description for the next spell level preview.
	 */
	virtual FString GetNextLevelDescription(int32 Level) const override;

	/* UI Description ends */
	/* UAuraGameplayAbility ends */
};
