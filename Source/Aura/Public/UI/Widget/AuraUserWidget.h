// @Copyright HaolunYuan

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "AuraUserWidget.generated.h"

/**
 * Base widget class for all Aura UI widgets.
 *
 * Design pattern: Model-View-Controller (MVC)
 *   - View  = this widget (and its Blueprint children)
 *   - Controller = WidgetController (provides data + delegates)
 *   - Model = GAS (ASC, AttributeSet, GameplayEffects)
 *
 * The widget never talks to GAS directly - it only knows about its WidgetController.
 * This keeps UI code decoupled from gameplay code.
 *
 * Flow:
 *   1. HUD calls SetWidgetController(controller) on this widget.
 *   2. SetWidgetController stores the reference and calls WidgetControllerSet().
 *   3. WidgetControllerSet is a BlueprintImplementableEvent - the Blueprint child
 *      overrides it to bind UI elements (progress bars, text) to the controller's delegates.
 */
UCLASS()
class AURA_API UAuraUserWidget : public UUserWidget
{
	GENERATED_BODY()
public:
	/** Assigns the controller and fires WidgetControllerSet for Blueprint to bind delegates. */
	UFUNCTION(BlueprintCallable)
	void SetWidgetController(UObject* Controller);

protected:
	/**
	 * BlueprintImplementableEvent - override this in the widget Blueprint.
	 * This is where you bind progress bars, text blocks, etc. to the controller's delegates.
	 * Fires once when the controller is first assigned.
	 */
	UFUNCTION(BlueprintImplementableEvent)
	void WidgetControllerSet();

	/**
	 * Typed as UObject* (not UAuraWidgetController*) for flexibility -
	 * any UObject can serve as a controller, making this widget reusable across different systems.
	 */
	UPROPERTY(BlueprintReadOnly)
	TObjectPtr<UObject> WidgetController;
};
