// @Copyright HaolunYuan

#pragma once

#include "CoreMinimal.h"
#include "AuraDamageGameplayAbility.h"
#include "AuraProjectileSpell.generated.h"

class AAuraProjectile;
struct FGameplayTag;

/**
 * Base gameplay ability helper for spells that launch pooled Aura projectiles.
 *
 * The ability stays responsible for authoring the projectile's initial state:
 *   - where it spawns
 *   - which direction it should travel
 *   - which damage Gameplay Effect spec it should carry into the hit callback
 */
UCLASS()
class AURA_API UAuraProjectileSpell : public UAuraDamageGameplayAbility
{
	GENERATED_BODY()

public:
	/* Projectile Casting */
	// Server-only helper that borrows a projectile from the pool, seeds its damage spec, and launches it.
	UFUNCTION(BlueprintCallable, Category = "Projectile")
	void SpawnProjectile(const FVector& ProjectileTargetLocation, const FGameplayTag& CombatSocket);

protected:
	/* UGameplayAbility */
	// Kept available for Blueprint child abilities that need normal GAS activation flow first.
	virtual void ActivateAbility(const FGameplayAbilitySpecHandle Handle,
		const FGameplayAbilityActorInfo* ActorInfo,
		const FGameplayAbilityActivationInfo ActivationInfo,
		const FGameplayEventData* TriggerEventData) override;

	/* Projectile Authoring */
	// Blueprint chooses which pooled projectile actor class this spell should launch.
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Projectile")
	TSubclassOf<AAuraProjectile> ProjectileClass;
};
