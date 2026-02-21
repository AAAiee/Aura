// @Copyright HaolunYuan


#include "UI/Widget/AuraUserWidget.h"

void UAuraUserWidget::SetWidgetController(UObject* Controller)
{
	check(Controller); 

	WidgetController = Controller;

	// Notify Blueprint that the controller is ready so it can bind delegates and pull initial data
	WidgetControllerSet();
}
