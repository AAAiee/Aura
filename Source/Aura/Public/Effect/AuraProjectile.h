#pragma once

#include "CoreMinimal.h"
#include "AuraAbilityTypes.h"
#include "Effect/AuraPooledGameplayActor.h"
#include "AuraProjectile.generated.h"

class UProjectileMovementComponent;
class UPrimitiveComponent;
class UAudioComponent;
class UNiagaraSystem;
class USoundBase;

UCLASS(Abstract)
class AURA_API AAuraProjectile : public AAuraPooledGameplayActor
{
	GENERATED_BODY()

public:
	AAuraProjectile();

	// Called by the owning ability after the pooled projectile has been positioned and configured.
	void LaunchInDirection(const FVector& Direction);
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	// The spell builds the outgoing damage spec up front, then the projectile applies it on impact.
	UPROPERTY(BlueprintReadOnly, Category = "Projectile|Effect", meta = (ExposeOnSpawn = true))
	FDamageEffectParameters DamageEffectParameters;

protected:
	virtual void BeginPlay() override;

	virtual void HandleTakenFromPool() override;
	virtual void ResetPooledState() override;

	// Shared overlap logic for all projectile collision shapes.
	UFUNCTION(BlueprintCallable)
	virtual void OnProjectileOverlap(
		UPrimitiveComponent* OverlappedComponent,
		AActor* OtherActor,
		UPrimitiveComponent* OtherComp,
		int32 OtherBodyIndex,
		bool bFromSweep,
		const FHitResult& SweepResult);

	UFUNCTION()
	void OnRep_ReplicatedProjectileActive();

	UFUNCTION(NetMulticast, Reliable)
	void MulticastPlayImpactEffects(const FVector_NetQuantize& ImpactLocation);

	// Cosmetic helpers split out so both authority and replicated clients can apply the same state.
	void PlayImpactEffects(const FVector& ImpactLocation);
	void ApplyActiveState();
	void ApplyInactiveState();

	// Child classes pass in whatever collision component they created.
	void SetCollisionComponent(UPrimitiveComponent* InCollisionComponent);

protected:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Projectile")
	bool bReturnToPoolOnAnyOverlap = false;

	// Guards duplicate hit resolution while the projectile is still active for this activation.
	UPROPERTY(Transient)
	bool bHasResolvedImpact = false;

	// Server-authored lifecycle flag that keeps replicated projectile copies aligned with pool borrow/return.
	UPROPERTY(ReplicatedUsing = OnRep_ReplicatedProjectileActive, Transient)
	bool bReplicatedProjectileActive = false;

	UPROPERTY(EditAnywhere, Category = "Projectile|Impact")
	TObjectPtr<UNiagaraSystem> ImpactEffect;

	// One-shot sound played when the projectile resolves its impact.
	UPROPERTY(EditAnywhere, Category = "Projectile|Impact")
	TObjectPtr<USoundBase> ImpactSound;

	// Optional looping flight audio that should start on borrow and stop on return / impact.
	UPROPERTY(EditAnywhere, Category = "Projectile|Impact")
	TObjectPtr<USoundBase> LoopingSound;

	// Cached runtime audio component so pooled reuse can stop and destroy the previous activation's loop.
	UPROPERTY()
	TObjectPtr<UAudioComponent> LoopingSoundComponent;

protected:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Projectile")
	TObjectPtr<UProjectileMovementComponent> ProjectileMovement;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Projectile")
	TObjectPtr<UPrimitiveComponent> CollisionComponent;
	
	UPROPERTY(Transient)
	TObjectPtr<USceneComponent> HomingTargetComponent;
	
	friend class UAuraFireBolt;
};
