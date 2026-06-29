// @Copyright HaolunYuan


#include "Widgets/WidgetController/InvSS_InventoryWidgetController.h"

#include "InventoryManagement/Component/InvSS_InventoryComponent.h"
#include "InventoryManagement/Grid/InvSS_InventoryGridManager.h"
#include "InventoryManagement/Utils/InvSS_InventoryStatics.h"
#include "Item/InvSS_InventoryItem.h"
#include "Widgets/HUD/InvSS_InventoryUIManager.h"
#include "Widgets/Utilis/InvSS_WidgetUtils.h"

void UInvSS_InventoryWidgetController::BroadcastInitialValues()
{

}

void UInvSS_InventoryWidgetController::BindAllDependencies()
{
	check(CachedInventoryComponent.Get());

	// Pipeline:
	// 1. Listen to inventory component data/model events.
	// 2. Re-broadcast UI-facing events through this controller.
	// 3. Let widgets bind only to controller delegates.
	CachedInventoryComponent->OnInventoryItemAddedDelegate.AddUniqueDynamic(this, &ThisClass::HandleInventoryItemAdded);
	CachedInventoryComponent->OnInventoryItemRemovedDelegate.AddUniqueDynamic(this, &ThisClass::HandleInventoryItemRemoved);
	CachedInventoryComponent->OnInventoryMessageRequestedDelegate.AddUniqueDynamic(this, &ThisClass::HandleInventoryMessageRequested);
	CachedInventoryComponent->OnInventoryGridChangedDelegate.AddUniqueDynamic(this, &ThisClass::HandleInventoryGridChanged);
	CachedInventoryComponent->OnHeldItemChangedDelegate.AddUniqueDynamic(this, &ThisClass::HandleInventoryHeldItemChanged);
}

const FInvSS_InventoryGridViewData* UInvSS_InventoryWidgetController::GetCachedGridViewData(
	const EInvSS_ItemCategory ItemCategory)
{
	const bool bNeedsRebuild =
		DirtyGridCategories.Contains(ItemCategory)
		|| !CachedGridViewData.Contains(ItemCategory);

	if (bNeedsRebuild)
	{
		if (!RebuildGridViewData(ItemCategory))
		{
			CachedGridViewData.Remove(ItemCategory);
			DirtyGridCategories.Remove(ItemCategory);
			return nullptr;
		}

		DirtyGridCategories.Remove(ItemCategory);
	}

	return CachedGridViewData.Find(ItemCategory);
}

bool UInvSS_InventoryWidgetController::GetGridViewData(
	const EInvSS_ItemCategory ItemCategory,
	FInvSS_InventoryGridViewData& OutViewData)
{
	const FInvSS_InventoryGridViewData* ViewData = GetCachedGridViewData(ItemCategory);
	if (!ViewData)
	{
		OutViewData = FInvSS_InventoryGridViewData();
		return false;
	}

	OutViewData = *ViewData;
	return true;
}

UInvSS_InventoryItem* UInvSS_InventoryWidgetController::GetInventoryItemByID(const FGuid& ItemId) const
{
	UInvSS_InventoryComponent* InventoryComponent = CachedInventoryComponent.Get();
	if (!IsValid(InventoryComponent)) return nullptr;

	return InventoryComponent->GetMutableInventoryItemByID(ItemId);
}

void UInvSS_InventoryWidgetController::RequestBeginDragItem(
	const EInvSS_ItemCategory ItemCategory,
	const int32 ParentIndex)
{
	UInvSS_InventoryComponent* InventoryComponent = CachedInventoryComponent.Get();
	check(InventoryComponent);

	InventoryComponent->Server_BeginDragItem(ItemCategory, ParentIndex);
}


void UInvSS_InventoryWidgetController::RequestPutDownHeldITemAtIndex(const EInvSS_ItemCategory ItemCategory , const int32 ItemParentIndex)
{
	UInvSS_InventoryComponent* InventoryComponent = CachedInventoryComponent.Get();
	check(InventoryComponent);
	
	InventoryComponent->Server_PutDownHeldItemAtIndex(ItemCategory, ItemParentIndex);
}

void UInvSS_InventoryWidgetController::RequestInteractHeldItemWithItemUnderCursor(
	const EInvSS_ItemCategory ItemCategory,
	const FGuid& ItemID,
	const int32 ItemParentIndex,
	const int32 HeldItemDropIndex)
{
	UInvSS_InventoryComponent* InventoryComponent = CachedInventoryComponent.Get();
	check(InventoryComponent);
	
	InventoryComponent->Server_RequestHeldItemInteractWithItemUnderCursor(ItemCategory, ItemID, ItemParentIndex, HeldItemDropIndex);
}


