// @Copyright HaolunYuan

#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "Highlightable.generated.h"

// This class does not need to be modified.
UINTERFACE(MinimalAPI)
class UHighlightable : public UInterface
{
	GENERATED_BODY()
};

/**
 * 
 */
class AURA_API IHighlightable
{
	GENERATED_BODY()

public:

	virtual void HighLightActor() = 0;
	virtual void UnhighLightActor() = 0;
};
