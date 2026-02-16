// @Copyright HaolunYuan


#include "UI/WidgetController/AuraWidgetController.h"


void UAuraWidgetController::SetWidgetControllerParams(const FWidgetControllerParameters& Parameters)
{
	CachedPlayerController = Parameters.PlayerController;
	CachedPlayerState = Parameters.PlayerState;
	CachedAbilitySystemComponent = Parameters.AbilitySystemComponent;
	CachedAttributeSet = Parameters.AttributeSet;
}
