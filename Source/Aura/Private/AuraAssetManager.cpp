// @Copyright HaolunYuan


#include "AuraAssetManager.h"
#include "AuraGameTag.h"

UAuraAssetManager& UAuraAssetManager::Get()
{
	check(GEngine); 
	UAuraAssetManager* Instance = Cast<UAuraAssetManager>(GEngine->AssetManager);

	return *Instance;
}

void UAuraAssetManager::StartInitialLoading()
{
	FAuraGameTag::InitializeAllNativeTags();
}
