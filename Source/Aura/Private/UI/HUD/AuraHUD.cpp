// @Copyright HaolunYuan


#include "UI/HUD/AuraHUD.h"
#include "UI/Widget/AuraUserWidget.h"
#include "UI/WidgetController/AuraOverlayWidgetController.h"

UAuraOverlayWidgetController* AAuraHUD::GetWidgetController(const FWidgetControllerParameters& Params)
{
	if (!OverlayWidgetController)
	{
		checkf(OverlayWidgetControllerClass, TEXT("OverlayWidgetControllerClass is not set in %s"), *GetName());

		// Create a new Widget Controller and cache it for future use
		OverlayWidgetController = NewObject<UAuraOverlayWidgetController>(this, OverlayWidgetControllerClass);
		OverlayWidgetController->SetWidgetControllerParams(Params);
	}
	return OverlayWidgetController;
}

void AAuraHUD::InitOverlayWidget(APlayerController* PC, APlayerState* PS, UAbilitySystemComponent* ASC, UAttributeSet* AS)
{
	checkf(OverlayWidgetClass, TEXT("OverlayWidgetClass is not set in %s"), *GetName());
	OverlayWidget = CreateWidget<UAuraUserWidget>(GetWorld(), OverlayWidgetClass);

	// Wire up the Widget Controller with the required GAS references
	const FWidgetControllerParameters Params(PC, PS, ASC, AS);
	UAuraOverlayWidgetController* WidgetController = GetWidgetController(Params);

	OverlayWidget->SetWidgetController(WidgetController);

	// Overlay WidgetControllerSet -> SetController For Widgets in the overlay -> (each)WidgetControllerSet-> Bind Delegates ->  BroadcastInitialValues 
	WidgetController->BindAlldDependencies();
	WidgetController->BroadcastInitialValues();

	OverlayWidget->AddToViewport();
}

