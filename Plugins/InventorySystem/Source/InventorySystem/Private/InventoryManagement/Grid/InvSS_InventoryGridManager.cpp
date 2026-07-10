// @Copyright HaolunYuan

#include "InventoryManagement/Grid/InvSS_InventoryGridManager.h"

#include "InventoryManagement/Component/InvSS_InventoryComponent.h"
#include "InventoryManagement/Utils/InvSS_InventoryStatics.h"
#include "Item/InvSS_InventoryItem.h"
#include "Item/Fragment/InvSS_ItemFragment.h"
#include "Item/Fragment/InvSS_ItemFragmentTag.h"
#include "Net/UnrealNetwork.h"
#include "Widgets/Utilis/InvSS_WidgetUtils.h"

int32 FInvSS_GridConfig::NumSlots() const
{
	return Rows * Columns;
}

bool FInvSS_GridSlotState::OccupiedByItem() const
{
	return ID_InventoryItemAtThisSlot.IsValid();
}

void FInvSS_GridSlotState::Reset()
{
	ID_InventoryItemAtThisSlot.Invalidate();
	ParentIndex = INDEX_NONE;
	SlotStackCount = 0;
}

bool UInvSS_InventoryGridManager::IsSupportedForNetworking() const
{
	return true;
}

void UInvSS_InventoryGridManager::GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const
{
	UObject::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME(ThisClass, GridStatesArray);
}

void UInvSS_InventoryGridManager::Initialize(const TArray<FInvSS_GridConfig>& InConfigArray)
{
	check(InConfigArray.Num() > 0);

	for (const FInvSS_GridConfig& Config : InConfigArray)
	{
		FInvSS_InventoryGridState& GridState = GridStatesArray.AddDefaulted_GetRef();
		GridState.GridConfiguration = Config;
		GridState.ReplicationRevision = 0;
		GridState.SlotStates.SetNum(Config.NumSlots());
	}
}

UInvSS_InventoryComponent* UInvSS_InventoryGridManager::GetInventoryComponent() const
{
	return GetTypedOuter<UInvSS_InventoryComponent>();
}

bool UInvSS_InventoryGridManager::TryAddItemAtGivenIndex(
	const EInvSS_ItemCategory ItemCategory,
	const int32 Index,
	const FGuid& ItemGuid,
	const int32 StackCount)
{
	if (!ItemGuid.IsValid()) return false;
	if (Index == INDEX_NONE) return false;

	UInvSS_InventoryComponent* InventoryComponent = GetInventoryComponent();
	check(InventoryComponent);

	UInvSS_InventoryItem* Item = InventoryComponent->GetMutableInventoryItemByID(ItemGuid);
	if (!IsValid(Item)) return false;
	if (Item->GetItemManifest().GetItemCategory() != ItemCategory) return false;
	if (Item->IsStackable() && StackCount <= 0) return false;

	FInvSS_InventoryGridState* GridState = GetMutableGridState(ItemCategory);
	if (!GridState) return false;

	const FIntPoint ItemGridDimensions = GetItemGridDimensions(Item->GetItemManifest());
	if (!CanPlaceItemAtEmptyIndex(*GridState, Index, ItemGridDimensions)) return false;

	WriteItemAtIndex(
		*GridState,
		Index,
		ItemGuid,
		ItemGridDimensions,
		Item->IsStackable() ? StackCount : 0);

	MarkGridStateDirty(*GridState);
	OnGridChanged.Broadcast(ItemCategory);
	return true;
}

bool UInvSS_InventoryGridManager::CanPlaceItemAtEmptyIndex(
	const FInvSS_InventoryGridState& GridState,
	const int32 Index,
	const FIntPoint& ItemGridDimensions)
{
	const FIntPoint GridDimensions(GridState.GridConfiguration.Columns, GridState.GridConfiguration.Rows);
	if (!UInvSS_WidgetUtils::IsRangeInGridBounds(Index, GridDimensions, ItemGridDimensions)) return false;

	bool bCanPlaceItem = true;
	UInvSS_InventoryStatics::ForEach2D(
		GridState.SlotStates,
		Index,
		GridState.GridConfiguration.Columns,
		ItemGridDimensions,
		[&bCanPlaceItem](const FInvSS_GridSlotState& SlotState)
		{
			if (SlotState.OccupiedByItem())
			{
				bCanPlaceItem = false;
			}
		});

	return bCanPlaceItem;
}

