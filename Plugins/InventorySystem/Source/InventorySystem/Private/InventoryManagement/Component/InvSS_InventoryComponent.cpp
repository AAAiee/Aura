// @Copyright HaolunYuan


#include "InventoryManagement/Component/InvSS_InventoryComponent.h"

#include "Item/InvSS_InventoryItem.h"
#include "Item/InvSS_ItemComponent.h"
#include "Item/Fragment/InvSS_ItemFragment.h"
#include "Item/Fragment/InvSS_ItemFragmentTag.h"
#include "GameFramework/Pawn.h"
#include "Kismet/GameplayStatics.h"
#include "Net/UnrealNetwork.h"
#include "Type/InvSS_GridTypes.h"
#include "Widgets/HoverItem/InvSS_HoverItem.h"
#include "Widgets/HUD/InvSS_InventoryUIManager.h"


UInvSS_InventoryComponent::UInvSS_InventoryComponent()
	: InventoryList(this)
{
	PrimaryComponentTick.bCanEverTick = false;
	SetIsReplicatedByDefault(true);
	bReplicateUsingRegisteredSubObjectList = true;
}

void UInvSS_InventoryComponent::ReadyForReplication()
{
	Super::ReadyForReplication();
	
	if (!GetOwner() || !GetOwner()->HasAuthority()) return;
	
	GetOrCreateGridManager();
	AddRepSubObj(InventoryGridManager);
}

void UInvSS_InventoryComponent::TryAddItem(UInvSS_ItemComponent* InItem)
{
	check(InItem);

	// Pipeline:
	// 1. Validate authority and find available room in the category grid.
	// 2. Apply authoritative item and grid state on the server.
	// 3. Notify local host UI or mirror stack-only grid changes to the owning client.
	if (AActor* Owner = GetOwner(); !IsValid(Owner) || !Owner->HasAuthority())
	{
		return;
	}

	/*Situation: Inventory is full*/
	FInvSS_BagAvailabilityResult Result;
	if (!TryFindRoomForItem(InItem, Result))
	{
		Client_ShowInventoryMessage(FText::FromString(TEXT("Inventory is Full!")));
		return;
	}

	UInvSS_InventoryGridManager* GridManager = GetOrCreateGridManager(); 
	check(GridManager);

	// if item in result already exists and it is stackable
	if (IsValid(Result.Item.Get()) && Result.bStackable)
	{
		UInvSS_InventoryItem* StackItem = Result.Item.Get();
		StackItem->SetItemTotalStackCountInBag(StackItem->GetItemTotalStackCountInBag() + Result.TotalRoomToFill);
		GridManager->ApplyAvailability(StackItem, Result);
	}
	else
	{
		UInvSS_InventoryItem* NewItem = InventoryList.AddEntry(InItem);
		check(NewItem);
		const int32 StackCount = Result.bStackable ? Result.TotalRoomToFill : 0;
		NewItem->SetItemTotalStackCountInBag(StackCount);
		//SetInventoryItemStackCount(NewItem, StackCount);
		Result.Item = NewItem;
		GridManager->ApplyAvailability(NewItem, Result);
		
		if (OwningPlayerController.IsValid() && OwningPlayerController->IsLocalController())
		{
			OnInventoryItemAddedDelegate.Broadcast(NewItem);
		}
	}

	HandlePickupSourceAfterAdd(InItem, Result.Remainder);
}

void UInvSS_InventoryComponent::UpdateHeldItemStateFromItem(
	const EInvSS_ItemCategory ItemCategory,
	const int32 ItemParentIndex,
	const FGuid InventoryItemId,
	const int32 StackCount)
{
	HeldItemState.ItemId = InventoryItemId;
	HeldItemState.SourceCategory = ItemCategory;
	HeldItemState.SourceParentIndex = ItemParentIndex;
	HeldItemState.StackCount = StackCount;

	NotifyHeldItemStateChanged();
}

void UInvSS_InventoryComponent::ResetHeldItemState()
{
	HeldItemState.Reset();
	NotifyHeldItemStateChanged();
}

