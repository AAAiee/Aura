// @Copyright HaolunYuan

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/HUD.h"
#include "AuraHUD.generated.h"

class UAuraUserWidget;
class UAuraOverlayWidgetController;
struct FWidgetControllerParameters;
class UAbilitySystemComponent;
class UAttributeSet;

/**
 * AuraHUD is responsible for creating and managing the Overlay Widget and its associated Widget Controller.
 * It serves as the entry point for initializing the UI layer, wiring up the data flow between
 * the Gameplay Ability System and the on-screen widgets.
 */
UCLASS()
class AURA_API AAuraHUD : public AHUD
{
	GENERATED_BODY()

public:

	/**
	 * Returns the Overlay Widget Controller. Creates and initializes one if it does not yet exist.
	 * @param Params	The parameters containing references to PlayerController, PlayerState, ASC, and AttributeSet.
	 * @return			The cached or newly created Overlay Widget Controller.
	 */
	UAuraOverlayWidgetController* GetWidgetController(const FWidgetControllerParameters& Params);

	/**
	 * Creates the Overlay Widget, assigns its Widget Controller, binds dependencies, broadcasts
	 * initial attribute values, and adds the widget to the viewport.
	 * @param PC	The owning Player Controller.
	 * @param PS	The owning Player State.
	 * @param ASC	The Ability System Component providing attribute data.
	 * @param AS	The Attribute Set containing gameplay attributes (Health, Mana, etc.).
	 */
	void InitOverlayWidget(APlayerController* PC, APlayerState* PS, UAbilitySystemComponent* ASC, UAttributeSet* AS);

private:
	/*Overlay Widget Begins*/
	UPROPERTY()
	TObjectPtr<UAuraUserWidget> OverlayWidget;

	UPROPERTY(EditAnywhere)
	TSubclassOf<UAuraUserWidget> OverlayWidgetClass;
	/*Overlay Widget Ends*/

	/*Overlay Widget Controller Begins*/
	UPROPERTY()
	TObjectPtr<UAuraOverlayWidgetController> OverlayWidgetController;

	UPROPERTY(EditAnywhere)
	TSubclassOf<UAuraOverlayWidgetController> OverlayWidgetControllerClass;
	/*Overlay Widget Controller Ends*/
};
