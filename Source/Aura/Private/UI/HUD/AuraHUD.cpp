// @Copyright HaolunYuan


#include "UI/HUD/AuraHUD.h"

#include "UI/AuraWidgetControllerBootstrap.h"
#include "UI/Widget/AuraAttributeMenuWidget.h"
#include "UI/Widget/AuraOverlayRootWidget.h"
#include "UI/Widget/AuraSpellMenuWidget.h"
#include "UI/WidgetController/AttributeMenuWidgetController.h"
#include "UI/WidgetController/AuraOverlayWidgetController.h"
#include "UI/WidgetController/AuraSpellMenuWidgetController.h"

/**
 * Lazy singleton - creates the widget controller on first request, then caches it.
 * NewObject<> is used instead of CreateDefaultSubobject because the HUD is already constructed
 * by the time we know what params to pass (ASC, AS, etc.).
 */
UAuraOverlayWidgetController* AAuraHUD::GetOverlayWidgetController(const FWidgetControllerParameters& Params)
{
	if (!OverlayWidgetController)
	{
		OverlayWidgetController = CastChecked<UAuraOverlayWidgetController>(FAuraWidgetControllerBootstrap::CreateController(this, OverlayWidgetControllerClass, Params));
	}

	return OverlayWidgetController;
}

UAttributeMenuWidgetController* AAuraHUD::GetAttributeMenuWidgetController(const FWidgetControllerParameters& Params)
{
	if (!AttributeMenuWidgetController)
	{
		AttributeMenuWidgetController = CastChecked<UAttributeMenuWidgetController>(
			FAuraWidgetControllerBootstrap::CreateController(this, AttributeMenuWidgetControllerClass, Params));
	}

	return AttributeMenuWidgetController;
}

UAuraSpellMenuWidgetController* AAuraHUD::GetSpellMenuWidgetController(const FWidgetControllerParameters& Params)
{
	if (!SpellMenuWidgetController)
	{
		SpellMenuWidgetController = CastChecked<UAuraSpellMenuWidgetController>(
			FAuraWidgetControllerBootstrap::CreateController(this, SpellMenuWidgetControllerClass, Params));
	}

	return SpellMenuWidgetController;
}

/**
 * Full overlay initialization sequence:
 *
 *   1. CreateWidget         - instantiate the UMG widget from the Blueprint class.
 *   2. GetWidgetController  - create (or reuse) the data-provider controller.
 *   3. SetWidgetController  - give the widget its controller -> fires WidgetControllerSet in BP.
 *      (Blueprint uses this event to bind UI elements to the controller's delegates.)
 *   4. BindAllDependencies  - subscribe the controller to ASC attribute-change delegates.
 *   5. BroadcastInitialValues - push current Health/Mana so the UI doesn't start at 0.
 *   6. AddToViewport        - display the widget.
 *
 * ORDER MATTERS: steps 4-5 must happen after step 3 so the Blueprint bindings are in place.
 */
void AAuraHUD::InitOverlayWidget(APlayerController* PC, APlayerState* PS, UAbilitySystemComponent* ASC, UAttributeSet* AS)
{
	checkf(OverlayWidgetClass, TEXT("OverlayWidgetClass is not set in %s"), *GetName());
	OverlayWidget = CreateWidget<UAuraOverlayRootWidget>(GetWorld(), OverlayWidgetClass);

	const FWidgetControllerParameters Params(PC, PS, ASC, AS);
	UAuraOverlayWidgetController* WidgetController = GetOverlayWidgetController(Params);

	FAuraWidgetControllerBootstrap::AttachControllerToWidget(OverlayWidget, WidgetController);
	OverlayWidget->AddToViewport();
}

void AAuraHUD::ShowAttributeMenu()
{
	// The first call creates and parents the popup under the Overlay Root; later calls just reuse it.
	UAuraAttributeMenuWidget* AttributeMenu = CreateAttributeMenuWidgetIfNeeded();
	AttributeMenu->ShowAttributeMenu();
}

UAuraAttributeMenuWidget* AAuraHUD::CreateAttributeMenuWidgetIfNeeded()
{
	checkf(AttributeMenuWidgetClass, TEXT("Forget to set AttributeMenuWidgetClass in HUD!"));
	checkf(OverlayWidget, TEXT("OverlayWidget must exist before creating the Attribute Menu."));

	if (AttributeMenuWidget != nullptr)
	{
		return AttributeMenuWidget;
	}

	AttributeMenuWidget = CreateWidget<UAuraAttributeMenuWidget>(GetWorld(), AttributeMenuWidgetClass);
	checkf(AttributeMenuWidget, TEXT("Failed to create Attribute Menu Widget!"));

	// Host the menu inside the OverlayRoot's dedicated floating-window layer rather than as a second top-level viewport widget.
	OverlayWidget->AddWindowToLayer(AttributeMenuWidget, AttributeMenuWidget->GetInitialPosition(), 10);
	AttributeMenuWidget->SetVisibility(ESlateVisibility::Hidden);

	OnAttributeMenuWidgetInstanceConstructed.Broadcast();
	return AttributeMenuWidget;
}

void AAuraHUD::CloseAttributeMenu()
{
	if (AttributeMenuWidget)
	{
		// We hide instead of destroying so the dragged position survives repeated open/close cycles.
		AttributeMenuWidget->CloseAttributeMenu();
	}
}

void AAuraHUD::ShowSpellMenu()
{
	UAuraSpellMenuWidget* SpellMenu = CreateSpellMenuWidgetIfNeeded();
	SpellMenu->ShowSpellMenu();
}

UAuraSpellMenuWidget* AAuraHUD::CreateSpellMenuWidgetIfNeeded()
{
	checkf(SpellMenuWidgetClass, TEXT("Forget to set SpellMenuWidgetClass in HUD!"));
	checkf(OverlayWidget, TEXT("OverlayWidget must exist before creating the Spell Menu."));

	if (SpellMenuWidget != nullptr)
	{
		return SpellMenuWidget;
	}

	SpellMenuWidget = CreateWidget<UAuraSpellMenuWidget>(GetWorld(), SpellMenuWidgetClass);
	checkf(SpellMenuWidget, TEXT("Failed to create Spell Menu Widget!"));

	// Host the menu inside the OverlayRoot's dedicated floating-window layer rather than as a second top-level viewport widget.
	OverlayWidget->AddWindowToLayer(SpellMenuWidget, SpellMenuWidget->GetInitialPosition(), 10);
	SpellMenuWidget->SetVisibility(ESlateVisibility::Hidden);

	OnSpellMenuWidgetInstanceConstructed.Broadcast();
	return SpellMenuWidget;
}

void AAuraHUD::CloseSpellMenu()
{
	if (SpellMenuWidget)
	{
		SpellMenuWidget->CloseSpellMenu();
	}
}

bool AAuraHUD::IsAttributeMenuOnScreen() const
{
	if (AttributeMenuWidget)
	{
		return AttributeMenuWidget->IsOnScreen();
	}

	return false;
}

bool AAuraHUD::IsSpellMenuOnScreen() const
{
	if (SpellMenuWidget)
	{
		return SpellMenuWidget->IsOnScreen();
	}

	return false;
}
