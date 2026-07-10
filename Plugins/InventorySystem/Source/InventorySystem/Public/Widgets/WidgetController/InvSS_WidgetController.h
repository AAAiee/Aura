// @Copyright HaolunYuan

#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "InvSS_WidgetController.generated.h"

class UInvSS_InventoryUIManager;
class APlayerController;
class UInvSS_InventoryComponent;

/**
 * Dependency bundle used to initialize inventory widget controllers.
 */
USTRUCT(BlueprintType)
struct FInvSS_WidgetControllerParams
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadWrite, Category = "CachedInventoryRef")
	TObjectPtr<APlayerController> PlayerController = nullptr;

	UPROPERTY(BlueprintReadWrite, Category = "CachedInventoryRef")
	TObjectPtr<UInvSS_InventoryComponent> InventoryComponent = nullptr;

	UPROPERTY(BlueprintReadWrite, Category = "CachedInventoryRef")
	TObjectPtr<UInvSS_InventoryUIManager> UIManager;
};

/**
 * UInvSS_WidgetController
 *
 * Base object for inventory UI controllers.
 *
 * Widget controllers bridge gameplay-side inventory components and local UI widgets.
 * Derived controllers bind delegates and broadcast UI-ready events.
 */
UCLASS()
class INVENTORYSYSTEM_API UInvSS_WidgetController : public UObject
{
	GENERATED_BODY()

public:
	/**
	 * @brief Caches dependencies needed by inventory widgets.
	 */
	void SetWidgetControllerParams(const FInvSS_WidgetControllerParams& Params);

	UFUNCTION(BlueprintCallable, Category = "Inventory")
	virtual void BroadcastInitialValues();

	/**
	 * @brief Binds component/UI dependencies used by this controller.
	 */
	virtual void BindAllDependencies();

	UFUNCTION(BlueprintPure, Category = "Refs")
	const APlayerController* GetPlayerController() const;

	UFUNCTION(BlueprintPure, Category = "Refs")
	const UInvSS_InventoryComponent* GetInventoryComponent() const;

	UFUNCTION(BlueprintPure, Category = "Refs")
	UInvSS_InventoryUIManager* GetUIManager() const;

protected:
	UPROPERTY(BlueprintReadOnly, Category = "Refs")
	TWeakObjectPtr<APlayerController> CachedPlayerController;

	UPROPERTY(BlueprintReadOnly, Category = "Refs")
	TWeakObjectPtr<UInvSS_InventoryComponent> CachedInventoryComponent;

	UPROPERTY(BlueprintReadOnly, Category = "Refs")
	TObjectPtr<UInvSS_InventoryUIManager> CachedUIManager;
};
