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

#define DECLARE_GAMETAG_CUSTOM(PrefixName, TagName) \
	FGameplayTag  PrefixName##_##TagName;


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
struct AURA_API FAuraGameTagManager
{
public:
	struct FNativeGameplayTagInfo
	{

		FGameplayTag NativeTag;
		FName TagName;
		FString Description;

		FNativeGameplayTagInfo() = default;

		FNativeGameplayTagInfo(FGameplayTag InNativeTag, FName InTagName, FString InDescription)
			: NativeTag(MoveTemp(InNativeTag))
			, TagName(MoveTemp(InTagName))
			, Description(MoveTemp(InDescription)) { }

		FNativeGameplayTagInfo(const FNativeGameplayTagInfo&) = default;
		FNativeGameplayTagInfo& operator=(const FNativeGameplayTagInfo&) = default;
		FNativeGameplayTagInfo(FNativeGameplayTagInfo&&) = default;
		FNativeGameplayTagInfo& operator=(FNativeGameplayTagInfo&&) = default;

		bool operator==(const FNativeGameplayTagInfo& Other) const
		{
			return NativeTag.MatchesTagExact(Other.NativeTag);
		}
	};

public:
	/*Get the Instance*/
	static FAuraGameTagManager& Get();

	/*Populate all native tags*/
	static void InitializeAllNativeTags();

	static FORCEINLINE const TArray<FNativeGameplayTagInfo>& GetNativeGameplayTagInfos()
	{
		return Get().NativeGameplayTagInfos;
	}

	static FORCEINLINE bool IsNativeTagInfosValid()
	{
		return Get().bIsValid;
	}

	/*Delete copy and move constructors and assign operators*/ 
	FAuraGameTagManager(const FAuraGameTagManager&) = delete;
	FAuraGameTagManager& operator=(const FAuraGameTagManager&) = delete;
	FAuraGameTagManager(FAuraGameTagManager&&) = delete;
	FAuraGameTagManager& operator=(FAuraGameTagManager&&) = delete;

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

	/**
	 * Input related tags
	 */
	DECLARE_GAMETAG_CUSTOM(Input, AuraSpell1)
	DECLARE_GAMETAG_CUSTOM(Input, AuraSpell2)
	DECLARE_GAMETAG_CUSTOM(Input, AuraSpell3)

	DECLARE_GAMETAG_CUSTOM(Input, AuraPrimaryClick)
	DECLARE_GAMETAG_CUSTOM(Input, AuraSecondaryClick)

	// Combat tags bridge authored GE data to runtime combat logic such as ExecCalcs and hit-react abilities.
	DECLARE_GAMETAG_CUSTOM(Combat, Damage);
	DECLARE_GAMETAG_CUSTOM(Combat, HitReact);

private:
	//Helper to register native Gameplay tag info while register native tag info
	static void AddNativeGameplayTagInfo(FGameplayTag NativeTag, FName TagName, FString Description);
	FAuraGameTagManager() = default;

	bool bIsValid = false;

	TArray<FNativeGameplayTagInfo> NativeGameplayTagInfos;
};
