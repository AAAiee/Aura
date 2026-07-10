// @Copyright HaolunYuan


#include "Widgets/Composite/InvSS_Leaf_Text.h"
#include "Components/TextBlock.h"

void UInvSS_Leaf_Text::SetText(const FText& InText) const
{
	Text_LeafText->SetText(InText);
}

void UInvSS_Leaf_Text::NativePreConstruct()
{
	Super::NativePreConstruct();

	FSlateFontInfo FontInfo = Text_LeafText->GetFont();
	FontInfo.Size = FontSize;
	Text_LeafText->SetFont(FontInfo);
}
