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
class UAttributeMenuWidgetController;

/**
 * The HUD class for Aura ！ creates and owns the Overlay Widget + its Widget Controller.
 *
 * The HUD is spawned automatically by the GameMode's HUDClass setting.
 * It only exists on the OWNING CLIENT (not on the server or other clients).
 *
 * Initialization flow (called from AAuraCharacter::InitAbilityActorInfo):
 *   1. InitOverlayWidget() is called with the player's PC, PS, ASC, AS.
 *   2. The Overlay Widget (UMG) is created from OverlayWidgetClass (set in Blueprint).
 *   3. GetWidgetController() creates the WidgetController (lazy singleton pattern).
 *   4. The widget receives the controller ★ WidgetControllerSet fires in Blueprint.
 *   5. BindAllDependencies() subscribes the controller to ASC attribute-change delegates.
 *   6. BroadcastInitialValues() pushes current attribute values so the UI starts correct.
 *   7. Widget is added to viewport.
 */
UCLASS()
class AURA_API AAuraHUD : public AHUD
{
	GENERATED_BODY()

public:

	/**
	 * Returns the cached OverlayWidgetController, or creates one if it doesn't exist yet.
	 * Uses a lazy singleton pattern ！ only one controller per HUD lifetime.
	 */
	UAuraOverlayWidgetController* GetOverlayWidgetController(const FWidgetControllerParameters& Params);


	UAttributeMenuWidgetController* GetAttributeMenuWidgetController(const FWidgetControllerParameters& Params);
	/**
	 * Full overlay initialization ！ creates the widget, wires up the controller,
	 * binds delegates, broadcasts initial values, and adds to viewport.
	 * Must be called BEFORE AbilityActorInfoSet() so the UI listeners exist before GE events fire.
	 */
	void InitOverlayWidget(APlayerController* PC, APlayerState* PS, UAbilitySystemComponent* ASC, UAttributeSet* AS);

private:
	/*Overlay Widget ！ the actual UMG widget displayed on screen*/
	UPROPERTY()
	TObjectPtr<UAuraUserWidget> OverlayWidget;

	/** Set in Blueprint ！ the UMG widget class to create (e.g., WBP_Overlay). */
	UPROPERTY(EditAnywhere)
	TSubclassOf<UAuraUserWidget> OverlayWidgetClass;

	/*Overlay Widget Controller ！ data provider for the overlay*/
	UPROPERTY()
	TObjectPtr<UAuraOverlayWidgetController> OverlayWidgetController;

	/** Set in Blueprint ！ the controller class to create (e.g., BP_OverlayWidgetController). */
	UPROPERTY(EditAnywhere)
	TSubclassOf<UAuraOverlayWidgetController> OverlayWidgetControllerClass;


	/*Attribute Menu Widget ！ the actual UMG widget displayed on screen*/
	UPROPERTY(EditAnywhere)
	TSubclassOf<UAttributeMenuWidgetController> AttributeMenuWidgetControllerClass;

	UPROPERTY()
	TObjectPtr<UAttributeMenuWidgetController> AttributeMenuWidgetController;
};
	