void UInvSS_InventoryComponent::NotifyHeldItemStateChanged()
{
	if (OwningPlayerController.IsValid() && OwningPlayerController->IsLocalController())
	{
		OnHeldItemChangedDelegate.Broadcast(HeldItemState);
	}
}

void UInvSS_InventoryComponent::Server_BeginDragItem_Implementation(
	const EInvSS_ItemCategory ItemCategory,
	const int32 ItemParentIndex)
{
	if (HeldItemState.IsValid()) return;

	UInvSS_InventoryGridManager* GridManager = GetOrCreateGridManager();
	check(GridManager);

	FGuid RemovedItemId;
	int32 RemovedStackCount = 0;
	if (!GridManager->TryRemoveItemAtParentIndex(
		ItemCategory,
		ItemParentIndex,
		RemovedItemId,
		RemovedStackCount))
	{
		return;
	}

	check(RemovedItemId.IsValid());
	check(RemovedStackCount >= 0);
	check(GetInventoryItemByID(RemovedItemId));

	UpdateHeldItemStateFromItem(ItemCategory, ItemParentIndex, RemovedItemId, RemovedStackCount);
}

void UInvSS_InventoryComponent::Server_PutDownHeldItemAtIndex_Implementation(const EInvSS_ItemCategory ItemCategory,
                                                                             int32 ItemParentIndex)
{
	if (!HeldItemState.IsValid()) return;

	UInvSS_InventoryGridManager* GridManager = GetOrCreateGridManager();
	check(GridManager);

	if (!GridManager->TryAddItemAtGivenIndex(
		ItemCategory,
		ItemParentIndex,
		HeldItemState.ItemId,
		HeldItemState.StackCount))
	{
		return;
	}

	ResetHeldItemState();
}

bool UInvSS_InventoryComponent::IsSameAndStackable(const UInvSS_InventoryItem* SlottedItemInPlace,
                                                   const UInvSS_InventoryItem* HeldItem)
{
	const bool IsSameItem = SlottedItemInPlace == HeldItem;
	bool IsStackable = SlottedItemInPlace->IsStackable();
	bool IsSameType = SlottedItemInPlace->GetItemManifest().GetItemTypeTag() == HeldItem->GetItemManifest().GetItemTypeTag();
	return  IsSameItem && IsStackable && IsSameType;
}


void UInvSS_InventoryComponent::Server_RequestBeginSplit_Implementation(const EInvSS_ItemCategory ItemCategory, const int32 GridSlotIndex, const int32 SplitBarVal)
{
	if (HeldItemState.IsValid()) return;
	if (ItemCategory == EInvSS_ItemCategory::None) return;
	if (GridSlotIndex == INDEX_NONE) return;
	if (SplitBarVal <= 0) return;
	
	UInvSS_InventoryGridManager* GridManager = GetOrCreateGridManager();
	check(GridManager);

	FGuid ItemId;
	int32 OldStackCount = 0;
	if (!GridManager->TryGetItemIdAtParentIndex(ItemCategory, GridSlotIndex, ItemId)) return;
	if (!GridManager->TryGetStackCountAtParentIndex(ItemCategory, GridSlotIndex, OldStackCount)) return;

	const UInvSS_InventoryItem* ItemToSplit = GetInventoryItemByID(ItemId);
	if (!IsValid(ItemToSplit)) return;
	if (!ItemToSplit->IsStackable()) return;
	if (ItemToSplit->GetItemManifest().GetItemCategory() != ItemCategory) return;
	if (SplitBarVal >= OldStackCount) return;

	const int32 NewStackCountAtSlot = OldStackCount - SplitBarVal;
	check(NewStackCountAtSlot > 0);
	
	int32 PreviousStackCount = 0;
	if (!GridManager->TryReplaceStackCountAtParentIndex(
		ItemCategory,
		GridSlotIndex,
		NewStackCountAtSlot,
		PreviousStackCount))
	{
		return;
	}

	check(PreviousStackCount == OldStackCount);
	UpdateHeldItemStateFromItem(ItemCategory, GridSlotIndex, ItemId, SplitBarVal);
}

