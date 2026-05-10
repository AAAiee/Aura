// @Copyright HaolunYuan


#include "AuraAssetManager.h"

#include "AbilitySystemGlobals.h"
#include "AuraGameTagManager.h"
#include "Engine/Engine.h"

UAuraAssetManager& UAuraAssetManager::Get()
{
	check(GEngine);
	UAuraAssetManager* Instance = Cast<UAuraAssetManager>(GEngine->AssetManager);

	return *Instance;
}

void UAuraAssetManager::StartInitialLoading()
{
	FAuraGameTagManager::InitializeAllNativeTags();

	// Required once for GAS target data support before abilities begin producing target handles.
	UAbilitySystemGlobals::Get().InitGlobalData();
}
