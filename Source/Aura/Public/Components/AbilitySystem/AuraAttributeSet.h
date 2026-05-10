// @Copyright HaolunYuan

#pragma once

#include "CoreMinimal.h"
#include "AttributeSet.h"
#include "AbilitySystemComponent.h"
#include "AuraAttributeSet.generated.h"

class ACharacter;
class AController;

/**
 * ATTRIBUTE_ACCESSORS is a convenience macro provided by GAS.
 * For each attribute (e.g. Health), it generates four helper functions:
 *   - GetHealthAttribute()  - returns the FGameplayAttribute handle (used to bind delegates)
 *   - GetHealth()           - returns the current float value
 *   - SetHealth(float)      - sets the value directly (server-only, skips clamping)
 *   - InitHealth(float)     - sets the base AND current value (used in constructors)
 */
#define ATTRIBUTE_ACCESSORS(ClassName, PropertyName) \
	GAMEPLAYATTRIBUTE_PROPERTY_GETTER(ClassName, PropertyName) \
	GAMEPLAYATTRIBUTE_VALUE_GETTER(PropertyName) \
	GAMEPLAYATTRIBUTE_VALUE_SETTER(PropertyName) \
	GAMEPLAYATTRIBUTE_VALUE_INITTER(PropertyName)

/**
 * Holds Source and Target information extracted from a Gameplay Effect execution.
 * Populated in PostGameplayEffectExecute so we know WHO applied the effect and WHO received it.
 * Useful for damage calculations, kill credits, UI feedback, etc.
 */
USTRUCT()
struct FEffectProperties
{
	GENERATED_BODY()

	FEffectProperties() = default;

	/* Target - the actor receiving the effect. */
	UPROPERTY()
	TObjectPtr<UAbilitySystemComponent> TargetASC;

	/* Source - the actor that caused/applied the effect. */
	UPROPERTY()
	TObjectPtr<UAbilitySystemComponent> SourceASC;

	UPROPERTY()
	TObjectPtr<AActor> SourceAvatarActor;

	UPROPERTY()
	TObjectPtr<AActor> TargetAvatarActor;

	UPROPERTY()
	TObjectPtr<AController> SourceController;

	UPROPERTY()
	TObjectPtr<AController> TargetController;

	UPROPERTY()
	TObjectPtr<ACharacter> SourceCharacter;

	UPROPERTY()
	TObjectPtr<ACharacter> TargetCharacter;

	UPROPERTY()
	FGameplayEffectContextHandle GameplayEffectContextHandle;
};


/**
 * Defines the gameplay attributes (Health, Mana, etc.) for Aura characters.
 *
 * Key GAS lifecycle callbacks used here:
 *   - PreAttributeChange - clamp values BEFORE they are applied (preview only, can be overridden)
 *   - PostGameplayEffectExecute - runs AFTER an effect modifies an attribute (final, authoritative)
 *   - OnRep_<Attribute> - client-side replication callback, keeps client in sync with server
 *
 * Replication: each attribute uses REPNOTIFY_Always so the client is notified even if the
 * new value equals the old value (important for UI refresh on respawn/reset).
 */
UCLASS()
class AURA_API UAuraAttributeSet : public UAttributeSet
{
	GENERATED_BODY()

public:
	UAuraAttributeSet();

	/* Attribute Accessors */
	// Generated helpers for vital attributes.
	ATTRIBUTE_ACCESSORS(UAuraAttributeSet, Health);
	ATTRIBUTE_ACCESSORS(UAuraAttributeSet, Mana);

	// Generated helpers for primary attributes.
	ATTRIBUTE_ACCESSORS(UAuraAttributeSet, Strength);
	ATTRIBUTE_ACCESSORS(UAuraAttributeSet, Intelligence);
	ATTRIBUTE_ACCESSORS(UAuraAttributeSet, Resilience);
	ATTRIBUTE_ACCESSORS(UAuraAttributeSet, Vigor);

	// Generated helpers for secondary attributes.
	ATTRIBUTE_ACCESSORS(UAuraAttributeSet, Armor);
	ATTRIBUTE_ACCESSORS(UAuraAttributeSet, ArmorPenetration);
	ATTRIBUTE_ACCESSORS(UAuraAttributeSet, BlockChance);
	ATTRIBUTE_ACCESSORS(UAuraAttributeSet, CriticalHitChance);
	ATTRIBUTE_ACCESSORS(UAuraAttributeSet, CriticalHitDamage);
	ATTRIBUTE_ACCESSORS(UAuraAttributeSet, CriticalHitResilience);
	ATTRIBUTE_ACCESSORS(UAuraAttributeSet, HealthRegeneration);
	ATTRIBUTE_ACCESSORS(UAuraAttributeSet, ManaRegeneration);
	ATTRIBUTE_ACCESSORS(UAuraAttributeSet, MaxHealth);
	ATTRIBUTE_ACCESSORS(UAuraAttributeSet, MaxMana);

