// @Copyright HaolunYuan


#include "UI/Widget/AuraAttributeMenuWidget.h"

#include "UI/WidgetController/AttributeMenuWidgetController.h"

FVector2D UAuraAttributeMenuWidget::GetInitialPosition() const
{
	return FVector2D(OnScreenPositionX, OnScreenPositionY);
}

void UAuraAttributeMenuWidget::ShowAttributeMenu()
{
	// The HUD creates the widget once; showing it again only restores visibility.
	SetVisibility(ESlateVisibility::Visible);
	OnAttributeMenuOnWindowStateChanged.Broadcast(true);

	UAttributeMenuWidgetController* Controller = Cast<UAttributeMenuWidgetController>(WidgetController);
	Controller->BeginAssignmentSession();

	bOnScreen = true;
}

void UAuraAttributeMenuWidget::CloseAttributeMenu()
{
	// Keep the widget alive so drag state/position stay on the cached instance.
	SetVisibility(ESlateVisibility::Hidden);
	OnAttributeMenuOnWindowStateChanged.Broadcast(false);

	UAttributeMenuWidgetController* Controller = Cast<UAttributeMenuWidgetController>(WidgetController);
	Controller->EndAssignmentSession();

	bOnScreen = false;
}
