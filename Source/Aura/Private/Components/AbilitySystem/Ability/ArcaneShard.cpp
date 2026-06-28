// @Copyright HaolunYuan

#include "Components/AbilitySystem/Ability/ArcaneShard.h"

/* UI Description : GetDescription() GetNextLevelDescription() *****************************/
FString UArcaneShard::GetDescription(int32 Level) const
{
	// Pipeline:
	// 1. Read level-scaled combat and resource values from the ability's authored data.
	// 2. Clamp the displayed shard count to the spell's maximum supported shard count.
	// 3. Choose singular or plural rich-text copy so UI text matches the current level behavior.
	const float Damage = DamageMagnitude.GetValueAtLevel(Level);
	const float ManaCost = GetManaCost(Level) * -1.f;
	const float Cooldown = GetCooldown(Level);
	const int32 NumShards = FMath::Min(Level, MaxNumShards);

	if (NumShards <= 1)
	{
		return FString::Printf(TEXT(
			"<Title>ARCANE SHARD</>\n\n"
			"<Small>Level: %d</>\n"
			"<Small>ManaCost: %.1f</>\n"
			"<Cooldown>Cooldown: %.1f Seconds</>\n\n"
			"<Default>Summons an arcane shard that detonates, dealing up to </>"
			"<Damage>%d</>"
			"<Default> arcane damage based on distance. Enemies closer to the shard take more damage and are knocked back.</>\n\n"),
			Level,
			ManaCost,
			Cooldown,
			FMath::RoundToInt(Damage));
	}

	return FString::Printf(TEXT(
		"<Title>ARCANE SHARD</>\n\n"
		"<Small>Level: %d</>\n"
		"<Small>ManaCost: %.1f</>\n"
		"<Cooldown>Cooldown: %.1f Seconds</>\n\n"
		"<Default>Summons </>"
		"<Small>%d</>"
		"<Default> arcane shards that detonate, each dealing up to </>"
		"<Damage>%d</>"
		"<Default> arcane damage based on distance. Enemies closer to a shard take more damage and are knocked back.</>\n\n"),
		Level,
		ManaCost,
		Cooldown,
		NumShards,
		FMath::RoundToInt(Damage));
}

FString UArcaneShard::GetNextLevelDescription(int32 Level) const
{
	// Pipeline:
	// 1. Read the same level-scaled values used by the active tooltip.
	// 2. Clamp shard count so the preview never promises more shards than the spell supports.
	// 3. Format the text under the NEXT LEVEL heading for the upgrade preview UI.
	const float Damage = DamageMagnitude.GetValueAtLevel(Level);
	const float ManaCost = GetManaCost(Level) * -1.f;
	const float Cooldown = GetCooldown(Level);
	const int32 NumShards = FMath::Min(Level, MaxNumShards);

	if (NumShards <= 1)
	{
		return FString::Printf(TEXT(
			"<Title>NEXT LEVEL</>\n\n"
			"<Small>Level: %d</>\n"
			"<Small>ManaCost: %.1f</>\n"
			"<Cooldown>Cooldown: %.1f Seconds</>\n\n"
			"<Default>Summons an arcane shard that detonates, dealing up to </>"
			"<Damage>%d</>"
			"<Default> arcane damage based on distance. Enemies closer to the shard take more damage and are knocked back.</>\n\n"),
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
		"<Default>Summons </>"
		"<Small>%d</>"
		"<Default> arcane shards that detonate, each dealing up to </>"
		"<Damage>%d</>"
		"<Default> arcane damage based on distance. Enemies closer to a shard take more damage and are knocked back.</>\n\n"),
		Level,
		ManaCost,
		Cooldown,
		NumShards,
		FMath::RoundToInt(Damage));
}
