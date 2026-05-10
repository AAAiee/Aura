// @Copyright HaolunYuan


#include "UI/WidgetController/AuraWidgetController.h"
#include "AbilitySystemComponent.h"

/**
 * Caches the four core references so derived controllers (e.g., OverlayWidgetController)
 * can access them without requiring additional function parameters.
 * Called once during HUD initialization (AAuraHUD::GetWidgetController).
 */
void UAuraWidgetController::SetWidgetControllerParams(const FWidgetControllerParameters& Parameters)
{
	CachedPlayerController = Parameters.PlayerController;
	CachedPlayerState = Parameters.PlayerState;
	CachedAbilitySystemComponent = Parameters.AbilitySystemComponent;
	CachedAttributeSet = Parameters.AttributeSet;
}

/**
 * Base implementation - intentionally empty.
 * Derived classes override this to subscribe to ASC delegates.
 */
void UAuraWidgetController::BindAllDependencies()
{

}

AActor* UAuraWidgetController::GetAvatarActor() const
{
	if (!CachedAbilitySystemComponent)
	{
		return nullptr;
	}

	return CachedAbilitySystemComponent->GetAvatarActor();
}

AActor* UAuraWidgetController::GetOwningActor() const
{
	if (!CachedAbilitySystemComponent)
	{
		return nullptr;
	}

	return CachedAbilitySystemComponent->GetOwner();
}
