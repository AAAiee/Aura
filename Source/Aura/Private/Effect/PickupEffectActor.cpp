// @Copyright HaolunYuan

#include "Effect/PickupEffectActor.h"

#include "Aura/Aura.h"
#include "Components/PrimitiveComponent.h"
#include "UI/WidgetComponent/PickupsTextDisplayComponent.h"

APickupEffectActor::APickupEffectActor()
{
	PrimaryActorTick.bCanEverTick = false;
	
	TextDisplay = CreateDefaultSubobject<UPickupsTextDisplayComponent>(TEXT("TextDisplay"));
	TextDisplay->SetupAttachment(RootComponent);
}

/* Pickup Highlighting : HighLightActor() UnhighLightActor() *****************************/
void APickupEffectActor::HighLightActor()
{
	SetHighlightEnabled(true);
	UpdateItemNamePlate();
}

void APickupEffectActor::UnhighLightActor()
{
	SetHighlightEnabled(false);
	UpdateItemNamePlate();
}

void APickupEffectActor::Pickup_Implementation(AActor* PickupTarget)
{
	OnOverlap(PickupTarget);
}

void APickupEffectActor::ResetPooledState()
{
	Super::ResetPooledState();
	SetHighlightEnabled(false);
}

void APickupEffectActor::BeginPlay()
{
	Super::BeginPlay();
}


void APickupEffectActor::UpdateItemNamePlate()
{
	if (IsValid(TextDisplay))
	{
		if (bIsHighlighted)
		{
			TextDisplay->ShowDisplayText(ItemComponent->GetItemDisplayName());
		}
		else
		{
			TextDisplay->HideText();
		}
	}
}

void APickupEffectActor::SetHighlightEnabled(bool bInHighlighted)
{
	if (bIsHighlighted == bInHighlighted)
	{
		return;
	}

	bIsHighlighted = bInHighlighted;

	TArray<UPrimitiveComponent*> PrimitiveComponents;
	GetComponents<UPrimitiveComponent>(PrimitiveComponents);

	for (UPrimitiveComponent* PrimitiveComponent : PrimitiveComponents)
	{
		if (!IsValid(PrimitiveComponent))
		{
			continue;
		}

		if (PrimitiveComponent == TextDisplay)
		{
			continue;
		}

		// Do not enable highlight on currently invisible components
		if (bIsHighlighted && !PrimitiveComponent->IsVisible())
		{
			continue;
		}

		PrimitiveComponent->SetRenderCustomDepth(bIsHighlighted);

		if (bIsHighlighted)
		{
			PrimitiveComponent->SetCustomDepthStencilValue(CUSTOM_DEPTH_RED);
		}
	}
}
