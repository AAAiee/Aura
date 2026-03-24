// @Copyright HaolunYuan


#include "UI/HUD/AuraHUD.h"
#include "UI/Widget/AuraUserWidget.h"
#include "UI/WidgetController/AuraOverlayWidgetController.h"
#include "UI/WidgetController/AttributeMenuWidgetController.h"
#include "Components/AbilitySystem/AuraAbilitySystemLibrary.h"
#include "UI/Widget/AuraAttributeMenuWidget.h"
#include "UI/Widget/AuraOverlayRootWidget.h"

/**
 * Lazy singleton — creates the widget controller on first request, then caches it.
 * NewObject<> is used instead of CreateDefaultSubobject because the HUD is already constructed
 * by the time we know what params to pass (ASC, AS, etc.).
 */
UAuraOverlayWidgetController* AAuraHUD::GetOverlayWidgetController(const FWidgetControllerParameters& Params)
{
	if (!OverlayWidgetController)
	{
		checkf(OverlayWidgetControllerClass, TEXT("OverlayWidgetControllerClass is not set in %s"), *GetName());

		OverlayWidgetController = NewObject<UAuraOverlayWidgetController>(this, OverlayWidgetControllerClass);
		OverlayWidgetController->SetWidgetControllerParams(Params);
		OverlayWidgetController->BindAllDependencies();

	}
	return OverlayWidgetController;
}



UAttributeMenuWidgetController* AAuraHUD::GetAttributeMenuWidgetController(const FWidgetControllerParameters& Params)
{
	if (!AttributeMenuWidgetController)
	{
		checkf(AttributeMenuWidgetControllerClass, TEXT("AttributeMenuWidgetControllerClass is not set in %s"), *GetName());

		AttributeMenuWidgetController = NewObject<UAttributeMenuWidgetController>(this, AttributeMenuWidgetControllerClass);
		AttributeMenuWidgetController->SetWidgetControllerParams(Params);
		AttributeMenuWidgetController->BindAllDependencies();

	}
	return AttributeMenuWidgetController;
}

/**
 * Full overlay initialization sequence:
 *
 *   1. CreateWidget         — instantiate the UMG widget from the Blueprint class.
 *   2. GetWidgetController  — create (or reuse) the data-provider controller.
 *   3. SetWidgetController  — give the widget its controller -> fires WidgetControllerSet in BP.
 *      (Blueprint uses this event to bind UI elements to the controller's delegates.)
 *   4. BindAllDependencies  — subscribe the controller to ASC attribute-change delegates.
 *   5. BroadcastInitialValues — push current Health/Mana so the UI doesn't start at 0.
 *   6. AddToViewport        — display the widget.
 *
 * ORDER MATTERS: steps 4–5 must happen after step 3 so the Blueprint bindings are in place.
 */
void AAuraHUD::InitOverlayWidget(APlayerController* PC, APlayerState* PS, UAbilitySystemComponent* ASC, UAttributeSet* AS)
{
	checkf(OverlayWidgetClass, TEXT("OverlayWidgetClass is not set in %s"), *GetName());
	OverlayWidget = CreateWidget<UAuraOverlayRootWidget>(GetWorld(), OverlayWidgetClass);

	const FWidgetControllerParameters Params(PC, PS, ASC, AS);
	UAuraOverlayWidgetController* WidgetController = GetOverlayWidgetController(Params);

	OverlayWidget->SetWidgetController(WidgetController);

	WidgetController->BroadcastInitialValues();
	OverlayWidget->AddToViewport();
}

void AAuraHUD::ShowAttributeMenu()
{
	// The first call creates and parents the popup under the Overlay Root; later calls just reuse it.
    UAuraAttributeMenuWidget* Menu = CreateAttributeMenuWidgetIfNeeded(); 
	Menu->ShowAttributeMenu();
	bIsAttributeMenuOpen = true;
}

void AAuraHUD::CloseAttributeMenu()
{
	UE_LOG(LogTemp, Warning, TEXT("Closing Attribute Menu"));
	if (AttributeMenuWidget)
	{
		// We hide instead of destroying so the dragged position survives repeated open/close cycles.
		AttributeMenuWidget->CloseAttributeMenu();
		bIsAttributeMenuOpen = false;
	}
}

UAuraAttributeMenuWidget* AAuraHUD::CreateAttributeMenuWidgetIfNeeded()
{
	checkf(AttributeMenuWidgetClass, TEXT("Forget to set AttributeMenuWidgetClass in HUD!"));
	checkf(OverlayWidget, TEXT("OverlayWidget must exist before creating the Attribute Menu."));

	if (AttributeMenuWidget != nullptr)
	{
		return  AttributeMenuWidget;
	}

	AttributeMenuWidget = CreateWidget<UAuraAttributeMenuWidget>(GetWorld(), AttributeMenuWidgetClass); 
	checkf(AttributeMenuWidget, TEXT("Failed to create Attribute Menu Widget!"));

	// Host the menu inside the OverlayRoot's dedicated floating-window layer rather than as a second top-level viewport widget.
	OverlayWidget->AddWindowToLayer(AttributeMenuWidget, AttributeMenuWidget->GetInitialPosition(), 10);
	AttributeMenuWidget->SetVisibility(ESlateVisibility::Hidden);

	OnAttributeMenuWidgetInstanceConstructed.Broadcast();
	return AttributeMenuWidget;
}
