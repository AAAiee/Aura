// @Copyright HaolunYuan

#pragma once

#include "CoreMinimal.h"
#include "GameplayEffectTypes.h"
#include "AuraAbilityTypes.generated.h"

class UAbilitySystemComponent;
class UGameplayEffect;

/**
 * Data bundle used when an ability wants the shared Aura damage pipeline to build and apply
 * a GameplayEffect spec for it.
 *
 * The ability owns authoring values such as base damage, damage type, and debuff tuning. The
 * library function owns the repetitive GAS plumbing: create a context, create an outgoing spec,
 * write set-by-caller magnitudes, and apply the spec to the target ASC.
 */
USTRUCT(BlueprintType)
struct FDamageEffectParameters
{
	GENERATED_BODY()

	FDamageEffectParameters() = default;

	/** World/object context kept with the payload for Blueprint callers that need normal UE lookup chains. */
	UPROPERTY(BlueprintReadWrite)
	TObjectPtr<UObject> WorldContextObject = nullptr;

	/** Gameplay Effect class whose execution calculation converts set-by-caller data into IncomingDamage. */
	UPROPERTY(BlueprintReadWrite)
	TSubclassOf<UGameplayEffect> DamageGameplayEffectClass = nullptr;

	/** Source ASC that authors the outgoing spec and is credited as the damage instigator. */
	UPROPERTY(BlueprintReadWrite)
	TObjectPtr<UAbilitySystemComponent> SourceAbilitySystemComponent = nullptr;

	/** Target ASC that receives the authored damage/debuff GameplayEffect spec. */
	UPROPERTY(BlueprintReadWrite)
	TObjectPtr<UAbilitySystemComponent> TargetAbilitySystemComponent = nullptr;

	/** Raw damage amount before ExecCalc_Damage applies resistance, block, crit, and armor rules. */
	UPROPERTY(BlueprintReadWrite)
	float BaseDamage = 0.0f;

	/** Ability level used by scalable floats and GameplayEffect spec level calculations. */
	UPROPERTY(BlueprintReadWrite)
	float AbilityLevel = 1.f;

	/** DamageType.* tag used as the set-by-caller key and as the resistance/debuff lookup key. */
	UPROPERTY(BlueprintReadWrite)
	FGameplayTag DamageType = FGameplayTag();

	/** Periodic damage applied by the dynamic debuff effect if the debuff roll succeeds. */
	UPROPERTY(BlueprintReadWrite)
	float DebuffDamage = 0.f;

	/** Base chance, before target resistance, that this hit applies the mapped debuff. */
	UPROPERTY(BlueprintReadWrite)
	float DebuffChance = 0.f;

	/** Tick interval for the dynamic debuff effect's periodic damage. */
	UPROPERTY(BlueprintReadWrite)
	float DebuffFrequency = 0.f;

	/** Total duration for the dynamic debuff effect. */
	UPROPERTY(BlueprintReadWrite)
	float DebuffDuration = 0.f;
	
	UPROPERTY(BlueprintReadWrite)
	float DeathImpulseMagnitude =0.f; 
	
	UPROPERTY(BlueprintReadWrite)
	FVector DeathImpulse = FVector::ZeroVector;
	
	UPROPERTY(BlueprintReadWrite)
	FVector KnockBackForce = FVector::ZeroVector;
	
	UPROPERTY(BlueprintReadWrite)
	float KnockBackMagnitude = 0.f; 
	
	UPROPERTY(BlueprintReadWrite)
	float KnockBackChance = 0.f;
	
	UPROPERTY(BlueprintReadWrite)
	bool bIsRadialDamage = false; 
	
	UPROPERTY(BlueprintReadWrite)
	float  RadialDamageInnerRadius = 0.f;
	
	UPROPERTY(BlueprintReadWrite)
	float  RadialDamageOuterRadius = 0.f;
	
	UPROPERTY(BlueprintReadWrite)
	FVector RadialDamageOrigin = FVector::ZeroVector;
};


/**
 * Aura's custom GameplayEffect context.
 *
 * GAS copies this context into outgoing specs, so it is a good home for small pieces of
 * per-hit metadata that should travel from the server-side ExecCalc to later consumers such
 * as AttributeSets, GameplayCues, and client UI. The actual damage still lives in attributes;
 * these flags only describe how that damage was resolved.
 */
USTRUCT()
struct FAuraGameplayEffectContext : public FGameplayEffectContext
{
	GENERATED_BODY()

public:
	/* FGameplayEffectContext */
	virtual UScriptStruct* GetScriptStruct() const override;
	virtual bool NetSerialize(FArchive& Ar, class UPackageMap* Map, bool& bOutSuccess) override;
	virtual FAuraGameplayEffectContext* Duplicate() const override;

