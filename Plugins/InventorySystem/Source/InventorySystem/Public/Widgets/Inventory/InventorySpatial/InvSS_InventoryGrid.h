// @Copyright HaolunYuan

#pragma once

#include "CoreMinimal.h"
#include "Item/Fragment/InvSS_ItemFragment.h"
#include "Type/InvSS_GridTypes.h"
#include "Widgets/GridSlot/InvSS_GridSlot.h"
#include "Widgets/Inventory/InvSS_InvWidgetBase.h"
#include "InvSS_InventoryGrid.generated.h"

class UInvSS_HoverItem;
class UInvSS_InventoryWidgetController;
struct FInvSS_GridSlotViewData;
struct FInvSS_InventoryGridViewData;
struct FInvSS_GridFragment;
class UInvSS_SlottedItem;
class UInvSS_InventoryItem;
class UCanvasPanel;
class UInvSS_GridSlot;

struct FInvSS_GridSlotRenderSnapshot
{
	FGuid ItemInstanceId;
	TWeakObjectPtr<UInvSS_InventoryItem> Item;
	int32 StackCount = 0;
	bool bOccupied = false;
	bool bParentSlot = false;
};

/**
 * UInvSS_InventoryGrid
 *
 * View-only widget for one category inventory grid.
 *
 * The grid manager owns spatial state. This widget builds the visual slot layout from
 * manager dimensions, listens for grid-change broadcasts, and renders slot/item widgets
 * from the manager's current slot state.
 */
UCLASS()
class INVENTORYSYSTEM_API UInvSS_InventoryGrid : public UInvSS_InvWidgetBase
{
	GENERATED_BODY()

public:
	FORCEINLINE EInvSS_ItemCategory GetItemCategory() const { return ItemCategory; }
	void SetItemCategory(const EInvSS_ItemCategory InItemCategory);
	void ShowCursor();
	void HideCursor();

protected:
	/* UUserWidget begins */
	virtual void NativeTick(const FGeometry& MyGeometry, float InDeltaTime) override;
	virtual void NativeWidgetControllerSet() override;
	/* UUserWidget ends */

private:
	void UpdateTileParameters(const FVector2D& MouseLocalPosition);
	FInvSS_TileParameters BuildTileParametersFromLocalPosition(const FVector2D& MouseLocalPosition) const;
	void OnTileParametersUpdated(FInvSS_TileParameters InTileParameters);
	bool UpdateDropQueryFromTileParameters(const FInvSS_TileParameters& TileParameters);
	FInvSS_SpaceQueryResult CheckHoveredPosition(const FIntPoint& Position, const FIntPoint& Dimensions);
	void HighlightSlots(int32 StartIndex, FIntPoint Range, const EInvSS_GridSlotVisualState ToState);
	void UnHighlightSlots(int32 StartIndex, FIntPoint Range);
	void ClearHighlightedSlots();
	void ResetHighlightTracking();

	void ConstructGrid(int32 GridRows, int32 GridColumns);
	bool IsGridBuiltForViewData(const FInvSS_InventoryGridViewData& ViewData) const;
	UFUNCTION()
	void RefreshFromViewData(EInvSS_ItemCategory Category);
	void RenderFullViewData(const FInvSS_InventoryGridViewData& ViewData);
	void RenderViewDataDelta(const FInvSS_InventoryGridViewData& ViewData);
	static bool HasSlotChanged(const FInvSS_GridSlotRenderSnapshot& OldSlot, const FInvSS_GridSlotViewData& NewSlot);
	void CacheRenderedViewData(const FInvSS_InventoryGridViewData& ViewData);
	void ClearVisualItems();
	void RemoveSlottedItemAtIndex(int32 ParentSlotIndex);
	void RenderGridSlotOccupancy(int32 SlotIndex, bool bOccupied);
	void RenderSlotViewData(int32 SlotIndex, const FInvSS_GridSlotViewData& SlotViewData);
	void AddItemAtIndex(UInvSS_InventoryItem* InItem, int32 InSlotIndex, bool bInIsStackable, int32 InStackCount);
	UInvSS_SlottedItem* CreateSlottedItem() const;
	bool TryConfigureSlottedItem(
		UInvSS_SlottedItem* InSlottedItem,
		UInvSS_InventoryItem* InItem,
		int32 InSlotIndex,
		bool bInIsStackable,
		int32 InStackCount) const;
	void PositionSlottedItem(int32 InIndex, const FInvSS_GridFragment* InGridFragment, UInvSS_SlottedItem* InSlottedItem) const;
	FVector2D GetDrawSize(const FInvSS_GridFragment* GridFragment) const;
	void SetSlottedItemIconBrush(
		const FInvSS_GridFragment* InGridFragment,
		const FInvSS_ImageFragment* InImageFragment,
		UInvSS_SlottedItem* InSlottedItem) const;

