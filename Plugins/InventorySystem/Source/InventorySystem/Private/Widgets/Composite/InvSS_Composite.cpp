// @Copyright HaolunYuan


#include "Widgets/Composite/InvSS_Composite.h"

#include "Blueprint/WidgetTree.h"

void UInvSS_Composite::NativeWidgetControllerSet()
{
	Super::NativeWidgetControllerSet();
	Children.Empty();

	WidgetTree->ForEachWidget([this](UWidget* Widget)
	{
		UInvSS_CompositeBase* Composite = Cast<UInvSS_CompositeBase>(Widget);
		if (IsValid(Composite) && Composite != this)
		{
			Children.Add(Composite);
			Composite->Collapse();
		}
	});
}

void UInvSS_Composite::ApplyFunction(FuncType Function)
{
	for (auto Child : Children)
	{
		Child->ApplyFunction(Function);
	}
}

void UInvSS_Composite::Collapse()
{
	for (auto Child : Children)
	{
		Child->Collapse();
	}
}