	// Generated helpers for resistance attributes.
	ATTRIBUTE_ACCESSORS(UAuraAttributeSet, FireResistance);
	ATTRIBUTE_ACCESSORS(UAuraAttributeSet, LightningResistance);
	ATTRIBUTE_ACCESSORS(UAuraAttributeSet, ArcaneResistance);
	ATTRIBUTE_ACCESSORS(UAuraAttributeSet, PhysicalResistance);

	// Generated helpers for transient meta attributes.
	ATTRIBUTE_ACCESSORS(UAuraAttributeSet, IncomingDamage);
	ATTRIBUTE_ACCESSORS(UAuraAttributeSet, InComingXPReward);

	/* UAttributeSet Overrides */
	/**
	 * Called before an attribute value is changed. Use this to clamp incoming values.
	 * NOTE: This only clamps the "proposed" value - if a GE overrides the attribute,
	 * you must re-clamp in PostGameplayEffectExecute as well.
	 */
	virtual void PreAttributeChange(const FGameplayAttribute& Attribute, float& NewValue) override;

	/** Registers all attributes for replication. Required for any replicated UPROPERTY in an AttributeSet. */
	virtual void GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const override;

	/**
	 * Called after a GameplayEffect has been executed and the attribute value has been committed.
	 * This is the authoritative place to react to attribute changes (e.g., death check, final clamping).
	 */
	virtual void PostGameplayEffectExecute(const struct FGameplayEffectModCallbackData& Data) override;

	virtual void PostAttributeChange(const FGameplayAttribute& Attribute, float OldValue, float NewValue);

private:
	/* Replication Callbacks */
	// Vital attributes.
	UFUNCTION()
	void OnRep_Health(const FGameplayAttributeData& OldHealth) const;

	UFUNCTION()
	void OnRep_Mana(const FGameplayAttributeData& OldMana) const;

	// Primary attributes.
	UFUNCTION()
	void OnRep_Strength(const FGameplayAttributeData& OldStrength) const;

	UFUNCTION()
	void OnRep_Intelligence(const FGameplayAttributeData& OldIntelligence) const;

	UFUNCTION()
	void OnRep_Resilience(const FGameplayAttributeData& OldResilience) const;

	UFUNCTION()
	void OnRep_Vigor(const FGameplayAttributeData& OldVigor) const;

	// Secondary attributes.
	UFUNCTION()
	void OnRep_MaxHealth(const FGameplayAttributeData& OldMaxHealth) const;

	UFUNCTION()
	void OnRep_MaxMana(const FGameplayAttributeData& OldMaxMana) const;

	UFUNCTION()
	void OnRep_Armor(const FGameplayAttributeData& OldArmor) const;

	UFUNCTION()
	void OnRep_ArmorPenetration(const FGameplayAttributeData& OldArmorPenetration) const;

	UFUNCTION()
	void OnRep_BlockChance(const FGameplayAttributeData& OldBlockChance) const;

	UFUNCTION()
	void OnRep_CriticalHitChance(const FGameplayAttributeData& OldCriticalHitChance) const;

	UFUNCTION()
	void OnRep_CriticalHitDamage(const FGameplayAttributeData& OldCriticalHitDamage) const;

	UFUNCTION()
	void OnRep_CriticalHitResilience(const FGameplayAttributeData& OldCriticalHitResilience) const;

	UFUNCTION()
	void OnRep_HealthRegeneration(const FGameplayAttributeData& OldHealthRegeneration) const;

	UFUNCTION()
	void OnRep_ManaRegeneration(const FGameplayAttributeData& OldManaRegeneration) const;

	// Resistance attributes.
	UFUNCTION()
	void OnRep_FireResistance(const FGameplayAttributeData& OldFireResistance) const;

	UFUNCTION()
	void OnRep_LightningResistance(const FGameplayAttributeData& OldLightningResistance) const;

	UFUNCTION()
	void OnRep_ArcaneResistance(const FGameplayAttributeData& OldArcaneResistance) const;

	UFUNCTION()
	void OnRep_PhysicalResistance(const FGameplayAttributeData& OldPhysicalResistance) const;

	// Damage text is local UI feedback, but the result flags come from server-authored effect context.
	void ShowFloatingText(const FEffectProperties& Props, float Damage, bool bBlockedHit = false, bool bCriticalHit = false);

	/* Internal Helpers */
	/** Extracts Source/Target actor info from a GE execution into an FEffectProperties struct. */
	void SetEffectProperties(const FGameplayEffectModCallbackData& Data, FEffectProperties& Props);

