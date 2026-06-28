// @Copyright HaolunYuan

#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "InvSS_InventoryPlayerControllerInterface.generated.h"

class UInvSS_InventoryComponent;

UINTERFACE(MinimalAPI, BlueprintType, NotBlueprintable)
class UInvSS_InventoryPlayerControllerInterface : public UInterface
{
	GENERATED_BODY()
};

/**
 * Interface implemented by player controllers that expose inventory menu and component access.
 */
class INVENTORYSYSTEM_API IInvSS_InventoryPlayerControllerInterface
{
	GENERATED_BODY()

public:
	/**
	 * @brief Toggles the inventory menu for the implementing player controller.
	 */
	UFUNCTION(BlueprintCallable, Category = "Inventory Menu Interaction")
	virtual void ToggleInventoryMenu() = 0;

	/**
	 * @brief Returns the inventory component owned by the implementing player controller.
	 */
	UFUNCTION(BlueprintCallable)
	virtual UInvSS_InventoryComponent* GetInventoryComponent() = 0;
};
