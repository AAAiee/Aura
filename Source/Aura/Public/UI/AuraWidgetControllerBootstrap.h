// @Copyright HaolunYuan

#pragma once

#include "CoreMinimal.h"

class UAuraWidgetController;
class UUserWidget;
struct FWidgetControllerParameters;

/**
 * 
 */
struct AURA_API FAuraWidgetControllerBootstrap
{
public:
	// Shared factory used by both the HUD and world-space widget components so controller
	// creation always follows the same steps: allocate, cache references, then bind delegates.
	static UAuraWidgetController* CreateController(
		UObject* Outer,
		TSubclassOf<UAuraWidgetController> WidgetControllerClass,
		const FWidgetControllerParameters& Params);

	// Shared attach step that connects an already-created controller to a user widget and,
	// when requested, immediately pushes the current model values into the view.
	static void AttachControllerToWidget(
		UUserWidget* Widget,
		UAuraWidgetController* WidgetController,
		bool bBroadcastInitialValues = true);
};
