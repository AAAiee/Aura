// @Copyright HaolunYuan


#include "Widgets/WidgetController/InvSS_WidgetController.h"

#include "InventoryManagement/Component/InvSS_InventoryComponent.h"

void UInvSS_WidgetController::SetWidgetControllerParams(const FInvSS_WidgetControllerParams& Params)
{
	CachedPlayerController = Params.PlayerController;
	CachedInventoryComponent = Params.InventoryComponent;
	CachedUIManager = Params.UIManager;
}

void UInvSS_WidgetController::BroadcastInitialValues()
{
}

void UInvSS_WidgetController::BindAllDependencies()
{
}

const APlayerController* UInvSS_WidgetController::GetPlayerController() const
{
	return CachedPlayerController.Get();
}

const UInvSS_InventoryComponent* UInvSS_WidgetController::GetInventoryComponent() const
{
	return CachedInventoryComponent.Get();
}

UInvSS_InventoryUIManager* UInvSS_WidgetController::GetUIManager() const
{
	return CachedUIManager;
}
