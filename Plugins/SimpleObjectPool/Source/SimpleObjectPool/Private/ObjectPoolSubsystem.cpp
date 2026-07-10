#include "ObjectPoolSubsystem.h"

#include "AIController.h"
#include "Engine/World.h"
#include "GameFramework/Controller.h"
#include "GameFramework/Pawn.h"
#include "ObjectPoolConfigDataAsset.h"
#include "ObjectPoolDeveloperSettings.h"
#include "PoolableActor.h"
#include "SimpleObjectPool.h"
#include "TimerManager.h"

UObjectPoolSubsystem::UObjectPoolSubsystem()
	: HiddenTransform(FTransform(
		FRotator::ZeroRotator,
		FVector(0.f, 0.f, -50000.f),
		FVector::OneVector))
{
}

void UObjectPoolSubsystem::Deinitialize()
{
	// World teardown must cancel delayed returns before pooled actors and lookup maps are discarded.
	if (UWorld* World = GetWorld())
	{
		for (TPair<TWeakObjectPtr<AActor>, FTimerHandle>& ActiveTimerPair : ActiveReturnTimers)
		{
			World->GetTimerManager().ClearTimer(ActiveTimerPair.Value);
		}
	}

	ActiveReturnTimers.Empty();
	ActorToPoolClassMap.Empty();
	Pool.Empty();

	Super::Deinitialize();
}

bool UObjectPoolSubsystem::IsPoolInitialized(TSubclassOf<AActor> InActorClass) const
{
	return InActorClass && Pool.Contains(InActorClass);
}

const UObjectPoolConfigDataAsset* UObjectPoolSubsystem::GetPoolConfig() const
{
	return LoadPoolConfigIfNeeded();
}

const UObjectPoolConfigDataAsset* UObjectPoolSubsystem::LoadPoolConfigIfNeeded() const
{
	if (CachedPoolConfig.IsValid())
	{
		return CachedPoolConfig.Get();
	}

	const UObjectPoolDeveloperSettings* DeveloperSettings = GetDefault<UObjectPoolDeveloperSettings>();
	if (!DeveloperSettings)
	{
		return nullptr;
	}

	const TSoftObjectPtr<UObjectPoolConfigDataAsset>& ConfigReference = DeveloperSettings->DefaultPoolConfig;
	if (ConfigReference.IsNull())
	{
		return nullptr;
	}

	UObjectPoolConfigDataAsset* LoadedConfig = ConfigReference.LoadSynchronous();
	CachedPoolConfig = LoadedConfig;
	return LoadedConfig;
}

void UObjectPoolSubsystem::InitializePool(TSubclassOf<AActor> InActorClass, int32 InInitialSize)
{
	checkf(InInitialSize > 0, TEXT("ObjectPoolSubsystem: Initial pool size must be greater than zero."));
	checkf(InActorClass, TEXT("ObjectPoolSubsystem: Actor class must not be null."));

	if (Pool.Contains(InActorClass))
	{
		UE_LOG(LogSimpleObjectPool, Verbose, TEXT("Object pool for class %s is already initialized."), *InActorClass->GetName());
		return;
	}

	TArray<FPoolItem> PoolItems;
	PoolItems.Reserve(InInitialSize);

	// Initial actors are spawned once, parked at the hidden transform, and then reused by borrow requests.
	for (int32 Index = 0; Index < InInitialSize; ++Index)
	{
		AActor* SpawnedActor = SpawnPooledActor(InActorClass);
		checkf(SpawnedActor, TEXT("ObjectPoolSubsystem: Failed to spawn actor while initializing pool for class %s."), *InActorClass->GetName());

		DeactivateActor(SpawnedActor, false);
		PoolItems.Add(FPoolItem{ SpawnedActor, false });
	}

	Pool.Add(InActorClass, MoveTemp(PoolItems));
	const TArray<FPoolItem>* InitializedPool = Pool.Find(InActorClass);
	checkf(InitializedPool, TEXT("ObjectPoolSubsystem: Failed to register pool for class %s."), *InActorClass->GetName());
	LogPoolState(TEXT("Initialized object pool"), InActorClass, *InitializedPool);
}

