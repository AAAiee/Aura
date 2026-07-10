// @Copyright HaolunYuan


#include "Widgets/Utilis/InvSS_WidgetUtils.h"

#include "InteractiveToolManager.h"
#include "Blueprint/SlateBlueprintLibrary.h"
#include "Components/Widget.h"
#include "Framework/Application/SlateApplication.h"
#include "InventorySystem.h"

int32 UInvSS_WidgetUtils::GetIndexFromPosition(const FIntPoint& InPosition, const int32 InTotalColumns)
{
	return InPosition.Y * InTotalColumns + InPosition.X;
}

FIntPoint UInvSS_WidgetUtils::GetPositionFromIndex(const int32 InIndex, const int32 InTotalColumns)
{
	return { InIndex % InTotalColumns, InIndex / InTotalColumns };
}

FVector2D UInvSS_WidgetUtils::GetWidgetPosition(UWidget* Widget)
{
	const FGeometry& Geometry = Widget->GetCachedGeometry();
	FVector2D PixelPosition;
	FVector2D ViewportPosition;
	USlateBlueprintLibrary::LocalToViewport(Widget, Geometry, USlateBlueprintLibrary::GetLocalTopLeft(Geometry), PixelPosition, ViewportPosition);
	return ViewportPosition;
}

FVector2D UInvSS_WidgetUtils::GetWidgetSize(const UWidget* Widget)
{
	return IsValid(Widget) ? Widget->GetCachedGeometry().GetLocalSize() : FVector2D{-1.0,-1.0};
}

bool UInvSS_WidgetUtils::TryGetMousePositionInWidgetLocal(
	const UWidget* Widget,
	FVector2D& OutLocalPosition)
{
	if (!IsValid(Widget)) return false;

	const FGeometry& Geometry = Widget->GetCachedGeometry();
	OutLocalPosition = Geometry.AbsoluteToLocal(FSlateApplication::Get().GetCursorPos());

	return IsPositionInBounds(FVector2D::ZeroVector, Geometry.GetLocalSize(), OutLocalPosition);
}

FIntPoint UInvSS_WidgetUtils::GetGridCoordinatesFromLocalPosition(
	const FVector2D& LocalPosition,
	const float TileSize)
{
	check(TileSize > 0.f);

	return FIntPoint{
		FMath::FloorToInt32(LocalPosition.X / TileSize),
		FMath::FloorToInt32(LocalPosition.Y / TileSize)
	};
}

EInvSS_TileQuadrant UInvSS_WidgetUtils::GetTileQuadrantFromLocalPosition(
	const FVector2D& LocalPosition,
	const float TileSize)
{
	check(TileSize > 0.f);

	const float TileLocalX = FMath::Fmod(LocalPosition.X, TileSize);
	const float TileLocalY = FMath::Fmod(LocalPosition.Y, TileSize);

	const float TileCenterValue = TileSize / 2.f;
	const bool bIsTop = TileLocalY < TileCenterValue;
	const bool bIsLeft = TileLocalX < TileCenterValue;

	if (bIsTop && bIsLeft) return EInvSS_TileQuadrant::EInvSS_TopLeft;
	if (bIsTop && !bIsLeft) return EInvSS_TileQuadrant::EInvSS_TopRight;
	if (!bIsTop && bIsLeft) return EInvSS_TileQuadrant::EInvSS_BottomLeft;
	return EInvSS_TileQuadrant::EInvSS_BottomRight;
}

FIntPoint UInvSS_WidgetUtils::CalculateItemStartingCoordinate(
	const FIntPoint& Coordinate,
	const FIntPoint& Dimensions,
	const EInvSS_TileQuadrant Quadrant)
{
	const int32 HasEvenWidth = Dimensions.X % 2 == 0 ? 1 : 0;
	const int32 HasEvenHeight = Dimensions.Y % 2 == 0 ? 1 : 0;
	const int32 HalfWidth = FMath::FloorToInt32(0.5f * Dimensions.X);
	const int32 HalfHeight = FMath::FloorToInt32(0.5f * Dimensions.Y);

	switch (Quadrant)
	{
	case EInvSS_TileQuadrant::EInvSS_TopLeft:
		return {
			Coordinate.X - HalfWidth,
			Coordinate.Y - HalfHeight
		};
	case EInvSS_TileQuadrant::EInvSS_TopRight:
		return {
			Coordinate.X - HalfWidth + HasEvenWidth,
			Coordinate.Y - HalfHeight
		};
	case EInvSS_TileQuadrant::EInvSS_BottomLeft:
		return {
			Coordinate.X - HalfWidth,
			Coordinate.Y - HalfHeight + HasEvenHeight
		};
	case EInvSS_TileQuadrant::EInvSS_BottomRight:
		return {
			Coordinate.X - HalfWidth + HasEvenWidth,
			Coordinate.Y - HalfHeight + HasEvenHeight
		};
	default:
		UE_LOG(LogInvSS, Error, TEXT("Invalid tile quadrant."));
		return FIntPoint{ -1, -1 };
	}
}

bool UInvSS_WidgetUtils::IsRangeInGridBounds(const int32 StartIndex, const FIntPoint& GridDimensions, const FIntPoint& Extent)
{
	if (GridDimensions.X <= 0 || GridDimensions.Y <= 0) return false;
	if (Extent.X <= 0 || Extent.Y <= 0) return false;

	const int32 TotalNumSlots = GridDimensions.X * GridDimensions.Y;
	if (StartIndex < 0 || StartIndex >= TotalNumSlots) return false;

	const int32 ItemEndReachX = (StartIndex % GridDimensions.X) + Extent.X;
	const int32 ItemEndReachY = (StartIndex / GridDimensions.X) + Extent.Y;
	return ItemEndReachX <= GridDimensions.X && ItemEndReachY <= GridDimensions.Y;
}

bool UInvSS_WidgetUtils::IsPositionInBounds(const FVector2D& BoundaryPos, const FVector2D& BoundarySize,
	const FVector2D& Position)
{
	return Position.X >= (BoundaryPos.X)  && Position.X <= (BoundaryPos.X + BoundarySize.X)
		&& Position.Y >= (BoundaryPos.Y) && Position.Y <= (BoundaryPos.Y + BoundarySize.Y);
}

FVector2D UInvSS_WidgetUtils::GetClampedWidgetPosition(
	const FVector2D& BoundarySize,
	const FVector2D& WidgetSize,
	const FVector2D& DesiredPosition)
{
	const FVector2D MaxPosition(
		FMath::Max(0.f, BoundarySize.X - WidgetSize.X),
		FMath::Max(0.f, BoundarySize.Y - WidgetSize.Y));

	return FVector2D(
		FMath::Clamp(DesiredPosition.X, 0.f, MaxPosition.X),
		FMath::Clamp(DesiredPosition.Y, 0.f, MaxPosition.Y));
}
