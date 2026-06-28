// @Copyright HaolunYuan

#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "PickupInterface.generated.h"

/**
 * Reflection shell for actors that can be collected by a pickup target.
 */
UINTERFACE(MinimalAPI)
class UPickupInterface : public UInterface
{
	GENERATED_BODY()
};

/**
 * Interface for actors that expose pickup behavior to characters, controllers, and interaction traces.
 */
class AURA_API IPickupInterface
{
	GENERATED_BODY()

public:
	/** Applies this pickup to the actor that collected it. */
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Pickup")
	void Pickup(AActor* PickupTarget);
};
