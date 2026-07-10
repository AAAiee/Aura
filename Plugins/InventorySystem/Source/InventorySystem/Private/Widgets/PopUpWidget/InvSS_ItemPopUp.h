// @Copyright HaolunYuan

#pragma once

#include "CoreMinimal.h"
#include "Type/InvSS_GridTypes.h"
#include "Widgets/Inventory/InvSS_InvWidgetBase.h"
#include "InvSS_ItemPopUp.generated.h"

class UInvSS_InventoryWidgetController;
class USlider;
class USizeBox;
class UTextBlock;
class UButton;
/**
 * @brief Represents a popup widget used within the inventory system UI.
 *
 */
UCLASS()
class INVENTORYSYSTEM_API UInvSS_ItemPopUp : public UInvSS_InvWidgetBase
{
	GENERATED_BODY()

public:
	virtual auto NativeOnMouseLeave(const FPointerEvent& InMouseEvent) -> void override;

	int32 GetSplitAmount() const;
	int32 GetGridIndex() const;
	void SetSliderParams(const float  Max, const float Value) const;
	void SetGridIndex(int32 InIndex);
	void SetItemCategory(EInvSS_ItemCategory InCategory);

	void ResetOptions();
	void CollapseSplitButton();
	void CollapseConsumeButton();
	void CollapseDropButton();
	FVector2D GetBoxSize() const;

protected:
	virtual void NativeWidgetControllerSet() override;

private:
	UFUNCTION()
	void SplitButtonClicked();

	UFUNCTION()
	void DropButtonClicked();

	UFUNCTION()
	void ConsumeButtonClicked();

	UFUNCTION()
	void OnSliderChange(float Value);

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<USizeBox> SizeBox_Root;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UButton> Button_Split;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UButton> Button_Drop;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UButton> Button_Consume;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<USlider> Slider_Split;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UTextBlock> Text_SplitAmount;

	int32 GridIndex = INDEX_NONE;
	EInvSS_ItemCategory ItemCategory = EInvSS_ItemCategory::None;

	TWeakObjectPtr<UInvSS_InventoryWidgetController> CachedInventoryWidgetController = nullptr;
};
