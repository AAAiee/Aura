// @Copyright HaolunYuan

#pragma once

#include "CoreMinimal.h"
#include "UI/Widget/AuraUserWidget.h"
#include "AuraOverlayRootWidget.generated.h"

class UCanvasPanelSlot;
class UCanvasPanel;

/**
 * Full-screen HUD root widget.
 *
 * Project Extension:
 *   This window-host layer is custom work added on top of the tutorial baseline so
 *   popup widgets can live inside the overlay instead of being spawned as separate
 *   top-level viewport widgets.
 *
 * Besides hosting the always-on HUD pieces (health globe, mana globe, buttons),
 * this widget also exposes a dedicated CanvasPanel named WindowLayer where
 * floating popup windows can be attached and moved around.
 */
UCLASS()
class AURA_API UAuraOverlayRootWidget : public UAuraUserWidget
{
	GENERATED_BODY()

public:
	/** Adds a popup window to the dedicated WindowLayer and returns its Canvas slot for positioning. */
	UCanvasPanelSlot* AddWindowToLayer(UUserWidget* Window, const FVector2D& InPosition, int32 ZOrder = 0);
	
protected:
	/** Bound from WBP_Overlay - must be a full-screen CanvasPanel that hosts draggable popup widgets. */
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UCanvasPanel> WindowLayer;
};
