// @Copyright HaolunYuan

#pragma once

#include "CoreMinimal.h"
#include "StructUtils/InstancedStruct.h"
#include "UObject/Object.h"
#include "Manifest/InvSS_ItemManifest.h"
#include "InvSS_InventoryItem.generated.h"

class FLifetimeProperty;

/**
 * UInvSS_InventoryItem
 *
 * Runtime UObject representation of an item stored inside an inventory.
 *
 * Inventory entries replicate this object as a subobject. It owns an instanced item manifest
 * and the total stack count currently stored in the bag for that item.
 */
UCLASS()
class INVENTORYSYSTEM_API UInvSS_InventoryItem : public UObject
{
	GENERATED_BODY()

public:
	/**
	 * @brief Replaces this item's manifest data.
	 */
	void SetItemManifest(const FInvSS_ItemManifest& Manifest);

	/**
	 * @brief Returns the immutable manifest stored on this item.
	 */
	FORCEINLINE const FInvSS_ItemManifest& GetItemManifest() const { return ItemManifest.Get<FInvSS_ItemManifest>(); }

	/**
	 * @brief Returns a mutable manifest for server-side inventory updates.
	 */
	FORCEINLINE FInvSS_ItemManifest& GetItemManifest() { return ItemManifest.GetMutable<FInvSS_ItemManifest>(); }

	/**
	 * @brief Returns true when the manifest contains a stackable fragment.
	 */
	bool IsStackable() const;

	FORCEINLINE int32 GetItemTotalStackCountInBag() const { return TotalStackCountInBag; }
	FORCEINLINE void SetItemTotalStackCountInBag(const int32 InCount) { TotalStackCountInBag = InCount; }
	FORCEINLINE FGuid GetItemInstanceId() const { return ItemInstanceId; }
	void GenerateItemInstanceId();

	/* UObject begins */
	virtual bool IsSupportedForNetworking() const override;
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	/* UObject ends */

private:
	UFUNCTION()
	void OnRep_TotalStackCount();

	UPROPERTY(VisibleAnywhere, meta = (BaseStruct = "/Script/InventorySystem.InvSS_ItemManifest"), Replicated)
	FInstancedStruct ItemManifest;

	UPROPERTY(ReplicatedUsing = OnRep_TotalStackCount)
	int32 TotalStackCountInBag = 0;

	UPROPERTY(Replicated)
	FGuid ItemInstanceId;
};

template<typename FragmentType>
requires std::derived_from<FragmentType, FInvSS_ItemFragment>
const FragmentType* GetFragment(const UInvSS_InventoryItem* InItem, const FGameplayTag& InFragmentTag)
{
	if (!IsValid(InItem)) return nullptr;
	return InItem->GetItemManifest().GetFragmentOfTypeWithTag<FragmentType>(InFragmentTag);
}

template<typename FragmentType>
requires std::derived_from<FragmentType, FInvSS_ItemFragment>
FragmentType* GetFragmentMutable(UInvSS_InventoryItem* InItem, const FGameplayTag& InFragmentTag)
{
	if (!IsValid(InItem)) return nullptr;
	return InItem->GetItemManifest().GetMutableFragmentOfTypeWithTag<FragmentType>(InFragmentTag);
}
