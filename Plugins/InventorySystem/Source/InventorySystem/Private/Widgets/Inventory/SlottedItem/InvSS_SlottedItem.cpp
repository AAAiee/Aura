// @Copyright HaolunYuan


#include "Widgets/Inventory/SlottedItem/InvSS_SlottedItem.h"
#include "Item/InvSS_InventoryItem.h"
#include "Components/Image.h"
#include "Components/TextBlock.h"

FReply UInvSS_SlottedItem::NativeOnMouseButtonDown(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent)
{
	OnSlottedItemClickedDelegate.Broadcast(this , GridIndex, InMouseEvent);
	return FReply::Handled();
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
