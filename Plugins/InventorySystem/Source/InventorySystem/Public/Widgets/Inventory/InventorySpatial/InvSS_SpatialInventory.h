// @Copyright HaolunYuan

#pragma once

#include "CoreMinimal.h"
#include "Widgets/Inventory/InventoryBase/InvSS_InventoryBase.h"
#include "InvSS_SpatialInventory.generated.h"

class UCanvasPanel;
class UButton;
class UWidgetSwitcher;
class UInvSS_InventoryGrid;

/**
 * UInvSS_SpatialInventory
 *
 * Container widget for the category-specific spatial inventory grids.
 *
 * It owns the category buttons, switches between grid widgets, and passes the widget
 * controller down into each child inventory grid.
 */
UCLASS()
class INVENTORYSYSTEM_API UInvSS_SpatialInventory : public UInvSS_InventoryBase
{
	GENERATED_BODY()
	

protected:
	/* UUserWidget begins */
	virtual void NativeOnInitialized() override;
	virtual void NativeWidgetControllerSet() override;
	virtual FReply NativeOnMouseButtonDown(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent) override;
	/* UUserWidget ends */

private:
	UFUNCTION()
	void ShowEquippables();

	UFUNCTION()
	void ShowConsumables();

	UFUNCTION()
	void ShowCraftables();

	UFUNCTION()
	void SetActiveGrid(UInvSS_InventoryGrid* Grid, UButton* Button);

	void DisableButton(UButton* Button) const;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UWidgetSwitcher> Switcher;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UInvSS_InventoryGrid> Grid_Equippables;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UInvSS_InventoryGrid> Grid_Consumables;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UInvSS_InventoryGrid> Grid_Craftables;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UButton> Button_Equippable;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UButton> Button_Consumable;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UButton> Button_Craftable;

	TWeakObjectPtr<UInvSS_InventoryGrid> ActiveGrid = nullptr;
};