void UInvSS_InventoryGridManager::WriteItemAtIndex(
	FInvSS_InventoryGridState& GridState,
	const int32 Index,
	const FGuid& ItemGuid,
	const FIntPoint& ItemGridDimensions,
	const int32 StackCount)
{
	check(ItemGuid.IsValid());
	check(GridState.SlotStates.IsValidIndex(Index));

	// The parent slot stores stack count; every covered slot points back to that parent.
	UInvSS_InventoryStatics::ForEach2D(
		GridState.SlotStates,
		Index,
		GridState.GridConfiguration.Columns,
		ItemGridDimensions,
		[&ItemGuid, Index](FInvSS_GridSlotState& SlotState)
		{
			SlotState.ID_InventoryItemAtThisSlot = ItemGuid;
			SlotState.ParentIndex = Index;
			SlotState.SlotStackCount = 0;
		});

	GridState.SlotStates[Index].SlotStackCount = StackCount;
}

bool UInvSS_InventoryGridManager::FindRoomForItem(
	const FInvSS_ItemManifest& InItemManifest,
	UInvSS_InventoryItem* ExistingStackItem,
	FInvSS_BagAvailabilityResult& OutResult) const
{
	OutResult = FInvSS_BagAvailabilityResult();
	OutResult.Item = ExistingStackItem;

	const FInvSS_StackableFragment* StackableFragment = InItemManifest.GetFragmentOfTypeWithTag<FInvSS_StackableFragment>(ItemFragmentTag::StackableFragment);
	OutResult.bStackable = StackableFragment != nullptr;

	const int32 MaxStackSize = StackableFragment ? StackableFragment->GetMaxStackSize() : 1;
	int32 AmountToFill = StackableFragment ? StackableFragment->GetStackCount() : 1;
	const FIntPoint ItemGridDimensions = GetItemGridDimensions(InItemManifest);

	const EInvSS_ItemCategory ItemCategory = InItemManifest.GetItemCategory();
	const TArray<FInvSS_GridSlotState>* SlotsStateArray = GetSlotStateArrayByType(ItemCategory);
	const FInvSS_GridConfig* GridConfig = GetGridConfigByType(ItemCategory);
	checkf(SlotsStateArray && GridConfig, TEXT("No matching Inventory Grid for Item Category: %s "), *UEnum::GetValueAsString(ItemCategory));
	const FIntPoint GridDimensions(GridConfig->Columns, GridConfig->Rows);

	TSet<int32> CheckedSlotIndices;
	for (int32 SlotIndex = 0; SlotIndex < SlotsStateArray->Num(); ++SlotIndex)
	{
		// Stack count is not valid
		if (AmountToFill == 0) break;

		// if already checked, skip them
		if (CheckedSlotIndices.Contains(SlotIndex)) continue;

		// valid stack count, new slot, but no enough room for the item
		if (!UInvSS_WidgetUtils::IsRangeInGridBounds(SlotIndex, GridDimensions, ItemGridDimensions)) continue;

		// valid position, now start to check each slot in the range
		TSet<int32> TentativelyClaimedIndices;
		if (!HasRoomAtIndex(*SlotsStateArray, *GridConfig,SlotIndex, ItemGridDimensions,
			CheckedSlotIndices,  InItemManifest.GetItemTypeTag(), MaxStackSize,
			TentativelyClaimedIndices))
		{
			continue;
		}

		const int32 AmountToFillAtSlot = DetermineFillAmount(*SlotsStateArray, SlotIndex, MaxStackSize, OutResult.bStackable, AmountToFill);
		if (AmountToFillAtSlot == 0) continue;

		CheckedSlotIndices.Append(TentativelyClaimedIndices);

		const bool bSlotHasItem = (*SlotsStateArray)[SlotIndex].OccupiedByItem();
		OutResult.TotalRoomToFill += AmountToFillAtSlot;
		OutResult.SlotAvailability.Emplace(
			bSlotHasItem ? (*SlotsStateArray)[SlotIndex].ParentIndex : SlotIndex,
			OutResult.bStackable ? AmountToFillAtSlot : 0,
			bSlotHasItem
		);

		AmountToFill -= AmountToFillAtSlot;
		OutResult.Remainder = AmountToFill;
	}

	OutResult.Remainder = AmountToFill;
	return OutResult.TotalRoomToFill > 0;
}

