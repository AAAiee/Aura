// @Copyright HaolunYuan


#include "InventoryManagement/Component/InvSS_InventoryComponent.h"

#include "InventoryManagement/Utils/InvSS_InventoryStatics.h"
#include "Item/InvSS_InventoryItem.h"
#include "Item/InvSS_ItemComponent.h"
#include "Item/Fragment/InvSS_ItemFragment.h"
#include "Item/Fragment/InvSS_ItemFragmentTag.h"
#include "Net/UnrealNetwork.h"
#include "Type/InvSS_GridTypes.h"
#include "Widgets/HUD/InvSS_InventoryUIManager.h"


UInvSS_InventoryComponent::UInvSS_InventoryComponent()
	: InventoryList(this)
{
	PrimaryComponentTick.bCanEverTick = false;
	SetIsReplicatedByDefault(true);
	bReplicateUsingRegisteredSubObjectList = true;
}

void UInvSS_InventoryComponent::ReadyForReplication()
{
	Super::ReadyForReplication();
	
	if (!GetOwner() || !GetOwner()->HasAuthority()) return;
	
	GetOrCreateGridManager();
	AddRepSubObj(InventoryGridManager);
}

void UInvSS_InventoryComponent::TryAddItem(UInvSS_ItemComponent* InItem)
{
	check(InItem);

	// Pipeline:
	// 1. Validate authority and find available room in the category grid.
	// 2. Apply authoritative item and grid state on the server.
	// 3. Notify local host UI or mirror stack-only grid changes to the owning client.
	if (AActor* Owner = GetOwner(); !IsValid(Owner) || !Owner->HasAuthority())
	{
		return;
	}

	/*Situation: Inventory is full*/
	FInvSS_BagAvailabilityResult Result;
	if (!TryFindRoomForItem(InItem, Result))
	{
		Client_ShowInventoryMessage(FText::FromString(TEXT("Inventory is Full!")));
		return;
	}

	UInvSS_InventoryGridManager* GridManager = GetOrCreateGridManager(); 
	check(GridManager);

	// if item in result already exists and it is stackable
	if (IsValid(Result.Item.Get()) && Result.bStackable)
	{
		UInvSS_InventoryItem* StackItem = Result.Item.Get();
		StackItem->SetItemTotalStackCountInBag(StackItem->GetItemTotalStackCountInBag() + Result.TotalRoomToFill);
		GridManager->ApplyAvailability(StackItem, Result);
	}
	else
	{
		UInvSS_InventoryItem* NewItem = InventoryList.AddEntry(InItem);
		check(NewItem);
		const int32 StackCount = Result.bStackable ? Result.TotalRoomToFill : 0;
		NewItem->SetItemTotalStackCountInBag(StackCount);
		//SetInventoryItemStackCount(NewItem, StackCount);
		Result.Item = NewItem;
		GridManager->ApplyAvailability(NewItem, Result);
		
		if (OwningPlayerController.IsValid() &&
	OwningPlayerController->IsLocalController())
		{
			OnInventoryItemAddedDelegate.Broadcast(NewItem);
		}
	}

	HandlePickupSourceAfterAdd(InItem, Result.Remainder);
}

void UInvSS_InventoryComponent::Server_BeginDragItem_Implementation(
	const EInvSS_ItemCategory ItemCategory,
	const int32 ItemParentIndex)
{
	if (HeldItemState.IsValid()) return;

	UInvSS_InventoryGridManager* GridManager = GetOrCreateGridManager();
	check(GridManager);

	FGuid RemovedItemId;
	int32 RemovedStackCount = 0;
	if (!GridManager->TryRemoveItemAtParentIndex(
		ItemCategory,
		ItemParentIndex,
		RemovedItemId,
		RemovedStackCount))
	{
		return;
	}

	check(RemovedItemId.IsValid());
	check(RemovedStackCount >= 0);
	check(GetInventoryItemByID(RemovedItemId));

	HeldItemState.ItemId = RemovedItemId;
	HeldItemState.SourceCategory = ItemCategory;
	HeldItemState.SourceParentIndex = ItemParentIndex;
	HeldItemState.StackCount = RemovedStackCount;

	if (OwningPlayerController.IsValid() && OwningPlayerController->IsLocalController())
	{
		OnHeldItemChangedDelegate.Broadcast(HeldItemState);
	}
}


