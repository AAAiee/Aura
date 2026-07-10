#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "ObjectPoolConfigDataAsset.generated.h"

/**
 * FPoolClassConfig
 *
 * Authored configuration for one actor class managed by the object pool.
 *
 * This struct keeps pool sizing and default recycle behavior in data so
 * gameplay code can request pooled actors without hardcoding spawn counts.
 */
USTRUCT(BlueprintType)
struct SIMPLEOBJECTPOOL_API FPoolClassConfig
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "ObjectPool")
	TSubclassOf<AActor> ActorClass = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "ObjectPool", meta = (ClampMin = "1"))
	int32 InitialPoolSize = 1;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "ObjectPool")
	bool bUseDefaultRecycleDelay = false;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "ObjectPool", meta = (ClampMin = "0.0", EditCondition = "bUseDefaultRecycleDelay", EditConditionHides))
	float DefaultRecycleDelay = 0.f;
};

/**
 * UObjectPoolConfigDataAsset
 *
 * Stores the actor-class configuration consumed by UObjectPoolSubsystem.
 *
 * Projects create this asset in the editor, then point developer settings at it
 * so each world subsystem can resolve pool sizes and recycle behavior at runtime.
 *
 * Important functions:
 *   - FindPoolConfigByClass() - Finds the authored config for a specific actor class.
 */
UCLASS(BlueprintType)
class SIMPLEOBJECTPOOL_API UObjectPoolConfigDataAsset : public UDataAsset
{
	GENERATED_BODY()

public:
	/** Finds the config row for ActorClass and writes it to OutConfig when present. */
	UFUNCTION(BlueprintCallable, Category = "ObjectPool")
	bool FindPoolConfigByClass(TSubclassOf<AActor> ActorClass, FPoolClassConfig& OutConfig) const;

	/* Authored Pool Data */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "ObjectPool")
	TArray<FPoolClassConfig> PoolClassConfigs;
};
