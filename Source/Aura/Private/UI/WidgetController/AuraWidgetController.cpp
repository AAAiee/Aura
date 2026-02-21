// @Copyright HaolunYuan


#include "UI/WidgetController/AuraWidgetController.h"

void UAuraWidgetController::SetWidgetControllerParams(const FWidgetControllerParameters& Parameters)
{
	// Cache all references so derived controllers can access them without additional lookups
	CachedPlayerController = Parameters.PlayerController;
	CachedPlayerState = Parameters.PlayerState;
	CachedAbilitySystemComponent = Parameters.AbilitySystemComponent;
	CachedAttributeSet = Parameters.AttributeSet;
}