void UInvSS_InventoryComponent::Server_PutDownHeldItemAtIndex_Implementation(const EInvSS_ItemCategory ItemCategory,
	int32 ItemParentIndex)
{
	if (!HeldItemState.IsValid()) return;

	UInvSS_InventoryGridManager* GridManager = GetOrCreateGridManager();
	check(GridManager);

	if (!GridManager->TryAddItemAtGivenIndex(
		ItemCategory,
		ItemParentIndex,
		HeldItemState.ItemId,
		HeldItemState.StackCount))
	{
		return;
	}

	HeldItemState.Reset();

	if (OwningPlayerController.IsValid() && OwningPlayerController->IsLocalController())
	{
		OnHeldItemChangedDelegate.Broadcast(HeldItemState);
	}
}



bool UInvSS_InventoryComponent::TryFindRoomForItem(const UInvSS_ItemComponent* ItemComponent, FInvSS_BagAvailabilityResult& OutResult) const
{
	check(ItemComponent);

	const FInvSS_ItemManifest& ItemManifest = ItemComponent->GetItemManifest();
	const UInvSS_InventoryGridManager* GridManager =  TryGetGridManager();
	check(GridManager);

	UInvSS_InventoryItem* ExistingItem = InventoryList.FindFirstItemByType(ItemManifest.GetItemTypeTag());
	if (IsValid(ExistingItem)
		&& HeldItemState.IsValid()
		&& ExistingItem->GetItemInstanceId() == HeldItemState.ItemId)
	{
		ExistingItem = nullptr;
	}

	return GridManager->FindRoomForItem(ItemManifest, ExistingItem, OutResult);
}

void UInvSS_InventoryComponent::Client_ShowInventoryMessage_Implementation(const FText& Message)
{
	if (GetOrCreateInventoryUIManager())
	{
		OnInventoryMessageRequestedDelegate.Broadcast(Message);
	}
}

void UInvSS_InventoryComponent::SetInventoryItemStackCount(UInvSS_InventoryItem* InItem, const int32 StackCount)
{
	if (!IsValid(InItem)) return;

	if (FInvSS_StackableFragment* StackFragment = InItem->GetItemManifest().GetMutableFragmentOfTypeWithTag<FInvSS_StackableFragment>(ItemFragmentTag::StackableFragment))
	{
		StackFragment->SetStackCount(StackCount);
	}
}

void UInvSS_InventoryComponent::OnInventoryGridChange(EInvSS_ItemCategory InventoryCategory) const
{
	if (!OwningPlayerController.Get() || !OwningPlayerController->IsLocalController()) return;
	OnInventoryGridChangedDelegate.Broadcast(InventoryCategory);
}

void UInvSS_InventoryComponent::OnRep_InventoryGridManager()
{
	check(InventoryGridManager);

	InventoryGridManager->OnGridChanged.RemoveAll(this);
	InventoryGridManager->OnGridChanged.AddUObject(
		this,
		&ThisClass::OnInventoryGridChange);	
	
	// initialization 
	check(InventoryGridManager->GetGridStatesArray().Num() > 0);
	for (const FInvSS_InventoryGridState& GridState : InventoryGridManager->GetGridStatesArray())
	{
		OnInventoryGridChange(GridState.GridConfiguration.Category);
	}
}

void UInvSS_InventoryComponent::OnRep_HeldItemState()
{
	OnHeldItemChangedDelegate.Broadcast(HeldItemState);
}


void UInvSS_InventoryComponent::HandlePickupSourceAfterAdd(UInvSS_ItemComponent* InItemComponent, const int32 Remainder)
{
	check(InItemComponent);

	if (Remainder <= 0)
	{
		InItemComponent->PickUp();
		return;
	}

	InItemComponent->TrySetStackCount(Remainder);
}

void UInvSS_InventoryComponent::ToggleInventoryMenu()
{
	if (UInvSS_InventoryUIManager* UIManager =  GetOrCreateInventoryUIManager())
	{
		UIManager->OnToggleInventoryMenu();
	}
}