	void SendXPEvent(const FEffectProperties& Props);

public:
	/* Vital Attributes */
	// Replicated with REPNOTIFY_Always so the client UI always refreshes.
	UPROPERTY(BlueprintReadOnly, ReplicatedUsing = OnRep_Health, Category = "Vital Attributes")
	FGameplayAttributeData Health;

	UPROPERTY(BlueprintReadOnly, ReplicatedUsing = OnRep_Mana, Category = "Vital Attributes")
	FGameplayAttributeData Mana;

	bool bTopOffHealth = false;
	bool bTopoffMana = false;


	/* Meta Attributes */
	// Meta attribute written by the damage ExecCalc and consumed immediately in PostGameplayEffectExecute.
	UPROPERTY(BlueprintReadOnly, Category = "Meta Attributes")
	FGameplayAttributeData IncomingDamage;

	UPROPERTY(BlueprintReadOnly, Category = "Meta Attributes")
	FGameplayAttributeData InComingXPReward;


	/* Primary Attributes */
	// Replicated with REPNOTIFY_Always so client-side delegates always receive refreshes.
	UPROPERTY(BlueprintReadOnly, ReplicatedUsing = OnRep_Strength, Category = "Primary Attributes")
	FGameplayAttributeData Strength;

	UPROPERTY(BlueprintReadOnly, ReplicatedUsing = OnRep_Intelligence, Category = "Primary Attributes")
	FGameplayAttributeData Intelligence;

	UPROPERTY(BlueprintReadOnly, ReplicatedUsing = OnRep_Resilience, Category = "Primary Attributes")
	FGameplayAttributeData Resilience;

	UPROPERTY(BlueprintReadOnly, ReplicatedUsing = OnRep_Vigor, Category = "Primary Attributes")
	FGameplayAttributeData Vigor;

	/* Secondary Attributes */
	// Derived combat stats also use REPNOTIFY_Always so UI refresh stays deterministic after clamping.
	UPROPERTY(BlueprintReadOnly, ReplicatedUsing = OnRep_MaxHealth, Category = "Secondary Attributes")
	FGameplayAttributeData MaxHealth;

	UPROPERTY(BlueprintReadOnly, ReplicatedUsing = OnRep_MaxMana, Category = "Secondary Attributes")
	FGameplayAttributeData MaxMana;

	UPROPERTY(BlueprintReadOnly, ReplicatedUsing = OnRep_Armor, Category = "Secondary Attributes")
	FGameplayAttributeData Armor;

	UPROPERTY(BlueprintReadOnly, ReplicatedUsing = OnRep_ArmorPenetration, Category = "Secondary Attributes")
	FGameplayAttributeData ArmorPenetration;

	UPROPERTY(BlueprintReadOnly, ReplicatedUsing = OnRep_BlockChance, Category = "Secondary Attributes")
	FGameplayAttributeData BlockChance;

	UPROPERTY(BlueprintReadOnly, ReplicatedUsing = OnRep_CriticalHitChance, Category = "Secondary Attributes")
	FGameplayAttributeData CriticalHitChance;

	UPROPERTY(BlueprintReadOnly, ReplicatedUsing = OnRep_CriticalHitDamage, Category = "Secondary Attributes")
	FGameplayAttributeData CriticalHitDamage;

	UPROPERTY(BlueprintReadOnly, ReplicatedUsing = OnRep_CriticalHitResilience, Category = "Secondary Attributes")
	FGameplayAttributeData CriticalHitResilience;

	UPROPERTY(BlueprintReadOnly, ReplicatedUsing = OnRep_HealthRegeneration, Category = "Secondary Attributes")
	FGameplayAttributeData HealthRegeneration;

	UPROPERTY(BlueprintReadOnly, ReplicatedUsing = OnRep_ManaRegeneration, Category = "Secondary Attributes")
	FGameplayAttributeData ManaRegeneration;

	/* Resistance Attributes */
	UPROPERTY(BlueprintReadOnly, ReplicatedUsing = OnRep_FireResistance, Category = "Secondary Attributes")
	FGameplayAttributeData FireResistance;

	UPROPERTY(BlueprintReadOnly, ReplicatedUsing = OnRep_LightningResistance, Category = "Secondary Attributes")
	FGameplayAttributeData LightningResistance;

	UPROPERTY(BlueprintReadOnly, ReplicatedUsing = OnRep_ArcaneResistance, Category = "Secondary Attributes")
	FGameplayAttributeData ArcaneResistance;

	UPROPERTY(BlueprintReadOnly, ReplicatedUsing = OnRep_PhysicalResistance, Category = "Secondary Attributes")
	FGameplayAttributeData PhysicalResistance;
};

