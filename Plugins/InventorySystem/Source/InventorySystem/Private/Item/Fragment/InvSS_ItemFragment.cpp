// @Copyright HaolunYuan

#include "Item/Fragment/InvSS_ItemFragment.h"

#include "AbilitySystemBlueprintLibrary.h"
#include "AbilitySystemComponent.h"
#include "GameFramework/Pawn.h"
#include "GameFramework/PlayerController.h"
#include "Widgets/Composite/InvSS_CompositeBase.h"
#include "Widgets/Composite/InvSS_Leaf_Image.h"
#include "Widgets/Composite/InvSS_Leaf_LabeledValue.h"
#include "Widgets/Composite/InvSS_Leaf_Text.h"

FGameplayTag FInvSS_ItemFragment::GetFragmentTag() const
{
	return FragmentTag;
}

bool FInvSS_ConsumableFragment::OnConsume(APlayerController* PlayerController) const
{
	if (!IsValid(PlayerController)) return false;
	if (!GameplayEffectClass) return false;

	APawn* Pawn = PlayerController->GetPawn();
	if (!IsValid(Pawn)) return false;

	UAbilitySystemComponent* AbilitySystemComponent =
		UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(Pawn);
	if (!IsValid(AbilitySystemComponent)) return false;

	FGameplayEffectContextHandle ContextHandle = AbilitySystemComponent->MakeEffectContext();
	ContextHandle.AddSourceObject(Pawn);

	const FGameplayEffectSpecHandle SpecHandle =
		AbilitySystemComponent->MakeOutgoingSpec(GameplayEffectClass, EffectLevel, ContextHandle);
	if (!SpecHandle.IsValid()) return false;

	AbilitySystemComponent->ApplyGameplayEffectSpecToSelf(*SpecHandle.Data.Get());
	return true;
}

bool FInvSS_InventoryItemFragment::Assimilate(UInvSS_CompositeBase* Composite) const
{
	if (!IsValid(Composite)) return false;
	if (!MatchesFragmentTag(Composite)) return false;

	Composite->Expand();
	return true;
}

bool FInvSS_InventoryItemFragment::MatchesFragmentTag(const UInvSS_CompositeBase* CompositeBase) const
{
	return CompositeBase->GetFragmentTag().MatchesTagExact(GetFragmentTag());
}

FIntPoint FInvSS_GridFragment::GetGridSize() const
{
	return GridSize;
}

float FInvSS_GridFragment::GetGridPadding() const
{
	return GridPadding;
}

bool FInvSS_ImageFragment::Assimilate(UInvSS_CompositeBase* Composite) const
{
	if (!FInvSS_InventoryItemFragment::Assimilate(Composite)) return false;
	if (!IsValid(Icon)) return false;

	UInvSS_Leaf_Image* ImageLeaf = Cast<UInvSS_Leaf_Image>(Composite);
	if (!IsValid(ImageLeaf)) return false;

	ImageLeaf->SetImage(Icon);
	ImageLeaf->SetBoxSize(IconDimensions);
	ImageLeaf->SetImageSize(IconDimensions);
	return true;
}

TObjectPtr<UTexture2D> FInvSS_ImageFragment::GetMutableImage()
{
	return Icon;
}

void FInvSS_TextFragment::SetText(const FText& InText)
{
	FragmentText = InText;
}

FText FInvSS_TextFragment::GetText() const
{
	return FragmentText;
}

bool FInvSS_TextFragment::Assimilate(UInvSS_CompositeBase* Composite) const
{
	if (!FInvSS_InventoryItemFragment::Assimilate(Composite)) return false;

	UInvSS_Leaf_Text* Leaf_Text = Cast<UInvSS_Leaf_Text>(Composite);
	if (!IsValid(Leaf_Text)) return false ;

	Leaf_Text->SetText(FragmentText);
	return true;
}

bool FInvSS_LabeledNumberFragment::Assimilate(UInvSS_CompositeBase* Composite) const
{
	if (! FInvSS_InventoryItemFragment::Assimilate(Composite)) return false;

	UInvSS_Leaf_LabeledValue* LabeledValueLeaf = Cast<UInvSS_Leaf_LabeledValue>(Composite);
	if (!IsValid(LabeledValueLeaf)) return false;

	LabeledValueLeaf->SetText_Label(Text_Label, bCollapsedLabel);
	FNumberFormattingOptions NumberFormattingOption;
	NumberFormattingOption.MinimumFractionalDigits = MinFractionalDigits;
	NumberFormattingOption.MaximumFractionalDigits = MaxFractionalDigits;
	LabeledValueLeaf->SetText_Value(FText::AsNumber(Text_Value, &NumberFormattingOption), bCollapsedValue);
	return true;
}
