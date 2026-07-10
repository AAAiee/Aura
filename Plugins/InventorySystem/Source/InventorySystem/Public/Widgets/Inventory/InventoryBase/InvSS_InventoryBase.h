// @Copyright HaolunYuan

#pragma once

#include "CoreMinimal.h"
#include "Widgets/Inventory/InvSS_InvWidgetBase.h"
#include "InvSS_InventoryBase.generated.h"

class UCanvasPanel;
/**
 * UInvSS_InventoryBase
 *
 * Root inventory menu widget type.
 *
 * The UI manager creates this widget and assigns an inventory widget controller.
 * Specific menu layouts can inherit from this base while sharing controller plumbing
 * from UInvSS_InvWidgetBase.
 */
UCLASS()
class INVENTORYSYSTEM_API UInvSS_InventoryBase : public UInvSS_InvWidgetBase
{
	GENERATED_BODY()

public:
	UCanvasPanel* GetCanvasPanel() const;

protected:
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UCanvasPanel> CanvasPanel_MenuRoot;
};
