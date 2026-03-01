// @Copyright HaolunYuan

#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "Highlightable.generated.h"

/**
 * UInterface boilerplate ¡ª UHighlightable is the UObject half required by Unreal's
 * interface system. You never modify this class directly.
 * MinimalAPI: no DLL export needed since the interface is header-only.
 */
UINTERFACE(MinimalAPI)
class UHighlightable : public UInterface
{
	GENERATED_BODY()
};

/**
 * Interface for actors that can be highlighted by the player's cursor.
 *
 * How it works:
 *   1. AuraPlayerController performs a line trace under the cursor every tick (CursorTrace).
 *   2. If the hit actor Implements<UHighlightable>(), the controller calls HighLightActor().
 *   3. When the cursor leaves, UnhighLightActor() is called.
 *
 * Implementors (e.g., AAuraEnemy) toggle Custom Depth rendering to trigger
 * a post-process outline effect. Both methods are pure virtual ¡ª every implementor
 * must provide its own highlight/unhighlight logic.
 */
class AURA_API IHighlightable
{
	GENERATED_BODY()

public:
	/** Enable visual highlight (e.g., Custom Depth outline). Called when the cursor hovers over this actor. */
	virtual void HighLightActor() = 0;

	/** Disable visual highlight. Called when the cursor leaves this actor. */
	virtual void UnhighLightActor() = 0;
};
