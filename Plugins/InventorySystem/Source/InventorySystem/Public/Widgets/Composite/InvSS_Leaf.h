// @Copyright HaolunYuan

#pragma once

#include "CoreMinimal.h"
#include "InvSS_CompositeBase.h"
#include "InvSS_Leaf.generated.h"

/**
 * Leaf item description widget node that applies fragment functions directly to itself.
 */
UCLASS()
class INVENTORYSYSTEM_API UInvSS_Leaf : public UInvSS_CompositeBase
{
	GENERATED_BODY()

public:
	virtual void ApplyFunction(FuncType Function) override;
};
