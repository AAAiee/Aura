// @Copyright HaolunYuan

#pragma once

#include "CoreMinimal.h"
#include "AuraAbilityTypes.h"
#include "Components/AbilitySystem/Ability/AuraGameplayAbility.h"
#include "GameplayTagContainer.h"
#include "ScalableFloat.h"
#include "AuraDamageGameplayAbility.generated.h"

class UGameplayEffect;

/**
 * Shared base class for abilities that author damage payloads.
 *
 * Projectile and melee abilities can build their delivery behavior differently, but they should
 * speak the same data language to the damage GameplayEffect: a damage effect class plus one or
 * more set-by-caller magnitudes keyed by DamageType.* tags.
 */
UCLASS()
class AURA_API UAuraDamageGameplayAbility : public UAuraGameplayAbility
{
	GENERATED_BODY()

public:
	/* Damage Application */
	/** Directly applies this ability's damage effect to a target actor. Projectile spells usually use the parameter bundle instead. */
	UFUNCTION(BlueprintCallable, Category = "Damage")
	void CauseDamage(AActor* TargetActor);

	/** Copies the class-default damage/debuff tuning into a payload that projectiles can carry until impact. */
	UFUNCTION(BlueprintPure)
	FDamageEffectParameters MakeDamageEffectParametersFromClassDefault(AActor* TargetActor = nullptr) const;

	/* Damage Authoring */
	// Gameplay Effect class that owns the ExecCalc writing IncomingDamage on the target AttributeSet.
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Damage")
	TSubclassOf<UGameplayEffect> DamageEffectClass;

	/** DamageType.* set-by-caller key. ExecCalc_Damage maps this to resistance and debuff type. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Damage")
	FGameplayTag DamageTypeTag;

	/** Base damage curve for this ability before resistance, armor, block, and critical-hit rules. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Damage")
	FScalableFloat DamageMagnitude;

	/** Authored debuff success chance before target resistance reduces it. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Debuff")
	float DebuffChance = 20.f;

	/** Periodic damage dealt by the dynamic debuff effect after the debuff succeeds. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Debuff")
	float DebuffDamage = 5.0f;

	/** Tick interval for the dynamic debuff effect. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Debuff")
	float DebuffFrequency = 1.0f;

	/** Total lifetime for the dynamic debuff effect. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Debuff")
	float DebuffDuration = 5.0f;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Death")
	float DeathImpulseMagnitude = 1000.f;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "KnockBack")
	float KnockbackForceMagnitude = 60.f;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "KnockBack")
	float KnockbackChance = 0.f;
};
