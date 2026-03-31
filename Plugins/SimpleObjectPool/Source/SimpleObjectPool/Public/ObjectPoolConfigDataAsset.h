#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "ObjectPoolConfigDataAsset.generated.h"

/**
 * Configuration entry for a pooled actor class.
 * This is intentionally lightweight so projects can extend or replace the
 * lookup policy later without changing the core pool subsystem API.
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
 * Data asset that stores pooled actor class configuration.
 * Projects are expected to create asset instances in the editor and decide how
 * the runtime pool subsystem should discover and use them.
 */
UCLASS(BlueprintType)
class SIMPLEOBJECTPOOL_API UObjectPoolConfigDataAsset : public UDataAsset
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable, Category = "ObjectPool")
	bool FindPoolConfigByClass(TSubclassOf<AActor> ActorClass, FPoolClassConfig& OutConfig) const;

public:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "ObjectPool")
	TArray<FPoolClassConfig> PoolClassConfigs;
};