bool UObjectPoolSubsystem::InitializePoolFromConfig(TSubclassOf<AActor> InActorClass)
{
	checkf(InActorClass, TEXT("ObjectPoolSubsystem: Actor class must not be null."));

	if (IsPoolInitialized(InActorClass))
	{
		return true;
	}

	FPoolClassConfig PoolClassConfig;
	if (!TryGetPoolClassConfig(InActorClass, PoolClassConfig))
	{
		return false;
	}

	InitializePool(InActorClass, PoolClassConfig.InitialPoolSize);
	return IsPoolInitialized(InActorClass);
}

bool UObjectPoolSubsystem::EnsurePoolInitializedFromConfig(TSubclassOf<AActor> InActorClass)
{
	checkf(InActorClass, TEXT("ObjectPoolSubsystem: Actor class must not be null."));

	if (!IsPoolInitialized(InActorClass))
	{
		return InitializePoolFromConfig(InActorClass);
	}

	return true;
}

FPoolRecyclePolicy UObjectPoolSubsystem::GetRecyclePolicyForClass(TSubclassOf<AActor> InActorClass) const
{
	checkf(InActorClass, TEXT("ObjectPoolSubsystem: Actor class must not be null."));

	FPoolClassConfig PoolClassConfig;
	if (!TryGetPoolClassConfig(InActorClass, PoolClassConfig))
	{
		return FPoolRecyclePolicy{};
	}

	return BuildRecyclePolicy(PoolClassConfig);
}

bool UObjectPoolSubsystem::TryGetPoolClassConfig(TSubclassOf<AActor> InActorClass, FPoolClassConfig& OutConfig) const
{
	checkf(InActorClass, TEXT("ObjectPoolSubsystem: Actor class must not be null."));

	const UObjectPoolConfigDataAsset* PoolConfig = LoadPoolConfigIfNeeded();
	if (!ensureMsgf(PoolConfig, TEXT("ObjectPoolSubsystem: No pool config asset is assigned in developer settings while resolving class %s."), *InActorClass->GetName()))
	{
		return false;
	}

	const bool bFoundConfig = PoolConfig->FindPoolConfigByClass(InActorClass, OutConfig);
	ensureMsgf(bFoundConfig, TEXT("ObjectPoolSubsystem: No pool config entry was found for class %s in the assigned config asset."), *InActorClass->GetName());
	return bFoundConfig;
}

FPoolRecyclePolicy UObjectPoolSubsystem::BuildRecyclePolicy(const FPoolClassConfig& PoolClassConfig)
{
	FPoolRecyclePolicy RecyclePolicy;
	if (PoolClassConfig.bUseDefaultRecycleDelay)
	{
		RecyclePolicy.bShouldAutomaticallyReturn = true;
		RecyclePolicy.RecycleDelay = PoolClassConfig.DefaultRecycleDelay;
	}

	return RecyclePolicy;
}

AActor* UObjectPoolSubsystem::GetPooledActor(TSubclassOf<AActor> InActorClass, const FTransform& InSpawnTransform)
{
	checkf(InActorClass, TEXT("ObjectPoolSubsystem: Actor class must not be null."));

	// Pipeline:
	// 1. Use an existing pool when it has already been initialized manually or from config.
	// 2. Initialize from config when no pool exists yet.
	// 3. Borrow using the resolved recycle policy so auto-return behavior stays centralized.
	// Preserve the original "manual pool + optional config" behavior:
	//   - If a pool already exists, borrow from it even when there is no config entry.
	//   - If no pool exists yet, fall back to config-driven initialization.
	FPoolRecyclePolicy RecyclePolicy;
	if (!IsPoolInitialized(InActorClass))
	{
		FPoolClassConfig PoolClassConfig;
		checkf(TryGetPoolClassConfig(InActorClass, PoolClassConfig), TEXT("ObjectPoolSubsystem: Missing pool config for class %s."), *InActorClass->GetName());
		InitializePool(InActorClass, PoolClassConfig.InitialPoolSize);
		RecyclePolicy = BuildRecyclePolicy(PoolClassConfig);
	}
	else
	{
		RecyclePolicy = GetRecyclePolicyForClass(InActorClass);
	}

	return BorrowPooledActor(InActorClass, InSpawnTransform, RecyclePolicy.bShouldAutomaticallyReturn, RecyclePolicy.RecycleDelay);
}

