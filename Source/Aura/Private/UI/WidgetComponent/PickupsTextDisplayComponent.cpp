// @Copyright HaolunYuan


#include "UI/WidgetComponent/PickupsTextDisplayComponent.h"

UPickupsTextDisplayComponent::UPickupsTextDisplayComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
	HideText();
	
	SetDrawAtDesiredSize(true) ;
	SetGenerateOverlapEvents(false);
	UPrimitiveComponent::SetCollisionEnabled(ECollisionEnabled::NoCollision);
}

void UPickupsTextDisplayComponent::HideText()
{
	SetVisibility(false, true);
}

