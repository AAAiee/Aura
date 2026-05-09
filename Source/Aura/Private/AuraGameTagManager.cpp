// @Copyright HaolunYuan

#include "AuraGameTagManager.h"

#include "Engine/Engine.h"
#include "GameplayTagsManager.h"

/**
 * These macros register a native GameplayTag with the engine AND return the FGameplayTag handle
 * so it can be stored in the FAuraGameTagManager singleton for fast C++ access.
 *
 * Why AddNativeGameplayTag instead of RequestGameplayTag?
 *   - Native tags are registered at startup and visible in the editor tag picker.
 *   - RequestGameplayTag only looks up existing tags; it does not create them.
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

	/* Vital Attribute Tags */
	Get().Attribute_Vital_Health = ADD_VITAL_ATTRIBUTE("Health", "Current health of the character");
	Get().Attribute_Vital_Mana = ADD_VITAL_ATTRIBUTE("Mana", "Current mana of the character, used for casting spells");

	/* Primary Attribute Tags */
	Get().Attribute_Primary_Strength = ADD_PRIMARY_ATTRIBUTE("Strength", "Increases physical damage");
	Get().Attribute_Primary_Intelligence = ADD_PRIMARY_ATTRIBUTE("Intelligence", "Increases magical damage and max mana");
	Get().Attribute_Primary_Resilience = ADD_PRIMARY_ATTRIBUTE("Resilience", "Increases Armor and Armor Penetration");
	Get().Attribute_Primary_Vigor = ADD_PRIMARY_ATTRIBUTE("Vigor", "Increases max health");

	/* Secondary Attribute Tags */
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

	/*
	 * Resistance attributes are normal secondary attributes, not special-case combat data.
	 * Registering them here lets AttributeSets, UI data assets, and ExecCalcs all refer to the
	 * same native tags instead of duplicating string names.
	 */
	Get().Attribute_Secondary_FireDamageResistance = ADD_SECONDARY_ATTRIBUTE("FireResistance", "Reduces incoming fire damage");
	Get().Attribute_Secondary_LightningDamageResistance = ADD_SECONDARY_ATTRIBUTE("LightningResistance", "Reduces incoming lightning damage");
	Get().Attribute_Secondary_ArcaneDamageResistance = ADD_SECONDARY_ATTRIBUTE("ArcaneResistance", "Reduces incoming arcane damage");
	Get().Attribute_Secondary_PhysicalDamageResistance = ADD_SECONDARY_ATTRIBUTE("PhysicalResistance", "Reduces incoming physical damage");

	/* Input Tags */
	Get().InputTag_1 = ADD_GAMETAG_CUSTOM("InputTag.1", "Input for input key 1");
	Get().InputTag_2 = ADD_GAMETAG_CUSTOM("InputTag.2", "Input for input key 2");
	Get().InputTag_3 = ADD_GAMETAG_CUSTOM("InputTag.3", "Input for input key 3");
	Get().InputTag_4 = ADD_GAMETAG_CUSTOM("InputTag.4", "Input for input key 4");
	Get().InputTag_AuraPrimaryClick = ADD_GAMETAG_CUSTOM("InputTag.AuraPrimaryClick", "Input for Primary Click");
	Get().InputTag_AuraSecondaryClick = ADD_GAMETAG_CUSTOM("InputTag.AuraSecondaryClick", "Input for Secondary Click");

	/* Combat State Tags */
	Get().Combat_Damage = ADD_GAMETAG_CUSTOM("Combat.Damage", "Tag for damage dealt in combat");
	Get().Combat_HitReact = ADD_GAMETAG_CUSTOM("Combat.HitReact", "Tag for hit reaction");

	/*
	 * Damage type tags are authored on abilities as set-by-caller keys. The resistance map below
	 * lets damage execution stay data-oriented: adding a new type means registering one type tag,
	 * one resistance attribute tag, and one map entry.
	 */
	Get().DamageType_Fire = ADD_GAMETAG_CUSTOM("DamageType.Fire", "Tag for fire damage type");
	Get().DamageType_Lightning = ADD_GAMETAG_CUSTOM("DamageType.Lightning", "Tag for lightning damage type");
	Get().DamageType_Arcane = ADD_GAMETAG_CUSTOM("DamageType.Arcane", "Tag for arcane damage type");
	Get().DamageType_Physical = ADD_GAMETAG_CUSTOM("DamageType.Physical", "Tag for physical damage type");

	Get().DamageTypesToResistance.Add(Get().DamageType_Fire, Get().Attribute_Secondary_FireDamageResistance);
	Get().DamageTypesToResistance.Add(Get().DamageType_Lightning, Get().Attribute_Secondary_LightningDamageResistance);
	Get().DamageTypesToResistance.Add(Get().DamageType_Arcane, Get().Attribute_Secondary_ArcaneDamageResistance);
	Get().DamageTypesToResistance.Add(Get().DamageType_Physical, Get().Attribute_Secondary_PhysicalDamageResistance);
	Get().Attributes_Meta_XP= ADD_GAMETAG_CUSTOM("Attributes.Meta.XP", "Tag for meta XP attribute");

	/* Ability Tags */
	Get().Ability_Attack = ADD_GAMETAG_CUSTOM("Ability.Attack", "Tag for attack abilities");
	Get().Ability_Summon = ADD_GAMETAG_CUSTOM("Ability.Summon", "Tag for summon abilities");
	Get().Ability_FireBolt = ADD_GAMETAG_CUSTOM("Ability.FireBolt", "Tag for fire bolt ability");

	/*Cooldown*/
	Get().Cooldown_FireBolt = ADD_GAMETAG_CUSTOM("Cooldown.FireBolt", "Tag for fire bolt cooldown");

	/* Combat Socket Tags */
	Get().CombatSocket_Weapon = ADD_GAMETAG_CUSTOM("CombatSocket.Weapon", "Tag for identifying combat weapon socket");
	Get().CombatSocket_LeftHand = ADD_GAMETAG_CUSTOM("CombatSocket.LeftHand", "Tag for identifying left hand combat socket");
	Get().CombatSocket_RightHand = ADD_GAMETAG_CUSTOM("CombatSocket.RightHand", "Tag for identifying right hand combat socket");
	Get().CombatSocket_TailTip = ADD_GAMETAG_CUSTOM("CombatSocket.TailTip", "Tag for identifying tail tip combat socket");

	/* Attack Montage Tags */
	Get().Montage_Attack_1 = ADD_GAMETAG_CUSTOM("Montage.Attack_1", "Tag for attack montage_1");
	Get().Montage_Attack_2 = ADD_GAMETAG_CUSTOM("Montage.Attack_2", "Tag for attack montage_2");
	Get().Montage_Attack_3 = ADD_GAMETAG_CUSTOM("Montage.Attack_3", "Tag for attack montage_3");
	Get().Montage_Attack_4 = ADD_GAMETAG_CUSTOM("Montage.Attack_4", "Tag for attack montage_4");

	Get().bIsValid = true;
}

void FAuraGameTagManager::AddNativeGameplayTagInfo(FGameplayTag NativeTag, FName TagName, FString Description)
{
	FNativeGameplayTagInfo Info(MoveTemp(NativeTag), MoveTemp(TagName), MoveTemp(Description));
	Get().NativeGameplayTagInfos.AddUnique(MoveTemp(Info));
}
