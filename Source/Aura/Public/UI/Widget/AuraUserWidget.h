// @Copyright HaolunYuan

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "AuraUserWidget.generated.h"

/**
 * Base widget class for the Aura project. Every UI widget that needs access to game data should
 * derive from this class. It holds a reference to a Widget Controller, which acts as the data
 * provider. When the controller is assigned via SetWidgetController, the Blueprint-implementable
 * event WidgetControllerSet is fired so derived widgets can bind delegates and retrieve initial data.
 */
UCLASS()
class AURA_API UAuraUserWidget : public UUserWidget
{
	GENERATED_BODY()
public:
	/**
	 * Assigns the Widget Controller and triggers the WidgetControllerSet event.
	 * @param Controller	The Widget Controller that will provide data to this widget.
	 */
	UFUNCTION(BlueprintCallable)
	void SetWidgetController(UObject* Controller);

protected:
	/**
	 * Blueprint-implementable event called immediately after the Widget Controller is set.
	 * Override in Blueprint to bind delegates, retrieve initial data, and configure child widgets.
	 */
	UFUNCTION(BlueprintImplementableEvent)
	void WidgetControllerSet(); 

private:
	/** Reference to the Widget Controller providing data and logic support for this widget. */
	UPROPERTY(BlueprintReadOnly, meta = (AllowPrivateAccess = true))
	TObjectPtr<UObject> WidgetController;
};
