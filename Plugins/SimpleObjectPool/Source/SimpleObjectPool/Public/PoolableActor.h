#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "PoolableActor.generated.h"

UINTERFACE(BlueprintType)
class SIMPLEOBJECTPOOL_API UPoolableActor : public UInterface
{
	GENERATED_BODY()
};

class SIMPLEOBJECTPOOL_API IPoolableActor
{
	GENERATED_BODY()

public:
	virtual void OnTakenFromPool() {}
	virtual void OnReturnedToPool() {}
};