void UInvSS_InventoryComponent::Server_RequestDropItem_Implementation(
	const EInvSS_ItemCategory ItemCategory,
	const int32 ParentIndex)
{
	if (HeldItemState.IsValid()) return;
	if (ItemCategory == EInvSS_ItemCategory::None) return;
	if (ParentIndex == INDEX_NONE) return;

	UInvSS_InventoryGridManager* GridManager = GetOrCreateGridManager();
	check(GridManager);

	FGuid DroppedItemId;
	int32 DroppedStackCount = 0;
	if (!GridManager->TryGetItemIdAtParentIndex(ItemCategory, ParentIndex, DroppedItemId)) return;
	if (!GridManager->TryGetStackCountAtParentIndex(ItemCategory, ParentIndex, DroppedStackCount)) return;

	UInvSS_InventoryItem* DroppedItem = GetMutableInventoryItemByID(DroppedItemId);
	if (!IsValid(DroppedItem)) return;

	FInvSS_ItemManifest DroppedManifest;
	if (!TryBuildDroppedItemManifest(DroppedItem, DroppedStackCount, DroppedManifest)) return;

	FTransform SpawnTransform;
	if (!TryBuildDropTransform(SpawnTransform)) return;

	AActor* DroppedActor = SpawnDroppedItemActorDeferred(DroppedManifest, SpawnTransform);
	if (!IsValid(DroppedActor)) return;

	FGuid RemovedItemId;
	int32 RemovedStackCount = 0;
	if (!GridManager->TryRemoveItemAtParentIndex(ItemCategory, ParentIndex, RemovedItemId, RemovedStackCount))
	{
		DroppedActor->Destroy();
		return;
	}

	check(RemovedItemId == DroppedItemId);
	check(RemovedStackCount == DroppedStackCount);

	RemoveStackFromInventoryListIfNecessary(DroppedItem, DroppedStackCount);
	UGameplayStatics::FinishSpawningActor(DroppedActor, SpawnTransform);
}

void UInvSS_InventoryComponent::Server_RequestDropHeldItem_Implementation()
{
	if (!HeldItemState.IsValid()) return;

	UInvSS_InventoryItem* DroppedItem = GetMutableInventoryItemByID(HeldItemState.ItemId);
	if (!IsValid(DroppedItem)) return;

	const int32 DroppedStackCount = HeldItemState.StackCount;
	FInvSS_ItemManifest DroppedManifest;
	if (!TryBuildDroppedItemManifest(DroppedItem, DroppedStackCount, DroppedManifest)) return;

	FTransform SpawnTransform;
	if (!TryBuildDropTransform(SpawnTransform)) return;

	AActor* DroppedActor = SpawnDroppedItemActorDeferred(DroppedManifest, SpawnTransform);
	if (!IsValid(DroppedActor)) return;

	ResetHeldItemState();
	RemoveStackFromInventoryListIfNecessary(DroppedItem, DroppedStackCount);
	UGameplayStatics::FinishSpawningActor(DroppedActor, SpawnTransform);
}