AActor* UObjectPoolSubsystem::GetPooledActorWithRecyclePolicy(
	TSubclassOf<AActor> InActorClass,
	const FTransform& InSpawnTransform,
	bool bInShouldAutomaticallyReturnPool,
	float InRecycleDelayTime)
{
	checkf(InActorClass, TEXT("ObjectPoolSubsystem: Actor class must not be null."));
	checkf(
		IsPoolInitialized(InActorClass) || EnsurePoolInitializedFromConfig(InActorClass),
		TEXT("ObjectPoolSubsystem: No pool exists for class %s, and config-based initialization failed."),
		*InActorClass->GetName());

	return BorrowPooledActor(InActorClass, InSpawnTransform, bInShouldAutomaticallyReturnPool, InRecycleDelayTime);
}

AActor* UObjectPoolSubsystem::BorrowPooledActor(
	TSubclassOf<AActor> InActorClass,
	const FTransform& InSpawnTransform,
	bool bInShouldAutomaticallyReturnPool,
	float InRecycleDelayTime)
{
	checkf(InActorClass, TEXT("ObjectPoolSubsystem: Actor class must not be null."));

	TArray<FPoolItem>* TargetPool = Pool.Find(InActorClass);
	checkf(TargetPool, TEXT("ObjectPoolSubsystem: No pool exists for class %s. Initialize the pool before borrowing actors."), *InActorClass->GetName());

	// Prefer reusing an inactive valid actor before expanding the pool.
	for (FPoolItem& Item : *TargetPool)
	{
		if (!Item.bInUse && IsValid(Item.ActorInstance))
		{
			Item.bInUse = true;
			AActor* FreeActor = Item.ActorInstance.Get();
			ActivateActor(FreeActor, InSpawnTransform, bInShouldAutomaticallyReturnPool, InRecycleDelayTime);
			return FreeActor;
		}
	}

	static constexpr float GrowthFactor = 0.5f;
	const int32 CurrentCount = TargetPool->Num();
	const int32 NumToSpawn = FMath::Max(1, FMath::CeilToInt(CurrentCount * GrowthFactor));

	// Pool expansion activates the first new actor for the current borrow and parks the rest.
	AActor* SpawnedActorToReturn = nullptr;
	for (int32 Index = 0; Index < NumToSpawn; ++Index)
	{
		AActor* CurrentSpawnedActor = SpawnPooledActor(InActorClass);
		ensureMsgf(CurrentSpawnedActor, TEXT("ObjectPoolSubsystem: Failed to spawn actor while expanding pool for class %s."), *InActorClass->GetName());
		if (!CurrentSpawnedActor)
		{
			continue;
		}

		if (!SpawnedActorToReturn)
		{
			SpawnedActorToReturn = CurrentSpawnedActor;
			TargetPool->Add(FPoolItem{ SpawnedActorToReturn, true });
			ActivateActor(SpawnedActorToReturn, InSpawnTransform, bInShouldAutomaticallyReturnPool, InRecycleDelayTime);
		}
		else
		{
			DeactivateActor(CurrentSpawnedActor, false);
			TargetPool->Add(FPoolItem{ CurrentSpawnedActor, false });
		}
	}

	checkf(SpawnedActorToReturn, TEXT("ObjectPoolSubsystem: Failed to expand pool for class %s."), *InActorClass->GetName());

	if (SpawnedActorToReturn)
	{
		LogPoolState(TEXT("Expanded object pool and borrowed actor"), InActorClass, *TargetPool, SpawnedActorToReturn);
	}

	return SpawnedActorToReturn;
}

