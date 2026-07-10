#include "InventoryManagement/FastArray/InvSS_FastArray.h"

#include "InventoryManagement/Component/InvSS_InventoryComponent.h"
#include "Item/InvSS_InventoryItem.h"
#include "Item/InvSS_ItemComponent.h"


TArray<UInvSS_InventoryItem*> FInvSS_FastArray::GetAllItems() const
{
	TArray<UInvSS_InventoryItem*> OutputArray;
	OutputArray.Reserve(InventoryEntries.Num());
	for (const FInvSS_InventoryEntry& Entry : InventoryEntries)
	{
		if (!IsValid(Entry.InventoryItem)) continue;

		OutputArray.Add(Entry.InventoryItem);
	}
	return OutputArray;
}

void FInvSS_FastArray::PreReplicatedRemove(const TArrayView<int32> RemovedIndices, int32 FinalSize)
{
	UInvSS_InventoryComponent* IC = Cast<UInvSS_InventoryComponent>(OwnerComponent);
	if (!IsValid(IC)) return;

	for (const int32 Index : RemovedIndices)
	{
		IC->OnInventoryItemRemovedDelegate.Broadcast(InventoryEntries[Index].InventoryItem);
	}
}

void FInvSS_FastArray::PostReplicatedAdd(const TArrayView<int32> AddedIndices, int32 FinalSize)
{
	UInvSS_InventoryComponent* IC = Cast<UInvSS_InventoryComponent>(OwnerComponent);
	if (!IsValid(IC)) return;

	// Pipeline:
	// 1. Fast-array replication creates the item entry on the client.
	// 2. The inventory component applies category-grid placement and notifies local UI listeners.
	for (const int32 Index : AddedIndices)
	{
		IC->OnInventoryItemReplicated(InventoryEntries[Index].InventoryItem);
	}
}

UInvSS_InventoryItem* FInvSS_FastArray::AddEntry(UInvSS_InventoryItem* InItem)
{
	check(InItem);
	check(OwnerComponent.IsValid());
	AActor* OwningActor = OwnerComponent->GetOwner();
	check(OwningActor && OwningActor->HasAuthority());

	FInvSS_InventoryEntry& AddedEntryRef = InventoryEntries.Emplace_GetRef(InItem);
	MarkItemDirty(AddedEntryRef);
	return AddedEntryRef.InventoryItem;
}

UInvSS_InventoryItem* FInvSS_FastArray::AddEntry(UInvSS_ItemComponent* InItemComponent)
{
	check(InItemComponent);
	check(OwnerComponent.IsValid());
	const AActor* OwningActor = OwnerComponent->GetOwner();
	UInvSS_InventoryComponent* InventoryComponent = Cast<UInvSS_InventoryComponent>(OwnerComponent);
	check(InventoryComponent);
	check(OwningActor && OwningActor->HasAuthority());

	FInvSS_InventoryEntry& AddedEntryRef = InventoryEntries.Emplace_GetRef();
	AddedEntryRef.InventoryItem = InItemComponent->GetItemManifest().Manifest(InventoryComponent);

	InventoryComponent->AddRepSubObj(AddedEntryRef.InventoryItem);
	MarkItemDirty(AddedEntryRef);

	return AddedEntryRef.InventoryItem;
}

void FInvSS_FastArray::RemoveEntry(UInvSS_InventoryItem* InItem)
{
	check(InItem);
	check(OwnerComponent.IsValid());
	AActor* OwningActor = OwnerComponent->GetOwner();
	check(OwningActor && OwningActor->HasAuthority());

	for (auto It = InventoryEntries.CreateIterator(); It; ++It)
	{
		if (FInvSS_InventoryEntry& Entry = *It; Entry.InventoryItem == InItem)
		{
			It.RemoveCurrent();
			MarkArrayDirty();
			return;
		}
	}
}

UInvSS_InventoryItem* FInvSS_FastArray::FindFirstItemByType(const FGameplayTag& ItemType) const
{
	const FInvSS_InventoryEntry* FoundItem = InventoryEntries.FindByPredicate([ItemType](const FInvSS_InventoryEntry& Entry)
	{
		return IsValid(Entry.InventoryItem) && Entry.InventoryItem->GetItemManifest().GetItemTypeTag().MatchesTagExact(ItemType);
	});
	return FoundItem ? FoundItem->InventoryItem : nullptr;
}

const UInvSS_InventoryItem* FInvSS_FastArray::FindItemByID(const FGuid& ID) const
{
	if (!ID.IsValid()) return nullptr;
	const FInvSS_InventoryEntry* FoundItem = InventoryEntries.FindByPredicate([TargetID = ID](const FInvSS_InventoryEntry& Entry)
	{
		return IsValid(Entry.InventoryItem) && Entry.InventoryItem->GetItemInstanceId() == TargetID;
	});
	return FoundItem ? FoundItem->InventoryItem : nullptr;
}

UInvSS_InventoryItem* FInvSS_FastArray::FindMutableItemByID(const FGuid& ID)
{
	if (!ID.IsValid()) return nullptr;
	FInvSS_InventoryEntry* FoundItem = InventoryEntries.FindByPredicate([TargetID = ID](const FInvSS_InventoryEntry& Entry)
	{
		return IsValid(Entry.InventoryItem) && Entry.InventoryItem->GetItemInstanceId() == TargetID;
	});
	return FoundItem ? FoundItem->InventoryItem : nullptr;

}
