// @Copyright HaolunYuan


#include "Widgets/HoverItem/InvSS_HoverItem.h"

#include "MaterialPropertyHelpers.h"
#include "Components/Image.h"
#include "Components/TextBlock.h"
#include "Item/InvSS_InventoryItem.h"

void UInvSS_HoverItem::SetImageBrush(const FSlateBrush& Brush) const
{
	Image_Icon->SetBrush(Brush);
}

void UInvSS_HoverItem::UpdateHoverItemStackCount(const int32 InStackCount) 
{
	this->StackCount = InStackCount;
	if (InStackCount > 0)
	{
		TextBlock_StackCount->SetText(FText::AsNumber(InStackCount));
		TextBlock_StackCount->SetVisibility(ESlateVisibility::Visible);
	}
	else
	{
		TextBlock_StackCount->SetVisibility(ESlateVisibility::Collapsed);
	}
}

FGameplayTag UInvSS_HoverItem::GetHoveredItemTypeTag() const
{
	if (LinkedInventoryItem.IsValid())
	{
		return LinkedInventoryItem.Get()->GetItemManifest().GetItemTypeTag();
	}
	return FGameplayTag(); 
}

void UInvSS_HoverItem::SetIsStackable(const bool bInIsStackable)
{
	this->bIsStackable = bInIsStackable;
	if (bIsStackable)
	{
		TextBlock_StackCount->SetVisibility(ESlateVisibility::Visible);
	}
	else
	{
		TextBlock_StackCount->SetVisibility(ESlateVisibility::Collapsed);
	}
}

void UInvSS_HoverItem::SetLinkedInventoryItem(UInvSS_InventoryItem* InItem)
{
	LinkedInventoryItem = InItem;
}
