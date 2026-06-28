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
	FGameplayTag PrefixName##_##TagName;


/**
 * Singleton struct that holds all native GameplayTags used in C++ for the Aura project.
 *
 * Native tags are registered at startup via InitializeAllNativeTags() (called from AuraAssetManager).
 * This gives C++ code compile-time access to tag handles without string lookups.
 *
 * Tag naming convention: "Attributes.<Category>.<AttributeName>"
 *   - Attributes.Vital.*      - Health, Mana (current values, not max)
 *   - Attributes.Primary.*    - Strength, Intelligence, Resilience, Vigor
 *   - Attributes.Secondary.*  - Derived stats (Armor, MaxHealth, CritChance, etc.)
 */
struct AURA_API FAuraGameTagManager
{
public:
	struct FNativeGameplayTagInfo
	{
		/** Native tag handle registered with the engine. */
		FGameplayTag NativeTag;

		/** Fully qualified tag name used in data assets and tag pickers. */
		FName TagName;

		/** Designer-facing description surfaced in tooling. */
		FString Description;

		FNativeGameplayTagInfo() = default;

		FNativeGameplayTagInfo(FGameplayTag InNativeTag, FName InTagName, FString InDescription)
			: NativeTag(MoveTemp(InNativeTag))
			, TagName(MoveTemp(InTagName))
			, Description(MoveTemp(InDescription))
		{
		}

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
	/* Singleton Access */
	static FAuraGameTagManager& Get();

	/** Registers every Aura native tag during startup. */
	static void InitializeAllNativeTags();

	static FORCEINLINE const TArray<FNativeGameplayTagInfo>& GetNativeGameplayTagInfos()
	{
		return Get().NativeGameplayTagInfos;
	}

	static FORCEINLINE bool IsNativeTagInfosValid()
	{
		return Get().bIsValid;
	}

	/* Non-Copyable Singleton */
	FAuraGameTagManager(const FAuraGameTagManager&) = delete;
	FAuraGameTagManager& operator=(const FAuraGameTagManager&) = delete;
	FAuraGameTagManager(FAuraGameTagManager&&) = delete;
	FAuraGameTagManager& operator=(FAuraGameTagManager&&) = delete;

public:
	/* Attribute Tags */
	// Vital attributes.
	DECLARE_VITAL_GAME_TAG(Health)
	DECLARE_VITAL_GAME_TAG(Mana)

	// Primary attributes.
	DECLARE_PRIMARY_GAME_TAG(Strength)
	DECLARE_PRIMARY_GAME_TAG(Intelligence)
	DECLARE_PRIMARY_GAME_TAG(Resilience)
	DECLARE_PRIMARY_GAME_TAG(Vigor)

	// Secondary attributes.
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

	// Resistance attributes used by damage execution.
	DECLARE_SECONDARY_GAME_TAG(FireDamageResistance)
	DECLARE_SECONDARY_GAME_TAG(LightningDamageResistance)
	DECLARE_SECONDARY_GAME_TAG(ArcaneDamageResistance)
	DECLARE_SECONDARY_GAME_TAG(PhysicalDamageResistance)
	DECLARE_GAMETAG_CUSTOM(Attributes, Meta_XP)

	/* Input Tags */
	DECLARE_GAMETAG_CUSTOM(InputTag, 1)
	DECLARE_GAMETAG_CUSTOM(InputTag, 2)
	DECLARE_GAMETAG_CUSTOM(InputTag, 3)
	DECLARE_GAMETAG_CUSTOM(InputTag, 4)
	DECLARE_GAMETAG_CUSTOM(InputTag, AuraPrimaryClick)
	DECLARE_GAMETAG_CUSTOM(InputTag, AuraSecondaryClick)
	DECLARE_GAMETAG_CUSTOM(InputTag, Passive1)
	DECLARE_GAMETAG_CUSTOM(InputTag, Passive2)

	/* Combat State Tags */
	// Combat tags bridge authored GE data to runtime combat logic such as ExecCalcs and hit-react abilities.
	DECLARE_GAMETAG_CUSTOM(Combat, Damage)
	DECLARE_GAMETAG_CUSTOM(Combat, HitReact)

	/* Damage Type Tags */
	/*
	 * Damage type tags are the public contract between abilities and ExecCalc_Damage.
	 * Abilities write set-by-caller magnitudes keyed by DamageType.*, and the ExecCalc
	 * maps each type to the target resistance attribute that should mitigate it.
	 */
	DECLARE_GAMETAG_CUSTOM(DamageType, Fire)
	DECLARE_GAMETAG_CUSTOM(DamageType, Lightning)
	DECLARE_GAMETAG_CUSTOM(DamageType, Arcane)
	DECLARE_GAMETAG_CUSTOM(DamageType, Physical)

