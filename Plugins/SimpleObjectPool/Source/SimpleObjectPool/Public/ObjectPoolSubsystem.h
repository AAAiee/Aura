#pragma once

#include "CoreMinimal.h"
#include "Subsystems/WorldSubsystem.h"
#include "ObjectPoolSubsystem.generated.h"

class AActor;
class UObjectPoolConfigDataAsset;
struct FPoolClassConfig;

/**
 * A single pooled item entry used by the object pool system.
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
 * Runtime recycle policy resolved for a pooled actor class.
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
 * A centralized object pooling system used for reusing actor instances at runtime.
 *
 * This subsystem:
 *   - Pre-spawns a configurable number of actors for a given class.
 *   - Provides already-spawned actors when requested, avoiding SpawnActor cost.
 *   - Expands pools dynamically when necessary.
 *   - Supports automatic return of actors to the pool after a delay.
 *   - Notifies pooled actors when they are borrowed and returned.
 *
 * This subsystem lives for the lifetime of the current world.
 */
UCLASS(BlueprintType)
class SIMPLEOBJECTPOOL_API UObjectPoolSubsystem : public UWorldSubsystem
{
	GENERATED_BODY()

public:
	/** Constructor - initializes the default hidden transform used for pooled actors. */
	UObjectPoolSubsystem();

	virtual void Deinitialize() override;

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
	bool TryGetPoolClassConfig(TSubclassOf<AActor> ActorClass, FPoolClassConfig& OutConfig) const;

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
	void DelayActor(AActor* InActor, float DelayTime, bool bAutomaticallyReturnPool);

	/** Cancels any pending auto-return timer before an actor is reused or returned. */
	void ClearReturnTimer(AActor* InActor);

	/** Hides, disables, and optionally notifies an actor before it rests in the pool. */
	void DeactivateActor(AActor* SpawnedActor, bool bNotifyPoolableActor = true);

	/** Positions, enables, optionally possesses, and notifies an actor as it leaves the pool. */
	void ActivateActor(
		AActor* FreeActor,
		const FTransform& SpawnTransform,
		bool bShouldAutomaticallyReturnPool,
		float RecycleDelayTime);

	/** Spawns one hidden actor and records its reverse lookup so it can be returned later. */
	AActor* SpawnPooledActor(TSubclassOf<AActor> ActorClass);

	/** Optional interface hook fired after an actor has been activated from the pool. */
	static void NotifyActorTakenFromPool(AActor* Actor);

	/** Optional interface hook fired before an actor is deactivated into the pool. */
	static void NotifyActorReturnedToPool(AActor* Actor);

	/** Emits a compact pool-usage message after initialize, borrow, expand, or return operations. */
	void LogPoolState(const TCHAR* Action, TSubclassOf<AActor> ActorClass, const TArray<FPoolItem>& TargetPool, const AActor* ActorInstance = nullptr) const;

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
