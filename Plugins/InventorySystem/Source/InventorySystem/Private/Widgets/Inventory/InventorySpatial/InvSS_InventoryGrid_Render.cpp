// @Copyright HaolunYuan


#include "Widgets/Inventory/InventorySpatial/InvSS_InventoryGrid.h"

#include "Blueprint/WidgetLayoutLibrary.h"
#include "Components/CanvasPanel.h"
#include "Components/CanvasPanelSlot.h"
#include "Item/InvSS_InventoryItem.h"
#include "Item/Fragment/InvSS_ItemFragment.h"
#include "Item/Fragment/InvSS_ItemFragmentTag.h"
#include "Widgets/GridSlot/InvSS_GridSlot.h"
#include "Widgets/Inventory/SlottedItem/InvSS_SlottedItem.h"
#include "Widgets/Utilis/InvSS_WidgetUtils.h"
#include "Widgets/WidgetController/InvSS_InventoryWidgetController.h"

void UInvSS_InventoryGrid::ConstructGrid(const int32 GridRows, const int32 GridColumns)
{
	checkf(GridCanvasPanel, TEXT("GridCanvasPanel is not bound on %s."), *GetName());
	checkf(GridSlotClass, TEXT("GridSlotClass is not set on %s."), *GetName());
	checkf(GridRows > 0, TEXT("GridRows must be greater than 0 on %s."), *GetName());
	checkf(GridColumns > 0, TEXT("GridColumns must be greater than 0 on %s."), *GetName());
	checkf(TileSize > 0.f, TEXT("TileSize must be greater than 0 on %s."), *GetName());

	GridCanvasPanel->ClearChildren();
	GridSlots.Empty();
	SlottedItemsMap.Empty();
	LastRenderedSlots.Reset();
	bHasRenderedViewData = false;

	RenderedGridRows = GridRows;
	RenderedGridColumns = GridColumns;

	GridSlots.Reserve(GridRows * GridColumns);
	for (int32 Row = 0; Row < GridRows; ++Row)
	{
		for (int32 Column = 0; Column < GridColumns; ++Column)
		{
			UInvSS_GridSlot* GridSlot = CreateWidget<UInvSS_GridSlot>(this, GridSlotClass);
			check(GridSlot);
			GridCanvasPanel->AddChild(GridSlot);

			const FIntPoint TilePosition(Column, Row);
			GridSlot->SetTileIndex(UInvSS_WidgetUtils::GetIndexFromPosition(TilePosition, GridColumns));

			UCanvasPanelSlot* GridCps = UWidgetLayoutLibrary::SlotAsCanvasSlot(GridSlot);
			GridCps->SetSize(FVector2D(TileSize));
			GridCps->SetPosition(FVector2D(TilePosition * TileSize));

			GridSlots.Add(GridSlot);
			GridSlot->OnGridSlotClicked.AddDynamic(this, &ThisClass::OnGridSlotClickedCallback);
			GridSlot->OnGridSlotHovered.AddDynamic(this, &ThisClass::OnGridSlotHoveredCallback);
			GridSlot->OnGridSlotUnhovered.AddDynamic(this, &ThisClass::OnGridSlotUnhoveredCallback);
		}
	}
}

bool UInvSS_InventoryGrid::IsGridBuiltForViewData(const FInvSS_InventoryGridViewData& ViewData) const
{
	return RenderedGridRows == ViewData.Rows
		&& RenderedGridColumns == ViewData.Columns
		&& GridSlots.Num() == ViewData.Slots.Num();
}

void UInvSS_InventoryGrid::RefreshFromViewData(const EInvSS_ItemCategory Category)
{
	if (Category != ItemCategory || !InventoryWidgetController.IsValid()) return;

	const FInvSS_InventoryGridViewData* ViewData = InventoryWidgetController->GetCachedGridViewData(ItemCategory);
	if (!ViewData) return;

	const bool bRequiresFullRender =
		!bHasRenderedViewData
		|| !IsGridBuiltForViewData(*ViewData)
		|| LastRenderedSlots.Num() != ViewData->Slots.Num();

	ClearHighlightedSlots();

	if (bRequiresFullRender)
	{
		RenderFullViewData(*ViewData);
	}
	else
	{
		RenderViewDataDelta(*ViewData);
	}

	CacheRenderedViewData(*ViewData);
	ResetHighlightTracking();
}

