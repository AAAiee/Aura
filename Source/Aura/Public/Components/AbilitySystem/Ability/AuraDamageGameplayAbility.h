// @Copyright HaolunYuan

#pragma once

#include "CoreMinimal.h"
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
	UFUNCTION(BlueprintCallable, Category = "Damage")
	void CauseDamage(AActor* TargetActor);




	/* Damage Authoring */
	// Gameplay Effect class that owns the ExecCalc writing IncomingDamage on the target AttributeSet.
	UPROPERTY(EditDefaultsOnly,BlueprintReadOnly, Category = "Damage")
	TSubclassOf<UGameplayEffect> DamageEffect;

	// Designer-authored typed damage. Each entry becomes one set-by-caller value on the outgoing spec.
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Damage")
	TMap<FGameplayTag, FScalableFloat> DamageType;
};
