// @Copyright HaolunYuan


#include "Widgets/Inventory/InventorySpatial/InvSS_InventoryGrid.h"

#include "Blueprint/WidgetLayoutLibrary.h"
#include "Components/CanvasPanel.h"
#include "InventoryManagement/Utils/InvSS_InventoryStatics.h"
#include "Item/InvSS_InventoryItem.h"
#include "Item/Fragment/InvSS_ItemFragment.h"
#include "Item/Fragment/InvSS_ItemFragmentTag.h"
#include "Widgets/GridSlot/InvSS_GridSlot.h"
#include "Widgets/HoverItem/InvSS_HoverItem.h"
#include "Widgets/Inventory/SlottedItem/InvSS_SlottedItem.h"
#include "Widgets/Utilis/InvSS_WidgetUtils.h"
#include "Widgets/WidgetController/InvSS_InventoryWidgetController.h"


void UInvSS_InventoryGrid::ShowCursor()
{
	APlayerController* OwningPlayer = GetOwningPlayer();
	if (!OwningPlayer) return;
	OwningPlayer->SetMouseCursorWidget(EMouseCursor::Default, GetCursorVisibleWidget());
}

void UInvSS_InventoryGrid::HideCursor()
{
	APlayerController* OwningPlayer = GetOwningPlayer();
	if (!OwningPlayer) return;
	OwningPlayer->SetMouseCursorWidget(EMouseCursor::Default, GetCursorHiddenWidget());
}

void UInvSS_InventoryGrid::NativeTick(const FGeometry& MyGeometry, float InDeltaTime)
{
	Super::NativeTick(MyGeometry, InDeltaTime);

	FVector2D MouseLocalPosition;
	bMouseInCanvasLastFrame = bMouseInCanvasThisFrame;
	bMouseInCanvasThisFrame = UInvSS_WidgetUtils::TryGetMousePositionInWidgetLocal(
		GridCanvasPanel.Get(),
		MouseLocalPosition);

	if (bMouseInCanvasLastFrame && !bMouseInCanvasThisFrame)
	{
		ClearHighlightedSlots();
		return;
	}

	if (!bMouseInCanvasThisFrame) return;

	UpdateTileParameters(MouseLocalPosition);
}

void UInvSS_InventoryGrid::UpdateTileParameters(const FVector2D& MouseLocalPosition)
{
	// find out where the mouse is in the grid
	const FInvSS_TileParameters NewTileParameters = BuildTileParametersFromLocalPosition(MouseLocalPosition);

	if (NewTileParameters == CurrentTileParameters) return;

	LastTileParameters = CurrentTileParameters;
	CurrentTileParameters = NewTileParameters;
	OnTileParametersUpdated(CurrentTileParameters);
}

FInvSS_TileParameters UInvSS_InventoryGrid::BuildTileParametersFromLocalPosition(
	const FVector2D& MouseLocalPosition) const
{
	check(RenderedGridColumns > 0);

	FInvSS_TileParameters TileParameters;
	TileParameters.TileCoordinates = UInvSS_WidgetUtils::GetGridCoordinatesFromLocalPosition(
		MouseLocalPosition,
		TileSize);
	TileParameters.TileIndex = UInvSS_WidgetUtils::GetIndexFromPosition(
		TileParameters.TileCoordinates,
		RenderedGridColumns);
	TileParameters.TileQuadrant = UInvSS_WidgetUtils::GetTileQuadrantFromLocalPosition(
		MouseLocalPosition,
		TileSize);

	return TileParameters;
}

void UInvSS_InventoryGrid::OnTileParametersUpdated(FInvSS_TileParameters InTileParameters)
{
	if (!IsValid(HoverItem)) return ;
	if (!UpdateDropQueryFromTileParameters(InTileParameters)) return;

	const FIntPoint GridDimension = HoverItem->GetHoveredItemGridDimensions();

	// if that area is empty, highlight to show it is landable
	if (CurrentQueryResult.bHasSpace)
	{
		HighlightSlots(ItemDropIndex, GridDimension, EInvSS_GridSlotVisualState::Occupied);
		return;
	}

	// if that area is not empty, unhighlight everything
	ClearHighlightedSlots();

	// A single blocking item may later support swap or stack-preview behavior.
	if (CurrentQueryResult.ValidItem.IsValid() && GridSlots.IsValidIndex(CurrentQueryResult.ItemParentIndex)) 
	{
		const FInvSS_GridFragment*  GridFragment = GetFragment<FInvSS_GridFragment>(CurrentQueryResult.ValidItem.Get(), ItemFragmentTag::GridFragment);
		check(GridFragment);
	
		HighlightSlots(CurrentQueryResult.ItemParentIndex, GridFragment->GetGridSize(), EInvSS_GridSlotVisualState::GrayedOut);
	}
}

