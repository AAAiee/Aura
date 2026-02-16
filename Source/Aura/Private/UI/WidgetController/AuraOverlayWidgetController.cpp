// @Copyright HaolunYuan


#include "UI/WidgetController/AuraOverlayWidgetController.h"
#include "Components/AbilitySystem/AuraAttributeSet.h"

void UAuraOverlayWidgetController::BroadcastInitialValues()
{
	if (const UAuraAttributeSet* AuraAttribute = Cast<UAuraAttributeSet>(CachedAttributeSet))
	{
		float Health = AuraAttribute->GetHealth();
		OnHealthChanged.Broadcast(Health);

		float MaxHealth = AuraAttribute->GetMaxHealth();
		OnMaxHealthChanged.Broadcast(MaxHealth);

		float Mana = AuraAttribute->GetMana();
		OnManaChanged.Broadcast(Mana);

		float MaxMana = AuraAttribute->GetMaxMana();
		OnMaxManaChanged.Broadcast(MaxMana); 
	}
}

void UAuraOverlayWidgetController::BindAlldDependencies()
{
	const UAuraAttributeSet* AuraAttribute = Cast<UAuraAttributeSet>(CachedAttributeSet);

	CachedAbilitySystemComponent->GetGameplayAttributeValueChangeDelegate(AuraAttribute->GetHealthAttribute()).AddUObject(this, &UAuraOverlayWidgetController::HealthChanged);

	CachedAbilitySystemComponent->GetGameplayAttributeValueChangeDelegate(AuraAttribute->GetMaxHealthAttribute()).AddUObject(this, &UAuraOverlayWidgetController::MaxHealthChanged);

	CachedAbilitySystemComponent->GetGameplayAttributeValueChangeDelegate(AuraAttribute->GetManaAttribute()).AddUObject(this, &UAuraOverlayWidgetController::ManaChanged);

	CachedAbilitySystemComponent->GetGameplayAttributeValueChangeDelegate(AuraAttribute->GetMaxManaAttribute()).AddUObject(this, &UAuraOverlayWidgetController::MaxManaChanged);
}

void UAuraOverlayWidgetController::HealthChanged(const FOnAttributeChangeData& ChangedData) const
{
	OnHealthChanged.Broadcast(ChangedData.NewValue);
}

void UAuraOverlayWidgetController::MaxHealthChanged(const FOnAttributeChangeData& ChangedData) const
{
	OnHealthChanged.Broadcast(ChangedData.NewValue); 
}

void UAuraOverlayWidgetController::ManaChanged(const FOnAttributeChangeData& ChangedData) const
{
	OnManaChanged.Broadcast(ChangedData.NewValue); 
}

void UAuraOverlayWidgetController::MaxManaChanged(const FOnAttributeChangeData& ChangedData) const
{
	OnMaxManaChanged.Broadcast(ChangedData.NewValue); 
}