void UInvSS_InventoryGrid::RenderFullViewData(const FInvSS_InventoryGridViewData& ViewData)
{
	if (!IsGridBuiltForViewData(ViewData))
	{
		ConstructGrid(ViewData.Rows, ViewData.Columns);
	}

	ClearVisualItems();
	checkf(GridSlots.Num() == ViewData.Slots.Num(), TEXT("Inventory grid widget and view-data slot counts should match."));

	for (int32 Index = 0; Index < ViewData.Slots.Num(); ++Index)
	{
		RenderSlotViewData(Index, ViewData.Slots[Index]);
	}
}

void UInvSS_InventoryGrid::RenderViewDataDelta(const FInvSS_InventoryGridViewData& ViewData)
{
	struct FDetachedSlottedItem
	{
		FGuid ItemInstanceId;
		UInvSS_SlottedItem* Widget = nullptr;
	};

	TArray<int32> ChangedIndices;
	ChangedIndices.Reserve(ViewData.Slots.Num());

	for (int32 Index = 0; Index < ViewData.Slots.Num(); ++Index)
	{
		if (HasSlotChanged(LastRenderedSlots[Index], ViewData.Slots[Index]))
		{
			ChangedIndices.Add(Index);
		}
	}

	if (ChangedIndices.IsEmpty()) return;

	TArray<FDetachedSlottedItem> DetachedSlottedItems;

	// Detach obsolete parent mappings first so moved items can reuse their existing widgets.
	for (const int32 Index : ChangedIndices)
	{
		const FInvSS_GridSlotRenderSnapshot& OldSlot = LastRenderedSlots[Index];
		const FInvSS_GridSlotViewData& NewSlot = ViewData.Slots[Index];
		if (!OldSlot.bParentSlot) continue;

		const bool bSameItemAtParent =
			NewSlot.bParentSlot
			&& NewSlot.ItemInstanceId == OldSlot.ItemInstanceId
			&& IsValid(NewSlot.Item);
		if (bSameItemAtParent) continue;

		if (UInvSS_SlottedItem* ExistingWidget = SlottedItemsMap.FindRef(Index))
		{
			DetachedSlottedItems.Add({ OldSlot.ItemInstanceId, ExistingWidget });
			SlottedItemsMap.Remove(Index);
		}
	}

	// Occupancy changes only repaint the affected underlying grid slots.
	for (const int32 Index : ChangedIndices)
	{
		if (LastRenderedSlots[Index].bOccupied != ViewData.Slots[Index].bOccupied)
		{
			RenderGridSlotOccupancy(Index, ViewData.Slots[Index].bOccupied);
		}
	}

	// Update retained parents, reuse moved widgets by item ID, and create only genuinely new visuals.
	for (const int32 Index : ChangedIndices)
	{
		const FInvSS_GridSlotViewData& NewSlot = ViewData.Slots[Index];
		if (!NewSlot.bParentSlot || !IsValid(NewSlot.Item)) continue;

		if (UInvSS_SlottedItem* ExistingWidget = SlottedItemsMap.FindRef(Index))
		{
			const FInvSS_GridSlotRenderSnapshot& OldSlot = LastRenderedSlots[Index];
			if (OldSlot.ItemInstanceId == NewSlot.ItemInstanceId
				&& OldSlot.Item.Get() == NewSlot.Item.Get())
			{
				// A retained item with the same object only needs its stack text refreshed.
				ExistingWidget->UpdateStackCountText(
					NewSlot.Item->IsStackable() ? NewSlot.StackCount : 0);
				continue;
			}

			if (!TryConfigureSlottedItem(
				ExistingWidget,
				NewSlot.Item,
				Index,
				NewSlot.Item->IsStackable(),
				NewSlot.StackCount))
			{
				RemoveSlottedItemAtIndex(Index);
			}
			continue;
		}

		const int32 ReusableIndex = DetachedSlottedItems.IndexOfByPredicate(
			[&NewSlot](const FDetachedSlottedItem& DetachedItem)
			{
				return DetachedItem.ItemInstanceId == NewSlot.ItemInstanceId
					&& IsValid(DetachedItem.Widget);
			});

		if (ReusableIndex != INDEX_NONE)
		{
			UInvSS_SlottedItem* ReusedWidget = DetachedSlottedItems[ReusableIndex].Widget;
			DetachedSlottedItems.RemoveAtSwap(ReusableIndex);

			if (TryConfigureSlottedItem(
				ReusedWidget,
				NewSlot.Item,
				Index,
				NewSlot.Item->IsStackable(),
				NewSlot.StackCount))
			{
				SlottedItemsMap.Add(Index, ReusedWidget);
			}
			else
			{
				ReusedWidget->RemoveFromParent();
			}
			continue;
		}

		AddItemAtIndex(
			NewSlot.Item,
			Index,
			NewSlot.Item->IsStackable(),
			NewSlot.StackCount);
	}

	// Widgets left detached belong to items that were removed rather than moved.
	for (const FDetachedSlottedItem& DetachedItem : DetachedSlottedItems)
	{
		if (IsValid(DetachedItem.Widget))
		{
			DetachedItem.Widget->RemoveFromParent();
		}
	}
}