bool UInvSS_InventoryGrid::UpdateDropQueryFromTileParameters(const FInvSS_TileParameters& TileParameters)
{
	if (!IsValid(HoverItem)) return false;
	check(RenderedGridColumns > 0);

	// Convert the mouse tile into the hovered item's top-left slot, then query that footprint.
	const FIntPoint GridDimension = HoverItem->GetHoveredItemGridDimensions();
	const FIntPoint StartingCoordinate = UInvSS_WidgetUtils::CalculateItemStartingCoordinate(
		TileParameters.TileCoordinates,
		GridDimension,
		TileParameters.TileQuadrant);

	ItemDropIndex = UInvSS_WidgetUtils::GetIndexFromPosition(StartingCoordinate, RenderedGridColumns);
	CurrentQueryResult = CheckHoveredPosition(StartingCoordinate, GridDimension);
	return GridSlots.IsValidIndex(ItemDropIndex);
}

FInvSS_SpaceQueryResult UInvSS_InventoryGrid::CheckHoveredPosition(
	const FIntPoint& Position,
	const FIntPoint& Dimensions)
{
	FInvSS_SpaceQueryResult QueryResult;

	const int32 TileIndexForPosition = UInvSS_WidgetUtils::GetIndexFromPosition(Position, RenderedGridColumns);
	if (!InventoryWidgetController.IsValid()) return QueryResult;

	InventoryWidgetController->QueryGridSpace(
		ItemCategory,
		TileIndexForPosition,
		Dimensions,
		QueryResult);

	return QueryResult;
}

void UInvSS_InventoryGrid::HighlightSlots(const int32 StartIndex, const FIntPoint Range, const EInvSS_GridSlotVisualState ToState)
{
	if (!bMouseInCanvasThisFrame) return;

	UnHighlightSlots(LastHighlightSlotStartIndex, LastHighlightSlotDimension);
	UInvSS_InventoryStatics::ForEach2D(
		GridSlots,
		StartIndex,
		RenderedGridColumns,
		Range,
		[ToState](TObjectPtr<UInvSS_GridSlot>& InSlot)
		{
			if (!IsValid(InSlot.Get())) return;
			InSlot->SetTemporaryVisualState(ToState);
		});

	LastHighlightSlotStartIndex = StartIndex;
	LastHighlightSlotDimension = Range;
}

void UInvSS_InventoryGrid::UnHighlightSlots(const int32 StartIndex, const FIntPoint Range)
{
	if (StartIndex == INDEX_NONE) return;
	if (Range.X <= 0 || Range.Y <= 0) return;

	UInvSS_InventoryStatics::ForEach2D(
		GridSlots,
		StartIndex,
		RenderedGridColumns,
		Range,
		[](TObjectPtr<UInvSS_GridSlot>& InSlot)
		{
			if (!IsValid(InSlot.Get())) return;
			InSlot->RestoreBaseVisualState();
		});
}

void UInvSS_InventoryGrid::ClearHighlightedSlots()
{
	UnHighlightSlots(LastHighlightSlotStartIndex, LastHighlightSlotDimension);
	ResetHighlightTracking();
}

void UInvSS_InventoryGrid::ResetHighlightTracking()
{
	LastHighlightSlotStartIndex = INDEX_NONE;
	LastHighlightSlotDimension = FIntPoint(-1, -1);
}

void UInvSS_InventoryGrid::RemoveItemFromGrid(UInvSS_SlottedItem* ClickedSlottedItem, const int32 GridIndex)
{
	check(IsValid(ClickedSlottedItem));
	check(GridSlots.IsValidIndex(GridIndex));
	check(ClickedSlottedItem->GetGridIndex() == GridIndex);
	check(SlottedItemsMap.FindRef(GridIndex) == ClickedSlottedItem);

	const FIntPoint GridDimensions = ClickedSlottedItem->GetGridDimensions();
	check(GridDimensions.X > 0 && GridDimensions.Y > 0);

	// Pickup removes the local visual immediately; the server request removes the authoritative grid placement.
	RemoveSlottedItemAtIndex(GridIndex);
	UInvSS_InventoryStatics::ForEach2D(
		GridSlots,
		GridIndex,
		RenderedGridColumns,
		GridDimensions,
		[](TObjectPtr<UInvSS_GridSlot>& GridSlot)
		{
			if (!IsValid(GridSlot.Get())) return;

			GridSlot->SetBaseVisualState(EInvSS_GridSlotVisualState::Unoccupied);
		});
}

