// @Copyright HaolunYuan


#include "Widgets/Inventory/SlottedItem/InvSS_SlottedItem.h"
#include "Item/InvSS_InventoryItem.h"
#include "Components/Image.h"
#include "Components/TextBlock.h"
#include "Widgets/WidgetController/InvSS_InventoryWidgetController.h"

FReply UInvSS_SlottedItem::NativeOnMouseButtonDown(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent)
{
	OnSlottedItemClickedDelegate.Broadcast(this , GridIndex, InMouseEvent);
	return FReply::Handled();
}

void UInvSS_SlottedItem::NativeOnMouseEnter(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent)
{
	Super::NativeOnMouseEnter(InGeometry, InMouseEvent);

	InventoryWidgetController->RequestShowItemDescription(InventoryItem.Get(), GridIndex);
}

void UInvSS_SlottedItem::NativeOnMouseLeave(const FPointerEvent& InMouseEvent)
{
	Super::NativeOnMouseLeave(InMouseEvent);

	InventoryWidgetController->RequestHideItemDescription();
}

void UInvSS_SlottedItem::NativeWidgetControllerSet()
{
	InventoryWidgetController = CastChecked<UInvSS_InventoryWidgetController>(GetWidgetController());
}

const UImage* UInvSS_SlottedItem::GetImage_Icon() const
{
	return Image_Icon;
}

UImage* UInvSS_SlottedItem::GetMutableImage()
{
	return Image_Icon;
}

int32 UInvSS_SlottedItem::GetGridIndex() const
{
	return GridIndex;
}

FIntPoint UInvSS_SlottedItem::GetGridDimensions() const
{
	return GridDimensions;
}

bool UInvSS_SlottedItem::GetIsStackable() const
{
	return bIsStackable;
}

UInvSS_InventoryItem* UInvSS_SlottedItem::GetInventoryItem() const
{
	return InventoryItem.Get();
}

void UInvSS_SlottedItem::SetGridIndex(const int32 InIndex)
{
	GridIndex = InIndex;
}

void UInvSS_SlottedItem::SetGridDimensions(const FInt32Point& InDimensions)
{
	GridDimensions = InDimensions;
}

void UInvSS_SlottedItem::SetIsStackable(const bool bInIsStackable)
{
	bIsStackable = bInIsStackable;
}

void UInvSS_SlottedItem::SetInventoryItem(UInvSS_InventoryItem* InItem)
{
	InventoryItem = InItem;
}

void UInvSS_SlottedItem::SetIconImageBrush(const FSlateBrush& InBrush)
{
	Image_Icon->SetBrush(InBrush);
}

void UInvSS_SlottedItem::UpdateStackCountText(const int32 InCount)
{
	RenderedStackCount = InCount;
	if (InCount > 0)
	{
		Text_StackCount->SetText(FText::AsNumber(InCount));
		Text_StackCount->SetVisibility(ESlateVisibility::Visible);
	}
	else
	{
		Text_StackCount->SetVisibility(ESlateVisibility::Collapsed);
	}
}

int32 UInvSS_SlottedItem::GetStackCount() const
{
	return RenderedStackCount;
}