bool UInvSS_InventoryGrid::HasSlotChanged(
	const FInvSS_GridSlotRenderSnapshot& OldSlot,
	const FInvSS_GridSlotViewData& NewSlot)
{
	return OldSlot.ItemInstanceId != NewSlot.ItemInstanceId
		|| OldSlot.Item.Get() != NewSlot.Item.Get()
		|| OldSlot.StackCount != NewSlot.StackCount
		|| OldSlot.bOccupied != NewSlot.bOccupied
		|| OldSlot.bParentSlot != NewSlot.bParentSlot;
}

void UInvSS_InventoryGrid::CacheRenderedViewData(const FInvSS_InventoryGridViewData& ViewData)
{
	LastRenderedSlots.SetNum(ViewData.Slots.Num());

	for (int32 Index = 0; Index < ViewData.Slots.Num(); ++Index)
	{
		const FInvSS_GridSlotViewData& Source = ViewData.Slots[Index];
		FInvSS_GridSlotRenderSnapshot& Destination = LastRenderedSlots[Index];

		Destination.ItemInstanceId = Source.ItemInstanceId;
		Destination.Item = Source.Item;
		Destination.StackCount = Source.StackCount;
		Destination.bOccupied = Source.bOccupied;
		Destination.bParentSlot = Source.bParentSlot;
	}

	bHasRenderedViewData = true;
}

void UInvSS_InventoryGrid::ClearVisualItems()
{
	for (const TPair<int32, TObjectPtr<UInvSS_SlottedItem>>& Pair : SlottedItemsMap)
	{
		if (IsValid(Pair.Value.Get()))
		{
			Pair.Value->RemoveFromParent();
		}
	}
	SlottedItemsMap.Empty();

	for (UInvSS_GridSlot* GridSlot : GridSlots)
	{
		if (!GridSlot) continue;

		GridSlot->SetBaseVisualState(EInvSS_GridSlotVisualState::Unoccupied);
	}
}

void UInvSS_InventoryGrid::RemoveSlottedItemAtIndex(const int32 ParentSlotIndex)
{
	if (UInvSS_SlottedItem* SlottedItem = SlottedItemsMap.FindRef(ParentSlotIndex))
	{
		SlottedItem->RemoveFromParent();
	}
	SlottedItemsMap.Remove(ParentSlotIndex);
}

void UInvSS_InventoryGrid::RenderGridSlotOccupancy(const int32 SlotIndex, const bool bOccupied)
{
	if (!GridSlots.IsValidIndex(SlotIndex)) return;

	UInvSS_GridSlot* GridSlot = GridSlots[SlotIndex];
	if (!IsValid(GridSlot)) return;

	if (bOccupied)
	{
		GridSlot->SetBaseVisualState(EInvSS_GridSlotVisualState::Occupied);
		return;
	}

	GridSlot->SetBaseVisualState(EInvSS_GridSlotVisualState::Unoccupied);
}

void UInvSS_InventoryGrid::RenderSlotViewData(
	const int32 SlotIndex,
	const FInvSS_GridSlotViewData& SlotViewData)
{
	RenderGridSlotOccupancy(SlotIndex, SlotViewData.bOccupied);

	if (SlotViewData.bOccupied
		&& SlotViewData.bParentSlot
		&& IsValid(SlotViewData.Item))
	{
		AddItemAtIndex(
			SlotViewData.Item,
			SlotIndex,
			SlotViewData.Item->IsStackable(),
			SlotViewData.StackCount);
	}
}

