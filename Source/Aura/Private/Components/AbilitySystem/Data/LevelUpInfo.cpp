// @Copyright HaolunYuan


#include "Components/AbilitySystem/Data/LevelUpInfo.h"

int32 ULevelUpInfo::FindLevelForXP(int32 XP) const
{
	check(LevelUpInfos.Num() > 0);

	// Level data is authored in ascending XP order; the first row above XP is the next level gate.
	const FAuraLevelUpInfo* LevelInfoForGivenXP = LevelUpInfos.FindByPredicate([XP](const FAuraLevelUpInfo& Info)
		{
			if (XP < Info.LevelUpRequirement)
			{
				return true;
			}

			return false;
		});

	if (LevelInfoForGivenXP)
	{
		return (LevelInfoForGivenXP - &LevelUpInfos[0]);
	}
	else
	{
		return LevelUpInfos.Num() - 1;
	}
}
