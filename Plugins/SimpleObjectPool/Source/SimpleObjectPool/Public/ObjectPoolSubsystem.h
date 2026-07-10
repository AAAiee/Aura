#pragma once

#include "CoreMinimal.h"
#include "Subsystems/WorldSubsystem.h"
#include "ObjectPoolSubsystem.generated.h"

class AActor;
class UObjectPoolConfigDataAsset;
struct FPoolClassConfig;

/**
 * FPoolItem
 *
 * Single pooled actor entry used by UObjectPoolSubsystem.
 *
 * Stores the actor instance and whether it is currently in use.
 */
USTRUCT(BlueprintType)
struct SIMPLEOBJECTPOOL_API FPoolItem
{
	GENERATED_BODY()

	/** The actor instance managed by the pool. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ObjectPool|PoolItem")
	TObjectPtr<AActor> ActorInstance = nullptr;

	/** Whether this actor is currently active and in use. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ObjectPool|PoolItem")
	bool bInUse = false;
};

/**
 * FPoolRecyclePolicy
 *
 * Runtime recycle policy resolved for a pooled actor class.
 *
 * This is derived from configuration data and used by gameplay code when
 * borrowing actors from the pool.
 */
USTRUCT(BlueprintType)
struct SIMPLEOBJECTPOOL_API FPoolRecyclePolicy
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ObjectPool")
	bool bShouldAutomaticallyReturn = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ObjectPool", meta = (ClampMin = "0.0"))
	float RecycleDelay = 0.f;
};

/**
 * UObjectPoolSubsystem
 *
 * Centralized world subsystem for reusing actor instances at runtime.
 *
 * The subsystem pre-spawns configured actors, hands out inactive instances on
 * demand, expands pools when needed, and optionally schedules actors to return
 * after a recycle delay. Pooled actors can implement IPoolableActor to receive
 * borrow and return callbacks.
 *
 * Important functions:
 *   - InitializePool() - Creates a manual pool for an actor class.
 *   - GetPooledActor() - Borrows an actor using config-driven recycle behavior.
 *   - GetPooledActorWithRecyclePolicy() - Borrows an actor with an explicit recycle override.
 *   - ReturnActorToPool() - Deactivates an actor and marks it available again.
 *
 * Lifetime:
 *   - One subsystem instance exists per world.
 *   - Deinitialize() clears timers and tracked actor maps for that world.
 */
UCLASS(BlueprintType)
class SIMPLEOBJECTPOOL_API UObjectPoolSubsystem : public UWorldSubsystem
{
	GENERATED_BODY()

public:
	UObjectPoolSubsystem();

	/* UWorldSubsystem begins */
	virtual void Deinitialize() override;
	/* UWorldSubsystem ends */

	/** Returns true if a pool has already been created for the supplied actor class. */
	UFUNCTION(BlueprintCallable, Category = "ObjectPool")
	bool IsPoolInitialized(TSubclassOf<AActor> ActorClass) const;

	UFUNCTION(BlueprintCallable, Category = "ObjectPool")
	const UObjectPoolConfigDataAsset* GetPoolConfig() const;

	/** Initializes a pool for the specified actor class. */
	UFUNCTION(BlueprintCallable, Category = "ObjectPool")
	void InitializePool(TSubclassOf<AActor> ActorClass, int32 InitialSize);

	/** Initializes a pool for the specified actor class using project config when available. */
	UFUNCTION(BlueprintCallable, Category = "ObjectPool")
	bool InitializePoolFromConfig(TSubclassOf<AActor> ActorClass);

	/** Ensures a pool exists for the specified actor class using project config when available. */
	UFUNCTION(BlueprintCallable, Category = "ObjectPool")
	bool EnsurePoolInitializedFromConfig(TSubclassOf<AActor> ActorClass);

	/** Returns the recycle policy for the specified actor class using project config when available. */
	UFUNCTION(BlueprintCallable, Category = "ObjectPool")
	FPoolRecyclePolicy GetRecyclePolicyForClass(TSubclassOf<AActor> ActorClass) const;

	/** Retrieves an available actor from the pool, expanding the pool if required. */
	UFUNCTION(BlueprintCallable, Category = "ObjectPool")
	AActor* GetPooledActor(
		TSubclassOf<AActor> ActorClass,
		const FTransform& SpawnTransform);