void UInvSS_InventoryComponent::Server_RequestConsumeItem_Implementation(
	const EInvSS_ItemCategory ItemCategory,
	const int32 ParentIndex)
{
	if (HeldItemState.IsValid()) return;
	if (ItemCategory != EInvSS_ItemCategory::Consumable) return;
	if (ParentIndex == INDEX_NONE) return;

	UInvSS_InventoryGridManager* GridManager = GetOrCreateGridManager();
	check(GridManager);

	FGuid ConsumedItemId;
	int32 SlotStackCount = 0;
	if (!GridManager->TryGetItemIdAtParentIndex(ItemCategory, ParentIndex, ConsumedItemId)) return;
	if (!GridManager->TryGetStackCountAtParentIndex(ItemCategory, ParentIndex, SlotStackCount)) return;
	if (SlotStackCount <= 0) return;

	UInvSS_InventoryItem* ConsumedItem = GetMutableInventoryItemByID(ConsumedItemId);
	if (!IsValid(ConsumedItem)) return;
	if (ConsumedItem->GetItemManifest().GetItemCategory() != EInvSS_ItemCategory::Consumable) return;
	if (!ConsumedItem->IsStackable()) return;

	const FInvSS_ConsumableFragment* ConsumableFragment =
		GetFragment<FInvSS_ConsumableFragment>(ConsumedItem, ItemFragmentTag::ConsumableFragment);
	if (!ConsumableFragment) return;

	const FInvSS_StackableFragment* StackableFragment =
		GetFragment<FInvSS_StackableFragment>(ConsumedItem, ItemFragmentTag::StackableFragment);
	if (!StackableFragment) return;

	if (!ConsumableFragment->OnConsume(OwningPlayerController.Get())) return;

	if (SlotStackCount > 1)
	{
		int32 PreviousStackCount = 0;
		const bool bReplaced = GridManager->TryReplaceStackCountAtParentIndex(
			ItemCategory,
			ParentIndex,
			SlotStackCount - 1,
			PreviousStackCount);

		check(bReplaced);
		check(PreviousStackCount == SlotStackCount);
	}
	else
	{
		FGuid RemovedItemId;
		int32 RemovedStackCount = 0;
		const bool bRemoved = GridManager->TryRemoveItemAtParentIndex(
			ItemCategory,
			ParentIndex,
			RemovedItemId,
			RemovedStackCount);

		check(bRemoved);
		check(RemovedItemId == ConsumedItemId);
		check(RemovedStackCount == SlotStackCount);
	}

	RemoveStackFromInventoryListIfNecessary(ConsumedItem, 1);
}

bool UInvSS_InventoryComponent::TryBuildDroppedItemManifest(
	const UInvSS_InventoryItem* DroppedItem,
	const int32 DroppedStackCount,
	FInvSS_ItemManifest& OutManifest) const
{
	if (!IsValid(DroppedItem)) return false;
	if (DroppedStackCount < 0) return false;

	OutManifest = DroppedItem->GetItemManifest();
	if (!OutManifest.GetWorldItemActorClass()) return false;

	if (DroppedItem->IsStackable())
	{
		if (DroppedStackCount <= 0) return false;

		FInvSS_StackableFragment* StackFragment =
			OutManifest.GetMutableFragmentOfTypeWithTag<FInvSS_StackableFragment>(
				ItemFragmentTag::StackableFragment);
		check(StackFragment);

		StackFragment->SetStackCount(DroppedStackCount);
	}

	return true;
}

bool UInvSS_InventoryComponent::TryBuildDropTransform(FTransform& OutTransform) const
{
	if (!OwningPlayerController.IsValid()) return false;

	const APawn* Pawn = OwningPlayerController->GetPawn();
	if (!IsValid(Pawn)) return false;

	constexpr float MinDropDistance = 120.f;
	constexpr float MaxDropDistance = 180.f;
	constexpr float MaxDropAngle = 25.f;
	constexpr float DropHeightOffset = 40.f;

	const float DropAngle = FMath::FRandRange(-MaxDropAngle, MaxDropAngle);
	const float DropDistance = FMath::FRandRange(MinDropDistance, MaxDropDistance);
	const FVector DropDirection = Pawn->GetActorForwardVector().RotateAngleAxis(DropAngle, FVector::UpVector);
	const FVector DropLocation = Pawn->GetActorLocation()
		+ DropDirection * DropDistance
		+ FVector(0.f, 0.f, DropHeightOffset);

	OutTransform = FTransform(Pawn->GetActorRotation(), DropLocation);
	return true;
}

AActor* UInvSS_InventoryComponent::SpawnDroppedItemActorDeferred(
	const FInvSS_ItemManifest& DroppedManifest,
	const FTransform& SpawnTransform) const
{
	UClass* DroppedActorClass = DroppedManifest.GetWorldItemActorClass();
	if (!DroppedActorClass) return nullptr;
	if (!GetWorld()) return nullptr;

	AActor* DroppedActor = GetWorld()->SpawnActorDeferred<AActor>(
		DroppedActorClass,
		SpawnTransform,
		GetOwner(),
		nullptr,
		ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn);

	if (!IsValid(DroppedActor)) return nullptr;

	UInvSS_ItemComponent* ItemComponent = DroppedActor->FindComponentByClass<UInvSS_ItemComponent>();
	if (!IsValid(ItemComponent))
	{
		DroppedActor->Destroy();
		return nullptr;
	}

	ItemComponent->InitializeItemManifest(DroppedManifest);
	return DroppedActor;
}

