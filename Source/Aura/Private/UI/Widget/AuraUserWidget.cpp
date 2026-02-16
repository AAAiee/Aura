// @Copyright HaolunYuan


#include "UI/Widget/AuraUserWidget.h"

void UAuraUserWidget::SetWidgetController(UObject* Controller)
{
	check(Controller); 

	WidgetController = Controller;
	WidgetControllerSet();
}
