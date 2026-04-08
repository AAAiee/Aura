// @Copyright HaolunYuan

#pragma once

#include "CoreMinimal.h"
#include "Components/AbilitySystem/Ability/AuraGameplayAbility.h"
#include "AuraProjectileSpell.generated.h"

class AAuraProjectile;
class UObjectPoolSubsystem;
class UGameplayEffect;

/**
 * Base gameplay ability helper for spells that launch pooled Aura projectiles.
 *
 * The ability stays responsible for authoring the projectile's initial state:
 *   - where it spawns
 *   - which direction it should travel
 *   - which damage Gameplay Effect spec it should carry into the hit callback
 */
UCLASS()
class AURA_API UAuraProjectileSpell : public UAuraGameplayAbility
{
	GENERATED_BODY()

public:
	// Server-only helper that borrows a projectile from the pool, seeds its damage spec, and launches it.
	UFUNCTION(BlueprintCallable, Category="Projectile")
	void SpawnProjectile(const FVector& ProjectileTargetLocation);

protected:
	virtual void ActivateAbility(const FGameplayAbilitySpecHandle Handle,
		const FGameplayAbilityActorInfo* ActorInfo,
		const FGameplayAbilityActivationInfo ActivationInfo,
		const FGameplayEventData* TriggerEventData) override;


protected:
	// Blueprint chooses which pooled projectile actor class this spell should launch.
	UPROPERTY(EditDefaultsOnly, Category = "Projectile")
	TSubclassOf<AAuraProjectile> ProjectileClass;

	// Gameplay Effect class used to build the outgoing damage spec carried by the projectile.
	UPROPERTY(EditDefaultsOnly, Category = "Projectile")
	TSubclassOf<UGameplayEffect>  DamageEffect;

};
