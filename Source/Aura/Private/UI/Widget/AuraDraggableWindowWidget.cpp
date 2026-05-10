// @Copyright HaolunYuan


#include "UI/Widget/AuraDraggableWindowWidget.h"

#include "Components/CanvasPanelSlot.h"
#include "Components/PanelWidget.h"

FReply UAuraDraggableWindowWidget::NativeOnMouseButtonDown(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent)
{
	const FReply SuperReply = Super::NativeOnMouseButtonDown(InGeometry, InMouseEvent);

	if (!CanStartWindowDrag(InGeometry, InMouseEvent))
	{
		return SuperReply;
	}

	// The initial grab offset is "where inside this window did the player click?"
	// We store that in the widget's own local space so the same spot stays under the cursor while dragging.
	bIsDraggingWindow = true;
	DragOffset = InGeometry.AbsoluteToLocal(InMouseEvent.GetScreenSpacePosition());

	return FReply::Handled().CaptureMouse(TakeWidget());
}

FReply UAuraDraggableWindowWidget::NativeOnMouseButtonUp(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent)
{
	const FReply SuperReply = Super::NativeOnMouseButtonUp(InGeometry, InMouseEvent);

	if (!bIsDraggingWindow || InMouseEvent.GetEffectingButton() != EKeys::LeftMouseButton)
	{
		return SuperReply;
	}

	StopWindowDrag();
	return FReply::Handled().ReleaseMouseCapture();
}

FReply UAuraDraggableWindowWidget::NativeOnMouseMove(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent)
{
	const FReply SuperReply = Super::NativeOnMouseMove(InGeometry, InMouseEvent);

	if (!bIsDraggingWindow)
	{
		return SuperReply;
	}

	UCanvasPanelSlot* CanvasSlot = GetWindowCanvasSlot();
	if (!CanvasSlot || !GetParent())
	{
		return SuperReply;
	}

	// CanvasPanelSlot positions are expressed in the parent canvas's local space.
	// Converting the mouse into that same space keeps drag math correct in PIE, standalone, and fullscreen.
	const FGeometry& ParentGeometry = GetParent()->GetCachedGeometry();
	const FVector2D MousePositionInParent = ParentGeometry.AbsoluteToLocal(InMouseEvent.GetScreenSpacePosition());

	FVector2D NewPosition = MousePositionInParent - DragOffset;

	if (bClampToViewport)
	{
		// Clamp against the parent canvas, not the viewport.
		// The popup is hosted inside WindowLayer, so that canvas is the true positioning space.
		const FVector2D ParentSize = ParentGeometry.GetLocalSize();
		const FVector2D WindowSize = InGeometry.GetLocalSize();

		NewPosition.X = FMath::Clamp(NewPosition.X, 0.f, FMath::Max(0.f, ParentSize.X - WindowSize.X));
		NewPosition.Y = FMath::Clamp(NewPosition.Y, 0.f, FMath::Max(0.f, ParentSize.Y - WindowSize.Y));
	}

	CanvasSlot->SetPosition(NewPosition);

	return FReply::Handled();
}

void UAuraDraggableWindowWidget::NativeOnMouseLeave(const FPointerEvent& InMouseEvent)
{
	Super::NativeOnMouseLeave(InMouseEvent);

	if (bIsDraggingWindow && !IsLeftMouseButtonDown(InMouseEvent))
	{
		StopWindowDrag();
	}
}

bool UAuraDraggableWindowWidget::CanStartWindowDrag(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent) const
{
	if (!IsLeftMouseButtonDown(InMouseEvent))
	{
		return false;
	}

	// Title-bar style drag: only presses inside the top strip begin a window move.
	const FVector2D LocalMousePosition = InGeometry.AbsoluteToLocal(InMouseEvent.GetScreenSpacePosition());
	return LocalMousePosition.Y <= DragHandleHeight;
}

void UAuraDraggableWindowWidget::StopWindowDrag()
{
	bIsDraggingWindow = false;
}

UCanvasPanelSlot* UAuraDraggableWindowWidget::GetWindowCanvasSlot() const
{
	return Cast<UCanvasPanelSlot>(Slot);
}

bool UAuraDraggableWindowWidget::IsLeftMouseButtonDown(const FPointerEvent& InMouseEvent) const
{
	return InMouseEvent.IsMouseButtonDown(EKeys::LeftMouseButton);
}