void UInvSS_InventoryComponent::RemoveStackFromInventoryListIfNecessary(
	UInvSS_InventoryItem* InItem,
	const int32 StackCountToRemove)
{
	check(IsValid(InItem));
	check(StackCountToRemove >= 0);

	if (InItem->IsStackable())
	{
		const int32 NewTotalStackCount = InItem->GetItemTotalStackCountInBag() - StackCountToRemove;
		check(NewTotalStackCount >= 0);

		InItem->SetItemTotalStackCountInBag(NewTotalStackCount);
		if (NewTotalStackCount > 0) return;
	}

	InventoryList.RemoveEntry(InItem);
	if (OwningPlayerController.IsValid() && OwningPlayerController->IsLocalController())
	{
		OnInventoryItemRemovedDelegate.Broadcast(InItem);
	}
}

void UInvSS_InventoryComponent::Server_RequestHeldItemInteractWithItemUnderCursor_Implementation(
	const EInvSS_ItemCategory ItemCategory,
	const FGuid& ItemID,
	const int32 ItemParentIndex,
	const int32 HeldItemDropIndex)
{
	if (!HeldItemState.IsValid()) return;
	if (ItemParentIndex == INDEX_NONE) return;
	if (HeldItemDropIndex == INDEX_NONE) return;
	
	const UInvSS_InventoryItem* SlottedItemInPlace = GetInventoryItemByID(ItemID);
	const UInvSS_InventoryItem* HeldItem = GetInventoryItemByID(HeldItemState.ItemId);
	check(SlottedItemInPlace && HeldItem);
	
	// the item is the same type, and item is stackable, using stacking to simplify swap logic
	if (IsSameAndStackable(SlottedItemInPlace, HeldItem))
	{
		// Add stack path
		UInvSS_InventoryGridManager* GridManager = GetOrCreateGridManager();
		check(GridManager);
		
		const FInvSS_StackableFragment* ItemStackableFragment = GetFragment<FInvSS_StackableFragment>(
			SlottedItemInPlace,
			ItemFragmentTag::StackableFragment);
		check(ItemStackableFragment);

		const int32 MaxItemStackSize = ItemStackableFragment->GetMaxStackSize();
		int32 SlottedItemStackCount = 0;
		if (!GridManager->TryGetStackCountAtParentIndex(ItemCategory, ItemParentIndex, SlottedItemStackCount))
		{
			return;
		}

		const int32 HoveredItemStackCount = HeldItemState.StackCount;
		const int32 RoomCanFillAtSlot = MaxItemStackSize - SlottedItemStackCount;
		if (ShouldSwapStackCount(RoomCanFillAtSlot, MaxItemStackSize, HoveredItemStackCount))
		{
			SwapStackCount(*GridManager, ItemCategory, ItemParentIndex, HoveredItemStackCount);
			return;
		}
		
		if (ShouldConsumeHoverItemStack(HoveredItemStackCount, RoomCanFillAtSlot))
		{
			ConsumeHoveredItemStackCount(*GridManager, ItemCategory, ItemParentIndex, HoveredItemStackCount, SlottedItemStackCount);
			return;
		}
		
		if (ShouldFillInStack(HoveredItemStackCount, RoomCanFillAtSlot))
		{
			FillInStack(*GridManager, ItemCategory, ItemParentIndex, MaxItemStackSize,  HoveredItemStackCount);
			return;
		}
		
		if (RoomCanFillAtSlot == 0)
		{
			return;
		}
		
	}
	// The item is valid, swap with the item
	else
	{
		// Remove the target item, place the original held item at its drop index, then hold the removed target item.
		const FInvSS_HeldItemState OriginalHeldItemState = HeldItemState;
		UInvSS_InventoryGridManager* GridManager = GetOrCreateGridManager();
		check(GridManager);
		
		FGuid RemovedItemID;
		int32 RemovedStackCount = 0;
		if (!GridManager->TryRemoveItemAtParentIndex(
			ItemCategory,
			ItemParentIndex,
			RemovedItemID,
			RemovedStackCount))
		{
			return;
		}

		if (RemovedItemID != ItemID)
		{
			const bool bRolledBack = GridManager->TryAddItemAtGivenIndex(
				ItemCategory,
				ItemParentIndex,
				RemovedItemID,
				RemovedStackCount);
			check(bRolledBack);
			return;
		}

		if (!GridManager->TryAddItemAtGivenIndex(
			ItemCategory,
			HeldItemDropIndex,
			OriginalHeldItemState.ItemId,
			OriginalHeldItemState.StackCount))
		{
			const bool bRolledBack = GridManager->TryAddItemAtGivenIndex(
				ItemCategory,
				ItemParentIndex,
				RemovedItemID,
				RemovedStackCount);
			check(bRolledBack);
			return;
		}

		UpdateHeldItemStateFromItem(
			ItemCategory,
			ItemParentIndex,
			RemovedItemID,
			RemovedStackCount);
	}
}

