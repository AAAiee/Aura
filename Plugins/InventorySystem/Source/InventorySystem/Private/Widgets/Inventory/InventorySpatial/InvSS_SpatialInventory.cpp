// @Copyright HaolunYuan


#include "Widgets/Inventory/InventorySpatial/InvSS_SpatialInventory.h"

#include "Components/Button.h"
#include "Components/WidgetSwitcher.h"
#include "InventoryManagement/Utils/InvSS_InventoryStatics.h"
#include "Type/InvSS_GridTypes.h"
#include "Widgets/Inventory/InventorySpatial/InvSS_InventoryGrid.h"
#include "Widgets/WidgetController/InvSS_InventoryWidgetController.h"

void UInvSS_SpatialInventory::NativeOnInitialized()
{
	Super::NativeOnInitialized();

	checkf(Switcher, TEXT("Switcher is not bound on %s."), *GetName());
	checkf(Grid_Consumables, TEXT("Grid_Consumables is not bound on %s."), *GetName());
	checkf(Grid_Craftables, TEXT("Grid_Craftables is not bound on %s."), *GetName());
	checkf(Grid_Equippables, TEXT("Grid_Equippables is not bound on %s."), *GetName());
	checkf(Button_Consumable, TEXT("Button_Consumable is not bound on %s."), *GetName());
	checkf(Button_Craftable, TEXT("Button_Craftable is not bound on %s."), *GetName());
	checkf(Button_Equippable, TEXT("Button_Equippable is not bound on %s."), *GetName());

	Button_Consumable->OnClicked.AddUniqueDynamic(this, &ThisClass::ShowConsumables);
	Button_Craftable->OnClicked.AddUniqueDynamic(this, &ThisClass::ShowCraftables);
	Button_Equippable->OnClicked.AddUniqueDynamic(this, &ThisClass::ShowEquippables);
	
	Grid_Consumables->SetItemCategory(EInvSS_ItemCategory::Consumable);
	Grid_Craftables->SetItemCategory(EInvSS_ItemCategory::Craftable);
	Grid_Equippables->SetItemCategory(EInvSS_ItemCategory::Equippable);

	ShowEquippables();
}

void UInvSS_SpatialInventory::NativeWidgetControllerSet()
{
	Super::NativeWidgetControllerSet();

	Grid_Consumables->SetWidgetController(GetWidgetController());
	Grid_Craftables->SetWidgetController(GetWidgetController());
	Grid_Equippables->SetWidgetController(GetWidgetController());
}

FReply UInvSS_SpatialInventory::NativeOnMouseButtonDown(
	const FGeometry& InGeometry,
	const FPointerEvent& InMouseEvent)
{
	if (UInvSS_InventoryStatics::IsLeftMouseButtonPressed(InMouseEvent))
	{
		UInvSS_InventoryWidgetController* InventoryWidgetController =
			CastChecked<UInvSS_InventoryWidgetController>(GetWidgetController());

		if (InventoryWidgetController->RequestDropHeldItem())
		{
			return FReply::Handled();
		}
	}

	return Super::NativeOnMouseButtonDown(InGeometry, InMouseEvent);
}

void UInvSS_SpatialInventory::ShowEquippables()
{
	SetActiveGrid(Grid_Equippables, Button_Equippable);
}

void UInvSS_SpatialInventory::ShowConsumables()
{
	SetActiveGrid(Grid_Consumables, Button_Consumable);
}

void UInvSS_SpatialInventory::ShowCraftables()
{
	SetActiveGrid(Grid_Craftables, Button_Craftable);
}

void UInvSS_SpatialInventory::SetActiveGrid(UInvSS_InventoryGrid* Grid, UButton* Button)
{
	check(Grid);
	check(Button);
	if (ActiveGrid.IsValid()) ActiveGrid->HideCursor();
	ActiveGrid = Grid;
	if (ActiveGrid.IsValid()) Grid->ShowCursor();
	DisableButton(Button);
	Switcher->SetActiveWidget(Grid);
}

void UInvSS_SpatialInventory::DisableButton(UButton* Button) const
{
	check(Button);

	Button_Consumable->SetIsEnabled(true);
	Button_Equippable->SetIsEnabled(true);
	Button_Craftable->SetIsEnabled(true);
	Button->SetIsEnabled(false);
}