	/* Combat Result Flags */
	/** True when ExecCalc_Damage resolved this hit as a critical hit. Used for damage text styling. */
	bool IsCriticalHit() const { return bIsCriticalHit; }
	/** True when ExecCalc_Damage resolved this hit as blocked. Used for damage text styling. */
	bool IsBlockedHit() const { return bIsBlockedHit; }
	/** True when the target should trigger the shared hit-react ability after damage lands. */
	bool ShouldHitReact() const { return bShouldHitReact; }
	/** True when the server-side debuff roll succeeded and the AttributeSet should build a timed effect. */
	bool IsSuccessfulDebuff() const { return bIsSuccessfulDebuff; }
	/** Periodic damage value copied from the incoming spec for the dynamic debuff effect. */
	float GetDebuffDamage() const { return DebuffDamage; }
	/** Duration copied from the incoming spec for the dynamic debuff effect. */
	float GetDebuffDuration() const { return DebuffDuration; }
	/** Tick frequency copied from the incoming spec for the dynamic debuff effect. */
	float GetDebuffFrequency() const { return DebuffFrequency; }
	/** Damage type that won the debuff roll; the AttributeSet maps it to the matching Debuff.* tag. */
	TSharedPtr<FGameplayTag> GetDamageTypeTag() const { return DebuffDamageTypeTag; }
	FVector GetDeathImpulse() const { return DeathImpulse; }
	FVector GetKnockBackForce() const {return KnockBackForce; }
	bool GetIsRadialDamage() const { return bIsRadialDamage; }
	float GetRadialDamageInnerRadius() const { return RadialDamageInnerRadius; }
	float GetRadialDamageOuterRadius() const { return RadialDamageOuterRadius; }
	FVector GetRadialDamageOrigin() const { return RadialDamageOrigin; }

	void SetCriticalHit(bool bInIsCriticalHit) { bIsCriticalHit = bInIsCriticalHit; }
	void SetBlockedHit(bool bInIsBlockedHit) { bIsBlockedHit = bInIsBlockedHit; }
	void SetShouldHitReact(bool bInShouldHitReact) { bShouldHitReact = bInShouldHitReact; }
	void SetSuccessfulDebuff(bool bInIsSuccessfulDebuff) { bIsSuccessfulDebuff = bInIsSuccessfulDebuff; }
	void SetDebuffDamage(float InDebuffDamage) { DebuffDamage = InDebuffDamage; }
	void SetDebuffDuration(float InDebuffDuration) { DebuffDuration = InDebuffDuration; }
	void SetDebuffFrequency(float InDebuffFrequency) { DebuffFrequency = InDebuffFrequency; }
	void SetDamageTypeTag(const FGameplayTag& InDamageTypeTag) { DebuffDamageTypeTag = MakeShared<FGameplayTag>(InDamageTypeTag); }
	void SetDeathImpulse(const FVector& InDeathImpulse) { DeathImpulse = InDeathImpulse; }
	void SetKnockBackForce(const FVector& InKnockBackForce) { KnockBackForce = InKnockBackForce; }
	void SetIsRadialDamage(bool bInIsRadialDamage) { bIsRadialDamage = bInIsRadialDamage; }
	void SetRadialDamageInnerRadius(float InRadialDamageInnerRadius) { RadialDamageInnerRadius = InRadialDamageInnerRadius; }
	void SetRadialDamageOuterRadius(float InRadialDamageOuterRadius) { RadialDamageOuterRadius = InRadialDamageOuterRadius; }
	void SetRadialDamageOrigin(const FVector& InRadialDamageOrigin) { RadialDamageOrigin = InRadialDamageOrigin; }

protected:
	/* Replicated Metadata */
	// These small flags are packed into a custom bitmask by NetSerialize instead of replicated as
	// independent UObject properties. The context travels inside GameplayEffect specs, not as a
	// replicated actor/subobject, so NetSerialize is the authoritative copy path.
	UPROPERTY()
	bool bIsCriticalHit = false;

	UPROPERTY()
	bool bIsBlockedHit = false;

	UPROPERTY()
	bool bShouldHitReact = false;

	UPROPERTY()
	bool bIsSuccessfulDebuff = false;

	UPROPERTY()
	float DebuffDamage = 0.f;

	UPROPERTY()
	float DebuffDuration = 0.f;

	UPROPERTY()
	float DebuffFrequency = 0.f;
	
	UPROPERTY()
	FVector DeathImpulse = FVector::ZeroVector;

	// TSharedPtr is the same storage shape used by the engine's FGameplayEffectContext for optional
	// nested payloads like hit results. It is not a UPROPERTY, so Duplicate() and NetSerialize() are
	// responsible for preserving it when GAS copies or replicates the context.
	TSharedPtr<FGameplayTag> DebuffDamageTypeTag;
	
	UPROPERTY()
	FVector KnockBackForce = FVector::ZeroVector;
	
	UPROPERTY()
	bool bIsRadialDamage = false; 
	
	UPROPERTY()
	float  RadialDamageInnerRadius = 0.f;
	
	UPROPERTY()
	float  RadialDamageOuterRadius = 0.f;
	
	UPROPERTY()
	FVector RadialDamageOrigin = FVector::ZeroVector;
};

template<>
struct TStructOpsTypeTraits<FAuraGameplayEffectContext> : public TStructOpsTypeTraitsBase2<FAuraGameplayEffectContext>
{
	enum
	{
		WithNetSerializer = true,
		WithCopy = true
	};
};