FIntPoint UInvSS_InventoryGridManager::GetItemGridDimensions(const FInvSS_ItemManifest& InItemManifest)
{
	const FInvSS_GridFragment* GridFragment = InItemManifest.GetFragmentOfTypeWithTag<FInvSS_GridFragment>(ItemFragmentTag::GridFragment);
	return GridFragment ? GridFragment->GetGridSize() : FIntPoint(1, 1);
}

bool UInvSS_InventoryGridManager::HasRoomAtIndex(
	const TArray<FInvSS_GridSlotState>& SlotStateArray,
	const FInvSS_GridConfig& GridConfig,
	const int32 StartIndex,
	const FIntPoint& GridDimensions,
	const TSet<int32>& CheckedSlotIndices,
	const FGameplayTag& ItemTypeTag,
	const int32 MaxStackSize,
	TSet<int32>& OutTentativeIndices
	) const
{
	bool bHasRoomAtIndex = true;

	UInvSS_InventoryStatics::ForEach2D(SlotStateArray, StartIndex, GridConfig.Columns, GridDimensions,
		[&](const FInvSS_GridSlotState&, const int32 SubIndex)
	{
		if (CheckSlotConstraints(SlotStateArray, StartIndex, SubIndex, CheckedSlotIndices, ItemTypeTag, MaxStackSize))
		{
			OutTentativeIndices.Add(SubIndex);
		}
		else
		{
			bHasRoomAtIndex = false;
		}
	});

	return bHasRoomAtIndex;
}

bool UInvSS_InventoryGridManager::CheckSlotConstraints(
	const TArray<FInvSS_GridSlotState>& SlotStateArray,
	const int32 StartIndex,
	const int32 SubIndex,
	const TSet<int32>& CheckedSlotIndices,
	const FGameplayTag& ItemTypeTag,
	const int32 MaxStackSize) const
{
	const UInvSS_InventoryComponent* InventoryComponent = GetInventoryComponent();
	check(InventoryComponent);
	if (CheckedSlotIndices.Contains(SubIndex)) return false;
	if (!SlotStateArray.IsValidIndex(SubIndex)) return false;
	if (!SlotStateArray[SubIndex].OccupiedByItem()) return true;
	if (SlotStateArray[SubIndex].ParentIndex != StartIndex) return false;

	const FGuid& ID_InventoryItemAtSubIndex =  SlotStateArray[SubIndex].ID_InventoryItemAtThisSlot;
	const UInvSS_InventoryItem* Item = InventoryComponent->GetInventoryItemByID(ID_InventoryItemAtSubIndex);
	if (!IsValid(Item) || !Item->IsStackable()) return false;
	if (!Item->GetItemManifest().GetItemTypeTag().MatchesTagExact(ItemTypeTag)) return false;
	if (GetSlotStackAmount(SlotStateArray, SubIndex) >= MaxStackSize) return false;

	return true;
}

int32 UInvSS_InventoryGridManager::GetSlotStackAmount(const TArray<FInvSS_GridSlotState>& SlotStateArray,const int32 SlotIndex)
{
	if (!SlotStateArray.IsValidIndex(SlotIndex)) return 0;

	const int32 ParentIndex = SlotStateArray[SlotIndex].ParentIndex;
	if (ParentIndex != INDEX_NONE && SlotStateArray.IsValidIndex(ParentIndex))
	{
		return SlotStateArray[ParentIndex].SlotStackCount;
	}

	return SlotStateArray[SlotIndex].SlotStackCount;
}

int32 UInvSS_InventoryGridManager::DetermineFillAmount(const TArray<FInvSS_GridSlotState>& SlotStateArray,
	const int32 SlotIndex, const int32 MaxStackSize, const bool bIsStackable, const int32 AmountToFill)
{
	const int32 AmountCanFill = MaxStackSize - GetSlotStackAmount(SlotStateArray, SlotIndex);
	return bIsStackable ? FMath::Min(AmountToFill, AmountCanFill) : 1;
}

