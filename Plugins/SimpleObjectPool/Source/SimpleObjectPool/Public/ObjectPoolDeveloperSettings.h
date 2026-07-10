#pragma once

#include "CoreMinimal.h"
#include "Engine/DeveloperSettings.h"
#include "ObjectPoolDeveloperSettings.generated.h"

class UObjectPoolConfigDataAsset;

/**
 * UObjectPoolDeveloperSettings
 *
 * Project-level settings for the SimpleObjectPool plugin.
 *
 * This keeps the default pool config asset path in project settings instead of
 * baking project-specific asset references into UObjectPoolSubsystem.
 */
UCLASS(Config = Game, DefaultConfig, meta = (DisplayName = "Simple Object Pool"))
class SIMPLEOBJECTPOOL_API UObjectPoolDeveloperSettings : public UDeveloperSettings
{
	GENERATED_BODY()

public:
	/* UDeveloperSettings begins */

	virtual FName GetCategoryName() const override;

	/* UDeveloperSettings ends */

	/* Config Asset */
	UPROPERTY(Config, EditAnywhere, BlueprintReadOnly, Category = "ObjectPool")
	TSoftObjectPtr<UObjectPoolConfigDataAsset> DefaultPoolConfig;
};
