// @Copyright HaolunYuan

#pragma once

#include "CoreMinimal.h"
#include "Components/AbilitySystem/Ability/AuraGameplayAbility.h"
#include "AuraProjectileSpell.generated.h"

class AAuraProjectile;
class UObjectPoolSubsystem;

/**
 * 
 */
UCLASS()
class AURA_API UAuraProjectileSpell : public UAuraGameplayAbility
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable, Category="Projectile")
	void SpawnProjectile();

protected:
	virtual void ActivateAbility(const FGameplayAbilitySpecHandle Handle,
		const FGameplayAbilityActorInfo* ActorInfo,
		const FGameplayAbilityActivationInfo ActivationInfo,
		const FGameplayEventData* TriggerEventData) override;



protected:
	UPROPERTY(EditDefaultsOnly, Category = "Projectile")
	TSubclassOf<AAuraProjectile> ProjectileClass;

};
