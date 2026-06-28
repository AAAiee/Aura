// @Copyright HaolunYuan

#pragma once

#include "CoreMinimal.h"
#include "Effect/AuraEffectActor.h"
#include "Interaction/Highlightable.h"
#include "Interaction/PickupInterface.h"
#include "PickupEffectActor.generated.h"

class UPickupsTextDisplayComponent;
class UWidgetComponent;
/**
 * APickupEffectActor
 *
 * Effect actor specialization for world pickups that can be highlighted and collected.
 *
 * The class keeps generic GameplayEffect application in AAuraEffectActor while adding two pickup-facing
 * contracts: IHighlightable for cursor outlines and IPickupInterface for explicit pickup calls. Blueprint
 * subclasses can still provide collision and visuals while C++ owns the shared interaction behavior.
 *
 * Important functions:
 *   - HighLightActor() - Enables the red custom-depth outline on visible primitive components.
 *   - UnhighLightActor() - Disables the custom-depth outline.
 *   - Pickup() - Applies configured pickup effects to the collecting actor.
 */
UCLASS()
class AURA_API APickupEffectActor : public AAuraEffectActor, public IHighlightable, public IPickupInterface
{
	GENERATED_BODY()

public:
	APickupEffectActor();

	/* IHighlightable begins */
	/* Pickup Highlighting begins */

	/**
	 * @brief Enables the red outline used for cursor-hover feedback.
	 *
	 * Mirrors the Aura enemy highlight style by setting Custom Depth with CUSTOM_DEPTH_RED on the pickup's
	 * visible primitive components.
	 */
	virtual void HighLightActor() override;

	/**
	 * @brief Disables the pickup's cursor-hover outline.
	 *
	 * Clears Custom Depth on the same primitive components used by HighLightActor().
	 */
	virtual void UnhighLightActor() override;

	/* Pickup Highlighting ends */
	/* IHighlightable ends */

	/* IPickupInterface begins */

	/**
	 * @brief Applies this pickup's configured effects to the collecting actor.
	 *
	 * Reuses AAuraEffectActor's overlap application path so pickup collection and overlap-driven effects
	 * share the same policy and GameplayEffect setup.
	 *
	 * @param PickupTarget Actor collecting this pickup.
	 */
	virtual void Pickup_Implementation(AActor* PickupTarget) override;

	/* IPickupInterface ends */

protected:
	/**
	 * @brief Clears transient pickup state before the actor is returned to the pool.
	 *
	 * Ensures pooled pickups do not keep a red outline after being reused.
	 */
	virtual void ResetPooledState() override;
	virtual void BeginPlay() override;
	void UpdateItemNamePlate();

	UPROPERTY(VisibleAnywhere)
	TObjectPtr<UPickupsTextDisplayComponent> TextDisplay;

	
private:
	void SetHighlightEnabled(bool bInHighlighted);
	bool bIsHighlighted = false;
};
