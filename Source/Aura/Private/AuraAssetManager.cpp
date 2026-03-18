// @Copyright HaolunYuan


#include "AuraAssetManager.h"
#include "AuraGameTagManager.h"
#include "Engine/Engine.h"
#include "Components/AbilitySystem/Data/AttributeDataAsset.h"

UAuraAssetManager& UAuraAssetManager::Get()
{
	check(GEngine); 
	UAuraAssetManager* Instance = Cast<UAuraAssetManager>(GEngine->AssetManager);

	return *Instance;
}

void UAuraAssetManager::StartInitialLoading()
{
	FAuraGameTagManager::InitializeAllNativeTags();


}