void UInvSS_InventoryGrid::OnSlottedItemClickedCallback(
	UInvSS_SlottedItem* SlottedItem,
	const int32 InSlotIndex,
	const FPointerEvent& InMouseEvent)
{
	check(GridSlots.IsValidIndex(InSlotIndex));
	check(IsValid(SlottedItem));

	if (!UInvSS_InventoryStatics::IsLeftMouseButtonPressed(InMouseEvent)) return;

	if (IsValid(HoverItem))
	{
		TryPutDownHeldItem(InMouseEvent);
		return;
	}

	PickUpFromSlot(SlottedItem, InSlotIndex);
}

void UInvSS_InventoryGrid::OnGridSlotClickedCallback(int32 GridIndex, const FPointerEvent& MouseEvent)
{
	TryPutDownHeldItem(MouseEvent);
}

bool UInvSS_InventoryGrid::TryPutDownHeldItem(const FPointerEvent& MouseEvent)
{
	if (!IsValid(HoverItem)) return false;
	if (!UInvSS_InventoryStatics::IsLeftMouseButtonPressed(MouseEvent)) return false;
	if (!RefreshDropQueryForHeldItem()) return false;
	check(GridSlots.IsValidIndex(ItemDropIndex));
	check(InventoryWidgetController.IsValid());
	
	if (CurrentQueryResult.ValidItem.IsValid())
	{
		check(GridSlots.IsValidIndex(CurrentQueryResult.ItemParentIndex));

		//swap path
		InventoryWidgetController->RequestInteractHeldItemWithItemUnderCursor(
			ItemCategory,
			CurrentQueryResult.ValidItem.Get()->GetItemInstanceId(),
			CurrentQueryResult.ItemParentIndex,
			ItemDropIndex); // Swap or stack
		return true;
	}

	if (!CurrentQueryResult.bHasSpace) return false;

	// PutItemDownAtaIndex Path
	InventoryWidgetController->RequestPutDownHeldITemAtIndex(ItemCategory, ItemDropIndex);
	return true;
}

bool UInvSS_InventoryGrid::RefreshDropQueryForHeldItem()
{
	if (!IsValid(HoverItem)) return false;
	if (RenderedGridColumns <= 0) return false;
	check(GridCanvasPanel);

	FVector2D MouseLocalPosition;
	if (!UInvSS_WidgetUtils::TryGetMousePositionInWidgetLocal(GridCanvasPanel.Get(), MouseLocalPosition))
	{
		return false;
	}

	const FInvSS_TileParameters TileParameters = BuildTileParametersFromLocalPosition(MouseLocalPosition);
	return UpdateDropQueryFromTileParameters(TileParameters);
}

void UInvSS_InventoryGrid::OnGridSlotHoveredCallback(int32 GridIndex, const FPointerEvent& MouseEvent)
{
	if (IsValid(HoverItem)) return;

	if (UInvSS_GridSlot* GridSlot =	GridSlots[GridIndex]; GridSlot->IsBaseState(EInvSS_GridSlotVisualState::Unoccupied))
	{
		GridSlot->SetTemporaryVisualState(EInvSS_GridSlotVisualState::Occupied); 
	}
}

void UInvSS_InventoryGrid::OnGridSlotUnhoveredCallback(int32 GridIndex, const FPointerEvent& MouseEvent)
{
	if (IsValid(HoverItem)) return;

	if (UInvSS_GridSlot* GridSlot =	GridSlots[GridIndex])
	{
		GridSlot->RestoreBaseVisualState();
	}
}

void UInvSS_InventoryGrid::PickUpFromSlot(UInvSS_SlottedItem* ClickedSlottedItem, const int32 GridIndex)
{
	check(InventoryWidgetController.IsValid());
	check(IsValid(ClickedSlottedItem));
	check(ClickedSlottedItem->GetGridIndex() == GridIndex);

	InventoryWidgetController->RequestBeginDragItem(ItemCategory, GridIndex);
}

