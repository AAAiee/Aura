#include "Item/Manifest/InvSS_ItemManifest.h"

#include "Item/InvSS_InventoryItem.h"
#include "Item/Fragment/InvSS_ItemFragment.h"
#include "Widgets/Composite/InvSS_CompositeBase.h"

UInvSS_InventoryItem* FInvSS_ItemManifest::Manifest(UObject* Outer) const
{
	UInvSS_InventoryItem* Item = NewObject<UInvSS_InventoryItem>(Outer);
	Item->SetItemManifest(*this);
	Item->GenerateItemInstanceId();
	return Item;
}

EInvSS_ItemCategory FInvSS_ItemManifest::GetItemCategory() const
{
	return ItemCategory;
}

FText FInvSS_ItemManifest::GetItemDisplayName() const
{
	return ItemDisplayName;
}

FGameplayTag FInvSS_ItemManifest::GetItemTypeTag() const
{
	return ItemTypeTag;
}

TSubclassOf<AActor> FInvSS_ItemManifest::GetWorldItemActorClass() const
{
	return WorldItemActorClass;
}

void FInvSS_ItemManifest::AssimilateInventoryFragment(UInvSS_CompositeBase* Composite) const
{
	const auto Fragments= GetAllFragmentOfType<FInvSS_InventoryItemFragment>();
	for (const auto Fragment: Fragments)
	{
		Composite->ApplyFunction([Fragment](UInvSS_CompositeBase* Widget)
		{
			Fragment->Assimilate(Widget);
		});
	}
}
