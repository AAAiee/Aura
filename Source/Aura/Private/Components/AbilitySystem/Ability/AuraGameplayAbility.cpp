// @Copyright HaolunYuan


#include "Components/AbilitySystem/Ability/AuraGameplayAbility.h"

#include "Components/AbilitySystem/AuraAttributeSet.h"
#include "GameplayEffect.h"

FString UAuraGameplayAbility::GetDescription(int32 Level) const
{
	return FString::Printf(TEXT("<Default>%s, </><Level>%d</>\n"), TEXT("Default Ability Name - LoremIpsum LoremIpsum LoremIpsum LoremIpsum LoremIpsum LoremIpsum"), Level);
}

FString UAuraGameplayAbility::GetNextLevelDescription(int32 Level) const
{
	return FString::Printf(TEXT("<Default>%s, </><Level>%d</>\n"), TEXT("Causes Much More Damage"), Level);
}

FString UAuraGameplayAbility::GetLockedDescription(int32 Level)
{
	return FString::Printf(TEXT("<Default>Spell is Locked Until</><Level> %d </>\n"), Level);
}

float UAuraGameplayAbility::GetManaCost(float InLevel) const
{
	float CostMagnitude = 0.f;
	if (const UGameplayEffect* CostEffect = GetCostGameplayEffect())
	{
		for (const FGameplayModifierInfo& ModInfo : CostEffect->Modifiers)
		{
			if (ModInfo.Attribute == UAuraAttributeSet::GetManaAttribute())
			{
				ModInfo.ModifierMagnitude.GetStaticMagnitudeIfPossible(InLevel, CostMagnitude);
				break;
			}
		}
	}

	return CostMagnitude;
}

float UAuraGameplayAbility::GetCooldown(float InLevel) const
{
	float CooldownMagnitude = 0.f;
	if (const UGameplayEffect* CooldownEffect = GetCooldownGameplayEffect())
	{
		CooldownEffect->DurationMagnitude.GetStaticMagnitudeIfPossible(InLevel, CooldownMagnitude);
		return CooldownMagnitude;
	}
	return CooldownMagnitude;
}
