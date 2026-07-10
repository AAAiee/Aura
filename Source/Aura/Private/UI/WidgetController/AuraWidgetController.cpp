// @Copyright HaolunYuan

#include "UI/WidgetController/AuraWidgetController.h"

#include "AbilitySystemComponent.h"
#include "Components/AbilitySystem/AuraAbilitySystemComponent.h"
#include "Components/AbilitySystem/AuraAttributeSet.h"
#include "Components/AbilitySystem/Data/AbilityInfo.h"
#include "Player/AuraPlayerController.h"
#include "Player/AuraPlayerState.h"

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

void UAuraWidgetController::BroadcastInitialValues()
{
}

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

AAuraPlayerController* UAuraWidgetController::GetAuraPlayerController()
{
	if (!CachedAuraPlayerController)
	{
		CachedAuraPlayerController = Cast<AAuraPlayerController>(CachedPlayerController);
	}

	return CachedAuraPlayerController;
}

AAuraPlayerState* UAuraWidgetController::GetAuraPlayerState()
{
	if (!CachedAuraPlayerState)
	{
		CachedAuraPlayerState = Cast<AAuraPlayerState>(CachedPlayerState);
	}

	return CachedAuraPlayerState;
}

UAuraAbilitySystemComponent* UAuraWidgetController::GetAuraAbilitySystemComponent()
{
	if (!CachedAuraAbilitySystemComponent)
	{
		CachedAuraAbilitySystemComponent = Cast<UAuraAbilitySystemComponent>(CachedAbilitySystemComponent);
	}

	return CachedAuraAbilitySystemComponent;
}

UAuraAttributeSet* UAuraWidgetController::GetAuraAttributeSet()
{
	if (!CachedAuraAttributeSet)
	{
		CachedAuraAttributeSet = Cast<UAuraAttributeSet>(CachedAttributeSet);
	}

	return CachedAuraAttributeSet;
}

void UAuraWidgetController::BroadcastAbilityInfo()
{
	// Ability UI combines two data sources:
	//   - static row data from AbilityInfoDataAsset (icons, cooldown tags, level requirement)
	//   - runtime spec data from the ASC (current input slot and unlock/equip status)
	const auto BroadcastAbilityInfo = [this](const FGameplayAbilitySpec& Spec)
		{
			const FGameplayTag AbilityTag = UAuraAbilitySystemComponent::GetAbilityTagFromSpec(Spec);

			if (!AbilityTag.IsValid())
			{
				return;
			}

			FAuraAbilityInfo* AbilityInfo = AbilityInfoDataAsset->FindAbilityInfoByTag(AbilityTag, true);
			if (AbilityInfo)
			{
				AbilityInfo->InputTag = UAuraAbilitySystemComponent::GetInputTagFromSpec(Spec);
				AbilityInfo->AbilityStatusTag = UAuraAbilitySystemComponent::GetStatusTagFromSpec(Spec);
				AbilityInfoDelegate.Broadcast(*AbilityInfo);
			}
		};

	GetAuraAbilitySystemComponent()->ForEachAbility(BroadcastAbilityInfo);
}
