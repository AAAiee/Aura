// @Copyright HaolunYuan

#pragma once

#include "CoreMinimal.h"
#include "Type/InvSS_GridTypes.h"
#include "Widgets/WidgetController/InvSS_WidgetController.h"
#include "InvSS_InventoryWidgetController.generated.h"

class UInvSS_InventoryItem;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FInvSS_ItemChangedSignature, UInvSS_InventoryItem*, Item);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FInvSS_InventoryGridChangedSignature, EInvSS_ItemCategory, InItemCategory);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FInvSS_HeldItemChangedSignature, FInvSS_HeldItemState, HeldItemState);

/**
 * Widget-friendly view of one inventory grid slot.
 */
USTRUCT(BlueprintType)
struct FInvSS_GridSlotViewData
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly)
	FGuid ItemInstanceId;

	UPROPERTY(BlueprintReadOnly)
	TObjectPtr<UInvSS_InventoryItem> Item = nullptr;

	UPROPERTY(BlueprintReadOnly)
	int32 ParentIndex = INDEX_NONE;

	UPROPERTY(BlueprintReadOnly)
	int32 StackCount = 0;

	UPROPERTY(BlueprintReadOnly)
	bool bOccupied = false;

	UPROPERTY(BlueprintReadOnly)
	bool bParentSlot = false;
};

/**
 * Cached widget-friendly view of one category grid.
 */
USTRUCT(BlueprintType)
struct FInvSS_InventoryGridViewData
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly)
	int32 Rows = 0;

	UPROPERTY(BlueprintReadOnly)
	int32 Columns = 0;

	UPROPERTY(BlueprintReadOnly)
	TArray<FInvSS_GridSlotViewData> Slots;
};

/**
 * UInvSS_InventoryWidgetController
 *
 * Bridges inventory component events to inventory widgets.
 *
 * The UI manager creates this controller for the local inventory menu. It listens to
 * component delegates and rebroadcasts widget-friendly events for grid refreshes,
 * item-list changes, and popup messages.
 */
UCLASS()
class INVENTORYSYSTEM_API UInvSS_InventoryWidgetController : public UInvSS_WidgetController
{
	GENERATED_BODY()

public:
	virtual void BroadcastInitialValues() override;
	virtual void BindAllDependencies() override;

	const FInvSS_InventoryGridViewData* GetCachedGridViewData(EInvSS_ItemCategory ItemCategory);
	bool GetGridViewData(EInvSS_ItemCategory ItemCategory, FInvSS_InventoryGridViewData& OutViewData);
	UInvSS_InventoryItem* GetInventoryItemByID(const FGuid& ItemId) const;
	void RequestBeginDragItem(EInvSS_ItemCategory ItemCategory, int32 ParentIndex);
	void QueryGridSpace(
		const EInvSS_ItemCategory ItemCategory,
		const int32 StartIndex,
		const FIntPoint& ItemDimensions,
		FInvSS_SpaceQueryResult& OutResult
		);

	void RequestPutDownHeldITemAtIndex(EInvSS_ItemCategory ItemCategory, int32 ItemParentIndex);
	void RequestInteractHeldItemWithItemUnderCursor(
		EInvSS_ItemCategory ItemCategory,
		const FGuid& ItemID,
		int32 ItemParentIndex,
		int32 HeldItemDropIndex);
	void ShowItemPopUpWindow(const EInvSS_ItemCategory ItemCategory, const int32 InSlotIndex);
	void RequestBeginSplit(EInvSS_ItemCategory, int32 InSlotIndex, int32 SplitBarVal);
	void RequestDropItem(EInvSS_ItemCategory ItemCategory, int32 ParentIndex);
	void RequestConsumeItem(EInvSS_ItemCategory ItemCategory, int32 ParentIndex);
	bool RequestDropHeldItem();

	void RequestShowItemDescription(UInvSS_InventoryItem* InventoryItem, int32 ParentIndex);
	void RequestHideItemDescription();


	UPROPERTY(BlueprintAssignable, Category = "Inventory")
	FInvSS_ItemChangedSignature OnInventoryItemAdded;

	UPROPERTY(BlueprintAssignable, Category = "Inventory")
	FInvSS_ItemChangedSignature OnInventoryItemRemoved;

	UPROPERTY(BlueprintAssignable, Category = "Inventory")
	FInvSS_InventoryGridChangedSignature OnInventoryGridChanged;

	UPROPERTY(BlueprintAssignable, Category = "Inventory")
	FInvSS_HeldItemChangedSignature OnInventoryHeldItemChanged;

private:
	UFUNCTION()
	void HandleInventoryItemAdded(UInvSS_InventoryItem* InItem);

	UFUNCTION()
	void HandleInventoryItemRemoved(UInvSS_InventoryItem* InItem);

	UFUNCTION()
	void HandleInventoryMessageRequested(const FText& InMessage) ;

	UFUNCTION()
	void HandleInventoryGridChanged(EInvSS_ItemCategory GridCategory) ;

	UFUNCTION()
	void HandleInventoryHeldItemChanged(FInvSS_HeldItemState HeldItemState);

	bool RebuildGridViewData(EInvSS_ItemCategory ItemCategory);
	void InvalidateGridViewData(EInvSS_ItemCategory ItemCategory);
	void InvalidateAllGridViewData();
	void HideTransientItemWindows();

	UPROPERTY(Transient)
	TMap<EInvSS_ItemCategory, FInvSS_InventoryGridViewData> CachedGridViewData;

	FInvSS_HeldItemState CachedHeldItemState;
	TSet<EInvSS_ItemCategory> DirtyGridCategories;
};
