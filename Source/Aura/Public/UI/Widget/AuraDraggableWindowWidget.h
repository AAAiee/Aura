// @Copyright HaolunYuan

#pragma once

#include "CoreMinimal.h"
#include "UI/Widget/AuraUserWidget.h"
#include "AuraDraggableWindowWidget.generated.h"

class UCanvasPanelSlot;

/**
 * Base widget for floating popup windows that live inside a CanvasPanel.
 *
 * Project Extension:
 *   This reusable draggable-window base was added beyond the tutorial flow to support
 *   custom movable UI panels (starting with the Attribute Menu) without pushing that
 *   behavior into the HUD or PlayerController.
 *
 * The key rule is that CanvasPanelSlot positions are stored in the parent canvas's
 * local coordinate space, so drag movement/clamping must also be computed in that space.
 */
UCLASS(Abstract)
class AURA_API UAuraDraggableWindowWidget : public UAuraUserWidget
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintPure, Category = "Drag")
	bool IsDraggingWindow() const { return bIsDraggingWindow; }

protected:
	/** Starts a drag when the player clicks inside the draggable header region. */
	virtual FReply NativeOnMouseButtonDown(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent) override;
	
	/** Ends the drag and releases mouse capture when the left mouse button is released. */
	virtual FReply NativeOnMouseButtonUp(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent) override;

	/** Moves the widget by updating its CanvasPanelSlot position inside the parent canvas. */
	virtual FReply NativeOnMouseMove(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent) override;

	/** Safety net for stopping a drag if the mouse leaves and the button is no longer held. */
	virtual void NativeOnMouseLeave(const FPointerEvent& InMouseEvent) override;

	/** True only when the press lands in the top draggable strip (title-bar region). */
	bool CanStartWindowDrag(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent) const;

	/** Clears runtime drag state once the interaction ends. */
	void StopWindowDrag();

	/** Helper for accessing the slot type that supports manual positioning. */
	UCanvasPanelSlot* GetWindowCanvasSlot() const;

protected:
	// Height of the area at the top of the window that can be used to drag it.
	UPROPERTY(EditAnywhere, Category = "Drag")
	float DragHandleHeight = 30.0f; 

	// Clamp the popup to the parent canvas bounds so it cannot be dragged off-screen.
	UPROPERTY(EditAnywhere, Category = "Drag")
	bool bClampToViewport = true;


private:
	bool IsLeftMouseButtonDown(const FPointerEvent& InMouseEvent) const;

	
private:
	bool bIsDraggingWindow = false;
	// Offset from the widget's top-left corner to the press location, stored in widget-local space.
	FVector2D DragOffset = FVector2D::ZeroVector;
};