void UInvSS_InventoryGridManager::ApplyAvailability(UInvSS_InventoryItem* InItem, const FInvSS_BagAvailabilityResult& InResult)
{
	if (!IsValid(InItem)) return;

	const EInvSS_ItemCategory ItemCategory = InItem->GetItemManifest().GetItemCategory();
	FInvSS_InventoryGridState* GridState = GetMutableGridState(ItemCategory);
	checkf(GridState, TEXT("No matching Inventory Grid for Item Category: %s "), *UEnum::GetValueAsString(ItemCategory));

	const FIntPoint ItemGridDimensions = GetItemGridDimensions(InItem->GetItemManifest());
	const FIntPoint GridDimensions(GridState->GridConfiguration.Columns, GridState->GridConfiguration.Rows);
	for (const FInvSS_InventorySlotAvailability& Availability : InResult.SlotAvailability)
	{
		const int32 ParentIndex = Availability.SlotIndex;
		checkf(GridState->SlotStates.IsValidIndex(ParentIndex), TEXT("Result contains  invalid availability data"));

		checkf(UInvSS_WidgetUtils::IsRangeInGridBounds(ParentIndex, GridDimensions, ItemGridDimensions),
			TEXT("Availability result contains an invalid item footprint."));

		const int32 NewStackCount = Availability.bItemAtIndex
			? GridState->SlotStates[ParentIndex].SlotStackCount + Availability.AmountToFill
			: Availability.AmountToFill;

		GridState->SlotStates[ParentIndex].ID_InventoryItemAtThisSlot = InItem->GetItemInstanceId();
		GridState->SlotStates[ParentIndex].ParentIndex = ParentIndex;
		if (InResult.bStackable)
		{
			GridState->SlotStates[ParentIndex].SlotStackCount = NewStackCount;
		}

		UInvSS_InventoryStatics::ForEach2D(GridState->SlotStates, ParentIndex, GridState->GridConfiguration.Columns, ItemGridDimensions, [&InItem, CapParentIndex = ParentIndex](FInvSS_GridSlotState& SubSlot)
		{
			SubSlot.ID_InventoryItemAtThisSlot = InItem->GetItemInstanceId();
			SubSlot.ParentIndex = CapParentIndex;
		});
	}

	MarkGridStateDirty(*GridState);
	OnGridChanged.Broadcast(ItemCategory);
}

bool UInvSS_InventoryGridManager::TryRemoveItemAtParentIndex(
	const EInvSS_ItemCategory Category,
	const int32 ParentIndex,
	FGuid& OutItemId,
	int32& OutStackCount)
{
	OutItemId.Invalidate();
	OutStackCount = 0;

	FInvSS_InventoryGridState* GridState = GetMutableGridState(Category);
	if (!GridState) return false;
	if (!GridState->SlotStates.IsValidIndex(ParentIndex)) return false;

	const FInvSS_GridSlotState& ParentSlot = GridState->SlotStates[ParentIndex];
	if (!ParentSlot.OccupiedByItem()) return false;
	if (ParentSlot.ParentIndex != ParentIndex) return false;

	OutItemId = ParentSlot.ID_InventoryItemAtThisSlot;
	OutStackCount = ParentSlot.SlotStackCount;

	check(OutItemId.IsValid());
	check(OutStackCount >= 0);

	// A cursor pickup changes spatial placement only. The inventory item remains owned by InventoryList.
	for (FInvSS_GridSlotState& SlotState : GridState->SlotStates)
	{
		if (SlotState.ID_InventoryItemAtThisSlot == OutItemId
			&& SlotState.ParentIndex == ParentIndex)
		{
			SlotState.Reset();
		}
	}

	MarkGridStateDirty(*GridState);
	OnGridChanged.Broadcast(Category);
	return true;
}

bool UInvSS_InventoryGridManager::TryGetStackCountAtParentIndex(
	const EInvSS_ItemCategory ItemCategory,
	const int32 ParentIndex,
	int32& OutStackCount) const
{
	OutStackCount = 0;

	const FInvSS_InventoryGridState* GridState = GetGridState(ItemCategory);
	if (!GridState) return false;
	if (!GridState->SlotStates.IsValidIndex(ParentIndex)) return false;

	const FInvSS_GridSlotState& ParentSlot = GridState->SlotStates[ParentIndex];
	if (!ParentSlot.OccupiedByItem()) return false;
	if (ParentSlot.ParentIndex != ParentIndex) return false;

	OutStackCount = ParentSlot.SlotStackCount;
	check(OutStackCount >= 0);
	return true;
}

