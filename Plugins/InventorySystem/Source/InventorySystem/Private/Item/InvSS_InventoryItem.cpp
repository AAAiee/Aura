// @Copyright HaolunYuan


#include "Item/InvSS_InventoryItem.h"

#include "Item/Fragment/InvSS_ItemFragment.h"
#include "Item/Fragment/InvSS_ItemFragmentTag.h"
#include "Net/UnrealNetwork.h"
#include "StructUtils/InstancedStruct.h"
#include "Item/Manifest/InvSS_ItemManifest.h"

void UInvSS_InventoryItem::SetItemManifest(const FInvSS_ItemManifest& Manifest)
{
	ItemManifest = FInstancedStruct::Make<FInvSS_ItemManifest>(Manifest);
}

bool UInvSS_InventoryItem::IsStackable() const
{
	return GetFragment<FInvSS_StackableFragment>(this, ItemFragmentTag::StackableFragment) != nullptr;
}

void UInvSS_InventoryItem::GenerateItemInstanceId()
{
	if (!ItemInstanceId.IsValid())
	{
		ItemInstanceId = FGuid::NewGuid();
	}
}

void UInvSS_InventoryItem::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	UObject::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME(ThisClass, ItemManifest);
	DOREPLIFETIME(ThisClass, TotalStackCountInBag);	
	DOREPLIFETIME(ThisClass, ItemInstanceId);
}

void UInvSS_InventoryItem::OnRep_TotalStackCount()
{
}
