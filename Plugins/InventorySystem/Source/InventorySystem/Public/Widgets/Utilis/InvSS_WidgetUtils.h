// @Copyright HaolunYuan

#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "Type/InvSS_GridTypes.h"
#include "InvSS_WidgetUtils.generated.h"

class UWidget;
/**
 * UInvSS_WidgetUtils
 *
 * Small conversion helpers for translating between 2D grid coordinates and flat slot indices.
 */
UCLASS()
class INVENTORYSYSTEM_API UInvSS_WidgetUtils : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:
	/**
	 * @brief Converts a 2D grid coordinate into a flat array index.
	 */
	static int32 GetIndexFromPosition(const FIntPoint& InPosition, const int32 InTotalColumns);

	/**
	 * @brief Converts a flat array index into a 2D grid coordinate.
	 */
	static FIntPoint GetPositionFromIndex(const int32 InIndex, const int32 InTotalColumns);
	
	static FVector2D GetWidgetPosition(UWidget* Widget);
	
	static FVector2D GetWidgetSize(const UWidget* Widget);

	static bool TryGetMousePositionInWidgetLocal(const UWidget* Widget, FVector2D& OutLocalPosition);

	static FIntPoint GetGridCoordinatesFromLocalPosition(const FVector2D& LocalPosition, float TileSize);

	static EInvSS_TileQuadrant GetTileQuadrantFromLocalPosition(const FVector2D& LocalPosition, float TileSize);

	static FIntPoint CalculateItemStartingCoordinate(
		const FIntPoint& Coordinate,
		const FIntPoint& Dimensions,
		EInvSS_TileQuadrant Quadrant);
	
	static bool IsRangeInGridBounds(int32 StartIndex, const FIntPoint& GridDimensions, const FIntPoint& Extent);
	
	static bool IsPositionInBounds(const FVector2D& BoundaryPos, const FVector2D& BoundarySize, const FVector2D& Position);
};
