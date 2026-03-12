// @Copyright HaolunYuan

#pragma once

#include "CoreMinimal.h"
#include "Engine/AssetManager.h"
#include "AuraAssetManager.generated.h"

/**
 * Custom Asset Manager for the Aura project.
 *
 * Overrides the engine's default UAssetManager so we can hook into the
 * earliest loading phase and register native GameplayTags before any
 * asset or GameplayEffect references them.
 *
 * Setup:
 *   - Set this class as the AssetManagerClassName in DefaultEngine.ini
 *     so the engine instantiates UAuraAssetManager instead of the base class.
 *   - StartInitialLoading() is called automatically by the engine during
 *     startup and forwards to FAuraGameTag::InitializeAllNativeTags().
 *
 * Get() provides a static accessor that casts the engine-level singleton
 * to this type for convenient access throughout C++ code.
 */
UCLASS()
class AURA_API UAuraAssetManager : public UAssetManager
{
	GENERATED_BODY()

public:
	/*Returns the project-level AssetManager singleton, cast from GEngine->AssetManager*/
	static UAuraAssetManager& Get();

protected:
	/*Called by the engine during startup ¡ª registers all native GameplayTags via FAuraGameTag*/
	virtual void StartInitialLoading() override;

	
};
