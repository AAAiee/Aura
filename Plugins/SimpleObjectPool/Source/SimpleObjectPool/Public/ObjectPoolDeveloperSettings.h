#pragma once

#include "CoreMinimal.h"
#include "Engine/DeveloperSettings.h"
#include "ObjectPoolDeveloperSettings.generated.h"

class UObjectPoolConfigDataAsset;

/**
 * Project-level settings for the SimpleObjectPool plugin.
 * This lets the game point the runtime pool system at a shared config data asset
 * without baking a project-specific asset path into the subsystem itself.
 */
UCLASS(Config = Game, DefaultConfig, meta = (DisplayName = "Simple Object Pool"))
class SIMPLEOBJECTPOOL_API UObjectPoolDeveloperSettings : public UDeveloperSettings
{
	GENERATED_BODY()

public:
	virtual FName GetCategoryName() const override;

public:
	UPROPERTY(Config, EditAnywhere, BlueprintReadOnly, Category = "ObjectPool")
	TSoftObjectPtr<UObjectPoolConfigDataAsset> DefaultPoolConfig;
};
