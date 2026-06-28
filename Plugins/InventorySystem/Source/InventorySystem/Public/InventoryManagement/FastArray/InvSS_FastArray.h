#pragma once

#include "CoreMinimal.h"
#include "Net/Serialization/FastArraySerializer.h"
#include "InvSS_FastArray.generated.h"

struct FGameplayTag;
class UInvSS_ItemComponent;
class UInvSS_InventoryItem;

/**
 * Replicated fast-array entry for one inventory item.
 */
USTRUCT(BlueprintType)
struct FInvSS_InventoryEntry : public FFastArraySerializerItem
{
	GENERATED_BODY()

public:
	FInvSS_InventoryEntry() : InventoryItem(nullptr) {}
	FInvSS_InventoryEntry(UInvSS_InventoryItem* InItem) : InventoryItem(InItem) {}

	friend class UInvSS_InventoryComponent;

	UPROPERTY()
	TObjectPtr<UInvSS_InventoryItem> InventoryItem;
};

/**
 * Replicated inventory item list with add/remove callbacks for client-side UI and grid state.
 */
USTRUCT(BlueprintType)
struct FInvSS_FastArray : public FFastArraySerializer
{
	GENERATED_BODY()

public:
	FInvSS_FastArray() : OwnerComponent(nullptr) {}
	FInvSS_FastArray(UActorComponent* InOwnerComponent) : OwnerComponent(InOwnerComponent) {}

	/**
	 * @brief Returns all valid inventory items in the replicated list.
	 */
	TArray<UInvSS_InventoryItem*> GetAllItems() const;

	/* FFastArraySerializer begins */
	void PreReplicatedRemove(const TArrayView<int32> RemovedIndices, int32 FinalSize);
	void PostReplicatedAdd(const TArrayView<int32> AddedIndices, int32 FinalSize);
	/* FFastArraySerializer ends */

	/**
	 * @brief Adds an already-created inventory item on authority.
	 */
	UInvSS_InventoryItem* AddEntry(UInvSS_InventoryItem* InItem);

	/**
	 * @brief Builds and adds an inventory item from an item component manifest on authority.
	 */
	UInvSS_InventoryItem* AddEntry(UInvSS_ItemComponent* InItemComponent);

	/**
	 * @brief Removes an inventory item on authority.
	 */
	void RemoveEntry(UInvSS_InventoryItem* InItem);

	/**
	 * @brief Finds the first valid item with the exact item type tag.
	 */
	UInvSS_InventoryItem* FindFirstItemByType(const FGameplayTag& ItemType) const;
	
	const UInvSS_InventoryItem* FindItemByID(const FGuid& ID) const; 
	UInvSS_InventoryItem* FindMutableItemByID(const FGuid& ID);
	

	bool NetDeltaSerialize(FNetDeltaSerializeInfo& DeltaParams)
	{
		return FastArrayDeltaSerialize<FInvSS_InventoryEntry, FInvSS_FastArray>(InventoryEntries, DeltaParams, *this);
	}


private:
	friend class UInvSS_InventoryComponent;

	UPROPERTY()
	TArray<FInvSS_InventoryEntry> InventoryEntries;

	UPROPERTY(NotReplicated)
	TWeakObjectPtr<UActorComponent> OwnerComponent;
};

template<>
struct TStructOpsTypeTraits<FInvSS_FastArray> : public TStructOpsTypeTraitsBase2<FInvSS_FastArray>
{
	enum
	{
		WithNetDeltaSerializer = true,
	};
};
