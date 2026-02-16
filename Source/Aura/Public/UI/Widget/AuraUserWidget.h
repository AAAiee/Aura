// @Copyright HaolunYuan

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "AuraUserWidget.generated.h"

/**
 * Widget base class for Aura. It has a reference to WidgetController, which is responsible for providing data and logic support for the widget. The widget will call functions in the WidgetController to retrieve data and execute logic, and then update the UI accordingly. 
 */
UCLASS()
class AURA_API UAuraUserWidget : public UUserWidget
{
	GENERATED_BODY()
public:
	UFUNCTION(BlueprintCallable)
	void SetWidgetController(UObject* Controller);

protected:
	// Called when WidgetController is set. We can implement this function in Blueprint to do some initialization work, such as binding delegates, retrieving initial data, etc.
	UFUNCTION(BlueprintImplementableEvent)
	void WidgetControllerSet(); 

private:
	UPROPERTY(BlueprintReadOnly, meta = (AllowPrivateAccess = true))
	TObjectPtr<UObject> WidgetController;
};
