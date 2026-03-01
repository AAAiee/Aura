// @Copyright HaolunYuan


#include "UI/HUD/AuraHUD.h"
#include "UI/Widget/AuraUserWidget.h"
#include "UI/WidgetController/AuraOverlayWidgetController.h"

/**
 * Lazy singleton ¡ª creates the widget controller on first request, then caches it.
 * NewObject<> is used instead of CreateDefaultSubobject because the HUD is already constructed
 * by the time we know what params to pass (ASC, AS, etc.).
 */
UAuraOverlayWidgetController* AAuraHUD::GetWidgetController(const FWidgetControllerParameters& Params)
{
	if (!OverlayWidgetController)
	{
		checkf(OverlayWidgetControllerClass, TEXT("OverlayWidgetControllerClass is not set in %s"), *GetName());

		OverlayWidgetController = NewObject<UAuraOverlayWidgetController>(this, OverlayWidgetControllerClass);
		OverlayWidgetController->SetWidgetControllerParams(Params);
	}
	return OverlayWidgetController;
}

/**
 * Full overlay initialization sequence:
 *
 *   1. CreateWidget         ¡ª instantiate the UMG widget from the Blueprint class.
 *   2. GetWidgetController  ¡ª create (or reuse) the data-provider controller.
 *   3. SetWidgetController  ¡ª give the widget its controller ¡ú fires WidgetControllerSet in BP.
 *      (Blueprint uses this event to bind UI elements to the controller's delegates.)
 *   4. BindAllDependencies  ¡ª subscribe the controller to ASC attribute-change delegates.
 *   5. BroadcastInitialValues ¡ª push current Health/Mana so the UI doesn't start at 0.
 *   6. AddToViewport        ¡ª display the widget.
 *
 * ORDER MATTERS: steps 4¨C5 must happen after step 3 so the Blueprint bindings are in place.
 */
void AAuraHUD::InitOverlayWidget(APlayerController* PC, APlayerState* PS, UAbilitySystemComponent* ASC, UAttributeSet* AS)
{
	checkf(OverlayWidgetClass, TEXT("OverlayWidgetClass is not set in %s"), *GetName());
	OverlayWidget = CreateWidget<UAuraUserWidget>(GetWorld(), OverlayWidgetClass);

	const FWidgetControllerParameters Params(PC, PS, ASC, AS);
	UAuraOverlayWidgetController* WidgetController = GetWidgetController(Params);

	OverlayWidget->SetWidgetController(WidgetController);

	WidgetController->BindAllDependencies();
	WidgetController->BroadcastInitialValues();

	OverlayWidget->AddToViewport();
}

