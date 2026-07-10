#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "PoolableActor.generated.h"

/**
 * UPoolableActor
 *
 * Reflection shell for actors that want lifecycle callbacks from UObjectPoolSubsystem.
 */
UINTERFACE(BlueprintType)
class SIMPLEOBJECTPOOL_API UPoolableActor : public UInterface
{
	GENERATED_BODY()
};

/**
 * IPoolableActor
 *
 * Optional interface for actors that need pool lifecycle callbacks.
 *
 * Actors implement this when they need to reset transient gameplay state on
 * borrow or clean up active behavior before returning to the inactive pool.
 */
class SIMPLEOBJECTPOOL_API IPoolableActor
{
	GENERATED_BODY()

public:
	/** Called after the pool activates the actor for gameplay use. */
	virtual void OnTakenFromPool();

	/** Called before or while the actor is parked back in the inactive pool. */
	virtual void OnReturnedToPool();
};
