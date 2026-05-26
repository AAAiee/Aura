// @Copyright HaolunYuan

#include "Components/AbilitySystem/Data/AbilityInfo.h"

#include "AuraLogCategory.h"

FAuraAbilityInfo* UAbilityInfo::FindAbilityInfoByTag(const FGameplayTag& InAbilityTag, bool LogOnNotFound /*=false*/)
{
	check(AbilitiesInfos.Num() > 0);

	// AbilityTag is the row's stable identity. Runtime fields like InputTag and AbilityStatusTag
	// are mutated by widget controllers after the ASC reports the current spec state.
	FAuraAbilityInfo* FoundAbilityInfo = AbilitiesInfos.FindByPredicate([&InAbilityTag](const FAuraAbilityInfo& Info)
		{
			return Info.AbilityTag.MatchesTagExact(InAbilityTag);
		}
	);

	if (!FoundAbilityInfo && LogOnNotFound)
	{
		UE_LOG(LogAura, Warning, TEXT("AbilityInfo not found for tag: %s"), *InAbilityTag.ToString());
	}

	return FoundAbilityInfo;
}
