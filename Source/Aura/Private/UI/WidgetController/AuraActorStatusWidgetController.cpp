// @Copyright HaolunYuan


#include "UI/WidgetController/AuraActorStatusWidgetController.h"
#include "Components/AbilitySystem/AuraAttributeSet.h"

void UAuraActorStatusWidgetController::BroadcastInitialValues()
{
	const UAuraAttributeSet* ActorAttribute = Cast<UAuraAttributeSet>(CachedAttributeSet);
	check(ActorAttribute);

	OnMaxHealthChangeDelegate.Broadcast(ActorAttribute->GetMaxHealth());
	OnHealthChangedDelegate.Broadcast(ActorAttribute->GetHealth());
	OnMaxManaChangeDelegate.Broadcast(ActorAttribute->GetMaxMana());
	OnManaChangedDelegate.Broadcast(ActorAttribute->GetMana());
}

void UAuraActorStatusWidgetController::BindAllDependencies()
{
	const UAuraAttributeSet* AttributeSet = Cast<UAuraAttributeSet>(CachedAttributeSet);
	check(AttributeSet);
	check(CachedAbilitySystemComponent);

	// Each attribute delegate forwards the newest server-authored value straight into the widget layer.
	CachedAbilitySystemComponent->GetGameplayAttributeValueChangeDelegate(AttributeSet->GetHealthAttribute()).AddLambda(
		[this](const FOnAttributeChangeData& ChangedData)
		{
			this->OnHealthChangedDelegate.Broadcast(ChangedData.NewValue);
		}
	);

	CachedAbilitySystemComponent->GetGameplayAttributeValueChangeDelegate(AttributeSet->GetMaxHealthAttribute()).AddLambda(
		[this](const FOnAttributeChangeData& ChangedData)
		{
			this->OnMaxHealthChangeDelegate.Broadcast(ChangedData.NewValue);
		}
	);

	CachedAbilitySystemComponent->GetGameplayAttributeValueChangeDelegate(AttributeSet->GetManaAttribute()).AddLambda(
		[this](const FOnAttributeChangeData& ChangedData)
		{
			this->OnManaChangedDelegate.Broadcast(ChangedData.NewValue);
		}
	);

	CachedAbilitySystemComponent->GetGameplayAttributeValueChangeDelegate(AttributeSet->GetMaxManaAttribute()).AddLambda(
		[this](const FOnAttributeChangeData& ChangedData)
		{
			this->OnMaxManaChangeDelegate.Broadcast(ChangedData.NewValue);
		}
	);
}
