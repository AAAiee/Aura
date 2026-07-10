#include "ObjectPoolBenchmarkActor.h"

#include "Engine/World.h"
#include "HAL/PlatformTime.h"
#include "ObjectPoolSubsystem.h"
#include "PoolableActor.h"
#include "ProfilingDebugging/CpuProfilerTrace.h"
#include "SimpleObjectPool.h"
#include "TimerManager.h"

void FObjectPoolBenchmarkStats::Reset()
{
	WavesCompleted = 0;
	ActorsAcquired = 0;
	ActorsReleased = 0;
	ColdInitializeMs = 0.f;
	TotalAcquireMs = 0.f;
	TotalReleaseMs = 0.f;
	MaxAcquireWaveMs = 0.f;
	MaxReleaseWaveMs = 0.f;
}

void FObjectPoolBenchmarkStats::RecordAcquire(float InAcquireMs, int32 InActorCount)
{
	ActorsAcquired += InActorCount;
	TotalAcquireMs += InAcquireMs;
	MaxAcquireWaveMs = FMath::Max(MaxAcquireWaveMs, InAcquireMs);
}

void FObjectPoolBenchmarkStats::RecordRelease(float InReleaseMs, int32 InActorCount)
{
	++WavesCompleted;
	ActorsReleased += InActorCount;
	TotalReleaseMs += InReleaseMs;
	MaxReleaseWaveMs = FMath::Max(MaxReleaseWaveMs, InReleaseMs);
}

float FObjectPoolBenchmarkStats::GetAverageAcquireMsPerActor() const
{
	return ActorsAcquired > 0 ? TotalAcquireMs / ActorsAcquired : 0.f;
}

float FObjectPoolBenchmarkStats::GetAverageReleaseMsPerActor() const
{
	return ActorsReleased > 0 ? TotalReleaseMs / ActorsReleased : 0.f;
}

AObjectPoolBenchmarkActor::AObjectPoolBenchmarkActor()
{
	PrimaryActorTick.bCanEverTick = false;
}

void AObjectPoolBenchmarkActor::BeginPlay()
{
	Super::BeginPlay();

	if (bRunOnBeginPlay)
	{
		StartBenchmark();
	}
}

void AObjectPoolBenchmarkActor::StartBenchmark()
{
	if (!ValidateBenchmark())
	{
		return;
	}

	bBenchmarkRunning = true;
	CurrentWaveIndex = 0;
	ActiveActors.Reset();
	LastRunStats.Reset();

	if (Mode == EObjectPoolBenchmarkMode::ObjectPool && bInitializePoolBeforeObjectPoolRun)
	{
		InitializePoolForRun();
	}

	UE_LOG(
		LogSimpleObjectPool,
		Warning,
		TEXT("Object pool benchmark started. Mode=%s Class=%s ActorsPerWave=%d WaveCount=%d ActiveDuration=%.3fs WaveInterval=%.3fs"),
		*GetModeLabel(),
		*GetNameSafe(ActorClass.Get()),
		ActorsPerWave,
		WaveCount,
		ActiveDuration,
		WaveInterval);

	RunNextWave();
}

void AObjectPoolBenchmarkActor::StopBenchmark()
{
	if (!bBenchmarkRunning)
	{
		return;
	}

	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().ClearTimer(ReleaseWaveTimerHandle);
		World->GetTimerManager().ClearTimer(NextWaveTimerHandle);
	}

	ReleaseOutstandingActorsWithoutMeasurement();
	bBenchmarkRunning = false;

	UE_LOG(LogSimpleObjectPool, Warning, TEXT("Object pool benchmark stopped manually. %s"), *BuildSummaryString());
}