UInvSS_InventoryUIManager* UInvSS_InventoryComponent::GetOrCreateInventoryUIManager()
{
	check(OwningPlayerController.IsValid());
	if (!OwningPlayerController->IsLocalController())
	{
		return nullptr;
	}

	if (IsValid(InventoryUIManager))
	{
		return InventoryUIManager;
	}

	TSubclassOf<UInvSS_InventoryUIManager> ManagerClass = InventoryUIManagerClass;
	if (!ManagerClass)
	{
		ManagerClass = UInvSS_InventoryUIManager::StaticClass();
	}

	InventoryUIManager = NewObject<UInvSS_InventoryUIManager>(this, ManagerClass);
	if (InventoryUIManager)
	{
		InventoryUIManager->OnInitialize(OwningPlayerController.Get(), this);
	}
	return InventoryUIManager;
}

const UInvSS_InventoryUIManager* UInvSS_InventoryComponent::GetUIManager() const
{
	if (!ensureMsgf(OwningPlayerController.IsValid(), TEXT("Invalid PC while creating UI Manager in Inventory Component")))
	{
		return nullptr;
	}
	if (!OwningPlayerController->IsLocalController())
	{
		return nullptr;
	}

	if (IsValid(InventoryUIManager))
	{
		return InventoryUIManager;
	}

	return nullptr;
}

UInvSS_InventoryGridManager* UInvSS_InventoryComponent::GetOrCreateGridManager()
{
	if  (IsValid(InventoryGridManager))
	{
		return InventoryGridManager;
	}
	
	if (!GetOwner() || !GetOwner()->HasAuthority()) {return nullptr; }
		
	checkf(InventoryGridManagerClass, TEXT("No grid manager class specified for inventory component %s"), *GetName());
	EnsureDefaultGridManagerConfigs();
	
	InventoryGridManager = NewObject<UInvSS_InventoryGridManager>(
		this,
		InventoryGridManagerClass,
		TEXT("InventoryGridManager"));
	InventoryGridManager->Initialize(GridManagerConfigs);
	InventoryGridManager->OnGridChanged.AddUObject(this, &ThisClass::OnInventoryGridChange);
	return InventoryGridManager;
}

const UInvSS_InventoryGridManager* UInvSS_InventoryComponent::TryGetGridManager() const
{
	return IsValid(InventoryGridManager)? InventoryGridManager : nullptr;
}

void UInvSS_InventoryComponent::AddRepSubObj(UObject* InSubObj)
{
	if (IsUsingRegisteredSubObjectList() && IsReadyForReplication() && IsValid(InSubObj))
	{
		AddReplicatedSubObject(InSubObj);
	}
}

void UInvSS_InventoryComponent::OnInventoryItemReplicated(UInvSS_InventoryItem* InItem) const
{
	if (!IsValid(InItem)) return;

	OnInventoryGridChange(InItem->GetItemManifest().GetItemCategory());
	OnInventoryItemAddedDelegate.Broadcast(InItem);
}

const UInvSS_InventoryItem* UInvSS_InventoryComponent::GetInventoryItemByID(const FGuid& ID) const
{
	return InventoryList.FindItemByID(ID);
}

UInvSS_InventoryItem* UInvSS_InventoryComponent::GetMutableInventoryItemByID(const FGuid& ID)
{
	return InventoryList.FindMutableItemByID(ID);
}

TArray<UInvSS_InventoryItem*> UInvSS_InventoryComponent::GetAllItems() const
{
	return InventoryList.GetAllItems();
}

void UInvSS_InventoryComponent::BeginPlay()
{
	Super::BeginPlay();

	OwningPlayerController = CastChecked<APlayerController>(GetOwner());
}

void UInvSS_InventoryComponent::GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME(ThisClass, InventoryList);
	DOREPLIFETIME(ThisClass, InventoryGridManager);
	DOREPLIFETIME(ThisClass, HeldItemState);
}

void UInvSS_InventoryComponent::EnsureDefaultGridManagerConfigs()
{
	if (!GridManagerConfigs.IsEmpty())
	{
		return;
	}

	GridManagerConfigs =
	{
		{ EInvSS_ItemCategory::Equippable, 4, 8 },
		{ EInvSS_ItemCategory::Consumable, 4, 8 },
		{ EInvSS_ItemCategory::Craftable, 4, 8 }
	};
}
