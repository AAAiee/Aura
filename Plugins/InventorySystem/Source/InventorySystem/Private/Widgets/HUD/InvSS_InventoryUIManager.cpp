// @Copyright HaolunYuan


#include "Widgets/HUD/InvSS_InventoryUIManager.h"

#include "Blueprint/WidgetLayoutLibrary.h"
#include "Components/CanvasPanel.h"
#include "Components/CanvasPanelSlot.h"
#include "InventoryManagement/Component/InvSS_InventoryComponent.h"
#include "Item/InvSS_InventoryItem.h"
#include "Widgets/HUD/InvSS_InfoMessageWidget.h"
#include "Widgets/Inventory/InventoryBase/InvSS_InventoryBase.h"
#include "Widgets/ItemDescription/InvSS_ItemDescriptionWindow.h"
#include "Widgets/PopUpWidget/InvSS_ItemPopUp.h"
#include "Widgets/WidgetController/InvSS_InventoryWidgetController.h"

void UInvSS_InventoryUIManager::OnInitialize(APlayerController* InPlayerController,
                                             UInvSS_InventoryComponent* InInventoryComponent)
{
	check(InPlayerController && InInventoryComponent);
	OwningPlayerController = InPlayerController;
	CachedInventoryComponent = InInventoryComponent;

	ConstructInventoryMenu();
}

void UInvSS_InventoryUIManager::ConstructInventoryMenu()
{
	if (!OwningPlayerController->IsLocalController()) return;

	checkf(InventoryClass, TEXT("InventoryClass is not set on %s."), *GetName());
	InventoryMenu = CreateWidget<UInvSS_InventoryBase>(OwningPlayerController.Get(), InventoryClass);
	check(InventoryMenu);

	TSubclassOf<UInvSS_InventoryWidgetController> ControllerClass = InventoryWidgetControllerClass;
	if (!ControllerClass)
	{
		ControllerClass = UInvSS_InventoryWidgetController::StaticClass();
	}

	InventoryWidgetController = NewObject<UInvSS_InventoryWidgetController>(this, ControllerClass);
	check(InventoryWidgetController);

	FInvSS_WidgetControllerParams WidgetControllerParams;
	WidgetControllerParams.PlayerController = OwningPlayerController.Get();
	WidgetControllerParams.InventoryComponent = CachedInventoryComponent.Get();
	WidgetControllerParams.UIManager = this;
	InventoryWidgetController->SetWidgetControllerParams(WidgetControllerParams);
	InventoryWidgetController->BindAllDependencies();
	InventoryMenu->SetWidgetController(InventoryWidgetController);
	InventoryWidgetController->BroadcastInitialValues();

	InventoryMenu->AddToViewport();
	CloseInventoryMenu();
}

void UInvSS_InventoryUIManager::OnToggleInventoryMenu()
{
	if (bIsOnScreen)
	{
		CloseInventoryMenu();
	}
	else
	{
		ShowInventoryMenu();
	}
}

void UInvSS_InventoryUIManager::ShowInventoryMenu()
{
	checkf(IsValid(InventoryMenu), TEXT("UInvSS_InventoryComponent::ShowInventoryMenu: Invalid Inventory Menu!"));
	InventoryMenu->SetVisibility(ESlateVisibility::Visible);
	bIsOnScreen = true;

	/* for 3d games*/
	/*if (!OwningPlayerController.IsValid()) return;
	FInputModeGameOnly InputMode = FInputModeGameOnly();
	OwningPlayerController->SetInputMode(InputMode);
	OwningPlayerController-> SetShowMouseCursor(false); */
}

void UInvSS_InventoryUIManager::CloseInventoryMenu()
{
	checkf(IsValid(InventoryMenu), TEXT("UInvSS_InventoryComponent::CloseInventoryMenu: Invalid Inventory Menu!"));
	InventoryMenu->SetVisibility(ESlateVisibility::Collapsed);

	bIsOnScreen = false;

	/* for 3d games*/
	/*if (!OwningPlayerController.IsValid()) return;
	FInputModeGameOnly InputMode = FInputModeGameOnly();
	OwningPlayerController->SetInputMode(InputMode);
	OwningPlayerController-> SetShowMouseCursor(false); */
}

void UInvSS_InventoryUIManager::ShowPopUpMessageWidget(const FText& InText)
{
	APlayerController* PC = OwningPlayerController.Get();
	if (!ensureMsgf(IsValid(PC), TEXT("InventoryUIManager has no valid owning PlayerController."))) { return; }
	if (!ensureMsgf(CachedInventoryComponent.IsValid(), TEXT("InventoryUIManager has no valid InventoryComponent."))) { return; }

	if (IsValid(InventoryMessageWidget))
	{
		InventoryMessageWidget->SetMessage(InText);
		return;
	}

	if (!ensureMsgf(InventoryMessageWidgetClass, TEXT("InventoryMessageWidgetClass is not set."))) { return; }

	InventoryMessageWidget = CreateWidget<UInvSS_InfoMessageWidget>(
		PC,
		InventoryMessageWidgetClass
	);

	if (!IsValid(InventoryMessageWidget))
	{
		return;
	}

	FIntPoint ViewportSize;
	PC->GetViewportSize(ViewportSize.X, ViewportSize.Y);

	const FVector2f RawFactor = InventoryMessageWidget->GetMessagePositionOnScreen();

	const FVector2f ClampedFactor(
		FMath::Clamp(RawFactor.X, 0.f, 1.f),
		FMath::Clamp(RawFactor.Y, 0.f, 1.f)
	);

	const FVector2D FinalShowPosition(
		ViewportSize.X * ClampedFactor.X,
		ViewportSize.Y * ClampedFactor.Y
	);

	InventoryMessageWidget->AddToViewport();
	InventoryMessageWidget->SetPositionInViewport(FinalShowPosition, true);
	InventoryMessageWidget->SetMessage(InText);
}