FString AObjectPoolBenchmarkActor::BuildSummaryString() const
{
	return FString::Printf(
		TEXT("Mode=%s Waves=%d/%d ActorsAcquired=%d ActorsReleased=%d ColdInitialize=%.3fms AcquireTotal=%.3fms AcquireAvg=%.6fms ReleaseTotal=%.3fms ReleaseAvg=%.6fms MaxAcquireWave=%.3fms MaxReleaseWave=%.3fms"),
		*GetModeLabel(),
		LastRunStats.WavesCompleted,
		WaveCount,
		LastRunStats.ActorsAcquired,
		LastRunStats.ActorsReleased,
		LastRunStats.ColdInitializeMs,
		LastRunStats.TotalAcquireMs,
		LastRunStats.GetAverageAcquireMsPerActor(),
		LastRunStats.TotalReleaseMs,
		LastRunStats.GetAverageReleaseMsPerActor(),
		LastRunStats.MaxAcquireWaveMs,
		LastRunStats.MaxReleaseWaveMs);
}

void AObjectPoolBenchmarkActor::RunNextWave()
{
	if (!bBenchmarkRunning)
	{
		return;
	}

	if (CurrentWaveIndex >= WaveCount)
	{
		FinishBenchmark();
		return;
	}

	++CurrentWaveIndex;
	ActiveActors.Reset(ActorsPerWave);

	const double StartSeconds = FPlatformTime::Seconds();

	if (Mode == EObjectPoolBenchmarkMode::SpawnDestroy)
	{
		TRACE_CPUPROFILER_EVENT_SCOPE(SimpleObjectPoolBenchmark_SpawnActorWave);

		for (int32 ActorIndex = 0; ActorIndex < ActorsPerWave; ++ActorIndex)
		{
			if (AActor* SpawnedActor = SpawnActorForBenchmark(BuildSpawnTransform(ActorIndex)))
			{
				ActiveActors.Add(SpawnedActor);
			}
		}
	}
	else
	{
		TRACE_CPUPROFILER_EVENT_SCOPE(SimpleObjectPoolBenchmark_BorrowFromPoolWave);

		for (int32 ActorIndex = 0; ActorIndex < ActorsPerWave; ++ActorIndex)
		{
			if (AActor* BorrowedActor = BorrowActorFromPoolForBenchmark(BuildSpawnTransform(ActorIndex)))
			{
				ActiveActors.Add(BorrowedActor);
			}
		}
	}

	const float AcquireMs = static_cast<float>((FPlatformTime::Seconds() - StartSeconds) * 1000.0);
	LastRunStats.RecordAcquire(AcquireMs, ActiveActors.Num());

	if (bLogEachWave)
	{
		UE_LOG(
			LogSimpleObjectPool,
			Warning,
			TEXT("Object pool benchmark wave acquired. Mode=%s Wave=%d/%d Requested=%d Acquired=%d AcquireMs=%.3f"),
			*GetModeLabel(),
			CurrentWaveIndex,
			WaveCount,
			ActorsPerWave,
			ActiveActors.Num(),
			AcquireMs);
	}

	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().SetTimer(
			ReleaseWaveTimerHandle,
			this,
			&AObjectPoolBenchmarkActor::ReleaseCurrentWave,
			ActiveDuration,
			false);
	}
}

