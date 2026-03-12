// @Copyright HaolunYuan

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"

#define DECLARE_PRIMARY_GAME_TAG(TagName) \
	FGameplayTag Attribute_Primary_##TagName;

#define DECLARE_SECONDARY_GAME_TAG(TagName) \
	FGameplayTag Attribute_Secondary_##TagName;

#define DECLARE_VITAL_GAME_TAG(TagName) \
	FGameplayTag Attribute_Vital_##TagName;


/**
 * Singleton struct that holds all native GameplayTags used in C++ for the Aura project.
 *
 * Native tags are registered at startup via InitializeAllNativeTags() (called from AuraAssetManager).
 * This gives C++ code compile-time access to tag handles without string lookups.
 *
 * Tag naming convention: "Attributes.<Category>.<AttributeName>"
 *   - Attributes.Vital.*      ¡ª Health, Mana (current values, not max)
 *   - Attributes.Primary.*    ¡ª Strength, Intelligence, Resilience, Vigor
 *   - Attributes.Secondary.*  ¡ª Derived stats (Armor, MaxHealth, CritChance, etc.)
 */
struct AURA_API FAuraGameTag
{
public:
	/*Get the Instance*/
	static FAuraGameTag& Get();

	/*Populate all native tags*/
	static void InitializeAllNativeTags();

	/*Delete copy and move constructors and assign operators*/ 
	FAuraGameTag(const FAuraGameTag&) = delete;
	FAuraGameTag& operator=(const FAuraGameTag&) = delete;
	FAuraGameTag(FAuraGameTag&&) = delete;
	FAuraGameTag& operator=(FAuraGameTag&&) = delete;

public:
	/*Vital Attributes*/
	DECLARE_VITAL_GAME_TAG(Health)
	DECLARE_VITAL_GAME_TAG(Mana)

	/*Primary Attributes*/
	DECLARE_PRIMARY_GAME_TAG(Strength)
	DECLARE_PRIMARY_GAME_TAG(Intelligence)
	DECLARE_PRIMARY_GAME_TAG(Resilience)
	DECLARE_PRIMARY_GAME_TAG(Vigor)

	/*Second Attributes*/
	DECLARE_SECONDARY_GAME_TAG(Armor)
	DECLARE_SECONDARY_GAME_TAG(ArmorPenetration)
	DECLARE_SECONDARY_GAME_TAG(BlockChance)
	DECLARE_SECONDARY_GAME_TAG(CriticalHitChance) 
	DECLARE_SECONDARY_GAME_TAG(CriticalHitDamage) 
	DECLARE_SECONDARY_GAME_TAG(CriticalHitResistance)
	DECLARE_SECONDARY_GAME_TAG(HealthRegeneration)
	DECLARE_SECONDARY_GAME_TAG(ManaRegeneration)
	DECLARE_SECONDARY_GAME_TAG(MaxHealth)
	DECLARE_SECONDARY_GAME_TAG(MaxMana)

private:
	FAuraGameTag() = default;
};
