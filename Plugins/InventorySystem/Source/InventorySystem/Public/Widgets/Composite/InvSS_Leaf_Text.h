// @Copyright HaolunYuan

#pragma once

#include "CoreMinimal.h"
#include "InvSS_Leaf.h"
#include "InvSS_Leaf_Text.generated.h"

class UTextBlock;
/**
 * Leaf widget that renders a text fragment in the item description tree.
 */
UCLASS()
class INVENTORYSYSTEM_API UInvSS_Leaf_Text : public UInvSS_Leaf
{
	GENERATED_BODY()

public:
	void SetText(const FText& InText) const;

protected:
	virtual void NativePreConstruct() override;

private:
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UTextBlock> Text_LeafText;

	UPROPERTY(EditAnywhere, Category= "Inventory")
	int32 FontSize{12};
};