bool UInvSS_InventoryComponent::TryFindRoomForItem(
	const UInvSS_ItemComponent* ItemComponent,
	FInvSS_BagAvailabilityResult& OutResult) const
{
	check(ItemComponent);

	const FInvSS_ItemManifest& ItemManifest = ItemComponent->GetItemManifest();
	const UInvSS_InventoryGridManager* GridManager =  TryGetGridManager();
	check(GridManager);

	UInvSS_InventoryItem* ExistingItem = InventoryList.FindFirstItemByType(ItemManifest.GetItemTypeTag());
	return GridManager->FindRoomForItem(ItemManifest, ExistingItem, OutResult);
}

void UInvSS_InventoryComponent::Client_ShowInventoryMessage_Implementation(const FText& Message)
{
	if (GetOrCreateInventoryUIManager())
	{
		OnInventoryMessageRequestedDelegate.Broadcast(Message);
	}
}

void UInvSS_InventoryComponent::OnInventoryGridChange(EInvSS_ItemCategory InventoryCategory) const
{
	if (!OwningPlayerController.Get() || !OwningPlayerController->IsLocalController()) return;
	OnInventoryGridChangedDelegate.Broadcast(InventoryCategory);
}

void UInvSS_InventoryComponent::OnRep_InventoryGridManager()
{
	check(InventoryGridManager);

	InventoryGridManager->OnGridChanged.RemoveAll(this);
	InventoryGridManager->OnGridChanged.AddUObject(
		this,
		&ThisClass::OnInventoryGridChange);	
	
	// initialization 
	check(InventoryGridManager->GetGridStatesArray().Num() > 0);
	for (const FInvSS_InventoryGridState& GridState : InventoryGridManager->GetGridStatesArray())
	{
		OnInventoryGridChange(GridState.GridConfiguration.Category);
	}
}

void UInvSS_InventoryComponent::OnRep_HeldItemState()
{
	OnHeldItemChangedDelegate.Broadcast(HeldItemState);
}

void UInvSS_InventoryComponent::HandlePickupSourceAfterAdd(UInvSS_ItemComponent* InItemComponent, const int32 Remainder)
{
	check(InItemComponent);

	if (Remainder <= 0)
	{
		InItemComponent->PickUp();
		return;
	}

	InItemComponent->TrySetStackCount(Remainder);
}

void UInvSS_InventoryComponent::SetInventoryItemStackCount(UInvSS_InventoryItem* InItem, const int32 StackCount)
{
	if (!IsValid(InItem)) return;

	if (FInvSS_StackableFragment* StackFragment = InItem->GetItemManifest().GetMutableFragmentOfTypeWithTag<FInvSS_StackableFragment>(ItemFragmentTag::StackableFragment))
	{
		StackFragment->SetStackCount(StackCount);
	}
}

