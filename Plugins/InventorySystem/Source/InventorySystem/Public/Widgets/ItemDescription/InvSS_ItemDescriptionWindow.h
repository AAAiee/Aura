// @Copyright HaolunYuan

#pragma once

#include "CoreMinimal.h"
#include "Widgets/Composite/InvSS_Composite.h"
#include "InvSS_ItemDescriptionWindow.generated.h"

class UCanvasPanel;
class USizeBox;
/**
 * Floating item description window driven by composite item fragments.
 *
 * The UI manager owns this widget and supplies the menu canvas so the window can follow
 * the cursor while clamping itself inside the inventory menu bounds.
 */
UCLASS()
class INVENTORYSYSTEM_API UInvSS_ItemDescriptionWindow : public UInvSS_Composite
{
	GENERATED_BODY()


public:
	FVector2D GetSize() const;
	void SetOwningCanvasPanel(UCanvasPanel* InCanvasPanel);

protected:
	virtual void NativeTick(const FGeometry& MyGeometry, float InDeltaTime) override;


private:
	void UpdateWindowPosition();

	UPROPERTY(EditDefaultsOnly, Category = "Item Description")
	FVector2D MouseOffset = FVector2D(16.f, 16.f);

	TWeakObjectPtr<UCanvasPanel> OwningCanvasPanel;

	UPROPERTY(meta=(BindWidget))
	TObjectPtr<USizeBox> SizeBox_Root;
};
