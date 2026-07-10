#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "ObjectPoolBenchmarkActor.generated.h"

/**
 * Benchmark mode used by AObjectPoolBenchmarkActor.
 */
UENUM(BlueprintType)
enum class EObjectPoolBenchmarkMode : uint8
{
	SpawnDestroy UMETA(DisplayName = "Spawn / Destroy"),
	ObjectPool UMETA(DisplayName = "Object Pool")
};

/**
 * Aggregate timing results for one benchmark run.
 */
USTRUCT(BlueprintType)
struct SIMPLEOBJECTPOOL_API FObjectPoolBenchmarkStats
{
	GENERATED_BODY()

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "ObjectPool|Benchmark")
	int32 WavesCompleted = 0;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "ObjectPool|Benchmark")
	int32 ActorsAcquired = 0;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "ObjectPool|Benchmark")
	int32 ActorsReleased = 0;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "ObjectPool|Benchmark")
	float ColdInitializeMs = 0.f;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "ObjectPool|Benchmark")
	float TotalAcquireMs = 0.f;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "ObjectPool|Benchmark")
	float TotalReleaseMs = 0.f;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "ObjectPool|Benchmark")
	float MaxAcquireWaveMs = 0.f;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "ObjectPool|Benchmark")
	float MaxReleaseWaveMs = 0.f;

	void Reset();
	void RecordAcquire(float InAcquireMs, int32 InActorCount);
	void RecordRelease(float InReleaseMs, int32 InActorCount);

	float GetAverageAcquireMsPerActor() const;
	float GetAverageReleaseMsPerActor() const;
};

/**
 * Placeable A/B benchmark actor for comparing SpawnActor/DestroyActor against SimpleObjectPool reuse.
 *
 * Put this actor in an empty test map, assign the actor class to benchmark, choose a mode, and run the
 * same wave settings once per mode. The log output gives quick numbers, while the CPU profiler markers
 * show up in Unreal Insights for frame-time and hitch analysis.
 */
UCLASS(Blueprintable)
class SIMPLEOBJECTPOOL_API AObjectPoolBenchmarkActor : public AActor
{
	GENERATED_BODY()

public:
	AObjectPoolBenchmarkActor();

	virtual void BeginPlay() override;

	UFUNCTION(BlueprintCallable, Category = "ObjectPool|Benchmark")
	void StartBenchmark();

	UFUNCTION(BlueprintCallable, Category = "ObjectPool|Benchmark")
	void StopBenchmark();

	UFUNCTION(BlueprintCallable, Category = "ObjectPool|Benchmark")
	FString BuildSummaryString() const;

protected:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "ObjectPool|Benchmark")
	TSubclassOf<AActor> ActorClass;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "ObjectPool|Benchmark")
	EObjectPoolBenchmarkMode Mode = EObjectPoolBenchmarkMode::ObjectPool;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "ObjectPool|Benchmark")
	bool bRunOnBeginPlay = false;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "ObjectPool|Benchmark", meta = (ClampMin = "1"))
	int32 ActorsPerWave = 500;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "ObjectPool|Benchmark", meta = (ClampMin = "1"))
	int32 WaveCount = 5;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "ObjectPool|Benchmark", meta = (ClampMin = "0.0"))
	float ActiveDuration = 0.25f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "ObjectPool|Benchmark", meta = (ClampMin = "0.0"))
	float WaveInterval = 1.f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "ObjectPool|Benchmark", meta = (EditCondition = "Mode == EObjectPoolBenchmarkMode::ObjectPool"))
	bool bInitializePoolBeforeObjectPoolRun = true;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "ObjectPool|Benchmark", meta = (ClampMin = "1", EditCondition = "bInitializePoolBeforeObjectPoolRun"))
	int32 InitialPoolSize = 500;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "ObjectPool|Benchmark", meta = (EditCondition = "Mode == EObjectPoolBenchmarkMode::SpawnDestroy"))
	bool bMirrorPoolableCallbacksInSpawnDestroyMode = true;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "ObjectPool|Benchmark|Layout", meta = (ClampMin = "1"))
	int32 ActorsPerRow = 50;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "ObjectPool|Benchmark|Layout", meta = (ClampMin = "1.0"))
	float ActorSpacing = 100.f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "ObjectPool|Benchmark|Layout")
	FVector SpawnAreaOffset = FVector::ZeroVector;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "ObjectPool|Benchmark|Logging")
	bool bLogEachWave = true;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "ObjectPool|Benchmark")
	FObjectPoolBenchmarkStats LastRunStats;

private:
	void RunNextWave();
	void ReleaseCurrentWave();
	void FinishBenchmark();
	void ReleaseOutstandingActorsWithoutMeasurement();
	bool ValidateBenchmark() const;
	void InitializePoolForRun();

	FTransform BuildSpawnTransform(int32 ActorIndex) const;
	AActor* SpawnActorForBenchmark(const FTransform& SpawnTransform) const;
	AActor* BorrowActorFromPoolForBenchmark(const FTransform& SpawnTransform) const;

	FString GetModeLabel() const;

private:
	FTimerHandle ReleaseWaveTimerHandle;
	FTimerHandle NextWaveTimerHandle;

	TArray<TWeakObjectPtr<AActor>> ActiveActors;

	int32 CurrentWaveIndex = 0;
	bool bBenchmarkRunning = false;
};