void UInvSS_InventoryWidgetController::QueryGridSpace(const EInvSS_ItemCategory ItemCategory, const int32 StartIndex,
                                                      const FIntPoint& ItemDimensions, FInvSS_SpaceQueryResult& OutResult)
{

	const FInvSS_InventoryGridViewData* ViewData = GetCachedGridViewData(ItemCategory);
	if (!ViewData) return;

	// if the item is placed is in bound
	if (!UInvSS_WidgetUtils::IsRangeInGridBounds( StartIndex, { ViewData->Columns, ViewData->Rows }, ItemDimensions)) { return; }
	OutResult.bHasSpace = true;

	TSet<int32> BlockingParentIndices;
	UInvSS_InventoryStatics::ForEach2D( ViewData->Slots, StartIndex, ViewData->Columns, ItemDimensions, 
		[&](const FInvSS_GridSlotViewData& SlotViewData, const int32 SlotIndex) 
		{
			if (!SlotViewData.bOccupied) return;

			BlockingParentIndices.Add( SlotViewData.ParentIndex);
			OutResult.bHasSpace = false;
			
		});

	if (BlockingParentIndices.Num() == 1)
	{
		const int32 BlockingParentIndex = *BlockingParentIndices.CreateConstIterator();
		check(ViewData->Slots.IsValidIndex(BlockingParentIndex));

		OutResult.ItemParentIndex = BlockingParentIndex;
		OutResult.ValidItem = ViewData->Slots[BlockingParentIndex].Item;
	}
}

bool UInvSS_InventoryWidgetController::RebuildGridViewData(
	const EInvSS_ItemCategory ItemCategory)
{
	FInvSS_InventoryGridViewData NewViewData;

	UInvSS_InventoryComponent* InventoryComponent = CachedInventoryComponent.Get();
	if (!IsValid(InventoryComponent))
	{
		return false;
	}

	const UInvSS_InventoryGridManager* GridManager = InventoryComponent->TryGetGridManager();
	if (!IsValid(GridManager))
	{
		return false;
	}

	const FInvSS_InventoryGridState* GridState = GridManager->GetGridState(ItemCategory);
	if (!GridState)
	{
		return false;
	}

	NewViewData.Rows = GridState->GridConfiguration.Rows;
	NewViewData.Columns = GridState->GridConfiguration.Columns;
	NewViewData.Slots.Reserve(GridState->SlotStates.Num());

	for (int32 SlotIndex = 0; SlotIndex < GridState->SlotStates.Num(); ++SlotIndex)
	{
		const FInvSS_GridSlotState& SlotState = GridState->SlotStates[SlotIndex];
		FInvSS_GridSlotViewData& SlotViewData = NewViewData.Slots.AddDefaulted_GetRef();

		SlotViewData.ItemInstanceId = SlotState.ID_InventoryItemAtThisSlot;
		SlotViewData.ParentIndex = SlotState.ParentIndex;
		SlotViewData.bOccupied = SlotState.OccupiedByItem();
		SlotViewData.bParentSlot = SlotViewData.bOccupied && SlotState.ParentIndex == SlotIndex;
		SlotViewData.StackCount = SlotState.SlotStackCount;

		// Only parent slots render an item widget, so child footprint slots do not need an item lookup.
		if (SlotViewData.bParentSlot)
		{
			SlotViewData.Item = InventoryComponent->GetMutableInventoryItemByID(
				SlotViewData.ItemInstanceId);
		}
	}

	CachedGridViewData.Add(ItemCategory, MoveTemp(NewViewData));
	return true;
}

void UInvSS_InventoryWidgetController::InvalidateGridViewData(
	const EInvSS_ItemCategory ItemCategory)
{
	DirtyGridCategories.Add(ItemCategory);
}

void UInvSS_InventoryWidgetController::InvalidateAllGridViewData()
{
	CachedGridViewData.Empty();
	DirtyGridCategories.Empty();
}

void UInvSS_InventoryWidgetController::HandleInventoryItemAdded(UInvSS_InventoryItem* InItem) 
{
	check(InItem);

	const EInvSS_ItemCategory ItemCategory = InItem->GetItemManifest().GetItemCategory();
	InvalidateGridViewData(ItemCategory);

	OnInventoryItemAdded.Broadcast(InItem);
	OnInventoryGridChanged.Broadcast(ItemCategory);
}

void UInvSS_InventoryWidgetController::HandleInventoryItemRemoved(UInvSS_InventoryItem* InItem)
{
	check(InItem);

	const EInvSS_ItemCategory ItemCategory = InItem->GetItemManifest().GetItemCategory();
	InvalidateGridViewData(ItemCategory);

	OnInventoryItemRemoved.Broadcast(InItem);
	OnInventoryGridChanged.Broadcast(ItemCategory);
}

void UInvSS_InventoryWidgetController::HandleInventoryMessageRequested(const FText& InMessage) 
{
	GetUIManager()->ShowPopUpMessageWidget(InMessage);
}

void UInvSS_InventoryWidgetController::HandleInventoryGridChanged(EInvSS_ItemCategory GridCategory) 
{
	InvalidateGridViewData(GridCategory);
	OnInventoryGridChanged.Broadcast(GridCategory);
}

void UInvSS_InventoryWidgetController::HandleInventoryHeldItemChanged(FInvSS_HeldItemState HeldItemState)
{
	OnInventoryHeldItemChanged.Broadcast(HeldItemState);
}
