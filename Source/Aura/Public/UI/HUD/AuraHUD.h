// @Copyright HaolunYuan

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/HUD.h"
#include "AuraHUD.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FAuraAttributeMenuWidgetConstructedSignature);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FAuraSpellMenuWidgetConstructedSignature);

class UAbilitySystemComponent;
class UAuraAttributeMenuWidget;
class UAuraOverlayRootWidget;
class UAuraOverlayWidgetController;
class UAttributeMenuWidgetController;
class UAttributeSet;
class UAuraSpellMenuWidget;
class UAuraSpellMenuWidgetController;
struct FWidgetControllerParameters;

/**
 * The HUD class for Aura - creates and owns the Overlay Widget + its Widget Controller.
 *
 * The HUD is spawned automatically by the GameMode's HUDClass setting.
 * It only exists on the OWNING CLIENT (not on the server or other clients).
 *
 * Initialization flow (called from AAuraCharacter::InitAbilityActorInfo):
 *   1. InitOverlayWidget() is called with the player's PC, PS, ASC, AS.
 *   2. The Overlay Widget (UMG) is created from OverlayWidgetClass (set in Blueprint).
 *   3. GetWidgetController() creates the WidgetController (lazy singleton pattern).
 *   4. The widget receives the controller -> WidgetControllerSet fires in Blueprint.
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
	 * Uses a lazy singleton pattern - only one controller per HUD lifetime.
	 */
	UAuraOverlayWidgetController* GetOverlayWidgetController(const FWidgetControllerParameters& Params);

	/**
	 * Returns the cached Attribute Menu Widget Controller, or creates one if it doesn't exist yet.
	 * Uses a lazy singleton pattern - only one controller per HUD lifetime.
	 */
	UAttributeMenuWidgetController* GetAttributeMenuWidgetController(const FWidgetControllerParameters& Params);

	/** Returns the cached Spell Menu Widget Controller, or creates one if it doesn't exist yet. */
	UAuraSpellMenuWidgetController* GetSpellMenuWidgetController(const FWidgetControllerParameters& Params);

	/**
	 * Full overlay initialization - creates the widget, wires up the controller,
	 * binds delegates, broadcasts initial values, and adds to viewport.
	 * Must be called BEFORE AbilityActorInfoSet() so the UI listeners exist before GE events fire.
	 */
	void InitOverlayWidget(APlayerController* PC, APlayerState* PS, UAbilitySystemComponent* ASC, UAttributeSet* AS);

	/**
	 * Floating menu helpers.
	 * The HUD owns each widget lifetime:
	 *   - create once on demand
	 *   - add it to the OverlayRoot's WindowLayer
	 *   - then just hide/show the cached instance
	 */
	UFUNCTION(BlueprintCallable)
	void ShowAttributeMenu();

	UFUNCTION(BlueprintCallable)
	void CloseAttributeMenu();

	UFUNCTION(BlueprintCallable)
	void ShowSpellMenu();

	UFUNCTION(BlueprintCallable)
	void CloseSpellMenu();

	/** Lazy-create the menu, parent it under the overlay's WindowLayer, and cache the instance. */
	UAuraAttributeMenuWidget* CreateAttributeMenuWidgetIfNeeded();
	UAuraSpellMenuWidget* CreateSpellMenuWidgetIfNeeded();

	/** True once the cached menu instance is visible on screen. */
	bool IsAttributeMenuOnScreen() const;
	bool IsSpellMenuOnScreen() const;

private:
	/* Overlay Root - the full-screen HUD widget that also exposes a dedicated WindowLayer for popups. */
	UPROPERTY()
	TObjectPtr<UAuraOverlayRootWidget> OverlayWidget;

	/** Set in Blueprint - the UMG widget class to create (e.g., WBP_Overlay). */
	UPROPERTY(EditAnywhere)
	TSubclassOf<UAuraOverlayRootWidget> OverlayWidgetClass;

	/* Overlay Widget Controller - data provider for the overlay. */
	UPROPERTY()
	TObjectPtr<UAuraOverlayWidgetController> OverlayWidgetController;

	/** Set in Blueprint - the controller class to create (e.g., BP_OverlayWidgetController). */
	UPROPERTY(EditAnywhere)
	TSubclassOf<UAuraOverlayWidgetController> OverlayWidgetControllerClass;

	/* Attribute Menu Controller - data provider for the popup menu. */
	UPROPERTY(EditAnywhere)
	TSubclassOf<UAttributeMenuWidgetController> AttributeMenuWidgetControllerClass;

	UPROPERTY()
	TObjectPtr<UAttributeMenuWidgetController> AttributeMenuWidgetController;

	/* Attribute Menu Widget - cached popup window hosted inside the Overlay Root's WindowLayer. */
	UPROPERTY(BlueprintReadOnly, meta = (AllowPrivateAccess = true))
	TObjectPtr<UAuraAttributeMenuWidget> AttributeMenuWidget;

	UPROPERTY(EditAnywhere)
	TSubclassOf<UAuraAttributeMenuWidget> AttributeMenuWidgetClass;

	UPROPERTY(BlueprintAssignable)
	FAuraAttributeMenuWidgetConstructedSignature OnAttributeMenuWidgetInstanceConstructed;

	/* Spell Menu Widget - cached popup window hosted inside the Overlay Root's WindowLayer. */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, meta = (AllowPrivateAccess = true))
	TObjectPtr<UAuraSpellMenuWidget> SpellMenuWidget;

	/** Set in Blueprint - the UMG widget class to create (e.g., WBP_SpellMenu). */
	UPROPERTY(EditAnywhere)
	TSubclassOf<UAuraSpellMenuWidget> SpellMenuWidgetClass;

	/** Fired after the cached Spell Menu widget is constructed so Blueprint can perform one-time setup. */
	UPROPERTY(BlueprintAssignable)
	FAuraSpellMenuWidgetConstructedSignature OnSpellMenuWidgetInstanceConstructed;

	/** Set in Blueprint - the controller class that feeds spell rows, descriptions, and equip state. */
	UPROPERTY(EditAnywhere)
	TSubclassOf<UAuraSpellMenuWidgetController> SpellMenuWidgetControllerClass;

	/** Spell Menu Controller - data provider for the popup spell tree/equipment menu. */
	UPROPERTY()
	TObjectPtr<UAuraSpellMenuWidgetController> SpellMenuWidgetController;
	
};

