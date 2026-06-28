// @Copyright HaolunYuan


#include "Widgets/Inventory/InventorySpatial/InvSS_InventoryGrid.h"

#include "Widgets/WidgetController/InvSS_InventoryWidgetController.h"

void UInvSS_InventoryGrid::SetItemCategory(const EInvSS_ItemCategory InItemCategory)
{
	if (ItemCategory == InItemCategory) return;

	ItemCategory = InItemCategory;
	LastRenderedSlots.Reset();
	bHasRenderedViewData = false;

	if (InventoryWidgetController.IsValid())
	{
		RefreshFromViewData(ItemCategory);
	}
}

void UInvSS_InventoryGrid::NativeWidgetControllerSet()
{
	Super::NativeWidgetControllerSet();

	// Pipeline:
	// 1. Bind grid refresh and held-item changes from the widget controller.
	// 2. Build this category's visual grid from manager dimensions.
	// 3. Render the current manager slot state.
	InventoryWidgetController = CastChecked<UInvSS_InventoryWidgetController>(GetWidgetController());
	InventoryWidgetController->OnInventoryGridChanged.AddUniqueDynamic(this, &ThisClass::RefreshFromViewData);
	InventoryWidgetController->OnInventoryHeldItemChanged.AddUniqueDynamic(this, &ThisClass::HandleInventoryHeldItemChanged);
	RefreshFromViewData(ItemCategory);
}
