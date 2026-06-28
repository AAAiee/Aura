#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "PoolableActor.h"
#include "AuraPooledGameplayActor.generated.h"

class UObjectPoolSubsystem;

/**
 * Base class for Aura actors that participate in the object pool.
 *
 * Responsibilities:
 *   - Bridge the plugin's pool lifecycle callbacks into Aura code.
 *   - Provide a single return-to-pool entry point for subclasses.
 *   - Offer overridable reset/activation hooks for gameplay-specific state.
 *
 * Subclasses should keep per-use state out of BeginPlay and instead reset/init
 * themselves through ResetPooledState and HandleTakenFromPool.
 */
UCLASS(Abstract)
class AURA_API AAuraPooledGameplayActor : public AActor, public IPoolableActor
{
	GENERATED_BODY()

public:
	AAuraPooledGameplayActor();

	/** Returns this actor to the world pool subsystem that owns it. */
	UFUNCTION(BlueprintCallable, Category = "Pooling")
	void RequestReturnToPool();

	UFUNCTION(BlueprintPure, Category = "Pooling")
	bool IsActiveInPool() const { return bIsActiveInPool; }

protected:
	/** IPoolableActor implementation - called by the pool when this actor is borrowed. */
	virtual void OnTakenFromPool() override final;

	/** IPoolableActor implementation - called by the pool when this actor is returned. */
	virtual void OnReturnedToPool() override final;

	/** Override to react when the actor is reactivated for gameplay. */
	virtual void HandleTakenFromPool();

	/** Override to react after the actor has been reset and returned to the pool. */
	virtual void HandleReturnedToPool();

	/** Override to scrub any per-use gameplay state before the actor is parked again. */
	virtual void ResetPooledState();

	/** Helper for subclasses that need direct access to the pool subsystem. */
	UObjectPoolSubsystem* GetObjectPoolSubsystem() const;

private:
	UPROPERTY(Transient)
	bool bIsActiveInPool = false;
};
