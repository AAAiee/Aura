// @Copyright HaolunYuan


#include "Widgets/HUD/InvSS_InventoryUIManager.h"

#include "InventoryManagement/Component/InvSS_InventoryComponent.h"
#include "Widgets/HUD/InvSS_InfoMessageWidget.h"
#include "Widgets/Inventory/InventoryBase/InvSS_InventoryBase.h"
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
