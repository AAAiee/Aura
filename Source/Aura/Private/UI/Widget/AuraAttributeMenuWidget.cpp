// @Copyright HaolunYuan


#include "UI/Widget/AuraAttributeMenuWidget.h"
#include "Components/AbilitySystem/AuraAbilitySystemLibrary.h"
#include "UI/WidgetController/AttributeMenuWidgetController.h"

void UAuraAttributeMenuWidget::ShowAttributeMenu()
{
	// The HUD creates the widget once; showing it again only restores visibility.
	SetVisibility(ESlateVisibility::Visible);
	OnAttributeMenuShown.Broadcast();
}

void UAuraAttributeMenuWidget::CloseAttributeMenu()
{
	// Keep the widget alive so drag state/position stay on the cached instance.
	SetVisibility(ESlateVisibility::Hidden);
	OnAttributeMenuClosed.Broadcast();
}
