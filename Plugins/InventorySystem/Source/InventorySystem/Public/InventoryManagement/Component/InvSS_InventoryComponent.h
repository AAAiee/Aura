// @Copyright HaolunYuan

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "InventoryManagement/FastArray/InvSS_FastArray.h"
#include "InventoryManagement/Grid/InvSS_InventoryGridManager.h"
#include "InvSS_InventoryComponent.generated.h"

class UInvSS_InventoryUIManager;
class UInvSS_ItemComponent;
class UInvSS_InventoryItem;
class AActor;
struct FInvSS_BagAvailabilityResult;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FInventoryItemChangeSignature, UInvSS_InventoryItem*, InChangedItem);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FInventoryMessageSignature, const FText&, InMessage);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FInventoryGridChangedSignature, EInvSS_ItemCategory, Category);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FInventoryHeldItemChangedSignature, FInvSS_HeldItemState, HeldItemState);

class UInvSS_InventoryBase;

/**
 * UInvSS_InventoryComponent
 *
 * Owns the replicated inventory item list and the server/client grid managers for one player controller.
 *
 * This component is the gameplay-facing inventory entry point. It validates pickup requests on authority,
 * updates the authoritative item list, mirrors spatial grid state to the owning client, and exposes UI-facing
 * delegates for local inventory widgets.
 *
 * Important functions:
 *   - TryAddItem() - Adds pickup item data to the inventory on authority.
 *   - HandleReplicatedItemAdded() - Reconstructs client grid state after fast-array item replication.
 *   - Client_ApplyGridAvailability() - Mirrors stack-only grid changes to the owning client.
 *
 * Networking:
 *   - The server owns InventoryList and authoritative grid placement.
 *   - New items replicate through FInvSS_FastArray.
 *   - Stack-only changes use a client RPC because no fast-array entry is added.
 */
UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent), Blueprintable)
class INVENTORYSYSTEM_API UInvSS_InventoryComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UInvSS_InventoryComponent();

	virtual void ReadyForReplication() override;
	UInvSS_InventoryGridManager* GetOrCreateGridManager() ;
	const UInvSS_InventoryGridManager* TryGetGridManager() const;
	
	UFUNCTION(BlueprintCallable, BlueprintAuthorityOnly, Category = Inventory)
	void TryAddItem(UInvSS_ItemComponent* InItem);

	UFUNCTION(Server, Reliable)
	void Server_BeginDragItem(const EInvSS_ItemCategory ItemCategory, const int32 ItemParentIndex);
	
	UFUNCTION(Server, Reliable)
	void Server_PutDownHeldItemAtIndex(const EInvSS_ItemCategory ItemCategory, int32 ItemParentIndex); 
	
	UFUNCTION(Server, Reliable)
	void Server_RequestHeldItemInteractWithItemUnderCursor(
		const EInvSS_ItemCategory ItemCategory,
		const FGuid& ItemID,
		const int32 ItemParentIndex,
		const int32 HeldItemDropIndex);
	
	UFUNCTION(Server, Reliable)
	void Server_RequestBeginSplit(const EInvSS_ItemCategory ItemCategory, const int32 SlotIndex, const int32 SplitBarVal);

	UFUNCTION(Server, Reliable)
	void Server_RequestDropItem(const EInvSS_ItemCategory ItemCategory, const int32 ParentIndex);

	UFUNCTION(Server, Reliable)
	void Server_RequestDropHeldItem();

	UFUNCTION(Server, Reliable)
	void Server_RequestConsumeItem(const EInvSS_ItemCategory ItemCategory, const int32 ParentIndex);
	
	UFUNCTION(BlueprintCallable, Category = "Inventory Menu")
	void ToggleInventoryMenu();

	UFUNCTION(BlueprintCallable, Category = "Inventory UI Manager")
	UInvSS_InventoryUIManager* GetOrCreateInventoryUIManager();
	const UInvSS_InventoryUIManager* GetUIManager() const;
	
	void AddRepSubObj(UObject* InSubObj);
	void OnInventoryItemReplicated(UInvSS_InventoryItem* InItem) const;
	
	const UInvSS_InventoryItem* GetInventoryItemByID(const FGuid& ID) const; 
	UInvSS_InventoryItem* GetMutableInventoryItemByID(const FGuid& ID);
	TArray<UInvSS_InventoryItem*> GetAllItems() const;

	/* UActorComponent begins */
	virtual void BeginPlay() override;
	virtual void GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const override;
	/* UActorComponent ends */

	FInventoryItemChangeSignature OnInventoryItemRemovedDelegate;
	FInventoryItemChangeSignature OnInventoryItemAddedDelegate;
	FInventoryMessageSignature OnInventoryMessageRequestedDelegate;
	FInventoryGridChangedSignature OnInventoryGridChangedDelegate;
	FInventoryHeldItemChangedSignature OnHeldItemChangedDelegate;

