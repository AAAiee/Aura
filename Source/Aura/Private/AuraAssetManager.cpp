// @Copyright HaolunYuan


#include "AuraAssetManager.h"
#include "AuraGameTagManager.h"
#include "Engine/Engine.h"
#include "Components/AbilitySystem/Data/AttributeDataAsset.h"
#include "AbilitySystemGlobals.h"

UAuraAssetManager& UAuraAssetManager::Get()
{
	check(GEngine); 
	UAuraAssetManager* Instance = Cast<UAuraAssetManager>(GEngine->AssetManager);

	return *Instance;
}

void UAuraAssetManager::StartInitialLoading()
{
	FAuraGameTagManager::InitializeAllNativeTags();

	//required for target data 
	UAbilitySystemGlobals::Get().InitGlobalData();
}
