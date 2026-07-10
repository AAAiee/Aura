// @Copyright HaolunYuan

#pragma once

#include "CoreMinimal.h"
#include "Widgets/Inventory/InvSS_InvWidgetBase.h"
#include "InvSS_InfoMessageWidget.generated.h"

class UTextBlock;

/**
 * UInvSS_InfoMessageWidget
 *
 * Popup text widget used for inventory status messages.
 *
 * The UI manager creates this widget for local players, positions it by viewport factor,
 * and calls SetMessage() to show/update text while the widget handles its own lifetime timer.
 */
UCLASS()
class INVENTORYSYSTEM_API UInvSS_InfoMessageWidget : public UInvSS_InvWidgetBase
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintImplementableEvent, Category = Inventory)
	void ShowMessage();

	UFUNCTION(BlueprintImplementableEvent, Category = Inventory)
	void HideMessage();

	/**
	 * @brief Sets the text and restarts the message lifetime timer.
	 */
	UFUNCTION(BlueprintCallable)
	void SetMessage(const FText& Message);

	FVector2f GetMessagePositionOnScreen() const;

protected:
	/* UUserWidget begins */
	virtual void NativeOnInitialized() override;
	/* UUserWidget ends */

private:
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UTextBlock> Text_Message;

	UPROPERTY(EditAnywhere, Category = "Message Properties")
	float MessageLifeTime = 3.f;

	FTimerHandle MessageTimerHandle;
	bool bIsMessageAlive{false};

	UPROPERTY(EditAnywhere, Category = "Message Properties")
	FVector2f PositionFactorOnScreen{0.5, 0.5}; // in the middle by default
};
