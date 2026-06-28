#include "Item/Manifest/InvSS_ItemManifest.h"

#include "Item/InvSS_InventoryItem.h"

UInvSS_InventoryItem* FInvSS_ItemManifest::Manifest(UObject* Outer) const
{
	UInvSS_InventoryItem* Item = NewObject<UInvSS_InventoryItem>(Outer);
	Item->SetItemManifest(*this);
	Item->GenerateItemInstanceId();
	return Item;
}
