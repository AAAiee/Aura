// @Copyright HaolunYuan


#include "UI/Widget/AuraUserWidget.h"

void UAuraUserWidget::SetWidgetController(UObject* Controller)
{
	check(Controller);

	WidgetController = Controller;

	// Fire the Blueprint event so the widget can bind its UI elements to the controller's delegates.
	// This must happen before BroadcastInitialValues() is called, otherwise the first broadcast
	// has no Blueprint listeners yet.
	WidgetControllerSet();
}
