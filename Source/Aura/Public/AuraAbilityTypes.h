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
	bool IsCriticalHit() const;
	bool IsBlockedHit() const;
	bool ShouldHitReact() const;
	bool IsSuccessfulDebuff() const;
	float GetDebuffDamage() const;
	float GetDebuffDuration() const;
	float GetDebuffFrequency() const;
	TSharedPtr<FGameplayTag> GetDamageTypeTag() const;
	FVector GetDeathImpulse() const;
	FVector GetKnockBackForce() const;
	bool GetIsRadialDamage() const;
	float GetRadialDamageInnerRadius() const;
	float GetRadialDamageOuterRadius() const;
	FVector GetRadialDamageOrigin() const;

	void SetCriticalHit(const bool bInIsCriticalHit);
	void SetBlockedHit(const bool bInIsBlockedHit);
	void SetShouldHitReact(const bool bInShouldHitReact);
	void SetSuccessfulDebuff(const bool bInIsSuccessfulDebuff);
	void SetDebuffDamage(const float InDebuffDamage);
	void SetDebuffDuration(const float InDebuffDuration);
	void SetDebuffFrequency(const float InDebuffFrequency);
	void SetDamageTypeTag(const FGameplayTag& InDamageTypeTag);
	void SetDeathImpulse(const FVector& InDeathImpulse);
	void SetKnockBackForce(const FVector& InKnockBackForce);
	void SetIsRadialDamage(const bool bInIsRadialDamage);
	void SetRadialDamageInnerRadius(const float InRadialDamageInnerRadius);
	void SetRadialDamageOuterRadius(const float InRadialDamageOuterRadius);
	void SetRadialDamageOrigin(const FVector& InRadialDamageOrigin);

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
