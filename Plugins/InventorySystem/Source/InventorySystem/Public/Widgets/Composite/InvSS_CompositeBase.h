// @Copyright HaolunYuan

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "Widgets/Inventory/InvSS_InvWidgetBase.h"
#include "InvSS_CompositeBase.generated.h"

/**
 * Base widget node for fragment-driven item description composition.
 *
 * Composite and leaf widgets share this interface so item fragments can traverse the
 * description tree and apply data to the widget whose fragment tag matches.
 */
UCLASS()
class INVENTORYSYSTEM_API UInvSS_CompositeBase : public UInvSS_InvWidgetBase
{
	GENERATED_BODY()

public:
	using FuncType = TFunction<void(UInvSS_CompositeBase*)>;

	virtual void ApplyFunction(FuncType Function);

	FGameplayTag GetFragmentTag() const;
	void SetFragmentTag(const FGameplayTag InTag);

	virtual void Collapse();
	void Expand();

private:
	UPROPERTY(EditAnywhere, Category = "Inventory")
	FGameplayTag FragmentTag;
};
