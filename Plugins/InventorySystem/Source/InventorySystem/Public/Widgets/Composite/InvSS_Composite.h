// @Copyright HaolunYuan

#pragma once

#include "CoreMinimal.h"
#include "InvSS_CompositeBase.h"
#include "InvSS_Composite.generated.h"

/**
 * Composite item description widget node that forwards fragment assimilation to child nodes.
 */
UCLASS()
class INVENTORYSYSTEM_API UInvSS_Composite : public UInvSS_CompositeBase
{
	GENERATED_BODY()

public:
	virtual void ApplyFunction(FuncType Function) override;
	virtual void Collapse() override;

protected:
	virtual void NativeWidgetControllerSet() override;

private:
	TArray<TObjectPtr<UInvSS_CompositeBase>> Children ;
};