void UInvSS_InventoryUIManager::ShowItemPopUpWindow(const EInvSS_ItemCategory ItemCategory,
													const int32 WindowAppearAtSlotIndex,
													const bool bIsItemStackable,
													const int32 SlotStackCount,
													const bool bCanConsume)
{
	if (!OwningPlayerController.IsValid()) return;

	UCanvasPanel* RootCanvasPanel =  InventoryMenu->GetCanvasPanel();
	check(RootCanvasPanel);

	if (!IsValid(ItemPopUpWindow))
	{
		check(ItemPopUpWindowClass);
		ItemPopUpWindow = CreateWidget<UInvSS_ItemPopUp>(OwningPlayerController.Get(), ItemPopUpWindowClass);
		ItemPopUpWindow->SetWidgetController(InventoryWidgetController);
		check(ItemPopUpWindow);
	}

	// Basic properties needed to pass down
	ItemPopUpWindow->ResetOptions();
	ItemPopUpWindow->SetGridIndex(WindowAppearAtSlotIndex);
	ItemPopUpWindow->SetItemCategory(ItemCategory);
	ItemPopUpWindow->SetVisibility(ESlateVisibility::Visible);

	// if the item is stackable and the stack count at the given slot is > 1 (so we have meaningful split)
	if (const int32 SliderMax = SlotStackCount - 1; bIsItemStackable && SliderMax  > 0)
	{
		ItemPopUpWindow->SetSliderParams(SliderMax, FMath::Max(1, SlotStackCount /2.0f));
	}
	else
	// otherwise just collapse the split button and bar, they are not needed in this interaction
	{
		ItemPopUpWindow->CollapseSplitButton();
	}

	// Consume is available only for items with authored consume behavior.
	if (!bCanConsume)
	{
		ItemPopUpWindow->CollapseConsumeButton();
	}

	// if the window is not in the panel, add it back
	if (!ItemPopUpWindow->GetParent())
	{
		RootCanvasPanel->AddChildToCanvas(ItemPopUpWindow);
	}

	//configure its location
	if (UCanvasPanelSlot* CanvasSlot = UWidgetLayoutLibrary::SlotAsCanvasSlot(ItemPopUpWindow))
	{
		// Get mouse local position in the canvas panel
		const FVector2D MouseViewportPosition =
			UWidgetLayoutLibrary::GetMousePositionOnViewport(OwningPlayerController.Get());

		CanvasSlot->SetPosition(MouseViewportPosition);
		CanvasSlot->SetSize(ItemPopUpWindow->GetBoxSize());
	}
}

void UInvSS_InventoryUIManager::HideItemPopUpWindow()
{
	if (!IsValid(ItemPopUpWindow)) return;

	ItemPopUpWindow->ResetOptions();
	ItemPopUpWindow->RemoveFromParent();
}

void UInvSS_InventoryUIManager::ShowItemDescriptionWindow(
	const EInvSS_ItemCategory ItemCategory,
	const FInvSS_GridSlotViewData& SlotViewData)
{
	if (!OwningPlayerController.IsValid()) return;
	check(ItemCategory != EInvSS_ItemCategory::None);
	check(SlotViewData.Item.Get());

	OwningPlayerController->GetWorldTimerManager().ClearTimer(ItemDescriptionWindowDelayTimerHandle);

	UCanvasPanel* RootCanvasPanel = InventoryMenu->GetCanvasPanel();
	check(RootCanvasPanel);

	if (!IsValid(ItemDescriptionWindowWidget))
	{
		check(ItemDescriptionWindowWidgetClass);
		ItemDescriptionWindowWidget = CreateWidget<UInvSS_ItemDescriptionWindow>(OwningPlayerController.Get(), ItemDescriptionWindowWidgetClass);
		ItemDescriptionWindowWidget->SetWidgetController(InventoryWidgetController);
		check(ItemDescriptionWindowWidget);
	}

	ItemDescriptionWindowWidget->SetOwningCanvasPanel(RootCanvasPanel);
	if (!ItemDescriptionWindowWidget->GetParent())
	{
		RootCanvasPanel->AddChildToCanvas(ItemDescriptionWindowWidget);
	}

	ItemDescriptionWindowWidget->SetVisibility(ESlateVisibility::Collapsed);

	const TWeakObjectPtr DescriptionItem = SlotViewData.Item.Get();
	FTimerDelegate DescriptionTimerDelegate  = FTimerDelegate::CreateWeakLambda(this, [this, DescriptionItem]()
	{
		UInvSS_InventoryItem* InventoryItem = DescriptionItem.Get();
		if (!IsValid(ItemDescriptionWindowWidget) || !IsValid(InventoryItem)) return;

		ItemDescriptionWindowWidget->SetVisibility(ESlateVisibility::HitTestInvisible);
		InventoryItem->GetItemManifest().AssimilateInventoryFragment(ItemDescriptionWindowWidget);
	});

	OwningPlayerController->GetWorldTimerManager(). SetTimer(
		ItemDescriptionWindowDelayTimerHandle,
		DescriptionTimerDelegate,
		DescriptionTimerDelay,
		false);
}

void UInvSS_InventoryUIManager::HideItemDescriptionWindow()
{
	if (!OwningPlayerController.IsValid()) return;
	OwningPlayerController->GetWorldTimerManager().ClearTimer(ItemDescriptionWindowDelayTimerHandle);
	if (!IsValid(ItemDescriptionWindowWidget)) return;

	ItemDescriptionWindowWidget->SetVisibility(ESlateVisibility::Collapsed);
}

