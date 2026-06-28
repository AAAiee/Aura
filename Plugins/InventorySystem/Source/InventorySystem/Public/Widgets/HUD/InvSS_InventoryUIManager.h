// @Copyright HaolunYuan

#pragma once

#include "CoreMinimal.h"
#include "UObject/Object.h"
#include "InvSS_InventoryUIManager.generated.h"

class UInvSS_InfoMessageWidget;
class UInvSS_InventoryBase;
class UInvSS_InventoryComponent;
class UInvSS_InventoryWidgetController;

/**
 * UInvSS_InventoryUIManager
 *
 * Owns inventory UI creation, visibility, widget-controller setup, and popup messages for a local player.
 *
 * The inventory component creates this manager only for local controllers. It keeps widgets out of replicated
 * inventory logic while still exposing a small API for toggling menus and showing inventory messages.
 */
UCLASS(Blueprintable)
class INVENTORYSYSTEM_API UInvSS_InventoryUIManager : public UObject
{
	GENERATED_BODY()

public:
	UInvSS_InventoryUIManager() = default; 

	/**
	 * @brief Caches local UI dependencies and constructs the inventory menu.
	 */
	void OnInitialize(APlayerController* InPlayerController, UInvSS_InventoryComponent* InInventoryComponent);

	/**
	 * @brief Toggles the inventory menu between visible and collapsed states.
	 */
	void OnToggleInventoryMenu();

	/**
	 * @brief Shows or updates the inventory popup message widget.
	 */
	void ShowPopUpMessageWidget(const FText& InText);

protected:
	UPROPERTY()
	TSubclassOf<UInvSS_InventoryBase> InventoryClass = nullptr;

	UPROPERTY(EditDefaultsOnly, Category = "Inventory Menu")
	TSubclassOf<UInvSS_InventoryWidgetController> InventoryWidgetControllerClass = nullptr;

	UPROPERTY(EditDefaultsOnly, Category = "Inventory Menu")
	TSubclassOf<UInvSS_InfoMessageWidget> InventoryMessageWidgetClass = nullptr;

private:
	void ConstructInventoryMenu();
	void CloseInventoryMenu();
	void ShowInventoryMenu();

	bool bIsOnScreen = false;

	TWeakObjectPtr<APlayerController> OwningPlayerController = nullptr;
	TWeakObjectPtr<UInvSS_InventoryComponent> CachedInventoryComponent = nullptr;

	UPROPERTY(Transient)
	TObjectPtr<UInvSS_InventoryBase> InventoryMenu = nullptr;

	UPROPERTY(Transient)
	TObjectPtr<UInvSS_InventoryWidgetController> InventoryWidgetController = nullptr;

	UPROPERTY(Transient)
	TObjectPtr<UInvSS_InfoMessageWidget> InventoryMessageWidget = nullptr;
};
