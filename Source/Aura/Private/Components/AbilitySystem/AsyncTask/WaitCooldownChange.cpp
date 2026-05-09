// @Copyright HaolunYuan


#include "Components/AbilitySystem/AsyncTask/WaitCooldownChange.h"
#include "AbilitySystemComponent.h"

UWaitCooldownChange* UWaitCooldownChange::WaitCooldownChange(UAbilitySystemComponent* InAbilitySystemComponent, const FGameplayTag& InCooldownTag)
{
	UWaitCooldownChange* Task = NewObject<UWaitCooldownChange>();
	Task->ASC = InAbilitySystemComponent;
	Task->CooldownTag = InCooldownTag;

	if (!IsValid(Task->ASC) || !Task->CooldownTag.IsValid())
	{
		Task->EndTask(); 
		return nullptr;
	}

	// to know when a cooldown tag is removed 
	InAbilitySystemComponent->RegisterGameplayTagEvent(InCooldownTag, EGameplayTagEventType::NewOrRemoved).AddUObject(Task, &UWaitCooldownChange::CooldownTagChanged);

	//to know when a cooldown tag is added, in case the cooldown is refreshed or changed before it end.
	InAbilitySystemComponent->OnActiveGameplayEffectAddedDelegateToSelf.AddUObject(Task, &UWaitCooldownChange::OnActiveGameplayEffectAdded);

	return Task;
}

void UWaitCooldownChange::EndTask()
{
	if (!IsValid(ASC)) return;
	ASC->RegisterGameplayTagEvent(CooldownTag, EGameplayTagEventType::NewOrRemoved).RemoveAll(this);

	SetReadyToDestroy();
	MarkAsGarbage();
}

void UWaitCooldownChange::CooldownTagChanged(const FGameplayTag CallbackTag, int32 NewCount)
{
	if (NewCount == 0)
	{
		CooldownEnd.Broadcast(0.f);
	}

}

void UWaitCooldownChange::OnActiveGameplayEffectAdded(UAbilitySystemComponent* TargetASC, const FGameplayEffectSpec& SpecApplied, FActiveGameplayEffectHandle ActiveHandle)
{
	FGameplayTagContainer AssetTags;
	SpecApplied.GetAllAssetTags(AssetTags); 


	FGameplayTagContainer GrantedTags;
	SpecApplied.GetAllGrantedTags(GrantedTags);


	if (AssetTags.HasTagExact(CooldownTag) || GrantedTags.HasTagExact(CooldownTag))
	{
		FGameplayEffectQuery GameplayEffectQuery = FGameplayEffectQuery::MakeQuery_MatchAnyOwningTags(CooldownTag.GetSingleTagContainer());
		TArray<float> TimeRemanings = ASC->GetActiveEffectsTimeRemaining(GameplayEffectQuery);
		if (TimeRemanings.Num() > 0)
		{
			float LongestTimeRemaining = TimeRemanings[0];
			for (int32 i = 0; i < TimeRemanings.Num(); i++)
			{
				if (TimeRemanings[i] > LongestTimeRemaining)
				{
					LongestTimeRemaining = TimeRemanings[i];
				}
			}

			float TimeRemaning = LongestTimeRemaining;

			CooldownStart.Broadcast(TimeRemaning);
		}
	}

}
