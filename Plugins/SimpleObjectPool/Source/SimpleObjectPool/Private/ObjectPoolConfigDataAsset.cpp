#include "ObjectPoolConfigDataAsset.h"

bool UObjectPoolConfigDataAsset::FindPoolConfigByClass(TSubclassOf<AActor> ActorClass, FPoolClassConfig& OutConfig) const
{
	if (!ActorClass)
	{
		return false;
	}

	for (const FPoolClassConfig& PoolConfig : PoolClassConfigs)
	{
		if (PoolConfig.ActorClass == ActorClass)
		{
			OutConfig = PoolConfig;
			return true;
		}
	}

	return false;
}
