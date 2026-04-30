// @Copyright HaolunYuan

#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "Summonable.generated.h"

// Reflection shell for actors that expose summon placement data.
UINTERFACE(MinimalAPI, BlueprintType, NotBlueprintable)
class USummonable : public UInterface
{
	GENERATED_BODY()
};

/**
 * Implemented by actors that need to adjust where summon effects place them vertically.
 */
class AURA_API ISummonable
{
	GENERATED_BODY()

public:
	// Returns the vertical offset that should be applied when this actor is placed as a summon.
	UFUNCTION(BlueprintCallable)
	virtual float GetZOffset() const;
};
