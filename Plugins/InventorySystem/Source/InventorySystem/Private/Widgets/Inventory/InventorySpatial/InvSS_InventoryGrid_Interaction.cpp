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
	FInvSS_TileParameters NewTileParameters;
	NewTileParameters.TileCoordinates = UInvSS_WidgetUtils::GetGridCoordinatesFromLocalPosition(
		MouseLocalPosition,
		TileSize);
	NewTileParameters.TileIndex = UInvSS_WidgetUtils::GetIndexFromPosition(
		NewTileParameters.TileCoordinates,
		RenderedGridColumns);
	NewTileParameters.TileQuadrant = UInvSS_WidgetUtils::GetTileQuadrantFromLocalPosition(
		MouseLocalPosition,
		TileSize);

	if (NewTileParameters == CurrentTileParameters) return;

	LastTileParameters = CurrentTileParameters;
	CurrentTileParameters = NewTileParameters;
	OnTileParametersUpdated(CurrentTileParameters);
}

void UInvSS_InventoryGrid::OnTileParametersUpdated(FInvSS_TileParameters InTileParameters)
{
	if (!IsValid(HoverItem)) return ;
	// find where the hovered item potentially lands based on current mouse pos
	const FIntPoint GridDimension = HoverItem->GetHoveredItemGridDimensions();
	const FIntPoint StartingCoordinate = UInvSS_WidgetUtils::CalculateItemStartingCoordinate(
		InTileParameters.TileCoordinates,
		GridDimension,
		InTileParameters.TileQuadrant);

	// get the info about that specific area
	ItemDropIndex = UInvSS_WidgetUtils::GetIndexFromPosition(StartingCoordinate, RenderedGridColumns);
	CurrentQueryResult = CheckHoveredPosition(StartingCoordinate, GridDimension);

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

FInvSS_SpaceQueryResult UInvSS_InventoryGrid::CheckHoveredPosition(
	const FIntPoint& Position,
	const FIntPoint& Dimensions)
{
	FInvSS_SpaceQueryResult QueryResult;

	const int32 TileIndexForPosition = UInvSS_WidgetUtils::GetIndexFromPosition(Position, RenderedGridColumns);
	if (!InventoryWidgetController.IsValid()) return QueryResult;

	const int32 IgnoredParentIndex = IsValid(HoverItem)
		? HoverItem->GetPreviousGridIndex()
		: INDEX_NONE;

	InventoryWidgetController->QueryGridSpace(
		ItemCategory,
		TileIndexForPosition,
		Dimensions,
		IgnoredParentIndex,
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

	if (IsValid(HoverItem)) return;
	if (!UInvSS_InventoryStatics::IsLeftMouseButtonPressed(InMouseEvent)) return;

	PickUpFromSlot(SlottedItem, InSlotIndex);
}

void UInvSS_InventoryGrid::OnGridSlotClickedCallback(int32 GridIndex, const FPointerEvent& MouseEvent)
{
	if (!IsValid(HoverItem)) return;
	if (!GridSlots.IsValidIndex(ItemDropIndex)) return;
	
	if (CurrentQueryResult.ValidItem.IsValid() && GridSlots.IsValidIndex(CurrentQueryResult.ItemParentIndex))
	{
		//swap path
		return;
	}
	// PutItemDownAtaIndex Path
	check(InventoryWidgetController.IsValid());
	InventoryWidgetController->RequestPutDownHeldITemAtIndex(ItemCategory, ItemDropIndex);
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

void UInvSS_InventoryGrid::AssignHoveredItem(UInvSS_SlottedItem* ClickedSlottedItem, const int32 PreviousGridIndex)
{
	check(IsValid(ClickedSlottedItem));

	UInvSS_InventoryItem* InventoryItem = ClickedSlottedItem->GetInventoryItem();
	check(IsValid(InventoryItem));

	const int32 StackCount = ClickedSlottedItem->GetIsStackable()
		? ClickedSlottedItem->GetStackCount()
		: 0;

	AssignHoveredItem(InventoryItem, PreviousGridIndex, StackCount);
}

void UInvSS_InventoryGrid::AssignHoveredItem(
	UInvSS_InventoryItem* HoveredInventoryItem,
	const int32 PreviousGridIndex,
	const int32 StackCount)
{
	check(IsValid(HoveredInventoryItem));

	AssignHoveredItem(HoveredInventoryItem);
	check(IsValid(HoverItem));

	HoverItem->UpdateHoverItemStackCount(HoveredInventoryItem->IsStackable() ? StackCount : 0);
	HoverItem->SetPreviousGridIndex(PreviousGridIndex);
}

void UInvSS_InventoryGrid::AssignHoveredItem(UInvSS_InventoryItem* HoveredInventoryItem)
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
	if (APlayerController* OwningPlayer = GetOwningPlayer())
	{
		OwningPlayer->SetMouseCursorWidget(EMouseCursor::Default, nullptr);
	}

	if (IsValid(HoverItem))
	{
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
	if (IsValid(HoverItem)) return;

	check(InventoryWidgetController.IsValid());
	UInvSS_InventoryItem* HeldItem = InventoryWidgetController->GetInventoryItemByID(HeldItemState.ItemId);
	check(IsValid(HeldItem));

	AssignHoveredItem(
		HeldItem,
		HeldItemState.SourceParentIndex,
		HeldItemState.StackCount);

	if (UInvSS_SlottedItem* SourceSlottedItem = SlottedItemsMap.FindRef(HeldItemState.SourceParentIndex))
	{
		RemoveItemFromGrid(SourceSlottedItem, HeldItemState.SourceParentIndex);
	}
}
