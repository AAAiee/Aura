// @Copyright HaolunYuan


#include "Item/InvSS_ItemComponent.h"

#include "Item/Fragment/InvSS_ItemFragment.h"
#include "Item/Fragment/InvSS_ItemFragmentTag.h"
#include "Net/UnrealNetwork.h"


UInvSS_ItemComponent::UInvSS_ItemComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
	
	SetIsReplicatedByDefault(true);
}

bool UInvSS_ItemComponent::TrySetStackCount(const int32 InStackCount)
{
	FInvSS_StackableFragment* StackFragment = ItemManifest.GetMutableFragmentOfTypeWithTag<FInvSS_StackableFragment>(ItemFragmentTag::StackableFragment);
	if (!StackFragment)
	{
		return false;
	}

	StackFragment->SetStackCount(InStackCount);
	return true;
}

void UInvSS_ItemComponent::InitializeItemManifest(const FInvSS_ItemManifest& InItemManifest)
{
	check(GetOwner());
	check(GetOwner()->HasAuthority());

	ItemManifest = InItemManifest;
}

void UInvSS_ItemComponent::PickUp() const
{
	OnPickUp();
	GetOwner()->Destroy();
}

void UInvSS_ItemComponent::GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME(ThisClass, ItemManifest);
}
