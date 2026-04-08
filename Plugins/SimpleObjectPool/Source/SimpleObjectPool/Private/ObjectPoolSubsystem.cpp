#include "ObjectPoolSubsystem.h"

#include "AIController.h"
#include "Engine/World.h"
#include "GameFramework/Controller.h"
#include "GameFramework/Pawn.h"
#include "ObjectPoolConfigDataAsset.h"
#include "ObjectPoolDeveloperSettings.h"
#include "PoolableActor.h"
#include "TimerManager.h"

DEFINE_LOG_CATEGORY_STATIC(LogObjectPoolSubsystem, Log, All);

UObjectPoolSubsystem::UObjectPoolSubsystem()
	: HiddenTransform(FTransform(
		FRotator::ZeroRotator,
		FVector(0.f, 0.f, -50000.f),
		FVector::OneVector))
{
}

void UObjectPoolSubsystem::Deinitialize()
{
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

void UObjectPoolSubsystem::InitializePool(TSubclassOf<AActor> InActorClass, int32 InInitialSize)
{
	checkf(InInitialSize > 0, TEXT("ObjectPoolSubsystem:: InitialSize must be greater than zero"));
	checkf(InActorClass, TEXT("ObjectPoolSubsystem:: ActorClass is null"));

	if (Pool.Contains(InActorClass))
	{
		UE_LOG(LogObjectPoolSubsystem, Verbose, TEXT("Pool for %s is already initialized."), *InActorClass->GetName());
		return;
	}

	TArray<FPoolItem> PoolItems;
	PoolItems.Reserve(InInitialSize);
	for (int32 Index = 0; Index < InInitialSize; ++Index)
	{
		AActor* SpawnedActor = SpawnPooledActor(InActorClass);
		checkf(SpawnedActor, TEXT("ObjectPoolSubsystem:: SpawnActor failed during pool initialization for class %s"), *InActorClass->GetName());

		DeactivateActor(SpawnedActor, false);
		PoolItems.Add(FPoolItem{ SpawnedActor, false });
	}

	Pool.Add(InActorClass, MoveTemp(PoolItems));
	const TArray<FPoolItem>* InitializedPool = Pool.Find(InActorClass);
	checkf(InitializedPool, TEXT("ObjectPoolSubsystem:: Pool registration failed for class %s."), *InActorClass->GetName());
	LogPoolState(TEXT("Initialized pool"), InActorClass, *InitializedPool);
}

bool UObjectPoolSubsystem::InitializePoolFromConfig(TSubclassOf<AActor> InActorClass)
{
	checkf(InActorClass, TEXT("ObjectPoolSubsystem:: ActorClass is null"));

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
	checkf(InActorClass, TEXT("ObjectPoolSubsystem:: ActorClass is null"));

	if (!IsPoolInitialized(InActorClass))
	{
		return InitializePoolFromConfig(InActorClass);
	}

	return true;
}

FPoolRecyclePolicy UObjectPoolSubsystem::GetRecyclePolicyForClass(TSubclassOf<AActor> InActorClass) const
{
	checkf(InActorClass, TEXT("ObjectPoolSubsystem:: ActorClass is null"));

	FPoolClassConfig PoolClassConfig;
	if (!TryGetPoolClassConfig(InActorClass, PoolClassConfig))
	{
		return FPoolRecyclePolicy{};
	}

	return BuildRecyclePolicy(PoolClassConfig);
}

bool UObjectPoolSubsystem::TryGetPoolClassConfig(TSubclassOf<AActor> InActorClass, FPoolClassConfig& OutConfig) const
{
	checkf(InActorClass, TEXT("ObjectPoolSubsystem:: ActorClass is null"));

	const UObjectPoolConfigDataAsset* PoolConfig = LoadPoolConfigIfNeeded();
	if (!ensureMsgf(PoolConfig, TEXT("ObjectPoolSubsystem:: No pool config asset is assigned in developer settings when resolving config for %s."), *InActorClass->GetName()))
	{
		return false;
	}

	const bool bFoundConfig = PoolConfig->FindPoolConfigByClass(InActorClass, OutConfig);
	ensureMsgf(bFoundConfig, TEXT("ObjectPoolSubsystem:: No config entry found for %s in the assigned pool config asset."), *InActorClass->GetName());
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
	checkf(InActorClass, TEXT("ObjectPoolSubsystem:: ActorClass is null"));

	// Preserve the original "manual pool + optional config" behavior:
	//   - If a pool already exists, borrow from it even when there is no config entry.
	//   - If no pool exists yet, fall back to config-driven initialization.
	FPoolRecyclePolicy RecyclePolicy;
	if (!IsPoolInitialized(InActorClass))
	{
		FPoolClassConfig PoolClassConfig;
		checkf(TryGetPoolClassConfig(InActorClass, PoolClassConfig), TEXT("ObjectPoolSubsystem:: Missing pool config for class %s."), *InActorClass->GetName());
		InitializePool(InActorClass, PoolClassConfig.InitialPoolSize);
		RecyclePolicy = BuildRecyclePolicy(PoolClassConfig);
	}
	else
	{
		RecyclePolicy = GetRecyclePolicyForClass(InActorClass);
	}

	return BorrowPooledActor(InActorClass, InSpawnTransform, RecyclePolicy.bShouldAutomaticallyReturn, RecyclePolicy.RecycleDelay);
}

AActor* UObjectPoolSubsystem::GetPooledActorWithRecyclePolicy(TSubclassOf<AActor> InActorClass, const FTransform& InSpawnTransform, bool bInShouldAutomaticallyReturnPool, float InRecycleDelayTime)
{
	checkf(InActorClass, TEXT("ObjectPoolSubsystem:: ActorClass is null"));
	checkf(IsPoolInitialized(InActorClass) || EnsurePoolInitializedFromConfig(InActorClass), TEXT("ObjectPoolSubsystem:: No pool found for class %s and config-based initialization failed."), *InActorClass->GetName());

	return BorrowPooledActor(InActorClass, InSpawnTransform, bInShouldAutomaticallyReturnPool, InRecycleDelayTime);
}

AActor* UObjectPoolSubsystem::BorrowPooledActor(TSubclassOf<AActor> InActorClass, const FTransform& InSpawnTransform, bool bInShouldAutomaticallyReturnPool, float InRecycleDelayTime)
{
	checkf(InActorClass, TEXT("ObjectPoolSubsystem:: ActorClass is null"));

	TArray<FPoolItem>* TargetPool = Pool.Find(InActorClass);
	checkf(TargetPool, TEXT("ObjectPoolSubsystem:: No pool found for class %s. Did you forget to initialize it?"), *InActorClass->GetName());

	for (FPoolItem& Item : *TargetPool)
	{
		if (!Item.bInUse && IsValid(Item.ActorInstance))
		{
			Item.bInUse = true;
			AActor* FreeActor = Item.ActorInstance.Get();
			ActivateActor(FreeActor, InSpawnTransform, bInShouldAutomaticallyReturnPool, InRecycleDelayTime);
			LogPoolState(TEXT("Borrowed actor"), InActorClass, *TargetPool, FreeActor);
			return FreeActor;
		}
	}

	static constexpr float GrowthFactor = 0.5f;
	const int32 CurrentCount = TargetPool->Num();
	const int32 NumToSpawn = FMath::Max(1, FMath::CeilToInt(CurrentCount * GrowthFactor));

	AActor* SpawnedActorToReturn = nullptr;
	for (int32 Index = 0; Index < NumToSpawn; ++Index)
	{
		AActor* CurrentSpawnedActor = SpawnPooledActor(InActorClass);
		ensureMsgf(CurrentSpawnedActor, TEXT("ObjectPoolSubsystem:: SpawnActor failed during pool expansion for class %s"), *InActorClass->GetName());
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

	checkf(SpawnedActorToReturn, TEXT("ObjectPoolSubsystem:: Failed to expand pool for class %s."), *InActorClass->GetName());

	if (SpawnedActorToReturn)
	{
		LogPoolState(TEXT("Expanded pool and borrowed actor"), InActorClass, *TargetPool, SpawnedActorToReturn);
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
		ensureMsgf(false, TEXT("ObjectPoolSubsystem:: No pool lookup found when returning actor %s."), *InActor->GetName());
		return;
	}

	TArray<FPoolItem>* TargetPool = Pool.Find(*ActorClass);
	if (!TargetPool)
	{
		ensureMsgf(false, TEXT("ObjectPoolSubsystem:: No pool found for class %s when returning actor %s."), *(*ActorClass)->GetName(), *InActor->GetName());
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
			LogPoolState(TEXT("Returned actor"), *ActorClass, *TargetPool, InActor);
			return;
		}
	}

	ensureMsgf(false, TEXT("ObjectPoolSubsystem:: The actor you returned is not tracked in its pool."));
}

void UObjectPoolSubsystem::DelayActor(AActor* InActor, float InDelayTime, bool bInAutomaticallyReturnPool)
{
	checkf(IsValid(InActor), TEXT("ObjectPoolSubsystem:: DelayActor received a null actor"));
	checkf(InDelayTime >= 0.f, TEXT("ObjectPoolSubsystem:: DelayTime must be non-negative"));

	ClearReturnTimer(InActor);
	if (!bInAutomaticallyReturnPool)
	{
		return;
	}

	UWorld* World = GetWorld();
	checkf(World, TEXT("ObjectPoolSubsystem:: World is null"));

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

void UObjectPoolSubsystem::DeactivateActor(AActor* SpawnedActor, bool bNotifyPoolableActor)
{
	checkf(IsValid(SpawnedActor), TEXT("ObjectPoolSubsystem:: Cannot deactivate an invalid actor."));

	ClearReturnTimer(SpawnedActor);
	if (bNotifyPoolableActor)
	{
		NotifyActorReturnedToPool(SpawnedActor);
	}

	SpawnedActor->SetActorEnableCollision(false);
	SpawnedActor->SetActorTickEnabled(false);
	SpawnedActor->SetActorHiddenInGame(true);
	SpawnedActor->SetActorTransform(HiddenTransform);

	if (APawn* PawnActor = Cast<APawn>(SpawnedActor))
	{
		if (AController* PawnController = PawnActor->GetController())
		{
			PawnController->UnPossess();
		}
	}
}

void UObjectPoolSubsystem::ActivateActor(AActor* FreeActor, const FTransform& SpawnTransform, bool bShouldAutomaticallyReturnPool, float RecycleDelayTime)
{
	checkf(IsValid(FreeActor), TEXT("ObjectPoolSubsystem:: Cannot activate an invalid actor."));

	ClearReturnTimer(FreeActor);
	FreeActor->SetActorTransform(SpawnTransform);
	FreeActor->SetActorTickEnabled(true);
	FreeActor->SetActorHiddenInGame(false);
	FreeActor->SetActorEnableCollision(true);

	if (APawn* PawnActor = Cast<APawn>(FreeActor))
	{
		UWorld* World = GetWorld();
		checkf(World, TEXT("ObjectPoolSubsystem:: World is null"));

		if (PawnActor->AIControllerClass && PawnActor->GetController() == nullptr)
		{
			AAIController* PawnAIController = World->SpawnActor<AAIController>(PawnActor->AIControllerClass);
			if (PawnAIController)
			{
				PawnAIController->Possess(PawnActor);
			}
		}
	}

	NotifyActorTakenFromPool(FreeActor);
	DelayActor(FreeActor, RecycleDelayTime, bShouldAutomaticallyReturnPool);
}

AActor* UObjectPoolSubsystem::SpawnPooledActor(TSubclassOf<AActor> InActorClass)
{
	checkf(InActorClass, TEXT("ObjectPoolSubsystem:: ActorClass is null"));

	UWorld* World = GetWorld();
	checkf(World, TEXT("ObjectPoolSubsystem:: World is null"));

	FActorSpawnParameters SpawnParams;
	SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

	AActor* SpawnedActor = World->SpawnActor<AActor>(InActorClass, HiddenTransform, SpawnParams);
	if (SpawnedActor)
	{
		ActorToPoolClassMap.Add(TWeakObjectPtr<AActor>(SpawnedActor), InActorClass);
	}

	return SpawnedActor;
}

void UObjectPoolSubsystem::NotifyActorTakenFromPool(AActor* Actor)
{
	if (IPoolableActor* PoolableActor = Cast<IPoolableActor>(Actor))
	{
		PoolableActor->OnTakenFromPool();
	}
}

void UObjectPoolSubsystem::NotifyActorReturnedToPool(AActor* Actor)
{
	if (IPoolableActor* PoolableActor = Cast<IPoolableActor>(Actor))
	{
		PoolableActor->OnReturnedToPool();
	}
}

void UObjectPoolSubsystem::LogPoolState(const TCHAR* Action, TSubclassOf<AActor> ActorClass, const TArray<FPoolItem>& TargetPool, const AActor* ActorInstance) const
{
	const int32 InUseCount = CountInUseActors(TargetPool);
	const TCHAR* ActorName = ActorInstance ? *ActorInstance->GetName() : TEXT("<none>");

	UE_LOG(
		LogObjectPoolSubsystem,
		Log,
		TEXT("%s: class=%s actor=%s pool_state=%d_in_use/%d_total"),
		Action,
		*ActorClass->GetName(),
		ActorName,
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
