// @Copyright HaolunYuan


#include "Components/AbilitySystem/Data/AbilityInfo.h"
#include "AuraLogCategory.h"



FAuraAbilityInfo* UAbilityInfo::FindAbilityInfoByTag(const FGameplayTag& InAbilityTag, bool LogOnNotFound /*=false*/)
{
	check(AbilitiesInfo.Num() > 0);

	 FAuraAbilityInfo* FoundAbilityInfo = AbilitiesInfo.FindByPredicate([&InAbilityTag](const FAuraAbilityInfo& info)
		{
			if (info.AbilityTag.MatchesTagExact(InAbilityTag))
			{
				return true;
			} 
			return false;
		}
	);

	if (!FoundAbilityInfo && LogOnNotFound)
	{
		UE_LOG(LogAura, Warning, TEXT("AbilityInfo not found for tag: %s"), *InAbilityTag.ToString());
	}

	return FoundAbilityInfo;
}
