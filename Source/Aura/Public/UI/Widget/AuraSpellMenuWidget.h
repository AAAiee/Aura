// @Copyright HaolunYuan

#pragma once

#include "CoreMinimal.h"
#include "UI/Widget/AuraDraggableWindowWidget.h"
#include "AuraSpellMenuWidget.generated.h"

/** Floating spell menu window hosted by the HUD overlay layer. */
UCLASS()
class AURA_API UAuraSpellMenuWidget : public UAuraDraggableWindowWidget
{
	GENERATED_BODY()

public:
	/** Shows the already-created popup without rebuilding its contents. */
	UFUNCTION(BlueprintCallable)
	void ShowSpellMenu();

	/** Hides the popup so its runtime position survives until the next open. */
	UFUNCTION(BlueprintCallable)
	void CloseSpellMenu();

	/** Editor-configured default spawn point used the first time the HUD creates the widget. */
	UFUNCTION(BlueprintPure)
	FVector2D GetInitialPosition() const { return FVector2D(OnScreenPositionX, OnScreenPositionY); }

	/** Broadcast when HUD show/hide changes this floating window's visible state. */
	UPROPERTY(BlueprintAssignable, Category = "Spell Menu")
	FIsWidgetAddedToScreenSignature OnSpellMenuOnWindowStateChanged;

	/** Cached visible state used by PlayerController input gating. */
	FORCEINLINE bool IsOnScreen() const { return bOnScreen; }

private:
	/** Default X position inside the OverlayRoot's WindowLayer. */
	UPROPERTY(EditAnywhere, Category = "UI Properties")
	int32 OnScreenPositionX;

	/** Default Y position inside the OverlayRoot's WindowLayer. */
	UPROPERTY(EditAnywhere, Category = "UI Properties")
	int32 OnScreenPositionY;

	/** True while the cached spell menu window is visible in the overlay layer. */
	UPROPERTY(Transient)
	bool bOnScreen;
};
