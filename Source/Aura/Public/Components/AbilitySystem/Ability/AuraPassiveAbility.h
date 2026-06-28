// @Copyright HaolunYuan

#pragma once

#include "CoreMinimal.h"
#include "Components/AbilitySystem/Ability/AuraGameplayAbility.h"
#include "AuraPassiveAbility.generated.h"

/**
 * UAuraPassiveAbility
 *
 * Base gameplay ability for passives that remain active until Aura's ability system requests shutdown.
 *
 * Passive abilities are activated through the Gameplay Ability System like other Aura abilities, but they
 * stay alive to provide ongoing effects, event listeners, or Blueprint behavior. This class centralizes
 * the deactivation-listener wiring so individual passive abilities can end when their matching ability tag
 * is unequipped, disabled, or otherwise deactivated by UAuraAbilitySystemComponent.
 *
 * Important functions:
 *   - ActivateAbility() - Registers this passive ability for deactivation events after activation.
 *   - ReceiveDeactivateAbility() - Ends the ability when the broadcast tag matches this ability's asset tags.
 */
UCLASS()
class AURA_API UAuraPassiveAbility : public UAuraGameplayAbility
{
	GENERATED_BODY()

protected:
	/* UGameplayAbility begins */

	/**
	 * @brief Activates the passive ability and subscribes it to passive deactivation events.
	 *
	 * Calls the base Gameplay Ability activation flow first, then binds to the owning Aura ability system
	 * component so this passive can end when its ability tag is later deactivated.
	 *
	 * @param Handle Gameplay Ability spec handle for this activation.
	 * @param ActorInfo Runtime actor information supplied by the Ability System.
	 * @param ActivationInfo Activation prediction and authority data for this ability.
	 * @param TriggerEventData Optional gameplay event payload that triggered activation.
	 *
	 * @note The owning Aura ability system component broadcasts deactivation by gameplay tag.
	 */
	virtual void ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData) override;

	/* UGameplayAbility ends */

private:
	/**
	 * @brief Ends this passive ability when the deactivation tag matches one of its asset tags.
	 *
	 * This callback is bound during ActivateAbility() and is invoked by UAuraAbilitySystemComponent when a
	 * passive ability should shut down.
	 *
	 * @param InAbilityTag Ability tag that was deactivated by the Aura ability system component.
	 *
	 * @note The method only ends this ability for an exact tag match.
	 */
	void ReceiveDeactivateAbility(const FGameplayTag& InAbilityTag);
};
