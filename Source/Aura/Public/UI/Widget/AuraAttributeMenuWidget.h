// @Copyright HaolunYuan

#pragma once

#include "CoreMinimal.h"
#include "UI/Widget/AuraDraggableWindowWidget.h"
#include "AuraAttributeMenuWidget.generated.h"


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

	/** Broadcast when HUD show/hide changes this floating window's visible state. */
	UPROPERTY(BlueprintAssignable, Category = "Attribute Menu")
	FIsWidgetAddedToScreenSignature OnAttributeMenuOnWindowStateChanged;

	/** Cached visible state used by PlayerController input gating. */
	FORCEINLINE bool IsOnScreen() const { return bOnScreen; }

private:
	/** Default X position inside the OverlayRoot's WindowLayer. */
	UPROPERTY(EditAnywhere, Category = "UI Properties")
	int32 OnScreenPositionX;

	/** Default Y position inside the OverlayRoot's WindowLayer. */
	UPROPERTY(EditAnywhere, Category = "UI Properties")
	int32 OnScreenPositionY;

	/** True while the cached attribute menu window is visible in the overlay layer. */
	UPROPERTY(Transient)
	bool bOnScreen;
};