bool UInvSS_InventoryGridManager::TryGetItemIdAtParentIndex(
	const EInvSS_ItemCategory ItemCategory,
	const int32 ParentIndex,
	FGuid& OutItemId) const
{
	OutItemId.Invalidate();

	const FInvSS_InventoryGridState* GridState = GetGridState(ItemCategory);
	if (!GridState) return false;
	if (!GridState->SlotStates.IsValidIndex(ParentIndex)) return false;

	const FInvSS_GridSlotState& ParentSlot = GridState->SlotStates[ParentIndex];
	if (!ParentSlot.OccupiedByItem()) return false;
	if (ParentSlot.ParentIndex != ParentIndex) return false;

	OutItemId = ParentSlot.ID_InventoryItemAtThisSlot;
	check(OutItemId.IsValid());
	return true;
}

bool UInvSS_InventoryGridManager::TryReplaceStackCountAtParentIndex(
	const EInvSS_ItemCategory ItemCategory,
	const int32 ParentIndex,
	const int32 NewStackCount,
	int32& OutPreviousStackCount)
{
	OutPreviousStackCount = 0;
	if (NewStackCount < 0) return false;

	FInvSS_InventoryGridState* GridState = GetMutableGridState(ItemCategory);
	if (!GridState) return false;
	if (!GridState->SlotStates.IsValidIndex(ParentIndex)) return false;

	FInvSS_GridSlotState& ParentSlot = GridState->SlotStates[ParentIndex];
	if (!ParentSlot.OccupiedByItem()) return false;
	if (ParentSlot.ParentIndex != ParentIndex) return false;

	OutPreviousStackCount = ParentSlot.SlotStackCount;
	ParentSlot.SlotStackCount = NewStackCount;

	MarkGridStateDirty(*GridState);
	OnGridChanged.Broadcast(ItemCategory);
	return true;
}

void UInvSS_InventoryGridManager::OnRep_GridStates()
{
	for (const FInvSS_InventoryGridState& GridState : GridStatesArray)
	{
		if (ShouldBroadcastReplicatedGridState(GridState))
		{
			OnGridChanged.Broadcast(GridState.GridConfiguration.Category);
		}
	}
}

bool UInvSS_InventoryGridManager::ShouldBroadcastReplicatedGridState(
	const FInvSS_InventoryGridState& GridState)
{
	const EInvSS_ItemCategory Category = GridState.GridConfiguration.Category;
	const uint32 NewRevision = GridState.ReplicationRevision;

	const uint32* LastSeenRevision = LastSeenGridRevisions.Find(Category);
	if (LastSeenRevision && *LastSeenRevision == NewRevision)
	{
		return false;
	}

	LastSeenGridRevisions.Add(Category, NewRevision);
	return true;
}

void UInvSS_InventoryGridManager::MarkGridStateDirty(FInvSS_InventoryGridState& GridState)
{
	++GridState.ReplicationRevision;
}

const FInvSS_InventoryGridState* UInvSS_InventoryGridManager::GetGridState(const EInvSS_ItemCategory Category) const
{
	check(GridStatesArray.Num() > 0);
	for (const auto& GridState: GridStatesArray)
	{
		if (GridState.GridConfiguration.Category == Category)
		{
			return &GridState;
		}
	}
	return nullptr;
}

FInvSS_InventoryGridState* UInvSS_InventoryGridManager::GetMutableGridState(const EInvSS_ItemCategory Category)
{
	check(GridStatesArray.Num() > 0);
	for (auto& GridState: GridStatesArray)
	{
		if (GridState.GridConfiguration.Category == Category)
		{
			return &GridState;
		}
	}
	return nullptr;
}

const TArray<FInvSS_GridSlotState>* UInvSS_InventoryGridManager::GetSlotStateArrayByType(
	const EInvSS_ItemCategory Category) const
{
	const FInvSS_InventoryGridState* GridState = GetGridState(Category);
	return GridState ? &GridState->SlotStates : nullptr;
}

TArray<FInvSS_GridSlotState>* UInvSS_InventoryGridManager::GetMutableSlotStateArrayByType(
	const EInvSS_ItemCategory Category)
{
	FInvSS_InventoryGridState* GridState = GetMutableGridState(Category);
	return GridState ? &GridState->SlotStates : nullptr;
}

const FInvSS_GridConfig* UInvSS_InventoryGridManager::GetGridConfigByType(
	const EInvSS_ItemCategory Category) const
{
	const FInvSS_InventoryGridState* GridState = GetGridState(Category);
	return GridState ? &GridState->GridConfiguration : nullptr;
}

const TArray<FInvSS_InventoryGridState>& UInvSS_InventoryGridManager::GetGridStatesArray() const
{
	return GridStatesArray;
}