	void RemoveItemFromGrid(UInvSS_SlottedItem* ClickedSlottedItem, int32 GridIndex);
	void PickUpFromSlot(UInvSS_SlottedItem* ClickedSlottedItem, int32 GridIndex); 
	bool TryPutDownHeldItem(const FPointerEvent& MouseEvent);
	bool RefreshDropQueryForHeldItem();
	void CreateOrUpdateHoverItemVisual(UInvSS_InventoryItem* HoveredInventoryItem);
	void AssignHoverItemFromHeldState(UInvSS_InventoryItem* HeldItem, int32 SourceParentIndex, int32 StackCount);
	void ClearHoveredItem();
	void ApplyValidHeldItemState(const FInvSS_HeldItemState& HeldItemState);
	void RemoveHeldItemSourceVisual(const FInvSS_HeldItemState& HeldItemState);
	
	UFUNCTION()
	void OnSlottedItemClickedCallback(UInvSS_SlottedItem* SlottedItem, int32 InSlotIndex, const FPointerEvent& InMouseEvent);
	UFUNCTION()
	void OnGridSlotClickedCallback(int32 GridIndex, const FPointerEvent& MouseEvent);
	UFUNCTION()
	void OnGridSlotHoveredCallback(int32 GridIndex, const FPointerEvent& MouseEvent);
	UFUNCTION()
	void OnGridSlotUnhoveredCallback(int32 GridIndex, const FPointerEvent& MouseEvent);
	
	
	UUserWidget* GetCursorVisibleWidget();
	UUserWidget* GetCursorHiddenWidget();


	UFUNCTION()
	void HandleInventoryHeldItemChanged(FInvSS_HeldItemState HeldItemState);
	
	UPROPERTY(EditAnywhere, Category = Inventory)
	TSubclassOf<UInvSS_GridSlot> GridSlotClass = nullptr;
	
	UPROPERTY(EditAnywhere, Category = Inventory)
	TSubclassOf<UInvSS_SlottedItem> SlottedItemClass = nullptr;
	
	UPROPERTY(EditAnywhere, Category = Inventory)
	TSubclassOf<UInvSS_HoverItem> HoveredItemClass = nullptr; 
	
	UPROPERTY(EditAnywhere, Category = Inventory)
	TSubclassOf<UUserWidget> VisibleCursorWidgetClass = nullptr;
	
	UPROPERTY(EditAnywhere, Category = Inventory)
	TSubclassOf<UUserWidget> HiddenCursorWidgetClass = nullptr;
	
	UPROPERTY(EditAnywhere, Category = Inventory)
	float TileSize;

	UPROPERTY(BlueprintReadOnly, Category = Inventory, meta = (AllowPrivateAccess = true))
	EInvSS_ItemCategory ItemCategory;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UCanvasPanel> GridCanvasPanel;

	UPROPERTY(Transient)
	TArray<TObjectPtr<UInvSS_GridSlot>> GridSlots;
	
	UPROPERTY(Transient)
	TMap<int32, TObjectPtr<UInvSS_SlottedItem>> SlottedItemsMap;

	UPROPERTY(Transient)
	TWeakObjectPtr<UInvSS_InventoryWidgetController>  InventoryWidgetController = nullptr;
	
	UPROPERTY(Transient)
	TObjectPtr<UInvSS_HoverItem> HoverItem;
	
	UPROPERTY(Transient)
	TObjectPtr<UUserWidget> VisibleCursorWidget;
	
	UPROPERTY(Transient)
	TObjectPtr<UUserWidget> HiddenCursorWidget;

	int32 RenderedGridRows = 0;
	int32 RenderedGridColumns = 0;
	TArray<FInvSS_GridSlotRenderSnapshot> LastRenderedSlots;
	bool bHasRenderedViewData = false;
	
	FInvSS_TileParameters CurrentTileParameters;
	FInvSS_TileParameters LastTileParameters;
	
	//index where an item would be placed if we click on the grid at a valid location;
	int32 ItemDropIndex{INDEX_NONE}; 
	FInvSS_SpaceQueryResult CurrentQueryResult; 
	bool bMouseInCanvasLastFrame{false};
	bool bMouseInCanvasThisFrame{false};
	int32 LastHighlightSlotStartIndex{-1};
	FIntPoint LastHighlightSlotDimension{-1,-1};
	
};