private:
	/**
	 * @brief Sends an inventory status message to the owning client.
	 */
	UFUNCTION(Client, Reliable)
	void Client_ShowInventoryMessage(const FText& Message);

	void EnsureDefaultGridManagerConfigs();
	bool IsSameAndStackable(const UInvSS_InventoryItem* SlottedItemInPlace, const UInvSS_InventoryItem* HeldItem);
	bool TryFindRoomForItem(const UInvSS_ItemComponent* ItemComponent, FInvSS_BagAvailabilityResult& OutResult) const;
	void UpdateHeldItemStateFromItem(EInvSS_ItemCategory ItemCategory, int32 ItemParentIndex, FGuid InventoryItemId,
	                                 int32 StackCount);
	void ResetHeldItemState();
	void NotifyHeldItemStateChanged();
	bool TryBuildDroppedItemManifest(const UInvSS_InventoryItem* DroppedItem, int32 DroppedStackCount,
	                                 FInvSS_ItemManifest& OutManifest) const;
	bool TryBuildDropTransform(FTransform& OutTransform) const;
	AActor* SpawnDroppedItemActorDeferred(const FInvSS_ItemManifest& DroppedManifest,
	                                      const FTransform& SpawnTransform) const;
	void RemoveStackFromInventoryListIfNecessary(UInvSS_InventoryItem* InItem, int32 StackCountToRemove);

	UFUNCTION()
	void OnInventoryGridChange(EInvSS_ItemCategory InventoryCategory) const;
	UFUNCTION()
	void OnRep_InventoryGridManager() ;
	UFUNCTION()
	void OnRep_HeldItemState();
	
	static void HandlePickupSourceAfterAdd(UInvSS_ItemComponent* InItemComponent, const int32 Remainder);
	static void SetInventoryItemStackCount(UInvSS_InventoryItem* InItem, const int32 StackCount);

	bool SwapStackCount(
		UInvSS_InventoryGridManager& GridManager,
		EInvSS_ItemCategory ItemCategory,
		int32 SlottedItemParentIndex,
		int32 HoveredItemStackCount);
	bool ConsumeHoveredItemStackCount(
		UInvSS_InventoryGridManager& GridManager,
		EInvSS_ItemCategory ItemCategory,
		int32 SlottedItemParentIndex,
		int32 HoveredItemStackCount, int32 SlotItemStackCount);
	bool FillInStack(UInvSS_InventoryGridManager& GridManager, EInvSS_ItemCategory ItemCategory, int32 SlottedItemParentIndex, int32
	                 ItemMaxStackSize, int32 HoveredItemStackCount);
	bool TryReplaceSlottedItemStackCount(
		UInvSS_InventoryGridManager& GridManager,
		EInvSS_ItemCategory ItemCategory,
		int32 SlottedItemParentIndex,
		int32 NewSlottedItemStackCount,
		int32& OutPreviousSlottedItemStackCount);
	
	
	static bool ShouldConsumeHoverItemStack(const int32 HoverItemCount , const int32 RoomCanFillAtSlot);
	static bool ShouldFillInStack(const int32 HoveredItemStackCount, const int32 RoomAvailable);
	static bool ShouldSwapStackCount(const int32 RoomToFill,  const int32 MaxItemStackSize, const int32 HoveredItemStackCount );

private:
	UPROPERTY()
	TWeakObjectPtr<APlayerController> OwningPlayerController = nullptr;
	UPROPERTY(EditDefaultsOnly, Category = "Inventory")
	TArray<FInvSS_GridConfig> GridManagerConfigs;
	
	UPROPERTY(EditDefaultsOnly, Category = "Inventory")
	TSubclassOf<UInvSS_InventoryGridManager> InventoryGridManagerClass; 
	
	UPROPERTY(EditAnywhere, Category = "Inventory")
	TSubclassOf<UInvSS_InventoryUIManager> InventoryUIManagerClass;
	
	UPROPERTY(ReplicatedUsing = OnRep_InventoryGridManager, Transient)
	TObjectPtr<UInvSS_InventoryGridManager> InventoryGridManager = nullptr;

	UPROPERTY(Transient)
	TObjectPtr<UInvSS_InventoryUIManager> InventoryUIManager = nullptr;

	UPROPERTY(ReplicatedUsing = OnRep_HeldItemState)
	FInvSS_HeldItemState HeldItemState;
	
	UPROPERTY(Replicated)
	FInvSS_FastArray InventoryList;
};