void UInvSS_InventoryGrid::AssignHoverItemFromHeldState(
	UInvSS_InventoryItem* HeldItem,
	const int32 SourceParentIndex,
	const int32 StackCount)
{
	check(IsValid(HeldItem));
	check(SourceParentIndex != INDEX_NONE);
	check(StackCount >= 0);

	CreateOrUpdateHoverItemVisual(HeldItem);
	check(IsValid(HoverItem));

	HoverItem->UpdateHoverItemStackCount(HeldItem->IsStackable() ? StackCount : 0);
	HoverItem->SetPreviousGridIndex(SourceParentIndex);
}

void UInvSS_InventoryGrid::CreateOrUpdateHoverItemVisual(UInvSS_InventoryItem* HoveredInventoryItem)
{
	check(IsValid(HoveredInventoryItem));

	if (!IsValid(HoverItem))
	{
		checkf(HoveredItemClass, TEXT("HoveredItemClass is not set on %s."), *GetName());
		HoverItem = CreateWidget<UInvSS_HoverItem>(GetOwningPlayer(), HoveredItemClass);
	}

	check(HoverItem);
	const FInvSS_GridFragment* GridFragment = GetFragment<FInvSS_GridFragment>(HoveredInventoryItem, ItemFragmentTag::GridFragment);
	const FInvSS_ImageFragment* ImageFragment = GetFragment<FInvSS_ImageFragment>(HoveredInventoryItem, ItemFragmentTag::IconFragment);
	checkf(GridFragment, TEXT("GridFragment is missing from %s."), *GetNameSafe(HoveredInventoryItem));
	checkf(ImageFragment, TEXT("ImageFragment is missing from %s."), *GetNameSafe(HoveredInventoryItem));

	const FVector2D DrawSize = GetDrawSize(GridFragment);

	FSlateBrush IconBrush;
	IconBrush.SetResourceObject(ImageFragment->GetImageResource());
	IconBrush.DrawAs = ESlateBrushDrawType::Image;
	IconBrush.ImageSize = DrawSize * UWidgetLayoutLibrary::GetViewportScale(this);

	HoverItem->SetImageBrush(IconBrush);
	HoverItem->SetHoveredItemGridDimensions(GridFragment->GetGridSize());
	HoverItem->SetLinkedInventoryItem(HoveredInventoryItem);
	HoverItem->SetIsStackable(HoveredInventoryItem->IsStackable());

	GetOwningPlayer()->SetMouseCursorWidget(EMouseCursor::Default, HoverItem);
}

void UInvSS_InventoryGrid::ClearHoveredItem()
{
	ShowCursor();

	if (IsValid(HoverItem))
	{
		HoverItem->Reset();
		HoverItem->RemoveFromParent();
		HoverItem = nullptr;
	}

	ClearHighlightedSlots();
}

void UInvSS_InventoryGrid::HandleInventoryHeldItemChanged(FInvSS_HeldItemState HeldItemState)
{
	if (!HeldItemState.IsValid())
	{
		ClearHoveredItem();
		return;
	}

	if (HeldItemState.SourceCategory != ItemCategory) return;

	const bool bWasAlreadyHoldingItem = IsValid(HoverItem);

	ApplyValidHeldItemState(HeldItemState);

	if (!bWasAlreadyHoldingItem)
	{
		RemoveHeldItemSourceVisual(HeldItemState);
	}
}

void UInvSS_InventoryGrid::ApplyValidHeldItemState(const FInvSS_HeldItemState& HeldItemState)
{
	check(InventoryWidgetController.IsValid());
	
	UInvSS_InventoryItem* HeldItem = InventoryWidgetController->GetInventoryItemByID(HeldItemState.ItemId);
	check(IsValid(HeldItem));

	AssignHoverItemFromHeldState(
		HeldItem,
		HeldItemState.SourceParentIndex,
		HeldItemState.StackCount);
}

void UInvSS_InventoryGrid::RemoveHeldItemSourceVisual(const FInvSS_HeldItemState& HeldItemState)
{
	if (UInvSS_SlottedItem* SourceSlottedItem = SlottedItemsMap.FindRef(HeldItemState.SourceParentIndex))
	{
		RemoveItemFromGrid(SourceSlottedItem, HeldItemState.SourceParentIndex);
	}
}
