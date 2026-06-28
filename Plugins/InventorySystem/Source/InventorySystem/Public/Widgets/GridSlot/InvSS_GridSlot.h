// @Copyright HaolunYuan

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "InvSS_GridSlot.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FInvSS_GridSlotEventSignature, int32, TileIndex, const FPointerEvent&, MouseEvent);

UENUM(BlueprintType)
enum class EInvSS_GridSlotVisualState : uint8
{
	Unoccupied,
	Occupied,
	Selected,
	GrayedOut,
};
class UImage;
/**
 * @class UInvSS_GridSlot
 * @brief Represents a visual and logical grid slot used in the inventory system.
 *
 * This class provides functionality to manage individual slots in a grid-based inventory.
 * Each slot can be associated with a specific index which determines its position in the grid.
 * The slot also visually represents its state using an image widget.
 */
UCLASS()
class INVENTORYSYSTEM_API UInvSS_GridSlot : public UUserWidget
{
	GENERATED_BODY()

public:
	virtual void NativeOnMouseEnter(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent) override;
	virtual void NativeOnMouseLeave(const FPointerEvent& InMouseEvent) override;
	virtual FReply NativeOnMouseButtonDown(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent) override;
	
	int32 GetTileIndex() const { return TileIndex; }
	bool IsBaseState(const EInvSS_GridSlotVisualState State) const { return BaseVisualState == State; };
	
	void SetTileIndex(const int32 Index) { TileIndex = Index; }
	void SetBaseVisualState(EInvSS_GridSlotVisualState InState);
	void SetTemporaryVisualState(EInvSS_GridSlotVisualState InState);
	void RestoreBaseVisualState();
	void SetOccupiedState();
	void SetUnoccupiedState();
	void SetSelectedState();
	void SetGrayedOutState();
	
	
	FInvSS_GridSlotEventSignature OnGridSlotClicked;
	FInvSS_GridSlotEventSignature OnGridSlotHovered;
	FInvSS_GridSlotEventSignature OnGridSlotUnhovered;

private:
	void ApplyVisualState(EInvSS_GridSlotVisualState InState);

	int32 TileIndex = INDEX_NONE;
	
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UImage> Image_GridSlot;

	UPROPERTY(EditAnywhere, Category = "GridSlotState")
	FSlateBrush UnoccupiedBrush;

	UPROPERTY(EditAnywhere, Category = "GridSlotState")
	FSlateBrush OccupiedBrush;

	UPROPERTY(EditAnywhere, Category = "GridSlotState")
	FSlateBrush SelectedBrush;

	UPROPERTY(EditAnywhere, Category = "GridSlotState")
	FSlateBrush GrayOutBrush;

	EInvSS_GridSlotVisualState BaseVisualState = EInvSS_GridSlotVisualState::Unoccupied;
	EInvSS_GridSlotVisualState CurrentVisualState = EInvSS_GridSlotVisualState::Unoccupied;
};