void UInvSS_InventoryGrid::AddItemAtIndex(
	UInvSS_InventoryItem* InItem,
	const int32 InSlotIndex,
	const bool bInIsStackable,
	const int32 InStackCount)
{
	UInvSS_SlottedItem* SlottedItem = CreateSlottedItem();
	if (!TryConfigureSlottedItem(SlottedItem, InItem, InSlotIndex, bInIsStackable, InStackCount))
	{
		SlottedItem->RemoveFromParent();
		return;
	}

	SlottedItemsMap.Add(InSlotIndex, SlottedItem);
}

UInvSS_SlottedItem* UInvSS_InventoryGrid::CreateSlottedItem() const
{
	checkf(SlottedItemClass, TEXT("SlottedItemClass is not set on %s."), *GetName());

	UInvSS_SlottedItem* SlottedItem = CreateWidget<UInvSS_SlottedItem>(GetOwningPlayer(), SlottedItemClass);
	SlottedItem->SetWidgetController(GetWidgetController());
	check(SlottedItem);
	SlottedItem->OnSlottedItemClickedDelegate.AddDynamic(this, &ThisClass::OnSlottedItemClickedCallback);
	return SlottedItem;
}

bool UInvSS_InventoryGrid::TryConfigureSlottedItem(
	UInvSS_SlottedItem* InSlottedItem,
	UInvSS_InventoryItem* InItem,
	const int32 InSlotIndex,
	const bool bInIsStackable,
	const int32 InStackCount) const
{
	if (!IsValid(InSlottedItem) || !IsValid(InItem)) return false;

	const FInvSS_GridFragment* GridFragment = GetFragment<FInvSS_GridFragment>(InItem, ItemFragmentTag::GridFragment);
	const FInvSS_ImageFragment* ImageFragment = GetFragment<FInvSS_ImageFragment>(InItem, ItemFragmentTag::IconFragment);
	if (!GridFragment || !ImageFragment) return false;

	InSlottedItem->SetInventoryItem(InItem);
	SetSlottedItemIconBrush(GridFragment, ImageFragment, InSlottedItem);
	InSlottedItem->SetGridIndex(InSlotIndex);
	InSlottedItem->SetWidgetController(GetWidgetController());
	InSlottedItem->SetGridDimensions(GridFragment->GetGridSize());
	InSlottedItem->SetIsStackable(bInIsStackable);
	InSlottedItem->UpdateStackCountText(bInIsStackable ? InStackCount : 0);
	PositionSlottedItem(InSlotIndex, GridFragment, InSlottedItem);
	return true;
}

void UInvSS_InventoryGrid::PositionSlottedItem(
	const int32 InIndex,
	const FInvSS_GridFragment* InGridFragment,
	UInvSS_SlottedItem* InSlottedItem) const
{
	UCanvasPanelSlot* GridCps = UWidgetLayoutLibrary::SlotAsCanvasSlot(InSlottedItem);
	if (!GridCps)
	{
		GridCps = GridCanvasPanel->AddChildToCanvas(InSlottedItem);
	}

	GridCps->SetSize(GetDrawSize(InGridFragment));
	const FVector2D DrawPos = UInvSS_WidgetUtils::GetPositionFromIndex(InIndex, RenderedGridColumns) * TileSize;
	const FVector2D DrawPosWithPadding = DrawPos + FVector2D(InGridFragment->GetGridPadding());
	GridCps->SetPosition(DrawPosWithPadding);
}

void UInvSS_InventoryGrid::SetSlottedItemIconBrush(
	const FInvSS_GridFragment* InGridFragment,
	const FInvSS_ImageFragment* InImageFragment,
	UInvSS_SlottedItem* InSlottedItem) const
{
	FSlateBrush Brush;
	Brush.SetResourceObject(InImageFragment->GetImageResource());
	Brush.DrawAs = ESlateBrushDrawType::Image;
	Brush.ImageSize = GetDrawSize(InGridFragment);
	InSlottedItem->SetIconImageBrush(Brush);
}

FVector2D UInvSS_InventoryGrid::GetDrawSize(const FInvSS_GridFragment* GridFragment) const
{
	const float IconTileWidth = TileSize - GridFragment->GetGridPadding() * 2.f;
	return GridFragment->GetGridSize() * IconTileWidth;
}
