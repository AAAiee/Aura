// @Copyright HaolunYuan


#include "Widgets/Inventory/InvSS_InvWidgetBase.h"


void UInvSS_InvWidgetBase::SetWidgetController(UObject* InWidgetController)
{
	check(InWidgetController);

	if (WidgetController == InWidgetController)
	{
		return;
	}

	// Pipeline:
	// 1. Store the controller object.
	// 2. Let native subclasses bind to it.
	// 3. Let Blueprint widgets react after native setup completes.
	WidgetController = InWidgetController;
	NativeWidgetControllerSet();
	WidgetControllerSet();
}

UObject* UInvSS_InvWidgetBase::GetWidgetController() const
{
	return WidgetController;
}

void UInvSS_InvWidgetBase::NativeWidgetControllerSet()
{
}