void AObjectPoolBenchmarkActor::ReleaseCurrentWave()
{
	if (!bBenchmarkRunning)
	{
		return;
	}

	int32 ReleasedActorCount = 0;
	const double StartSeconds = FPlatformTime::Seconds();

	if (Mode == EObjectPoolBenchmarkMode::SpawnDestroy)
	{
		TRACE_CPUPROFILER_EVENT_SCOPE(SimpleObjectPoolBenchmark_DestroyActorWave);

		for (const TWeakObjectPtr<AActor>& ActiveActor : ActiveActors)
		{
			if (AActor* Actor = ActiveActor.Get())
			{
				if (bMirrorPoolableCallbacksInSpawnDestroyMode)
				{
					if (IPoolableActor* PoolableActor = Cast<IPoolableActor>(Actor))
					{
						PoolableActor->OnReturnedToPool();
					}
				}

				Actor->Destroy();
				++ReleasedActorCount;
			}
		}
	}
	else
	{
		TRACE_CPUPROFILER_EVENT_SCOPE(SimpleObjectPoolBenchmark_ReturnToPoolWave);

		UObjectPoolSubsystem* PoolSubsystem = GetWorld() ? GetWorld()->GetSubsystem<UObjectPoolSubsystem>() : nullptr;
		if (!ensureMsgf(PoolSubsystem, TEXT("ObjectPoolBenchmarkActor: Missing ObjectPoolSubsystem while releasing pooled actors.")))
		{
			return;
		}

		for (const TWeakObjectPtr<AActor>& ActiveActor : ActiveActors)
		{
			if (AActor* Actor = ActiveActor.Get())
			{
				PoolSubsystem->ReturnActorToPool(Actor);
				++ReleasedActorCount;
			}
		}
	}

	const float ReleaseMs = static_cast<float>((FPlatformTime::Seconds() - StartSeconds) * 1000.0);
	LastRunStats.RecordRelease(ReleaseMs, ReleasedActorCount);
	ActiveActors.Reset();

	if (bLogEachWave)
	{
		UE_LOG(
			LogSimpleObjectPool,
			Warning,
			TEXT("Object pool benchmark wave released. Mode=%s Wave=%d/%d Released=%d ReleaseMs=%.3f"),
			*GetModeLabel(),
			CurrentWaveIndex,
			WaveCount,
			ReleasedActorCount,
			ReleaseMs);
	}

	if (CurrentWaveIndex >= WaveCount)
	{
		FinishBenchmark();
		return;
	}

	if (WaveInterval <= 0.f)
	{
		RunNextWave();
		return;
	}

	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().SetTimer(
			NextWaveTimerHandle,
			this,
			&AObjectPoolBenchmarkActor::RunNextWave,
			WaveInterval,
			false);
	}
}

void AObjectPoolBenchmarkActor::FinishBenchmark()
{
	bBenchmarkRunning = false;

	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().ClearTimer(ReleaseWaveTimerHandle);
		World->GetTimerManager().ClearTimer(NextWaveTimerHandle);
	}

	UE_LOG(LogSimpleObjectPool, Warning, TEXT("Object pool benchmark finished. %s"), *BuildSummaryString());
}

void AObjectPoolBenchmarkActor::ReleaseOutstandingActorsWithoutMeasurement()
{
	UObjectPoolSubsystem* PoolSubsystem = GetWorld() ? GetWorld()->GetSubsystem<UObjectPoolSubsystem>() : nullptr;

	for (const TWeakObjectPtr<AActor>& ActiveActor : ActiveActors)
	{
		AActor* Actor = ActiveActor.Get();
		if (!Actor)
		{
			continue;
		}

		if (Mode == EObjectPoolBenchmarkMode::ObjectPool && PoolSubsystem)
		{
			PoolSubsystem->ReturnActorToPool(Actor);
		}
		else
		{
			if (bMirrorPoolableCallbacksInSpawnDestroyMode)
			{
				if (IPoolableActor* PoolableActor = Cast<IPoolableActor>(Actor))
				{
					PoolableActor->OnReturnedToPool();
				}
			}

			Actor->Destroy();
		}
	}

	ActiveActors.Reset();
}

bool AObjectPoolBenchmarkActor::ValidateBenchmark() const
{
	if (bBenchmarkRunning)
	{
		UE_LOG(LogSimpleObjectPool, Warning, TEXT("ObjectPoolBenchmarkActor: benchmark is already running."));
		return false;
	}

	if (!GetWorld())
	{
		UE_LOG(LogSimpleObjectPool, Error, TEXT("ObjectPoolBenchmarkActor: missing World."));
		return false;
	}

	if (!ActorClass)
	{
		UE_LOG(LogSimpleObjectPool, Error, TEXT("ObjectPoolBenchmarkActor: ActorClass must be assigned before running the benchmark."));
		return false;
	}

	if (ActorsPerWave <= 0 || WaveCount <= 0 || ActorsPerRow <= 0 || ActorSpacing <= 0.f)
	{
		UE_LOG(LogSimpleObjectPool, Error, TEXT("ObjectPoolBenchmarkActor: benchmark counts and spacing must be positive."));
		return false;
	}

	if (Mode == EObjectPoolBenchmarkMode::ObjectPool && !GetWorld()->GetSubsystem<UObjectPoolSubsystem>())
	{
		UE_LOG(LogSimpleObjectPool, Error, TEXT("ObjectPoolBenchmarkActor: missing ObjectPoolSubsystem for object-pool benchmark mode."));
		return false;
	}

	return true;
}

