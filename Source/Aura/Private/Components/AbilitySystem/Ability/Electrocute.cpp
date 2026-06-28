// @Copyright HaolunYuan

#include "Components/AbilitySystem/Ability/Electrocute.h"

/* UI Description : GetDescription() GetNextLevelDescription() *****************************/
FString UElectrocute::GetDescription(int32 Level) const
{
	// Pipeline:
	// 1. Read level-scaled damage, resource, and cooldown values for the tooltip.
	// 2. Convert ability level into the number of additional chain targets supported by the beam.
	// 3. Choose single-target or chaining copy so the spell menu describes the current behavior.
	const float Damage = DamageMagnitude.GetValueAtLevel(Level);
	const float ManaCost = GetManaCost(Level) * -1.f;
	const float Cooldown = GetCooldown(Level);
	const int32 NumAdditionalTargets = FMath::Min(MaxNumShockTarget, FMath::Max(Level - 1, 0));

	if (NumAdditionalTargets == 0)
	{
		return FString::Printf(TEXT(
			"<Title>ELECTROCUTE</>\n\n"
			"<Small>Level: %d</>\n"
			"<Small>ManaCost: %.1f</>\n"
			"<Cooldown>Cooldown: %.1f Seconds</>\n\n"
			"<Default>Launches a beam of lightning at the target enemy, dealing </><Damage>%d</><Default> lightning damage.</>\n\n"),
			Level,
			ManaCost,
			Cooldown,
			FMath::RoundToInt(Damage));
	}

	return FString::Printf(TEXT(
		"<Title>ELECTROCUTE</>\n\n"
		"<Small>Level: %d</>\n"
		"<Small>ManaCost: %.1f</>\n"
		"<Cooldown>Cooldown: %.1f Seconds</>\n\n"
		"<Default>Launches a beam of lightning at the target enemy, then chains to up to </>"
		"<Small>%d</>"
		"<Default> nearby enemies, dealing </><Damage>%d</><Default> lightning damage to each target.</>\n\n"),
		Level,
		ManaCost,
		Cooldown,
		NumAdditionalTargets,
		FMath::RoundToInt(Damage));
}

FString UElectrocute::GetNextLevelDescription(int32 Level) const
{
	// Pipeline:
	// 1. Read the preview level's damage, resource, and cooldown values.
	// 2. Convert the preview level into the chain target count shown to the player.
	// 3. Format the text under the NEXT LEVEL heading for upgrade comparison.
	const float Damage = DamageMagnitude.GetValueAtLevel(Level);
	const float ManaCost = GetManaCost(Level) * -1.f;
	const float Cooldown = GetCooldown(Level);
	const int32 NumAdditionalTargets = FMath::Min(MaxNumShockTarget, FMath::Max(Level - 1, 0));

	if (NumAdditionalTargets == 0)
	{
		return FString::Printf(TEXT(
			"<Title>NEXT LEVEL</>\n\n"
			"<Small>Level: %d</>\n"
			"<Small>ManaCost: %.1f</>\n"
			"<Cooldown>Cooldown: %.1f Seconds</>\n\n"
			"<Default>Launches a beam of lightning at the target enemy, dealing </><Damage>%d</><Default> lightning damage.</>\n\n"),
			Level,
			ManaCost,
			Cooldown,
			FMath::RoundToInt(Damage));
	}

	return FString::Printf(TEXT(
		"<Title>NEXT LEVEL</>\n\n"
		"<Small>Level: %d</>\n"
		"<Small>ManaCost: %.1f</>\n"
		"<Cooldown>Cooldown: %.1f Seconds</>\n\n"
		"<Default>Launches a beam of lightning at the target enemy, then chains to up to </>"
		"<Small>%d</>"
		"<Default> nearby enemies, dealing </><Damage>%d</><Default> lightning damage to each target.</>\n\n"),
		Level,
		ManaCost,
		Cooldown,
		NumAdditionalTargets,
		FMath::RoundToInt(Damage));
}
