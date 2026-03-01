// @Copyright HaolunYuan


#include "UI/WidgetController/AuraOverlayWidgetController.h"
#include "Components/AbilitySystem/AuraAttributeSet.h"
#include "Components/AbilitySystem/AuraAbilitySystemComponent.h"

/**
 * Pushes current attribute values to the UI so widgets display correct data on startup.
 * Called once during HUD initialization (after WidgetControllerSet has fired in Blueprint,
 * so the Blueprint bindings are already in place).
 */
void UAuraOverlayWidgetController::BroadcastInitialValues()
{
	if (const UAuraAttributeSet* AuraAttribute = Cast<UAuraAttributeSet>(CachedAttributeSet))
	{
		OnHealthChanged.Broadcast(AuraAttribute->GetHealth());
		OnMaxHealthChanged.Broadcast(AuraAttribute->GetMaxHealth());
		OnManaChanged.Broadcast(AuraAttribute->GetMana());
		OnMaxManaChanged.Broadcast(AuraAttribute->GetMaxMana());
	}
}

/**
 * Subscribes to two types of events:
 *
 * 1. Attribute change delegates (via ASC::GetGameplayAttributeValueChangeDelegate):
 *    Fires whenever Health, MaxHealth, Mana, or MaxMana changes on the server.
 *    Each callback relays the new value to the corresponding Blueprint-assignable delegate.
 *
 * 2. Effect asset tag delegate (via AuraASC::OnGatherEffectAssetTags):
 *    Fires whenever a GE is applied to self. The lambda filters for tags under "Message",
 *    looks up the matching DataTable row, and broadcasts FUIWidgetRow to the UI.
 *    Example: a health potion GE has tag "Message.HealthPotion" ¡ú DataTable row defines
 *    the popup text, icon, and widget class ¡ú Blueprint shows the popup.
 */
void UAuraOverlayWidgetController::BindAllDependencies()
{
	const UAuraAttributeSet* AuraAttribute = Cast<UAuraAttributeSet>(CachedAttributeSet);

	// Pathway 1: Attribute value changes ¡ú C++ callback ¡ú Dynamic delegate ¡ú Blueprint
	CachedAbilitySystemComponent->GetGameplayAttributeValueChangeDelegate(AuraAttribute->GetHealthAttribute()).AddUObject(this, &UAuraOverlayWidgetController::HealthChanged);

	CachedAbilitySystemComponent->GetGameplayAttributeValueChangeDelegate(AuraAttribute->GetMaxHealthAttribute()).AddUObject(this, &UAuraOverlayWidgetController::MaxHealthChanged);

	CachedAbilitySystemComponent->GetGameplayAttributeValueChangeDelegate(AuraAttribute->GetManaAttribute()).AddUObject(this, &UAuraOverlayWidgetController::ManaChanged);

	CachedAbilitySystemComponent->GetGameplayAttributeValueChangeDelegate(AuraAttribute->GetMaxManaAttribute()).AddUObject(this, &UAuraOverlayWidgetController::MaxManaChanged);

	// Pathway 2: GE applied ¡ú extract asset tags ¡ú filter "Message.*" ¡ú DataTable lookup ¡ú UI
	Cast<UAuraAbilitySystemComponent>(CachedAbilitySystemComponent)->OnGatherEffectAssetTags.AddLambda(
		[this](const FGameplayTagContainer& TagContainer)
		{
			for (const FGameplayTag& Tag : TagContainer)
			{
				// MatchesTag checks if Tag is a child of "Message" (e.g., "Message.HealthPotion")
				FGameplayTag MessageTag = FGameplayTag::RequestGameplayTag(FName("Message"));
				if (Tag.MatchesTag(MessageTag))
				{
					const FUIWidgetRow* Row = GetDataTableRowFromTag<FUIWidgetRow>(this->MessageWidgetDataTable, Tag);
					if (Row)
					{
						OnSendMessageWidgetRow.Broadcast(*Row);
					}
				}
			}
		}
	);
}

/*Attribute Change Callbacks ¡ª simple relay from FOnAttributeChangeData to Dynamic delegate*/

void UAuraOverlayWidgetController::HealthChanged(const FOnAttributeChangeData& ChangedData) const
{
	OnHealthChanged.Broadcast(ChangedData.NewValue);
}

void UAuraOverlayWidgetController::MaxHealthChanged(const FOnAttributeChangeData& ChangedData) const
{
	OnMaxHealthChanged.Broadcast(ChangedData.NewValue); 
}

void UAuraOverlayWidgetController::ManaChanged(const FOnAttributeChangeData& ChangedData) const
{
	OnManaChanged.Broadcast(ChangedData.NewValue); 
}

void UAuraOverlayWidgetController::MaxManaChanged(const FOnAttributeChangeData& ChangedData) const
{
	OnMaxManaChanged.Broadcast(ChangedData.NewValue); 
}

