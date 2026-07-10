// @Copyright HaolunYuan

#pragma once

#include "CoreMinimal.h"
#include "InvSS_Leaf.h"
#include "InvSS_Leaf_LabeledValue.generated.h"

class UTextBlock;
/**
 * Leaf widget that renders a label and formatted value pair in the item description tree.
 */
UCLASS()
class INVENTORYSYSTEM_API UInvSS_Leaf_LabeledValue : public UInvSS_Leaf
{
	GENERATED_BODY()

public:
	void SetText_Label(const FText& Text, bool bCollapsed) const;
	void SetText_Value(const FText& Text, bool bCollapsed) const;

protected:
	virtual void NativePreConstruct() override;

private:
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UTextBlock> Text_Label;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UTextBlock> Text_Value;

	UPROPERTY(EditAnywhere, Category = "Inventory")
	int32 FontSize_Label {18};

	UPROPERTY(EditAnywhere, Category = "Inventory")
	int32 FontSize_Value {12};
};