bool UInvSS_InventoryComponent::ShouldSwapStackCount(const int32 RoomToFill, const int32 MaxItemStackSize,
	const int32 HoveredItemStackCount)
{
	return RoomToFill == 0 && HoveredItemStackCount < MaxItemStackSize; 
}

bool UInvSS_InventoryComponent::TryReplaceSlottedItemStackCount(
	UInvSS_InventoryGridManager& GridManager,
	const EInvSS_ItemCategory ItemCategory,
	const int32 SlottedItemParentIndex,
	const int32 NewSlottedItemStackCount,
	int32& OutPreviousSlottedItemStackCount)
{
	if (!HeldItemState.IsValid()) return false;

	return GridManager.TryReplaceStackCountAtParentIndex(
		ItemCategory,
		SlottedItemParentIndex,
		NewSlottedItemStackCount,
		OutPreviousSlottedItemStackCount);
}

bool UInvSS_InventoryComponent::SwapStackCount(
	UInvSS_InventoryGridManager& GridManager,
	const EInvSS_ItemCategory ItemCategory,
	const int32 SlottedItemParentIndex,
	const int32 HoveredItemStackCount)
{
	int32 SlottedItemStackCount = 0;
	if (!TryReplaceSlottedItemStackCount(GridManager, ItemCategory, SlottedItemParentIndex, HoveredItemStackCount, SlottedItemStackCount))
	{
		return false;
	}
	
	HeldItemState.StackCount = SlottedItemStackCount;
	NotifyHeldItemStateChanged();
	return true;
}

bool UInvSS_InventoryComponent::ConsumeHoveredItemStackCount(UInvSS_InventoryGridManager& GridManager,
	const EInvSS_ItemCategory ItemCategory, const int32 SlottedItemParentIndex, const int32 HoveredItemStackCount, const int32 SlotItemStackCount)
{
	const int32 NewStackCountAtSlot = SlotItemStackCount + HoveredItemStackCount;
	int32 SlottedItemPreviousStackCount = 0;
	if (!TryReplaceSlottedItemStackCount(GridManager, ItemCategory, SlottedItemParentIndex, NewStackCountAtSlot, SlottedItemPreviousStackCount))
	{
		return false;
	}
	
	ResetHeldItemState();
	return true;
}

bool UInvSS_InventoryComponent::FillInStack(UInvSS_InventoryGridManager& GridManager,
	const EInvSS_ItemCategory ItemCategory, const int32 SlottedItemParentIndex,  const int32 ItemMaxStackSize,   const int32 HoveredItemStackCount)
{
	const int32 NewSlottedItemCount =  ItemMaxStackSize;  
	int32 SlottedItemStackCount = 0;
	if (!TryReplaceSlottedItemStackCount(GridManager, ItemCategory, SlottedItemParentIndex, NewSlottedItemCount, SlottedItemStackCount))
	{
		return false;
	}
	
	const int32 NewHoveredItemStackCount = HoveredItemStackCount - (ItemMaxStackSize - SlottedItemStackCount);
	HeldItemState.StackCount = NewHoveredItemStackCount;
	NotifyHeldItemStateChanged();
	return true;
}

bool UInvSS_InventoryComponent::ShouldConsumeHoverItemStack(const int32 HoverItemCount,
                                                            const int32 RoomCanFillAtSlot)
{
	return HoverItemCount <= RoomCanFillAtSlot;
}

bool UInvSS_InventoryComponent::ShouldFillInStack(int32 HoveredItemStackCount, int32 RoomCanFillAtSlot)
{
	return RoomCanFillAtSlot < HoveredItemStackCount; 
}




void UInvSS_InventoryComponent::ToggleInventoryMenu()
{
	if (UInvSS_InventoryUIManager* UIManager =  GetOrCreateInventoryUIManager())
	{
		UIManager->OnToggleInventoryMenu();
	}
}