void AObjectPoolBenchmarkActor::InitializePoolForRun()
{
	UObjectPoolSubsystem* PoolSubsystem = GetWorld() ? GetWorld()->GetSubsystem<UObjectPoolSubsystem>() : nullptr;
	if (!ensureMsgf(PoolSubsystem, TEXT("ObjectPoolBenchmarkActor: Missing ObjectPoolSubsystem while initializing benchmark pool.")))
	{
		return;
	}

	if (PoolSubsystem->IsPoolInitialized(ActorClass))
	{
		UE_LOG(LogSimpleObjectPool, Warning, TEXT("Object pool benchmark cold setup skipped because pool already exists. Class=%s"), *GetNameSafe(ActorClass.Get()));
		return;
	}

	const double StartSeconds = FPlatformTime::Seconds();
	{
		TRACE_CPUPROFILER_EVENT_SCOPE(SimpleObjectPoolBenchmark_ColdInitializePool);
		PoolSubsystem->InitializePool(ActorClass, InitialPoolSize);
	}

	LastRunStats.ColdInitializeMs = static_cast<float>((FPlatformTime::Seconds() - StartSeconds) * 1000.0);
	UE_LOG(
		LogSimpleObjectPool,
		Warning,
		TEXT("Object pool benchmark cold setup completed. Class=%s InitialPoolSize=%d ColdInitializeMs=%.3f"),
		*GetNameSafe(ActorClass.Get()),
		InitialPoolSize,
		LastRunStats.ColdInitializeMs);
}

FTransform AObjectPoolBenchmarkActor::BuildSpawnTransform(int32 ActorIndex) const
{
	const int32 SafeActorsPerRow = FMath::Max(1, ActorsPerRow);
	const int32 Row = ActorIndex / SafeActorsPerRow;
	const int32 Column = ActorIndex % SafeActorsPerRow;

	const FVector LocalOffset(
		static_cast<float>(Column) * ActorSpacing,
		static_cast<float>(Row) * ActorSpacing,
		0.f);

	return FTransform(GetActorRotation(), GetActorLocation() + SpawnAreaOffset + LocalOffset, FVector::OneVector);
}

AActor* AObjectPoolBenchmarkActor::SpawnActorForBenchmark(const FTransform& SpawnTransform) const
{
	UWorld* World = GetWorld();
	if (!World)
	{
		return nullptr;
	}

	FActorSpawnParameters SpawnParameters;
	SpawnParameters.Owner = const_cast<AObjectPoolBenchmarkActor*>(this);
	SpawnParameters.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

	AActor* SpawnedActor = World->SpawnActor<AActor>(ActorClass, SpawnTransform, SpawnParameters);
	if (bMirrorPoolableCallbacksInSpawnDestroyMode)
	{
		if (IPoolableActor* PoolableActor = Cast<IPoolableActor>(SpawnedActor))
		{
			PoolableActor->OnTakenFromPool();
		}
	}

	return SpawnedActor;
}

AActor* AObjectPoolBenchmarkActor::BorrowActorFromPoolForBenchmark(const FTransform& SpawnTransform) const
{
	UObjectPoolSubsystem* PoolSubsystem = GetWorld() ? GetWorld()->GetSubsystem<UObjectPoolSubsystem>() : nullptr;
	if (!PoolSubsystem)
	{
		return nullptr;
	}

	return PoolSubsystem->GetPooledActorWithRecyclePolicy(ActorClass, SpawnTransform, false, 0.f);
}

FString AObjectPoolBenchmarkActor::GetModeLabel() const
{
	switch (Mode)
	{
	case EObjectPoolBenchmarkMode::SpawnDestroy:
		return TEXT("SpawnDestroy");
	case EObjectPoolBenchmarkMode::ObjectPool:
		return TEXT("ObjectPool");
	default:
		return TEXT("Unknown");
	}
}
