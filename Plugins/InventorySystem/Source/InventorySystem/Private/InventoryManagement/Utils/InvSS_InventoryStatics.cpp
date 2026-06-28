// @Copyright HaolunYuan


#include "InventoryManagement/Utils/InvSS_InventoryStatics.h"

#include "InventoryManagement/Component/InvSS_InventoryComponent.h"
#include "InventoryManagement/Interface/InvSS_InventoryPlayerControllerInterface.h"
#include "Item/InvSS_ItemComponent.h"

UInvSS_InventoryComponent* UInvSS_InventoryStatics::GetInventoryComponent(
	APlayerController* InPlayerController)
{
	if (!IsValid(InPlayerController)) return nullptr;

	if (IInvSS_InventoryPlayerControllerInterface* IPC = Cast<IInvSS_InventoryPlayerControllerInterface>(InPlayerController))
	{
		return IPC->GetInventoryComponent();
	}

	return nullptr;
}

EInvSS_ItemCategory UInvSS_InventoryStatics::GetItemCategoryFromItemComp(const UInvSS_ItemComponent* InItemComp)
{
	if (!IsValid(InItemComp)) return EInvSS_ItemCategory::None;
	return InItemComp->GetItemManifest().GetItemCategory();
}

bool UInvSS_InventoryStatics::IsLeftMouseButtonPressed(const FPointerEvent& MouseEvent)
{
	return MouseEvent.GetEffectingButton() == EKeys::LeftMouseButton;
}

bool UInvSS_InventoryStatics::IsRightMouseButtonPressed(const FPointerEvent& MouseEvent)
{
	return MouseEvent.GetEffectingButton() == EKeys::RightMouseButton;
}

