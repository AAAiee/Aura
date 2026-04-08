// @Copyright HaolunYuan


#include "UI/WidgetComponent/ActorStatusWidgetComponent.h"
#include "UI/WidgetController/AuraWidgetController.h"
#include "UI/AuraWidgetControllerBootstrap.h"

UActorStatusWidgetComponent::UActorStatusWidgetComponent()
{
	// Enemy status bars should behave like screen-facing overlays, not like collision-enabled 3D props.
	SetWidgetSpace(EWidgetSpace::Screen);
	SetDrawAtDesiredSize(true);
	SetCollisionEnabled(ECollisionEnabled::NoCollision);
	SetGenerateOverlapEvents(false);
}

void UActorStatusWidgetComponent::InitializeWidgetController(const FWidgetControllerParameters& Params)
{
	checkf(WidgetControllerClass, TEXT("WidgetControllerClass is not set on %s."), *GetName());

	/*
	 * World-space widget initialization flow:
	 *   1. Ensure the widget instance exists.
	 *   2. Create or refresh the backing widget controller.
	 *   3. Attach the controller to the widget.
	 *   4. Let the shared bootstrap push the current values so the bar is correct immediately.
	 *
	 * Reusing the same bootstrap as the HUD keeps screen-space and world-space widgets aligned:
	 * both rely on the same controller lifecycle and initial-value broadcast rules.
	 */
	if (!GetUserWidgetObject())
	{
		InitWidget();
	}

	UUserWidget* UserWidget = GetUserWidgetObject();
	checkf(UserWidget, TEXT("WidgetComponent %s failed to create its widget instance."), *GetName());

	if (!WidgetController)
	{
		// First-time setup: allocate a dedicated controller and bind it to the current actor data.
		WidgetController = FAuraWidgetControllerBootstrap::CreateController(this, WidgetControllerClass, Params);
	}
	else
	{
		// Reinitialization path: keep the existing controller object but refresh its cached references.
		WidgetController->SetWidgetControllerParams(Params);
	}

	FAuraWidgetControllerBootstrap::AttachControllerToWidget(UserWidget, WidgetController);
}
