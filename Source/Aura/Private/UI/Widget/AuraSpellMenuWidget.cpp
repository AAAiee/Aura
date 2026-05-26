// @Copyright HaolunYuan

#include "UI/Widget/AuraSpellMenuWidget.h"

void UAuraSpellMenuWidget::ShowSpellMenu()
{
	// The HUD creates this widget once and then toggles visibility. That preserves the window's
	// dragged CanvasPanelSlot position between opens.
	SetVisibility(ESlateVisibility::Visible);
	bOnScreen = true;
	OnSpellMenuOnWindowStateChanged.Broadcast(true);
}

void UAuraSpellMenuWidget::CloseSpellMenu()
{
	// Hiding instead of destroying keeps Blueprint bindings, controller state, and window position intact.
	SetVisibility(ESlateVisibility::Hidden);
	bOnScreen = false;
	OnSpellMenuOnWindowStateChanged.Broadcast(false);
}
