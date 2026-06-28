// @Copyright HaolunYuan

#include "Components/AbilitySystem/Ability/AuraPassiveAbility.h"

#include "AbilitySystemBlueprintLibrary.h"
#include "Components/AbilitySystem/AuraAbilitySystemComponent.h"
#include "Components/AbilitySystem/AuraAbilitySystemLibrary.h"

void UAuraPassiveAbility::ActivateAbility(const FGameplayAbilitySpecHandle Handle,
                                          const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo,
                                          const FGameplayEventData* TriggerEventData)
{
	// Pipeline:
	// 1. Let UGameplayAbility establish the active passive ability state.
	// 2. Resolve the owning Aura ASC from the avatar actor.
	// 3. Bind a tag-based shutdown callback so equipment or ability-system changes can end this passive.
	Super::ActivateAbility(Handle, ActorInfo, ActivationInfo, TriggerEventData);

	if (UAuraAbilitySystemComponent* AuraASC = Cast<UAuraAbilitySystemComponent>(UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(GetAvatarActorFromActorInfo())))
	{
		AuraASC->OnDeactivatePassiveAbility.AddUObject(this, &UAuraPassiveAbility::ReceiveDeactivateAbility);
	}
}

void UAuraPassiveAbility::ReceiveDeactivateAbility(const FGameplayTag& InAbilityTag)
{
	// Pipeline:
	// 1. Compare the deactivated tag against this ability's asset tags.
	// 2. End the passive only when the broadcast targets this exact ability.
	if (GetAssetTags().HasTagExact(InAbilityTag))
	{
		EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, true);
	}
}
