// @Copyright HaolunYuan

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "MagicCircle.generated.h"

class UDecalComponent;

/**
 * AMagicCircle
 *
 * Lightweight actor that owns the projected magic-circle decal used by targeting and spell placement UI.
 *
 * The actor gives gameplay abilities and Blueprints a world-space object for displaying a spell area,
 * while keeping the decal component and its lifetime in one place. It currently provides construction,
 * BeginPlay, and Tick hooks so future spell-targeting behavior can be added without changing callers.
 *
 * Important functions:
 *   - AMagicCircle() - Creates and attaches the decal component used for the targeting projection.
 *   - Tick() - Per-frame update hook for future placement or visual-follow behavior.
 *   - BeginPlay() - Actor startup hook for runtime initialization.
 */
UCLASS()
class AURA_API AMagicCircle : public AActor
{
	GENERATED_BODY()

public:
	/**
	 * @brief Creates the magic circle actor and its decal component.
	 *
	 * Enables ticking and attaches the decal component so the actor can represent a projected spell area
	 * in the world.
	 */
	AMagicCircle();


	// Visual components
	/** Decal projected into the world to display the magic circle. */
	UPROPERTY(VisibleAnywhere)
	TObjectPtr<UDecalComponent> MagicCircleDecal;
};