void UObjectPoolSubsystem::ReturnActorToPool(AActor* InActor)
{
	if (!IsValid(InActor))
	{
		return;
	}

	const TWeakObjectPtr<AActor> ActorKey(InActor);
	const TSubclassOf<AActor>* ActorClass = ActorToPoolClassMap.Find(ActorKey);
	if (!ActorClass)
	{
		ensureMsgf(false, TEXT("ObjectPoolSubsystem: No pool class lookup exists for returned actor %s."), *InActor->GetName());
		return;
	}

	TArray<FPoolItem>* TargetPool = Pool.Find(*ActorClass);
	if (!TargetPool)
	{
		ensureMsgf(
			false,
			TEXT("ObjectPoolSubsystem: No pool exists for class %s while returning actor %s."),
			*(*ActorClass)->GetName(),
			*InActor->GetName());
		return;
	}

	for (FPoolItem& Item : *TargetPool)
	{
		if (Item.ActorInstance == InActor)
		{
			if (!Item.bInUse)
			{
				return;
			}

			Item.bInUse = false;
			DeactivateActor(InActor);
			return;
		}
	}

	ensureMsgf(false, TEXT("ObjectPoolSubsystem: Returned actor is not tracked by its pool."));
}

void UObjectPoolSubsystem::DelayActor(AActor* InActor, float InDelayTime, bool bInAutomaticallyReturnPool)
{
	checkf(IsValid(InActor), TEXT("ObjectPoolSubsystem: DelayActor received an invalid actor."));
	checkf(InDelayTime >= 0.f, TEXT("ObjectPoolSubsystem: Recycle delay must be non-negative."));

	ClearReturnTimer(InActor);
	if (!bInAutomaticallyReturnPool)
	{
		return;
	}

	UWorld* World = GetWorld();
	checkf(World, TEXT("ObjectPoolSubsystem: World must not be null."));

	TWeakObjectPtr<UObjectPoolSubsystem> WeakSubsystem(this);
	TWeakObjectPtr<AActor> WeakActor(InActor);
	FTimerHandle TimerHandle;

	World->GetTimerManager().SetTimer(
		TimerHandle,
		[WeakSubsystem, WeakActor]()
		{
			if (WeakSubsystem.IsValid() && WeakActor.IsValid())
			{
				WeakSubsystem->ReturnActorToPool(WeakActor.Get());
			}
		},
		InDelayTime,
		false);

	ActiveReturnTimers.Add(TWeakObjectPtr<AActor>(InActor), TimerHandle);
}

void UObjectPoolSubsystem::ClearReturnTimer(AActor* InActor)
{
	if (!IsValid(InActor))
	{
		return;
	}

	UWorld* World = GetWorld();
	if (!World)
	{
		return;
	}

	const TWeakObjectPtr<AActor> ActorKey(InActor);
	if (FTimerHandle* TimerHandle = ActiveReturnTimers.Find(ActorKey))
	{
		World->GetTimerManager().ClearTimer(*TimerHandle);
		ActiveReturnTimers.Remove(ActorKey);
	}
}

void UObjectPoolSubsystem::DeactivateActor(AActor* InSpawnedActor, bool bNotifyPoolableActor)
{
	checkf(IsValid(InSpawnedActor), TEXT("ObjectPoolSubsystem: Cannot deactivate an invalid actor."));

	ClearReturnTimer(InSpawnedActor);
	if (bNotifyPoolableActor)
	{
		NotifyActorReturnedToPool(InSpawnedActor);
	}

	InSpawnedActor->SetActorEnableCollision(false);
	InSpawnedActor->SetActorTickEnabled(false);
	InSpawnedActor->SetActorHiddenInGame(true);
	InSpawnedActor->SetActorTransform(HiddenTransform);

	if (APawn* PawnActor = Cast<APawn>(InSpawnedActor))
	{
		if (AController* PawnController = PawnActor->GetController())
		{
			PawnController->UnPossess();
		}
	}
}

