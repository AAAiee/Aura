// @Copyright HaolunYuan


#include "Widgets/Composite/InvSS_CompositeBase.h"

void UInvSS_CompositeBase::ApplyFunction(FuncType Function)
{
}

FGameplayTag UInvSS_CompositeBase::GetFragmentTag() const
{
	return FragmentTag;
}

void UInvSS_CompositeBase::SetFragmentTag(const FGameplayTag InTag)
{
	FragmentTag = InTag;
}

void UInvSS_CompositeBase::Collapse()
{
}

void UInvSS_CompositeBase::Expand()
{
	SetVisibility(ESlateVisibility::Visible);
}

