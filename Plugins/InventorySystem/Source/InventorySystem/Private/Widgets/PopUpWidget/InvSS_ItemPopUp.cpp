// @Copyright HaolunYuan


#include "InvSS_ItemPopUp.h"

#include "Components/Button.h"
#include "Components/SizeBox.h"
#include "Components/Slider.h"
#include "Components/TextBlock.h"
#include "Widgets/WidgetController/InvSS_InventoryWidgetController.h"


void UInvSS_ItemPopUp::NativeOnMouseLeave(const FPointerEvent& InMouseEvent)
{
	Super::NativeOnMouseLeave(InMouseEvent);
	ResetOptions();
	RemoveFromParent();
}

int32 UInvSS_ItemPopUp::GetSplitAmount() const
{
	return FMath::FloorToInt32(Slider_Split->GetValue());
}

int32 UInvSS_ItemPopUp::GetGridIndex() const
{
	return GridIndex;
}

void UInvSS_ItemPopUp::SetSliderParams(const float Max, const float Value) const
{
	Slider_Split->SetMaxValue(Max);
	Slider_Split->SetMinValue(1);
	Slider_Split->SetValue(Value);
	Text_SplitAmount->SetText(FText::AsNumber(FMath::FloorToInt32(Value)));
}

void UInvSS_ItemPopUp::SetGridIndex(const int32 InIndex)
{
	GridIndex = InIndex;
}

void UInvSS_ItemPopUp::SetItemCategory(const EInvSS_ItemCategory InCategory)
{
	ItemCategory = InCategory;
}

void UInvSS_ItemPopUp::NativeWidgetControllerSet()
{
	Button_Split->OnClicked.AddDynamic(this, &UInvSS_ItemPopUp::SplitButtonClicked);
	Button_Drop->OnClicked.AddDynamic(this, &UInvSS_ItemPopUp::DropButtonClicked);
	Button_Consume->OnClicked.AddDynamic(this, &UInvSS_ItemPopUp::ConsumeButtonClicked);
	Slider_Split->OnValueChanged.AddDynamic(this, &UInvSS_ItemPopUp::OnSliderChange);

	CachedInventoryWidgetController = CastChecked<UInvSS_InventoryWidgetController>(WidgetController);
}

void UInvSS_ItemPopUp::SplitButtonClicked()
{
	if (ItemCategory == EInvSS_ItemCategory::None || GridIndex == INDEX_NONE) return;
	if (!CachedInventoryWidgetController.IsValid()) return;

	const int32 CurrentSplitBarVal = FMath::FloorToInt32(Slider_Split->GetValue());
	CachedInventoryWidgetController->RequestBeginSplit(ItemCategory,GridIndex, CurrentSplitBarVal);
	RemoveFromParent();
}

void UInvSS_ItemPopUp::DropButtonClicked()
{
	if (ItemCategory == EInvSS_ItemCategory::None || GridIndex == INDEX_NONE) return;
	if (!CachedInventoryWidgetController.IsValid()) return;

	CachedInventoryWidgetController->RequestDropItem(ItemCategory, GridIndex);
	RemoveFromParent();
}

void UInvSS_ItemPopUp::ConsumeButtonClicked()
{
	if (ItemCategory == EInvSS_ItemCategory::None || GridIndex == INDEX_NONE) return;
	if (!CachedInventoryWidgetController.IsValid()) return;

	CachedInventoryWidgetController->RequestConsumeItem(ItemCategory, GridIndex);
	RemoveFromParent();
}

void UInvSS_ItemPopUp::OnSliderChange(float Value)
{
	Text_SplitAmount->SetText(FText::AsNumber(FMath::FloorToInt32(Value)));
}

void UInvSS_ItemPopUp::ResetOptions()
{
	GridIndex = INDEX_NONE;
	ItemCategory = EInvSS_ItemCategory::None;

	Button_Split->SetVisibility(ESlateVisibility::Visible);
	Slider_Split->SetVisibility(ESlateVisibility::Visible);
	Text_SplitAmount->SetVisibility(ESlateVisibility::Visible);
	Button_Drop->SetVisibility(ESlateVisibility::Visible);
	Button_Consume->SetVisibility(ESlateVisibility::Visible);
}

void UInvSS_ItemPopUp::CollapseSplitButton()
{
	Button_Split->SetVisibility(ESlateVisibility::Collapsed);
	Slider_Split->SetVisibility(ESlateVisibility::Collapsed);
	Text_SplitAmount->SetVisibility(ESlateVisibility::Collapsed);
}

void UInvSS_ItemPopUp::CollapseConsumeButton()
{
	Button_Consume->SetVisibility(ESlateVisibility::Collapsed);
}

void UInvSS_ItemPopUp::CollapseDropButton()
{
	Button_Drop->SetVisibility(ESlateVisibility::Collapsed);
}

FVector2D UInvSS_ItemPopUp::GetBoxSize() const
{
	return FVector2D(SizeBox_Root->GetWidthOverride(), SizeBox_Root->GetHeightOverride());
}
