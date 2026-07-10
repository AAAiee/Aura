// @Copyright HaolunYuan

#include "Type/InvSS_GridTypes.h"

bool FInvSS_HeldItemState::IsValid() const
{
	return ItemId.IsValid();
}

void FInvSS_HeldItemState::Reset()
{
	ItemId.Invalidate();
	SourceCategory = EInvSS_ItemCategory::None;
	SourceParentIndex = INDEX_NONE;
	StackCount = 0;
}

bool FInvSS_TileParameters::operator==(const FInvSS_TileParameters& Other) const
{
	return TileCoordinates == Other.TileCoordinates
		&& TileIndex == Other.TileIndex
		&& TileQuadrant == Other.TileQuadrant;
}
