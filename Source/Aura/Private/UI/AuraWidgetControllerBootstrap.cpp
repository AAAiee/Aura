// @Copyright HaolunYuan


#include "UI/AuraWidgetControllerBootstrap.h"

#include "UI/Widget/AuraUserWidget.h"
#include "UI/WidgetController/AuraWidgetController.h"

UAuraWidgetController* FAuraWidgetControllerBootstrap::CreateController(UObject* Outer, TSubclassOf<UAuraWidgetController> WidgetControllerClass, const FWidgetControllerParameters& Params)
{
	check(Outer);
	check(WidgetControllerClass);

	// NewObject gives us a runtime-created controller instance whose lifetime is owned by the
	// HUD or widget component that requested it.
	UAuraWidgetController* WidgetController = NewObject<UAuraWidgetController>(Outer, WidgetControllerClass);
	check(WidgetController);

	// The setup order matters:
	//   1. cache the model references
	//   2. bind delegates while those references are valid
	WidgetController->SetWidgetControllerParams(Params);
	WidgetController->BindAllDependencies();

	return WidgetController;
}

void FAuraWidgetControllerBootstrap::AttachControllerToWidget(UUserWidget* Widget, UAuraWidgetController* WidgetController, bool bBroadcastInitialValues /*= true*/)
{
	/*
	 * Attach flow:
	 *   1. Verify the concrete widget is one of Aura's controller-aware widgets.
	 *   2. Give the widget its controller reference.
	 *   3. Optionally broadcast the current values immediately after attachment.
	 *
	 * This sequencing matters because BroadcastInitialValues often drives Blueprint logic that
	 * expects the widget to already know which controller owns it.
	 */
	check(Widget);
	check(WidgetController);

	UAuraUserWidget* AuraWidget = Cast<UAuraUserWidget>(Widget);
	check(AuraWidget);

	AuraWidget->SetWidgetController(WidgetController);
	if (bBroadcastInitialValues)
	{
		// After the view knows which controller drives it, push the current values so the widget
		// does not spend its first frame showing empty or stale data.
		WidgetController->BroadcastInitialValues();
	}
}