	/* Ability Tags */
	DECLARE_GAMETAG_CUSTOM(Ability, Attack)
	DECLARE_GAMETAG_CUSTOM(Ability, None)
	DECLARE_GAMETAG_CUSTOM(Ability, Summon)
	DECLARE_GAMETAG_CUSTOM(Ability, Fire_FireBolt)
	DECLARE_GAMETAG_CUSTOM(Ability, Lightning_Electrocute)
	DECLARE_GAMETAG_CUSTOM(Ability, Arcane_Shards)
	DECLARE_GAMETAG_CUSTOM(Ability, HitReact)
	
	/* Passive Abilities*/
	DECLARE_GAMETAG_CUSTOM(Ability, Passive_HaloOfProtection);
	DECLARE_GAMETAG_CUSTOM(Ability, Passive_LifeSiphon);
	DECLARE_GAMETAG_CUSTOM(Ability, Passive_ManaSiphon);
	

	/* Ability Status Tags */
	DECLARE_GAMETAG_CUSTOM(Ability, Status_Locked)
	DECLARE_GAMETAG_CUSTOM(Ability, Status_Eligible)
	DECLARE_GAMETAG_CUSTOM(Ability, Status_UnLocked)
	DECLARE_GAMETAG_CUSTOM(Ability, Status_Equipped)

	/* Ability Type Tags */
	DECLARE_GAMETAG_CUSTOM(Ability, Type_Offensive)
	DECLARE_GAMETAG_CUSTOM(Ability, Type_Passive)
	DECLARE_GAMETAG_CUSTOM(Ability, Type_None)

	/* Debuff Type Tags */
	/*
	 * Debuff tags are owned tags placed on dynamic timed GameplayEffects. They let abilities/UI
	 * reason about the status effect ("Burn", "Stun", etc.) without knowing which damage type
	 * produced it.
	 */
	DECLARE_GAMETAG_CUSTOM(Debuff, Burn)
	DECLARE_GAMETAG_CUSTOM(Debuff, Stun)
	DECLARE_GAMETAG_CUSTOM(Debuff, Physical)
	DECLARE_GAMETAG_CUSTOM(Debuff, Arcane)

	/* Debuff Set-By-Caller Tags */
	// These tags are keys into the outgoing damage spec. ExecCalc_Damage reads the values and copies
	// successful results onto FAuraGameplayEffectContext for the AttributeSet to consume.
	DECLARE_GAMETAG_CUSTOM(Debuff, Chance)
	DECLARE_GAMETAG_CUSTOM(Debuff, Frequency)
	DECLARE_GAMETAG_CUSTOM(Debuff, Damage)
	DECLARE_GAMETAG_CUSTOM(Debuff, Duration)

	/* Cooldown Tags */
	DECLARE_GAMETAG_CUSTOM(Cooldown,Fire_FireBolt)
	DECLARE_GAMETAG_CUSTOM(Cooldown,Lightning_Electrocute)
	DECLARE_GAMETAG_CUSTOM(Cooldown,Arcane_Shards)
	

	/* Combat Socket Tags */
	DECLARE_GAMETAG_CUSTOM(CombatSocket, Weapon)
	DECLARE_GAMETAG_CUSTOM(CombatSocket, LeftHand)
	DECLARE_GAMETAG_CUSTOM(CombatSocket, RightHand)
	DECLARE_GAMETAG_CUSTOM(CombatSocket, TailTip)

	/* Attack Montage Tags */
	DECLARE_GAMETAG_CUSTOM(Montage, Attack_1)
	DECLARE_GAMETAG_CUSTOM(Montage, Attack_2)
	DECLARE_GAMETAG_CUSTOM(Montage, Attack_3)
	DECLARE_GAMETAG_CUSTOM(Montage, Attack_4)
	
	DECLARE_GAMETAG_CUSTOM(PLayer,BlockInputPressed);
	DECLARE_GAMETAG_CUSTOM(Player,BlockInputHeld);
	DECLARE_GAMETAG_CUSTOM(PLayer,BlockInputReleased);
	DECLARE_GAMETAG_CUSTOM(PLayer,BlockCursorTrace);

	// Central lookup that keeps typed damage extensible without hard-coding a switch per ability.
	TMap<FGameplayTag, FGameplayTag> DamageTypesToResistance;

	// Maps the damage type that won a debuff roll to the gameplay tag applied by the timed effect.
	TMap<FGameplayTag, FGameplayTag> DamageTypeToDebuffType;

	TArray<FGameplayTag> PrimaryAttributeTags;

private:
	// Caches metadata for each native tag alongside the engine registration step.
	static void AddNativeGameplayTagInfo(FGameplayTag NativeTag, FName TagName, FString Description);
	FAuraGameTagManager() = default;

	bool bIsValid = false;
	TArray<FNativeGameplayTagInfo> NativeGameplayTagInfos;
};
