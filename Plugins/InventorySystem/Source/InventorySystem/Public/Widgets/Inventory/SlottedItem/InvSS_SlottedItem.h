// @Copyright HaolunYuan

#pragma once

#include "CoreMinimal.h"
#include "Widgets/Inventory/InvSS_InvWidgetBase.h"
#include "InvSS_SlottedItem.generated.h"

class UInvSS_InventoryItem;
class UImage;
class UTextBlock;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FOnSlottedItemClicked, 
	UInvSS_SlottedItem*, SlottedItem, 
	const int32,  ParentSlotIndex, 
	const FPointerEvent&, MouseEvent);

/**
 * UInvSS_SlottedItem
 *
 * Visual widget for an inventory item placed at a parent grid slot.
 *
 * The inventory grid creates this widget for parent slots only, assigns icon/stack data,
 * and places it on the canvas above the underlying grid slots.
 */
UCLASS()
class INVENTORYSYSTEM_API UInvSS_SlottedItem : public UInvSS_InvWidgetBase
{
	GENERATED_BODY()

public:
	const UImage* GetImage_Icon() const { return Image_Icon; }
	UImage* GetMutableImage() { return Image_Icon; }
	int32 GetGridIndex() const { return GridIndex; }
	FIntPoint GetGridDimensions() const { return GridDimensions; }
	bool GetIsStackable() const { return bIsStackable; }
	UInvSS_InventoryItem* GetInventoryItem() const { return InventoryItem.Get(); }

	void SetGridIndex(const int32 InIndex) { GridIndex = InIndex; }
	void SetGridDimensions(const FInt32Point& InDimensions) { GridDimensions = InDimensions; }
	void SetIsStackable(const bool InIsStackable) { bIsStackable = InIsStackable; }
	void SetInventoryItem(UInvSS_InventoryItem* InItem);
	void SetIconImageBrush(const FSlateBrush& InBrush);
	void UpdateStackCountText(int32 InCount);
	int32 GetStackCount() const { return RenderedStackCount; }

	FOnSlottedItemClicked OnSlottedItemClickedDelegate;
	
protected:
	virtual FReply NativeOnMouseButtonDown(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent) override;
	
private:
	UPROPERTY(meta = (BindWidget))
	UTextBlock* Text_StackCount;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UImage> Image_Icon;

	int32 GridIndex = INDEX_NONE;
	FIntPoint GridDimensions;
	bool bIsStackable{false};
	int32 RenderedStackCount =0;

	TWeakObjectPtr<UInvSS_InventoryItem> InventoryItem;
};