	/** Retrieves an available actor from the pool with an explicit runtime recycle policy override. */
	UFUNCTION(BlueprintCallable, Category = "ObjectPool")
	AActor* GetPooledActorWithRecyclePolicy(
		TSubclassOf<AActor> ActorClass,
		const FTransform& SpawnTransform,
		bool bShouldAutomaticallyReturnPool,
		float RecycleDelayTime);

	template<typename T>
	T* GetPooledActorTyped(
		TSubclassOf<T> ActorClass,
		const FTransform& SpawnTransform)
	{
		const TSubclassOf<AActor> BaseActorClass = ActorClass;
		return Cast<T>(GetPooledActor(BaseActorClass, SpawnTransform));
	}

	/** Returns an actor instance back into the pool. */
	UFUNCTION(BlueprintCallable, Category = "ObjectPool")
	void ReturnActorToPool(AActor* Actor);

private:
	/** Finds the project config row for an actor class, returning false when the class is not configured. */
	bool TryGetPoolClassConfig(TSubclassOf<AActor> InActorClass, FPoolClassConfig& OutConfig) const;

	/** Converts authored config data into the small runtime policy used during a borrow. */
	static FPoolRecyclePolicy BuildRecyclePolicy(const FPoolClassConfig& PoolClassConfig);

	/** Lazily loads the developer-settings pool config asset once per world subsystem. */
	const UObjectPoolConfigDataAsset* LoadPoolConfigIfNeeded() const;

	/** Internal borrow path shared by default config and explicit recycle-policy requests. */
	AActor* BorrowPooledActor(
		TSubclassOf<AActor> ActorClass,
		const FTransform& SpawnTransform,
		bool bShouldAutomaticallyReturnPool,
		float RecycleDelayTime);

	/** Starts or clears an auto-return timer for a borrowed actor. */
	void DelayActor(AActor* InActor, float InDelayTime, bool bInAutomaticallyReturnPool);

	/** Cancels any pending auto-return timer before an actor is reused or returned. */
	void ClearReturnTimer(AActor* InActor);

	/** Hides, disables, and optionally notifies an actor before it rests in the pool. */
	void DeactivateActor(AActor* InSpawnedActor, bool bNotifyPoolableActor = true);

	/** Positions, enables, optionally possesses, and notifies an actor as it leaves the pool. */
	void ActivateActor(
		AActor* InFreeActor,
		const FTransform& InSpawnTransform,
		bool bInShouldAutomaticallyReturnPool,
		float InRecycleDelayTime);

	/** Spawns one hidden actor and records its reverse lookup so it can be returned later. */
	AActor* SpawnPooledActor(TSubclassOf<AActor> ActorClass);

	/** Optional interface hook fired after an actor has been activated from the pool. */
	static void NotifyActorTakenFromPool(AActor* InActor);

	/** Optional interface hook fired before an actor is deactivated into the pool. */
	static void NotifyActorReturnedToPool(AActor* InActor);

	/** Emits a compact pool-usage message after initialize, borrow, expand, or return operations. */
	void LogPoolState(
		const TCHAR* Action,
		TSubclassOf<AActor> ActorClass,
		const TArray<FPoolItem>& TargetPool,
		const AActor* ActorInstance = nullptr) const;

	/** Counts valid actors currently marked in-use for diagnostics. */
	static int32 CountInUseActors(const TArray<FPoolItem>& TargetPool);

private:
	/** Mapping of actor class to pooled actor entries. */
	TMap<TSubclassOf<AActor>, TArray<FPoolItem>> Pool;

	/** Fast reverse lookup used when returning pooled actors. */
	TMap<TWeakObjectPtr<AActor>, TSubclassOf<AActor>> ActorToPoolClassMap;

	/** Tracks pending auto-return timers so stale timers can be cancelled on reuse. */
	TMap<TWeakObjectPtr<AActor>, FTimerHandle> ActiveReturnTimers;

	/** Transform used to hide inactive actors underground and out of view. */
	const FTransform HiddenTransform;

	/** Lazily loaded project-level pool config asset, if one is assigned in developer settings. */
	mutable TWeakObjectPtr<const UObjectPoolConfigDataAsset> CachedPoolConfig;
};
