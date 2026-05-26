// @Copyright HaolunYuan


#include "UI/WidgetController/AuraOverlayWidgetController.h"

#include "AbilitySystemComponent.h"
#include "AuraGameTagManager.h"
#include "Components/AbilitySystem/AuraAbilitySystemComponent.h"
#include "Components/AbilitySystem/AuraAttributeSet.h"
#include "Components/AbilitySystem/Data/AbilityInfo.h"
#include "Components/AbilitySystem/Data/LevelUpInfo.h"
#include "Player/AuraPlayerState.h"

/**
 * Pushes current attribute values to the UI so widgets display correct data on startup.
 * Called once during HUD initialization (after WidgetControllerSet has fired in Blueprint,
 * so the Blueprint bindings are already in place).
 */
void UAuraOverlayWidgetController::BroadcastInitialValues()
{
	UAuraAttributeSet* AuraAttribute = GetAuraAttributeSet();

	OnHealthChanged.Broadcast(AuraAttribute->GetHealth());
	OnMaxHealthChanged.Broadcast(AuraAttribute->GetMaxHealth());
	OnManaChanged.Broadcast(AuraAttribute->GetMana());
	OnMaxManaChanged.Broadcast(AuraAttribute->GetMaxMana());
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
 *    Example: a health potion GE has tag "Message.HealthPotion" -> DataTable row defines
 *    the popup text, icon, and widget class -> Blueprint shows the popup.
 */
void UAuraOverlayWidgetController::BindAllDependencies()
{
	UAuraAttributeSet* AuraAttribute = GetAuraAttributeSet();

	// Pathway 1: Attribute value changes -> C++ callback -> Dynamic delegate -> Blueprint
	CachedAbilitySystemComponent->GetGameplayAttributeValueChangeDelegate(AuraAttribute->GetHealthAttribute()).AddLambda(
		[this](const FOnAttributeChangeData& ChangedData)
		{
			OnHealthChanged.Broadcast(ChangedData.NewValue);
		}
	);

	CachedAbilitySystemComponent->GetGameplayAttributeValueChangeDelegate(AuraAttribute->GetMaxHealthAttribute()).AddLambda(
		[this](const FOnAttributeChangeData& ChangedData)
		{
			OnMaxHealthChanged.Broadcast(ChangedData.NewValue);
		}
	);

	CachedAbilitySystemComponent->GetGameplayAttributeValueChangeDelegate(AuraAttribute->GetManaAttribute()).AddLambda(
		[this](const FOnAttributeChangeData& ChangedData)
		{
			OnManaChanged.Broadcast(ChangedData.NewValue);
		}
	);

	CachedAbilitySystemComponent->GetGameplayAttributeValueChangeDelegate(AuraAttribute->GetMaxManaAttribute()).AddLambda(
		[this](const FOnAttributeChangeData& ChangedData)
		{
			OnMaxManaChanged.Broadcast(ChangedData.NewValue);
		}
	);

	if (UAuraAbilitySystemComponent* AuraASC = GetAuraAbilitySystemComponent())
	{
		AuraASC->OnAbilityEquipped.AddUObject(this, &UAuraOverlayWidgetController::OnAbilityEquippedCallback);
		// Startup abilities may be given before or after this controller initializes; handle both orders.
		if (AuraASC->bStartUpAbilitiesGiven)
		{
			BroadcastAbilityInfo();
		}
		else
		{
			AuraASC->OnAbilityGiven.AddUObject(this, &UAuraOverlayWidgetController::BroadcastAbilityInfo);
		}

		// Pathway 2: GE applied -> extract asset tags -> filter "Message.*" -> DataTable lookup -> UI
		AuraASC->OnGatherEffectAssetTags.AddLambda(
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

	if (AAuraPlayerState* AuraPlayerState = GetAuraPlayerState())
	{
		AuraPlayerState->OnXPChanged.AddUObject(this, &UAuraOverlayWidgetController::OnXpChanged);
		AuraPlayerState->OnLevelChanged.AddUObject(this, &UAuraOverlayWidgetController::OnLevelChanged);
	}
}

void UAuraOverlayWidgetController::OnXpChanged(const int32 NewXP)
{
	const AAuraPlayerState* AuraPlayerState = GetAuraPlayerState();
	ULevelUpInfo* LevelUpInfoInstance = AuraPlayerState->LevelUpInfo;
	check(LevelUpInfoInstance);
	const TArray<FAuraLevelUpInfo>& LevelUpInfoArray = LevelUpInfoInstance->LevelUpInfos;

	const float CurrentLevel = LevelUpInfoInstance->FindLevelForXP(NewXP);
	const FAuraLevelUpInfo CurrentLevelData = LevelUpInfoArray[CurrentLevel];

	const int32 MaxLevel = LevelUpInfoArray.Num() - 1;

	float BarPercentage = 0.f;
	if (CurrentLevel <= MaxLevel && CurrentLevel > 0)
	{
		const int32 RequiredXpToLevelUp = CurrentLevelData.LevelUpRequirement;
		const int32 PreviousRequiredXPToThisLevel = LevelUpInfoArray[CurrentLevel - 1].LevelUpRequirement;

		const int32 LevelGap = RequiredXpToLevelUp - PreviousRequiredXPToThisLevel;
		BarPercentage = (NewXP - PreviousRequiredXPToThisLevel) / static_cast<float>(LevelGap);
		OnPlayerXPChanged.Broadcast(BarPercentage);
	}
}

void UAuraOverlayWidgetController::OnLevelChanged(const int32 NewLevel) const
{
	OnPlayerLevelChanged.Broadcast(NewLevel);
}

void UAuraOverlayWidgetController::OnAbilityEquippedCallback(const FGameplayTag& AbilityTag,
	const FGameplayTag& StatusTag, const FGameplayTag& SlotInputTag, const FGameplayTag& PreviousSlotTag) const
{
	const FAuraGameTagManager& TagManager = FAuraGameTagManager::Get();
	FAuraAbilityInfo LastSlotInfo;
	LastSlotInfo.AbilityStatusTag = TagManager.Ability_Status_Locked;
	LastSlotInfo.InputTag = PreviousSlotTag;
	LastSlotInfo.AbilityTag = TagManager.Ability_None;
	AbilityInfoDelegate.Broadcast(LastSlotInfo);

	FAuraAbilityInfo* CurrentSlotInfo = AbilityInfoDataAsset->FindAbilityInfoByTag(AbilityTag);
	CurrentSlotInfo->AbilityStatusTag = StatusTag;
	CurrentSlotInfo->InputTag = SlotInputTag;
	AbilityInfoDelegate.Broadcast(*CurrentSlotInfo);
}
