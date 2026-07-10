// @Copyright HaolunYuan


#include "Widgets/GridSlot/InvSS_GridSlot.h"
#include "Components/Image.h"

void UInvSS_GridSlot::NativeOnMouseEnter(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent)
{
	Super::NativeOnMouseEnter(InGeometry, InMouseEvent);

	OnGridSlotHovered.Broadcast(TileIndex, InMouseEvent);
}

void UInvSS_GridSlot::NativeOnMouseLeave(const FPointerEvent& InMouseEvent)
{
	Super::NativeOnMouseLeave(InMouseEvent);
	OnGridSlotUnhovered.Broadcast(TileIndex, InMouseEvent);
}

FReply UInvSS_GridSlot::NativeOnMouseButtonDown(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent)
{
	OnGridSlotClicked.Broadcast(TileIndex, InMouseEvent);
	return  FReply::Handled();
}

int32 UInvSS_GridSlot::GetTileIndex() const
{
	return TileIndex;
}

bool UInvSS_GridSlot::IsBaseState(const EInvSS_GridSlotVisualState State) const
{
	return BaseVisualState == State;
}

void UInvSS_GridSlot::SetTileIndex(const int32 InIndex)
{
	TileIndex = InIndex;
}

void UInvSS_GridSlot::SetBaseVisualState(const EInvSS_GridSlotVisualState InState)
{
	BaseVisualState = InState;
	ApplyVisualState(InState);
}

void UInvSS_GridSlot::SetTemporaryVisualState(const EInvSS_GridSlotVisualState InState)
{
	ApplyVisualState(InState);
}

void UInvSS_GridSlot::RestoreBaseVisualState()
{
	ApplyVisualState(BaseVisualState);
}

void UInvSS_GridSlot::SetOccupiedState()
{
	SetTemporaryVisualState(EInvSS_GridSlotVisualState::Occupied);
}

void UInvSS_GridSlot::SetUnoccupiedState()
{
	SetTemporaryVisualState(EInvSS_GridSlotVisualState::Unoccupied);
}

void UInvSS_GridSlot::SetSelectedState()
{
	SetTemporaryVisualState(EInvSS_GridSlotVisualState::Selected);
}

void UInvSS_GridSlot::SetGrayedOutState()
{
	SetTemporaryVisualState(EInvSS_GridSlotVisualState::GrayedOut);
}

void UInvSS_GridSlot::ApplyVisualState(const EInvSS_GridSlotVisualState InState)
{
	check(Image_GridSlot);

	CurrentVisualState = InState;
	switch (CurrentVisualState)
	{
	case EInvSS_GridSlotVisualState::Unoccupied:
		Image_GridSlot->SetBrush(UnoccupiedBrush);
		break;
	case EInvSS_GridSlotVisualState::Occupied:
		Image_GridSlot->SetBrush(OccupiedBrush);
		break;
	case EInvSS_GridSlotVisualState::Selected:
		Image_GridSlot->SetBrush(SelectedBrush);
		break;
	case EInvSS_GridSlotVisualState::GrayedOut:
		Image_GridSlot->SetBrush(GrayOutBrush);
		break;
	default:
		checkNoEntry();
		break;
	}
}
