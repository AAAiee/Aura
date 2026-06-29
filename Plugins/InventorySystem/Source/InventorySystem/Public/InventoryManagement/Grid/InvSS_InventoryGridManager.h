// @Copyright HaolunYuan

#pragma once

#include "CoreMinimal.h"
#include "Item/Manifest/InvSS_ItemManifest.h"
#include "Type/InvSS_GridTypes.h"
#include "UObject/Object.h"
#include "InvSS_InventoryGridManager.generated.h"

class UInvSS_InventoryComponent;

USTRUCT(BlueprintType)
struct FInvSS_GridConfig
{
	GENERATED_BODY()

	UPROPERTY(EditDefaultsOnly, Category = "Inventory")
	EInvSS_ItemCategory Category = EInvSS_ItemCategory::None;

	UPROPERTY(EditDefaultsOnly, Category = "Inventory", meta = (ClampMin = "1"))
	int32 Rows = 4;

	UPROPERTY(EditDefaultsOnly, Category = "Inventory", meta = (ClampMin = "1"))
	int32 Columns = 8;
	
	int32 NumSlots() const { return Rows * Columns;}
};

USTRUCT(BlueprintType)
struct FInvSS_GridSlotState
{
	GENERATED_BODY()

	UPROPERTY()
	FGuid ID_InventoryItemAtThisSlot; 

	UPROPERTY()
	int32 ParentIndex = INDEX_NONE;

	UPROPERTY()
	int32 SlotStackCount = 0;

	bool OccupiedByItem() const { return ID_InventoryItemAtThisSlot.IsValid(); }
	void Reset();
};

USTRUCT(BlueprintType)
struct FInvSS_InventoryGridState
{
	GENERATED_BODY()

	UPROPERTY()
	FInvSS_GridConfig GridConfiguration;

	UPROPERTY()
	uint32 ReplicationRevision = 0;
	
	UPROPERTY() 
	TArray<FInvSS_GridSlotState> SlotStates;
};

UCLASS(BlueprintType, Blueprintable)
class INVENTORYSYSTEM_API UInvSS_InventoryGridManager : public UObject
{
	GENERATED_BODY()

public:
	virtual bool IsSupportedForNetworking() const override { return true; }
	virtual void GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const override;
	void Initialize(const TArray<FInvSS_GridConfig>& InConfigArray);
	bool FindRoomForItem(
		const FInvSS_ItemManifest& InItemManifest,
		UInvSS_InventoryItem* ExistingStackItem,
		FInvSS_BagAvailabilityResult& OutResult) const;
	void ApplyAvailability(UInvSS_InventoryItem* InItem, const FInvSS_BagAvailabilityResult& InResult);
	
	bool TryRemoveItemAtParentIndex(
		EInvSS_ItemCategory Category,
		int32 ParentIndex,
		FGuid& OutItemId,
		int32& OutStackCount);
	bool TryAddItemAtGivenIndex(EInvSS_ItemCategory ItemCategory, int32 Index, const FGuid& ItemGuid, int32 StackCount);
	bool TryGetStackCountAtParentIndex(EInvSS_ItemCategory ItemCategory, int32 ParentIndex, int32& OutStackCount) const;
	bool TryReplaceStackCountAtParentIndex(
		EInvSS_ItemCategory ItemCategory,
		int32 ParentIndex,
		int32 NewStackCount,
		int32& OutPreviousStackCount);
	
	const FInvSS_InventoryGridState* GetGridState(const EInvSS_ItemCategory Category) const;
	FInvSS_InventoryGridState* GetMutableGridState(const EInvSS_ItemCategory Category);
	const TArray<FInvSS_GridSlotState>* GetSlotStateArrayByType(const EInvSS_ItemCategory Category) const;
	TArray<FInvSS_GridSlotState>* GetMutableSlotStateArrayByType(const EInvSS_ItemCategory Category);
	const FInvSS_GridConfig* GetGridConfigByType(const EInvSS_ItemCategory Category) const;
	const TArray<FInvSS_InventoryGridState>& GetGridStatesArray() const { return GridStatesArray; }
	UInvSS_InventoryComponent* GetInventoryComponent() const;

	DECLARE_MULTICAST_DELEGATE_OneParam(FInvSS_GridChangedSignature, EInvSS_ItemCategory);
	FInvSS_GridChangedSignature OnGridChanged;
	
private:
	static FIntPoint GetItemGridDimensions(const FInvSS_ItemManifest& InItemManifest);
	static int32 GetSlotStackAmount(const TArray<FInvSS_GridSlotState>& SlotStateArray, const int32 SlotIndex);
	static int32 DetermineFillAmount(const TArray<FInvSS_GridSlotState>& SlotStateArray, const int32 SlotIndex,
	                                 const int32 MaxStackSize, const bool bIsStackable, const int32 AmountToFill);
	static bool CanPlaceItemAtEmptyIndex(const FInvSS_InventoryGridState& GridState, int32 Index,
	                                     const FIntPoint& ItemGridDimensions);
	static void WriteItemAtIndex(FInvSS_InventoryGridState& GridState, int32 Index, const FGuid& ItemGuid,
	                             const FIntPoint& ItemGridDimensions, int32 StackCount);
	
	bool HasRoomAtIndex(const TArray<FInvSS_GridSlotState>& SlotStateArray, const FInvSS_GridConfig& GridConfig,
	                    int32 StartIndex, const FIntPoint& GridDimensions, const TSet<int32>& CheckedSlotIndices,
	                    const FGameplayTag& ItemTypeTag, int32 MaxStackSize, TSet<int32>& OutTentativeIndices) const;

	bool CheckSlotConstraints(
		const TArray<FInvSS_GridSlotState>& SlotStateArray,
		int32 StartIndex,
		int32 SubIndex,
		const TSet<int32>& CheckedSlotIndices,
		const FGameplayTag& ItemTypeTag, int32 MaxStackSize) const;

	static void MarkGridStateDirty(FInvSS_InventoryGridState& GridState);
	bool ShouldBroadcastReplicatedGridState(const FInvSS_InventoryGridState& GridState);

	UFUNCTION()
	void OnRep_GridStates();
	
	UPROPERTY(ReplicatedUsing = OnRep_GridStates)
	TArray<FInvSS_InventoryGridState> GridStatesArray;

	UPROPERTY(Transient)
	TMap<EInvSS_ItemCategory, uint32> LastSeenGridRevisions;
};
