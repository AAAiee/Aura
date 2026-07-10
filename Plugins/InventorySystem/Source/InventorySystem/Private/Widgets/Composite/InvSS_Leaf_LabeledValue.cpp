// @Copyright HaolunYuan


#include "Widgets/Composite/InvSS_Leaf_LabeledValue.h"

#include "Components/TextBlock.h"

void UInvSS_Leaf_LabeledValue::SetText_Label(const FText& Text, bool bCollapsed) const
{
	if (bCollapsed)
	{
		Text_Label->SetVisibility(ESlateVisibility::Collapsed);
		return;
	}
	Text_Label->SetText(Text);
}

void UInvSS_Leaf_LabeledValue::SetText_Value(const FText& Text, const bool bCollapsed) const
{
	if (bCollapsed)
	{
		Text_Value->SetVisibility(ESlateVisibility::Collapsed);
		return;
	}
	Text_Value->SetText(Text);
}

void UInvSS_Leaf_LabeledValue::NativePreConstruct()
{
	Super::NativePreConstruct();

	FSlateFontInfo FontInfo_Label = Text_Label->GetFont();
	FontInfo_Label.Size =FontSize_Label;

	Text_Label->SetFont(FontInfo_Label);

	FSlateFontInfo FontInfo_Value = Text_Label->GetFont();
	FontInfo_Value.Size =  FontSize_Value;
	Text_Value->SetFont(FontInfo_Value);
}
