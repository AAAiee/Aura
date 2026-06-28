// @Copyright HaolunYuan


#include "Widgets/HUD/InvSS_InfoMessageWidget.h"

#include "Components/TextBlock.h"

void UInvSS_InfoMessageWidget::NativeOnInitialized()
{
	Super::NativeOnInitialized();

	checkf(Text_Message, TEXT("Text_Message is not bound on %s."), *GetName());

	HideMessage();
}

void UInvSS_InfoMessageWidget::SetMessage(const FText& Message)
{
	checkf(Text_Message, TEXT("Text_Message is not bound on %s."), *GetName());

	// Pipeline:
	// 1. Update text immediately.
	// 2. Show the widget if it is currently hidden.
	// 3. Restart the hide timer for the latest message.
	Text_Message->SetText(Message);

	if (!bIsMessageAlive)
	{
		ShowMessage();
		bIsMessageAlive = true;
	}

	GetWorld()->GetTimerManager().SetTimer(MessageTimerHandle,
		[this]()
		{
			HideMessage();
			bIsMessageAlive = false;
		}, MessageLifeTime, false);
}
