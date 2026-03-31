#include "ObjectPoolSubsystem.h"

#include "AIController.h"
#include "Engine/World.h"
#include "GameFramework/Controller.h"
#include "GameFramework/Pawn.h"
#include "ObjectPoolConfigDataAsset.h"
#include "ObjectPoolDeveloperSettings.h"
#include "PoolableActor.h"
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
		UE_LOG(LogTemp, Verbose, TEXT("ObjectPoolSubsystem:: Pool for %s already initialized."), *InActorClass->GetName());
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

#if WITH_EDITOR
	UE_LOG(LogTemp, Log, TEXT("ObjectPoolSubsystem:: Initialized pool for %s with %d actors."), *InActorClass->GetName(), InInitialSize);
#endif
}

void UObjectPoolSubsystem::InitializePoolFromConfig(TSubclassOf<AActor> InActorClass)
{
	checkf(InActorClass, TEXT("ObjectPoolSubsystem:: ActorClass is null"));

	if (IsPoolInitialized(InActorClass))
	{
		return;
	}

	const UObjectPoolConfigDataAsset* PoolConfig = LoadPoolConfigIfNeeded();
	if (!PoolConfig)
	{
		UE_LOG(LogTemp, Warning, TEXT("ObjectPoolSubsystem:: No pool config asset is assigned in developer settings. Falling back to manual initialization for %s."), *InActorClass->GetName());
		return;
	}

	FPoolClassConfig PoolClassConfig;
	if (!PoolConfig->FindPoolConfigByClass(InActorClass, PoolClassConfig))
	{
		UE_LOG(LogTemp, Warning, TEXT("ObjectPoolSubsystem:: No config entry found for %s in the assigned pool config asset."), *InActorClass->GetName());
		return;
	}

	InitializePool(InActorClass, PoolClassConfig.InitialPoolSize);
}

bool UObjectPoolSubsystem::EnsurePoolInitializedFromConfig(TSubclassOf<AActor> InActorClass)
{
	checkf(InActorClass, TEXT("ObjectPoolSubsystem:: ActorClass is null"));

	if (!IsPoolInitialized(InActorClass))
	{
		InitializePoolFromConfig(InActorClass);
	}

	return IsPoolInitialized(InActorClass);
}

FPoolRecyclePolicy UObjectPoolSubsystem::GetRecyclePolicyForClass(TSubclassOf<AActor> InActorClass) const
{
	FPoolRecyclePolicy RecyclePolicy;

	checkf(InActorClass, TEXT("ObjectPoolSubsystem:: ActorClass is null"));

	const UObjectPoolConfigDataAsset* PoolConfig = LoadPoolConfigIfNeeded();
	if (!PoolConfig)
	{
		UE_LOG(LogTemp, Warning, TEXT("ObjectPoolSubsystem:: No pool config asset is assigned in developer settings when resolving recycle policy for %s."), *InActorClass->GetName());
		return RecyclePolicy;
	}

	FPoolClassConfig PoolClassConfig;
	if (!PoolConfig->FindPoolConfigByClass(InActorClass, PoolClassConfig))
	{
		UE_LOG(LogTemp, Warning, TEXT("ObjectPoolSubsystem:: No config entry found for %s when resolving recycle policy."), *InActorClass->GetName());
		return RecyclePolicy;
	}

	if (PoolClassConfig.bUseDefaultRecycleDelay)
	{
		RecyclePolicy.bShouldAutomaticallyReturn = true;
		RecyclePolicy.RecycleDelay = PoolClassConfig.DefaultRecycleDelay;
	}

	return RecyclePolicy;
}

AActor* UObjectPoolSubsystem::GetPooledActor(TSubclassOf<AActor> InActorClass, const FTransform& InSpawnTransform, bool bInShouldAutomaticallyReturnPool, float InRecycleDelayTime)
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

#if WITH_EDITOR
			UE_LOG(LogTemp, Verbose, TEXT("ObjectPoolSubsystem:: Reused actor: %s"), *FreeActor->GetName());
#endif

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

#if WITH_EDITOR
	UE_LOG(LogTemp, Log, TEXT("ObjectPoolSubsystem:: Expanded pool for %s by %d actors."), *InActorClass->GetName(), NumToSpawn);
#endif

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
#if WITH_EDITOR
		UE_LOG(LogTemp, Warning, TEXT("ObjectPoolSubsystem:: No pool lookup found when returning actor %s."), *InActor->GetName());
#endif
		return;
	}

	TArray<FPoolItem>* TargetPool = Pool.Find(*ActorClass);
	if (!TargetPool)
	{
#if WITH_EDITOR
		UE_LOG(LogTemp, Warning, TEXT("ObjectPoolSubsystem:: No pool found for class %s when returning actor %s."), *(*ActorClass)->GetName(), *InActor->GetName());
#endif
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
#if WITH_EDITOR
			UE_LOG(LogTemp, Verbose, TEXT("ObjectPoolSubsystem:: Returned actor %s to pool."), *InActor->GetName());
#endif
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
