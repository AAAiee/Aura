// @Copyright HaolunYuan


#include "Widgets/ItemDescription/InvSS_ItemDescriptionWindow.h"

#include "Blueprint/WidgetLayoutLibrary.h"
#include "Components/CanvasPanel.h"
#include "Components/CanvasPanelSlot.h"
#include "Components/SizeBox.h"
#include "Widgets/Utilis/InvSS_WidgetUtils.h"

FVector2D UInvSS_ItemDescriptionWindow::GetSize() const
{
	return SizeBox_Root->GetDesiredSize();
}

void UInvSS_ItemDescriptionWindow::SetOwningCanvasPanel(UCanvasPanel* InCanvasPanel)
{
	check(InCanvasPanel);
	OwningCanvasPanel = InCanvasPanel;
}

void UInvSS_ItemDescriptionWindow::NativeTick(const FGeometry& MyGeometry, float InDeltaTime)
{
	Super::NativeTick(MyGeometry, InDeltaTime);

	UpdateWindowPosition();
}

void UInvSS_ItemDescriptionWindow::UpdateWindowPosition()
{
	UCanvasPanel* CanvasPanel = OwningCanvasPanel.Get();
	if (!IsValid(CanvasPanel)) return;

	UCanvasPanelSlot* ItemDescriptionWidgetCps = UWidgetLayoutLibrary::SlotAsCanvasSlot(this);
	if (!ItemDescriptionWidgetCps) return;

	FVector2D MouseLocalPosition;
	if (!UInvSS_WidgetUtils::TryGetMousePositionInWidgetLocal(CanvasPanel, MouseLocalPosition))
	{
		return;
	}

	const FVector2D WidgetSize = GetSize();
	if (WidgetSize.IsNearlyZero()) return;

	const FVector2D DesiredPosition = MouseLocalPosition + MouseOffset;
	const FVector2D ClampedPosition = UInvSS_WidgetUtils::GetClampedWidgetPosition(
		CanvasPanel->GetCachedGeometry().GetLocalSize(),
		WidgetSize,
		DesiredPosition
		);

	ItemDescriptionWidgetCps->SetPosition(ClampedPosition);
	ItemDescriptionWidgetCps->SetSize(WidgetSize);
}
