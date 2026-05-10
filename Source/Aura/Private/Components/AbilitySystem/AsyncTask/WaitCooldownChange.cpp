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

	// Cooldown end is signaled when the tracked tag count returns to zero.
	InAbilitySystemComponent->RegisterGameplayTagEvent(InCooldownTag, EGameplayTagEventType::NewOrRemoved).AddUObject(Task, &UWaitCooldownChange::CooldownTagChanged);

	// Cooldown start/refresh is signaled by matching newly applied gameplay effects.
	InAbilitySystemComponent->OnActiveGameplayEffectAddedDelegateToSelf.AddUObject(Task, &UWaitCooldownChange::OnActiveGameplayEffectAdded);

	return Task;
}

void UWaitCooldownChange::EndTask()
{
	if (!IsValid(ASC))
	{
		return;
	}

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
		TArray<float> TimeRemainings = ASC->GetActiveEffectsTimeRemaining(GameplayEffectQuery);
		if (TimeRemainings.Num() > 0)
		{
			float LongestTimeRemaining = TimeRemainings[0];
			for (int32 Index = 0; Index < TimeRemainings.Num(); ++Index)
			{
				if (TimeRemainings[Index] > LongestTimeRemaining)
				{
					LongestTimeRemaining = TimeRemainings[Index];
				}
			}

			CooldownStart.Broadcast(LongestTimeRemaining);
		}
	}
}
