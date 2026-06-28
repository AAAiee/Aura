// @Copyright HaolunYuan

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "PointCollection.generated.h"

/**
 * APointCollection
 *
 * Actor that owns reusable scene points for radial spell placement and ground alignment.
 *
 * Point collections give abilities a fixed set of child scene components that can be adjusted near a
 * ground location, traced onto walkable geometry, and returned to Blueprints or gameplay logic as
 * spawn or impact locations. The points are authored as stable components so designers can inspect and tune
 * their relative layout in the editor.
 *
 * Important functions:
 *   - APointCollection() - Creates the root point and all reusable child point components.
 *   - GetGroundPoints() - Aligns requested points to the ground and returns them for ability placement.
 *   - BeginPlay() - Actor startup hook for runtime initialization.
 */
UCLASS()
class AURA_API APointCollection : public AActor
{
	GENERATED_BODY()

public:
	/* Ground Point Placement begins */

	/**
	 * @brief Creates the reusable point components owned by this collection.
	 *
	 * Disables ticking and constructs the root point plus child points that can later be aligned to the
	 * ground for spell placement.
	 */
	APointCollection();

	/**
	 * @brief Returns point components adjusted to the ground near a requested location.
	 *
	 * Uses the stored scene components as reusable placement points, traces around the requested ground
	 * height, and aligns each returned point to the impact normal.
	 *
	 * @param GroundLocation World location whose Z value anchors the ground trace range.
	 * @param NumPoints Minimum number of authored points required before the request proceeds.
	 * @param YawOverride Requested yaw adjustment used by the point preparation path before tracing.
	 *
	 * @return Array of stored scene components positioned and rotated to match traced ground points.
	 *
	 * @pre NumPoints must be less than or equal to the number of stored immutable point components.
	 */
	UFUNCTION(BlueprintPure)
	TArray<USceneComponent*> GetGroundPoints(const FVector& GroundLocation, int32 NumPoints, float YawOverride = 0.f);

	/* Ground Point Placement ends */

protected:
	/* AActor begins */

	/**
	 * @brief Handles actor startup after the point collection enters play.
	 *
	 * Currently forwards to the base actor BeginPlay and reserves a runtime initialization point for future
	 * point collection behavior.
	 */
	virtual void BeginPlay() override;

	/* AActor ends */

	// Point storage
	/** Stable list of point components used by abilities when requesting ground-aligned positions. */
	UPROPERTY(BlueprintReadOnly, VisibleAnywhere)
	TArray<TObjectPtr<USceneComponent>> ImmutablePtrs;

	/** Root point and center reference for the collection. */
	UPROPERTY(BlueprintReadOnly, VisibleAnywhere)
	TObjectPtr<USceneComponent> Pt_0;

	/** Reusable child point for spell placement. */
	UPROPERTY(BlueprintReadOnly, VisibleAnywhere)
	TObjectPtr<USceneComponent> Pt_1;

	/** Reusable child point for spell placement. */
	UPROPERTY(BlueprintReadOnly, VisibleAnywhere)
	TObjectPtr<USceneComponent> Pt_2;

	/** Reusable child point for spell placement. */
	UPROPERTY(BlueprintReadOnly, VisibleAnywhere)
	TObjectPtr<USceneComponent> Pt_3;

	/** Reusable child point for spell placement. */
	UPROPERTY(BlueprintReadOnly, VisibleAnywhere)
	TObjectPtr<USceneComponent> Pt_4;

	/** Reusable child point for spell placement. */
	UPROPERTY(BlueprintReadOnly, VisibleAnywhere)
	TObjectPtr<USceneComponent> Pt_5;

	/** Reusable child point for spell placement. */
	UPROPERTY(BlueprintReadOnly, VisibleAnywhere)
	TObjectPtr<USceneComponent> Pt_6;

	/** Reusable child point for spell placement. */
	UPROPERTY(BlueprintReadOnly, VisibleAnywhere)
	TObjectPtr<USceneComponent> Pt_7;

	/** Reusable child point for spell placement. */
	UPROPERTY(BlueprintReadOnly, VisibleAnywhere)
	TObjectPtr<USceneComponent> Pt_8;

	/** Reusable child point for spell placement. */
	UPROPERTY(BlueprintReadOnly, VisibleAnywhere)
	TObjectPtr<USceneComponent> Pt_9;

	/** Reusable child point for spell placement. */
	UPROPERTY(BlueprintReadOnly, VisibleAnywhere)
	TObjectPtr<USceneComponent> Pt_10;

	/** Reusable child point for spell placement. */
	UPROPERTY(BlueprintReadOnly, VisibleAnywhere)
	TObjectPtr<USceneComponent> Pt_11;
};
