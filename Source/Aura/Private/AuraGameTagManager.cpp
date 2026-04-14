// @Copyright HaolunYuan


#include "AuraGameTagManager.h"
#include "GameplayTagsManager.h"
#include "Engine/Engine.h"

/**
 * These macros register a native GameplayTag with the engine AND return the FGameplayTag handle
 * so it can be stored in the FAuraGameTagManager singleton for fast C++ access.
 *
 * Why AddNativeGameplayTag instead of RequestGameplayTag?
 *   - Native tags are registered at startup and visible in the editor tag picker.
 *   - RequestGameplayTag only looks up existing tags ¡ª it doesn't create them.
 *
 * The comma operator is used to execute AddNativeGameplayTagInfo and return the Tag.
 */
#define ADD_VITAL_ATTRIBUTE(TagName, Description) \
	([&]() -> FGameplayTag \
	{ \
		FGameplayTag ToAdd = UGameplayTagsManager::Get().AddNativeGameplayTag( \
			FName(TEXT("Attributes.Vital." TagName)), \
			TEXT(Description)); \
		FAuraGameTagManager::AddNativeGameplayTagInfo(ToAdd, ToAdd.GetTagName(), TEXT(Description)); \
		return ToAdd; \
	}())

#define ADD_PRIMARY_ATTRIBUTE(TagName, Description) \
	([&]() -> FGameplayTag \
	{ \
		FGameplayTag ToAdd = UGameplayTagsManager::Get().AddNativeGameplayTag( \
			FName(TEXT("Attributes.Primary." TagName)), \
			TEXT(Description)); \
		FAuraGameTagManager::AddNativeGameplayTagInfo(ToAdd, ToAdd.GetTagName(), TEXT(Description)); \
		return ToAdd; \
	}())

#define ADD_SECONDARY_ATTRIBUTE(TagName, Description) \
	([&]() -> FGameplayTag \
	{ \
		FGameplayTag ToAdd = UGameplayTagsManager::Get().AddNativeGameplayTag( \
			FName(TEXT("Attributes.Secondary." TagName)), \
			TEXT(Description)); \
		FAuraGameTagManager::AddNativeGameplayTagInfo(ToAdd, ToAdd.GetTagName(), TEXT(Description)); \
		return ToAdd; \
	}())


#define ADD_GAMETAG_CUSTOM(TagName, Description) \
	([&]() -> FGameplayTag \
	{ \
		FGameplayTag ToAdd = UGameplayTagsManager::Get().AddNativeGameplayTag( \
			FName(TEXT(TagName)), \
			TEXT(Description)); \
		return ToAdd; \
	}())

FAuraGameTagManager& FAuraGameTagManager::Get()
{
	static FAuraGameTagManager Instance;
	return Instance;
}

void FAuraGameTagManager::InitializeAllNativeTags()
{
	check(GEngine);

	/*Register Vital Attributes' Tag*/
	Get().Attribute_Vital_Health = ADD_VITAL_ATTRIBUTE("Health", "Current health of the character");
	Get().Attribute_Vital_Mana = ADD_VITAL_ATTRIBUTE("Mana", "Current mana of the character, used for casting spells");

	/*Register Primary Attributes' Tag*/
	Get().Attribute_Primary_Strength = ADD_PRIMARY_ATTRIBUTE("Strength", "Increases physical damage");
	Get().Attribute_Primary_Intelligence = ADD_PRIMARY_ATTRIBUTE("Intelligence", "Increases magical damage and max mana");
	Get().Attribute_Primary_Resilience = ADD_PRIMARY_ATTRIBUTE("Resilience", "Increases Armor and Armor Penetration");
	Get().Attribute_Primary_Vigor = ADD_PRIMARY_ATTRIBUTE("Vigor", "Increases max health");

	/*Register Secondary Attributes' Tag*/
	Get().Attribute_Secondary_Armor = ADD_SECONDARY_ATTRIBUTE("Armor", "Reduces damage taken, improves Block Chance");
	Get().Attribute_Secondary_ArmorPenetration = ADD_SECONDARY_ATTRIBUTE("ArmorPenetration", "Ignores percentage of enemy Armor, increases Critical Hit Chance");
	Get().Attribute_Secondary_BlockChance = ADD_SECONDARY_ATTRIBUTE("BlockChance", "Chance to cut incoming damage in half");
	Get().Attribute_Secondary_CriticalHitChance = ADD_SECONDARY_ATTRIBUTE("CriticalHitChance", "Chance to double damage plus critical hit bonus");
	Get().Attribute_Secondary_CriticalHitDamage = ADD_SECONDARY_ATTRIBUTE("CriticalHitDamage", "Bonus damage added when a critical hit is scored");
	Get().Attribute_Secondary_CriticalHitResistance = ADD_SECONDARY_ATTRIBUTE("CriticalHitResistance", "Reduces Critical Hit Chance of attacking enemies");
	Get().Attribute_Secondary_HealthRegeneration = ADD_SECONDARY_ATTRIBUTE("HealthRegeneration", "Amount of Health regenerated every 1 second");
	Get().Attribute_Secondary_ManaRegeneration = ADD_SECONDARY_ATTRIBUTE("ManaRegeneration", "Amount of Mana regenerated every 1 second");
	Get().Attribute_Secondary_MaxHealth = ADD_SECONDARY_ATTRIBUTE("MaxHealth", "Maximum amount of Health obtainable");
	Get().Attribute_Secondary_MaxMana = ADD_SECONDARY_ATTRIBUTE("MaxMana", "Maximum amount of Mana obtainable");

	Get().Input_AuraSpell1 = ADD_GAMETAG_CUSTOM("InputTag.AuraSpell1", "Input for Aura Spell 1");
	Get().Input_AuraSpell2 = ADD_GAMETAG_CUSTOM("InputTag.AuraSpell2", "Input for Aura Spell 2");
	Get().Input_AuraSpell3 = ADD_GAMETAG_CUSTOM("InputTag.AuraSpell3", "Input for Aura Spell 3");
	Get().Input_AuraPrimaryClick = ADD_GAMETAG_CUSTOM("InputTag.AuraPrimaryClick", "Input for Primary Click");
	Get().Input_AuraSecondaryClick = ADD_GAMETAG_CUSTOM("InputTag.AuraSecondaryClick", "Input for Secondary Click");
	Get().Combat_Damage = ADD_GAMETAG_CUSTOM("Combat.Damage", "Tag for damage dealt in combat");
	Get().Combat_HitReact = ADD_GAMETAG_CUSTOM("Combat.HitReact", "Tag for hit reaction");

	Get().bIsValid = true;
}

void FAuraGameTagManager::AddNativeGameplayTagInfo(FGameplayTag NativeTag, FName TagName, FString Description)
{
	FNativeGameplayTagInfo Info(MoveTemp(NativeTag), MoveTemp(TagName), MoveTemp(Description));
	Get().NativeGameplayTagInfos.AddUnique(MoveTemp(Info));
}

