// @Copyright HaolunYuan


#include "AuraGameTag.h"
#include "GameplayTagsManager.h"

/**
 * These macros register a native GameplayTag with the engine AND return the FGameplayTag handle
 * so it can be stored in the FAuraGameTag singleton for fast C++ access.
 *
 * Why AddNativeGameplayTag instead of RequestGameplayTag?
 *   - Native tags are registered at startup and visible in the editor tag picker.
 *   - RequestGameplayTag only looks up existing tags ¡ª it doesn't create them.
 */
#define ADD_VITAL_ATTRIBUTE(TagName, Description) \
	UGameplayTagsManager::Get().AddNativeGameplayTag(FName("Attributes.Vital." #TagName), TEXT(Description))

#define ADD_PRIMARY_ATTRIBUTE(TagName, Description) \
	UGameplayTagsManager::Get().AddNativeGameplayTag(FName("Attributes.Primary." #TagName), TEXT(Description))

#define ADD_SECONDARY_ATTRIBUTE(TagName, Description) \
	UGameplayTagsManager::Get().AddNativeGameplayTag(FName("Attributes.Secondary." #TagName), TEXT(Description))

FAuraGameTag& FAuraGameTag::Get()
{
	static FAuraGameTag Instance;
	return Instance;
}

void FAuraGameTag::InitializeAllNativeTags()
{
	check(GEngine);

	/*Register Vital Attributes' Tag*/
	Get().Attribute_Vital_Health = ADD_VITAL_ATTRIBUTE(Health, "Current health of the character");
	Get().Attribute_Vital_Mana = ADD_VITAL_ATTRIBUTE(Mana, "Current mana of the character, used for casting spells");

	/*Register Primary Attributes' Tag*/
	Get().Attribute_Primary_Strength = ADD_PRIMARY_ATTRIBUTE(Strength, "Increases physical damage");
	Get().Attribute_Primary_Intelligence = ADD_PRIMARY_ATTRIBUTE(Intelligence, "Increases magical damage and max mana");
	Get().Attribute_Primary_Resilience = ADD_PRIMARY_ATTRIBUTE(Resilience, "Increases Armor and Armor Penetration");
	Get().Attribute_Primary_Vigor = ADD_PRIMARY_ATTRIBUTE(Vigor, "Increases max health");

	/*Register Secondary Attributes' Tag*/
	Get().Attribute_Secondary_Armor = ADD_SECONDARY_ATTRIBUTE(Armor, "Reduces damage taken, improves Block Chance");
	Get().Attribute_Secondary_ArmorPenetration = ADD_SECONDARY_ATTRIBUTE(ArmorPenetration, "Ignores percentage of enemy Armor, increases Critical Hit Chance");
	Get().Attribute_Secondary_BlockChance = ADD_SECONDARY_ATTRIBUTE(BlockChance, "Chance to cut incoming damage in half");
	Get().Attribute_Secondary_CriticalHitChance = ADD_SECONDARY_ATTRIBUTE(CriticalHitChance, "Chance to double damage plus critical hit bonus");
	Get().Attribute_Secondary_CriticalHitDamage = ADD_SECONDARY_ATTRIBUTE(CriticalHitDamage, "Bonus damage added when a critical hit is scored");
	Get().Attribute_Secondary_CriticalHitResistance = ADD_SECONDARY_ATTRIBUTE(CriticalHitResistance, "Reduces Critical Hit Chance of attacking enemies");
	Get().Attribute_Secondary_HealthRegeneration = ADD_SECONDARY_ATTRIBUTE(HealthRegeneration, "Amount of Health regenerated every 1 second");
	Get().Attribute_Secondary_ManaRegeneration = ADD_SECONDARY_ATTRIBUTE(ManaRegeneration, "Amount of Mana regenerated every 1 second");
	Get().Attribute_Secondary_MaxHealth = ADD_SECONDARY_ATTRIBUTE(MaxHealth, "Maximum amount of Health obtainable");
	Get().Attribute_Secondary_MaxMana = ADD_SECONDARY_ATTRIBUTE(MaxMana, "Maximum amount of Mana obtainable");
}
