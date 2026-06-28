// @Copyright HaolunYuan

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "InvSS_InvWidgetBase.generated.h"

class UInvSS_ItemComponent;

/**
 * UInvSS_InvWidgetBase
 *
 * Base class for inventory widgets driven by a widget controller.
 *
 * SetWidgetController() stores the controller, lets C++ widgets react through
 * NativeWidgetControllerSet(), then notifies Blueprint via WidgetControllerSet().
 */
UCLASS()
class INVENTORYSYSTEM_API UInvSS_InvWidgetBase : public UUserWidget
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable, Category = "Inventory")
	virtual void SetWidgetController(UObject* InWidgetController);

	UFUNCTION(BlueprintPure, Category = "Inventory")
	UObject* GetWidgetController() const { return WidgetController; }

protected:
	/**
	 * @brief Native hook called after a new widget controller is assigned.
	 */
	virtual void NativeWidgetControllerSet();

	UFUNCTION(BlueprintImplementableEvent, Category = "Inventory")
	void WidgetControllerSet();

	UPROPERTY(BlueprintReadOnly, Category = "Inventory")
	TObjectPtr<UObject> WidgetController;
};
