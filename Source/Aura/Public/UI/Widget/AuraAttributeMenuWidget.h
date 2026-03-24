// @Copyright HaolunYuan

#pragma once

#include "CoreMinimal.h"
#include "AuraDraggableWindowWidget.h"
#include "AuraAttributeMenuWidget.generated.h"


DECLARE_DYNAMIC_MULTICAST_DELEGATE(FAuraAttributeMenuWidgetClosed);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FAuraAttributeMenuWidgetShown);


UCLASS()
class AURA_API UAuraAttributeMenuWidget : public UAuraDraggableWindowWidget
{
	GENERATED_BODY()

public:
	/*
	 * Project Extension:
	 *   The tutorial menu popup was extended here to behave like a hosted floating window
	 *   that can preserve runtime position while being shown/hidden by the HUD.
	 */
	/** Shows the already-created popup without rebuilding its contents. */
	UFUNCTION(BlueprintCallable)
	void ShowAttributeMenu();

	/** Hides the popup so its runtime position survives until the next open. */
	UFUNCTION(BlueprintCallable)
	void CloseAttributeMenu();

	/** Editor-configured default spawn point used the first time the HUD creates the widget. */
	UFUNCTION(BlueprintPure)
    FVector2D GetInitialPosition() const { return FVector2D(OnScreenPositionX, OnScreenPositionY); }
	
	UPROPERTY(BlueprintAssignable, Category="Attribute Menu")
	FAuraAttributeMenuWidgetClosed OnAttributeMenuClosed;

	UPROPERTY(BlueprintAssignable, Category="Attribute Menu")
	FAuraAttributeMenuWidgetShown OnAttributeMenuShown;

private:
	/** Default X position inside the OverlayRoot's WindowLayer. */
	UPROPERTY(EditAnywhere, Category = "UI Properties")
	int32 OnScreenPositionX;

	/** Default Y position inside the OverlayRoot's WindowLayer. */
	UPROPERTY(EditAnywhere, Category = "UI Properties")
	int32 OnScreenPositionY;
};