void UObjectPoolSubsystem::ActivateActor(
	AActor* InFreeActor,
	const FTransform& InSpawnTransform,
	bool bInShouldAutomaticallyReturnPool,
	float InRecycleDelayTime)
{
	checkf(IsValid(InFreeActor), TEXT("ObjectPoolSubsystem: Cannot activate an invalid actor."));

	ClearReturnTimer(InFreeActor);
	InFreeActor->SetActorTransform(InSpawnTransform);
	InFreeActor->SetActorTickEnabled(true);
	InFreeActor->SetActorHiddenInGame(false);
	InFreeActor->SetActorEnableCollision(true);

	if (APawn* PawnActor = Cast<APawn>(InFreeActor))
	{
		UWorld* World = GetWorld();
		checkf(World, TEXT("ObjectPoolSubsystem: World must not be null."));

		if (PawnActor->AIControllerClass && PawnActor->GetController() == nullptr)
		{
			AAIController* PawnAIController = World->SpawnActor<AAIController>(PawnActor->AIControllerClass);
			if (PawnAIController)
			{
				PawnAIController->Possess(PawnActor);
			}
		}
	}

	NotifyActorTakenFromPool(InFreeActor);
	DelayActor(InFreeActor, InRecycleDelayTime, bInShouldAutomaticallyReturnPool);
}

AActor* UObjectPoolSubsystem::SpawnPooledActor(TSubclassOf<AActor> InActorClass)
{
	checkf(InActorClass, TEXT("ObjectPoolSubsystem: Actor class must not be null."));

	UWorld* World = GetWorld();
	checkf(World, TEXT("ObjectPoolSubsystem: World must not be null."));

	FActorSpawnParameters SpawnParams;
	SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

	AActor* SpawnedActor = World->SpawnActor<AActor>(InActorClass, HiddenTransform, SpawnParams);
	if (SpawnedActor)
	{
		ActorToPoolClassMap.Add(TWeakObjectPtr<AActor>(SpawnedActor), InActorClass);
	}

	return SpawnedActor;
}

void UObjectPoolSubsystem::NotifyActorTakenFromPool(AActor* InActor)
{
	if (IPoolableActor* PoolableActor = Cast<IPoolableActor>(InActor))
	{
		PoolableActor->OnTakenFromPool();
	}
}

void UObjectPoolSubsystem::NotifyActorReturnedToPool(AActor* InActor)
{
	if (IPoolableActor* PoolableActor = Cast<IPoolableActor>(InActor))
	{
		PoolableActor->OnReturnedToPool();
	}
}

void UObjectPoolSubsystem::LogPoolState(
	const TCHAR* Action,
	TSubclassOf<AActor> ActorClass,
	const TArray<FPoolItem>& TargetPool,
	const AActor* ActorInstance) const
{
	const int32 InUseCount = CountInUseActors(TargetPool);
	const FString ActorClassName = GetNameSafe(ActorClass.Get());

	if (IsValid(ActorInstance))
	{
		const FString ActorName = GetNameSafe(ActorInstance);
		UE_LOG(
			LogSimpleObjectPool,
			Log,
			TEXT("%s. Class: %s. Actor: %s. Pool usage: %d in use, %d total."),
			Action,
			*ActorClassName,
			*ActorName,
			InUseCount,
			TargetPool.Num());
		return;
	}

	UE_LOG(
		LogSimpleObjectPool,
		Log,
		TEXT("%s. Class: %s. Pool usage: %d in use, %d total."),
		Action,
		*ActorClassName,
		InUseCount,
		TargetPool.Num());
}

int32 UObjectPoolSubsystem::CountInUseActors(const TArray<FPoolItem>& TargetPool)
{
	int32 InUseCount = 0;
	for (const FPoolItem& Item : TargetPool)
	{
		if (Item.bInUse && IsValid(Item.ActorInstance))
		{
			++InUseCount;
		}
	}

	return InUseCount;
}
