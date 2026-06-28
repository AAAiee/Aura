// @Copyright HaolunYuan


#include "Widgets/WidgetController/InvSS_WidgetController.h"

#include "InventoryManagement/Component/InvSS_InventoryComponent.h"

void UInvSS_WidgetController::SetWidgetControllerParams(const FInvSS_WidgetControllerParams& Params)
{
	CachedPlayerController = Params.PlayerController;
	CachedInventoryComponent = Params.InventoryComponent;
	CachedUIManager = Params.UIManager;
}