UInvSS_InventoryUIManager* UInvSS_InventoryComponent::GetOrCreateInventoryUIManager()
{
	check(OwningPlayerController.IsValid());
	if (!OwningPlayerController->IsLocalController())
	{
		return nullptr;
	}

	if (IsValid(InventoryUIManager))
	{
		return InventoryUIManager;
	}

	TSubclassOf<UInvSS_InventoryUIManager> ManagerClass = InventoryUIManagerClass;
	if (!ManagerClass)
	{
		ManagerClass = UInvSS_InventoryUIManager::StaticClass();
	}

	InventoryUIManager = NewObject<UInvSS_InventoryUIManager>(this, ManagerClass);
	if (InventoryUIManager)
	{
		InventoryUIManager->OnInitialize(OwningPlayerController.Get(), this);
	}
	return InventoryUIManager;
}

const UInvSS_InventoryUIManager* UInvSS_InventoryComponent::GetUIManager() const
{
	if (!ensureMsgf(OwningPlayerController.IsValid(), TEXT("Invalid PC while creating UI Manager in Inventory Component")))
	{
		return nullptr;
	}
	if (!OwningPlayerController->IsLocalController())
	{
		return nullptr;
	}

	if (IsValid(InventoryUIManager))
	{
		return InventoryUIManager;
	}

	return nullptr;
}

UInvSS_InventoryGridManager* UInvSS_InventoryComponent::GetOrCreateGridManager()
{
	if  (IsValid(InventoryGridManager))
	{
		return InventoryGridManager;
	}
	
	if (!GetOwner() || !GetOwner()->HasAuthority()) {return nullptr; }
		
	checkf(InventoryGridManagerClass, TEXT("No grid manager class specified for inventory component %s"), *GetName());
	EnsureDefaultGridManagerConfigs();
	
	InventoryGridManager = NewObject<UInvSS_InventoryGridManager>(
		this,
		InventoryGridManagerClass,
		TEXT("InventoryGridManager"));
	InventoryGridManager->Initialize(GridManagerConfigs);
	InventoryGridManager->OnGridChanged.AddUObject(this, &ThisClass::OnInventoryGridChange);
	return InventoryGridManager;
}

const UInvSS_InventoryGridManager* UInvSS_InventoryComponent::TryGetGridManager() const
{
	return IsValid(InventoryGridManager)? InventoryGridManager : nullptr;
}

void UInvSS_InventoryComponent::AddRepSubObj(UObject* InSubObj)
{
	if (IsUsingRegisteredSubObjectList() && IsReadyForReplication() && IsValid(InSubObj))
	{
		AddReplicatedSubObject(InSubObj);
	}
}

void UInvSS_InventoryComponent::OnInventoryItemReplicated(UInvSS_InventoryItem* InItem) const
{
	if (!IsValid(InItem)) return;

	OnInventoryGridChange(InItem->GetItemManifest().GetItemCategory());
	OnInventoryItemAddedDelegate.Broadcast(InItem);
}

const UInvSS_InventoryItem* UInvSS_InventoryComponent::GetInventoryItemByID(const FGuid& ID) const
{
	return InventoryList.FindItemByID(ID);
}

UInvSS_InventoryItem* UInvSS_InventoryComponent::GetMutableInventoryItemByID(const FGuid& ID)
{
	return InventoryList.FindMutableItemByID(ID);
}

TArray<UInvSS_InventoryItem*> UInvSS_InventoryComponent::GetAllItems() const
{
	return InventoryList.GetAllItems();
}

void UInvSS_InventoryComponent::BeginPlay()
{
	Super::BeginPlay();

	OwningPlayerController = CastChecked<APlayerController>(GetOwner());
}

void UInvSS_InventoryComponent::GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME(ThisClass, InventoryList);
	DOREPLIFETIME(ThisClass, InventoryGridManager);
	DOREPLIFETIME(ThisClass, HeldItemState);
}

void UInvSS_InventoryComponent::EnsureDefaultGridManagerConfigs()
{
	if (!GridManagerConfigs.IsEmpty())
	{
		return;
	}

	GridManagerConfigs =
	{
		{ EInvSS_ItemCategory::Equippable, 4, 8 },
		{ EInvSS_ItemCategory::Consumable, 4, 8 },
		{ EInvSS_ItemCategory::Craftable, 4, 8 }
	};
}
