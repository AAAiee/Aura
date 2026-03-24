// @Copyright HaolunYuan


#include "UI/Widget/AuraOverlayRootWidget.h"
#include "Components/CanvasPanelSlot.h"
#include "Components/CanvasPanel.h"

UCanvasPanelSlot* UAuraOverlayRootWidget::AddWindowToLayer(UUserWidget* Window, const FVector2D& InPosition, int32 ZOrder /*= 0*/)
{
	if (!WindowLayer || !Window)
	{
		return nullptr;
	}

	UCanvasPanelSlot* CanvasSlot = WindowLayer->AddChildToCanvas(Window);

	if (!CanvasSlot)
	{
		return nullptr; 
	}

	// The popup sizes itself to its desired content, but its position is always interpreted as
	// the top-left corner inside WindowLayer's local canvas space.
	CanvasSlot->SetAutoSize(true);
	CanvasSlot->SetPosition(InPosition); 
	CanvasSlot->SetZOrder(ZOrder); 
	CanvasSlot->SetAlignment(FVector2D::ZeroVector);
	return CanvasSlot;
}
