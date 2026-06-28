// @Copyright HaolunYuan

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "Widgets/Inventory/InvSS_InvWidgetBase.h"
#include "InvSS_HoverItem.generated.h"

class UInvSS_InventoryItem;
class UTextBlock;
class UImage;
/**
 * 
 */
UCLASS()
class INVENTORYSYSTEM_API UInvSS_HoverItem : public UInvSS_InvWidgetBase
{
	GENERATED_BODY()
	
public:
	void SetImageBrush(const FSlateBrush& Brush) const; 
	void UpdateHoverItemStackCount(const int32 InStackCount); 
	
	FGameplayTag GetHoveredItemTypeTag() const;
	FORCEINLINE int32 GetHoveredItemStackCount() const { return StackCount;}
	FORCEINLINE bool IsStackable() const { return bIsStackable; }
	void SetIsStackable(const bool bInIsStackable); 
	FORCEINLINE int32 GetPreviousGridIndex() const{return PreviousGridIndex;}
	FORCEINLINE void SetPreviousGridIndex(const int32 InIndex){PreviousGridIndex = InIndex;}
	FORCEINLINE FIntPoint  GetHoveredItemGridDimensions() const { return GridDimensions; }
	FORCEINLINE void SetHoveredItemGridDimensions(const FIntPoint& InDimensions) { GridDimensions = InDimensions; }
	FORCEINLINE UInvSS_InventoryItem* GetLinkedInventoryItem() const { return LinkedInventoryItem.Get(); }
	void SetLinkedInventoryItem(UInvSS_InventoryItem* InItem);

private:
	UPROPERTY(meta=(BindWidget))
	TObjectPtr<UImage> Image_Icon;
	
	UPROPERTY(meta=(BindWidget))
	TObjectPtr<UTextBlock> TextBlock_StackCount;
	
	int32 PreviousGridIndex{INDEX_NONE}; 
	FIntPoint GridDimensions{1,1};
	TWeakObjectPtr<UInvSS_InventoryItem> LinkedInventoryItem; 
	bool bIsStackable{false};
	int32 StackCount{0};
};
