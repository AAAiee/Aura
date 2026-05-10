#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "PoolableActor.generated.h"

// Reflection shell for actors that want lifecycle callbacks from UObjectPoolSubsystem.
UINTERFACE(BlueprintType)
class SIMPLEOBJECTPOOL_API UPoolableActor : public UInterface
{
	GENERATED_BODY()
};

/**
 * Optional interface for pooled actors that need to reset or reinitialize per-use state.
 */
class SIMPLEOBJECTPOOL_API IPoolableActor
{
	GENERATED_BODY()

public:
	/** Called after the pool activates the actor for gameplay use. */
	virtual void OnTakenFromPool() {}

	/** Called before or while the actor is parked back in the inactive pool. */
	virtual void OnReturnedToPool() {}
};
