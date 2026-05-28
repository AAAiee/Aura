// @Copyright HaolunYuan

#include "UI/WidgetController/AuraSpellMenuWidgetController.h"

#include "AuraGameTagManager.h"
#include "Components/AbilitySystem/AuraAbilitySystemComponent.h"
#include "Components/AbilitySystem/Data/AbilityInfo.h"
#include "GameplayAbilitySpec.h"
#include "Player/AuraPlayerState.h"

UAuraSpellMenuWidgetController::UAuraSpellMenuWidgetController()
	: CurrentlySelectedAbility(FAuraGameTagManager::Get().Ability_None, FAuraGameTagManager::Get().Ability_Status_Locked)
{
	
}

void UAuraSpellMenuWidgetController::BroadcastInitialValues()
{
	BroadcastAbilityInfo();

	CurrentSpellPoints =  GetAuraPlayerState()->GetSpellPoints();
	OnSpellPointsChanged.Broadcast(GetAuraPlayerState()->GetSpellPoints());
}

void UAuraSpellMenuWidgetController::BindAllDependencies()
{
	GetAuraAbilitySystemComponent()->OnAbilityStatusChanged.AddLambda([this](const FGameplayTag& AbilityTag, const FGameplayTag& StatusTag, int32 NewAbilityLevel)
	{
		if (AbilityInfoDataAsset)
		{
			// AbilityInfo rows provide static UI art, while the ASC delegate supplies the runtime
			// status/level state. Mutating the row before broadcast gives Blueprint one complete packet.
			FAuraAbilityInfo* Info = AbilityInfoDataAsset->FindAbilityInfoByTag(AbilityTag);
			Info->AbilityStatusTag = StatusTag;
			AbilityInfoDelegate.Broadcast(*Info);

			if (CurrentlySelectedAbility.AbilityTag == AbilityTag)
			{
				CurrentlySelectedAbility.StatusTag = StatusTag;
				BroadcastAbilityGlobeEnabledStatusAndAbilityDescriptions(AbilityTag, StatusTag);
			}
		}
	});

	GetAuraAbilitySystemComponent()->OnAbilityEquipped.AddUObject(this, &UAuraSpellMenuWidgetController::OnAbilityEquippedCallback);

	GetAuraPlayerState()->OnSpellPointsChanged.AddLambda([this](int32 NewSpellPoints)
	{
		CurrentSpellPoints = NewSpellPoints;
		OnSpellPointsChanged.Broadcast(NewSpellPoints);

		BroadcastAbilityGlobeEnabledStatusAndAbilityDescriptions(CurrentlySelectedAbility.AbilityTag, CurrentlySelectedAbility.StatusTag);
	});
}

void UAuraSpellMenuWidgetController::StopWaitingForEquipSelection()
{
	if (bWaitingForEquipSelection)
	{
		const FGameplayTag AbilityTypeTag = GetGameplayTypeTagForAbilityTag(CurrentlySelectedAbility.AbilityTag);
		OnStopWaitingForEquipSlotSelection.Broadcast(AbilityTypeTag);
		bWaitingForEquipSelection = false;
	}
}

void UAuraSpellMenuWidgetController::BroadcastAbilityGlobeEnabledStatusAndAbilityDescriptions(
	const FGameplayTag& AbilityTag, const FGameplayTag& AbilityStatusTag)
{
	bool bEnabledSpendPoints = false;
	bool bEnabledEquip = false;
	FString OutDescription = FString();
	FString OutNextLevelDescription = FString();
	ShouldEnableButton(AbilityStatusTag, CurrentSpellPoints, bEnabledSpendPoints, bEnabledEquip);
	GetAuraAbilitySystemComponent()->GetDescriptionsByAbilityTag(AbilityTag, OutDescription, OutNextLevelDescription);
	SpellGlobeSelectedDelegate.Broadcast(bEnabledSpendPoints, bEnabledEquip, OutDescription, OutNextLevelDescription);
}

void UAuraSpellMenuWidgetController::OnAbilityEquippedCallback(const FGameplayTag& AbilityTag,
	const FGameplayTag& StatusTag, const FGameplayTag& SlotInputTag, const FGameplayTag& PreviousSlotTag)
{
	bWaitingForEquipSelection = false;

	// Clear the previous slot first so the UI does not temporarily show the same ability in two slots.
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

	OnStopWaitingForEquipSlotSelection.Broadcast(GetGameplayTypeTagForAbilityTag(AbilityTag));
	OnAbilityReassigned.Broadcast(AbilityTag);
	GlobeDeselect();
}

void UAuraSpellMenuWidgetController::SpellGlobeSelected(const FGameplayTag& AbilityTag)
{
	StopWaitingForEquipSelection();
	const FAuraGameTagManager& TagManager = FAuraGameTagManager::Get();
	const int32 SpellPoints = GetAuraPlayerState()->GetSpellPoints();

	const bool bTagValid = AbilityTag.IsValid();
	const bool bTagNone = AbilityTag.MatchesTag(TagManager.Ability_None);

	// Missing specs represent locked abilities that have not been granted yet.
	const FGameplayAbilitySpec* AbilitySpec = GetAuraAbilitySystemComponent()->GetAbilitySpecForAbilityTag(AbilityTag);
	const bool bAbilityNotGiven = AbilitySpec == nullptr;

	FGameplayTag AbilityStatusTag;
	if (!bTagValid || bTagNone || bAbilityNotGiven)
	{
		AbilityStatusTag = TagManager.Ability_Status_Locked;
	}
	else
	{
		AbilityStatusTag = GetAuraAbilitySystemComponent()->GetStatusTagFromSpec(*AbilitySpec);
	}

	CurrentlySelectedAbility.AbilityTag = AbilityTag;
	CurrentlySelectedAbility.StatusTag = AbilityStatusTag;

	BroadcastAbilityGlobeEnabledStatusAndAbilityDescriptions(AbilityTag, AbilityStatusTag);
}

void UAuraSpellMenuWidgetController::OnSpentPointsButtonPressed()
{
	if (GetAuraAbilitySystemComponent())
	{
		// Spending affects authoritative PlayerState/ASC data, so the widget controller only sends
		// the request. The resulting delegates update the UI after the server changes the spec.
		GetAuraAbilitySystemComponent()->Server_SpendSpellPoints(CurrentlySelectedAbility.AbilityTag);
	}
}

void UAuraSpellMenuWidgetController::GlobeDeselect()
{
	StopWaitingForEquipSelection();

	CurrentlySelectedAbility.AbilityTag = FGameplayTag();
	CurrentlySelectedAbility.StatusTag = FAuraGameTagManager::Get().Ability_Status_Locked;
	SpellGlobeSelectedDelegate.Broadcast(false, false, FString(), FString());
}

void UAuraSpellMenuWidgetController::OnEquipButtonPressed()
{
	FGameplayTag AbilityTypeTag = GetGameplayTypeTagForAbilityTag(CurrentlySelectedAbility.AbilityTag);
	OnWaitingForEquipSlotSelection.Broadcast(AbilityTypeTag);
	bWaitingForEquipSelection = true;

	// Equipped spells remember their existing slot while the player chooses a replacement row.
	const FGameplayTag StatusTagForCurrentSelectedAbility = GetAuraAbilitySystemComponent()->GetStatusTagForAbilityTag(CurrentlySelectedAbility.AbilityTag);
	if (StatusTagForCurrentSelectedAbility == FAuraGameTagManager::Get().Ability_Status_Equipped)
	{
		CachedAbilityInputTag = GetAuraAbilitySystemComponent()->GetAbilityInputTagFromAbilityTag(CurrentlySelectedAbility.AbilityTag);
	}
}

void UAuraSpellMenuWidgetController::OnEquipRowSpellGlobePressed(const FGameplayTag& SlotAbilityTypeTag,
	const FGameplayTag& SlotInputTag)
{
	if (!bWaitingForEquipSelection)
	{
		return;
	}

	// Enforce offensive/passive row compatibility before sending the equipment request.
	const FGameplayTag& SelectedAbilityType = AbilityInfoDataAsset->FindAbilityInfoByTag(CurrentlySelectedAbility.AbilityTag)->AbilityTypeTag;
	if (!SelectedAbilityType.MatchesTagExact(SlotAbilityTypeTag))
	{
		return;
	}

	GetAuraAbilitySystemComponent()->Server_EquipAbility(CurrentlySelectedAbility.AbilityTag, SlotInputTag);
}

void UAuraSpellMenuWidgetController::ShouldEnableButton(const FGameplayTag& AbilityStatus, int32 SpellPoints, bool& bShouldEnableSpellPointsButton, bool& bShouldEnableEquipButton)
{
	bShouldEnableSpellPointsButton = false;
	const FAuraGameTagManager& Tags = FAuraGameTagManager::Get();
	// Button rules mirror the spell lifecycle:
	//   Locked: inspect only.
	//   Eligible: can spend to unlock, cannot equip yet.
	//   UnLocked: can equip and can spend again to level.
	//   Equipped: remains equip-capable and can be leveled if spell points remain.
	if (AbilityStatus.MatchesTagExact(Tags.Ability_Status_Equipped))
	{
		bShouldEnableEquipButton = true;
		if (SpellPoints > 0)
		{
			bShouldEnableSpellPointsButton = true;
		}
	}
	else if (AbilityStatus.MatchesTagExact(Tags.Ability_Status_Eligible))
	{
		bShouldEnableEquipButton = false;
		if (SpellPoints > 0)
		{
			bShouldEnableSpellPointsButton = true;
		}
	}
	else if (AbilityStatus.MatchesTagExact(Tags.Ability_Status_Locked))
	{
		bShouldEnableEquipButton = false;
	}
	else if (AbilityStatus.MatchesTagExact(Tags.Ability_Status_UnLocked))
	{
		bShouldEnableEquipButton = true;
		if (SpellPoints > 0)
		{
			bShouldEnableSpellPointsButton = true;
		}
	}
}

FGameplayTag UAuraSpellMenuWidgetController::GetGameplayTypeTagForAbilityTag(const FGameplayTag& AbilityTag) const
{
	FGameplayTag AbilityTypeTag = FGameplayTag();
	const FAuraAbilityInfo* AbilityInfo = AbilityInfoDataAsset->FindAbilityInfoByTag(AbilityTag);
	if (AbilityInfo)
	{
		if (AbilityInfo->AbilityTypeTag.IsValid())
		{
			AbilityTypeTag = AbilityInfo->AbilityTypeTag;
		}
	}

	ensureMsgf(AbilityInfo, TEXT("The Ability with tag [%s] is not listed in ability info data asset!"),
		*CurrentlySelectedAbility.AbilityTag.ToString());
	ensureMsgf(AbilityInfo->AbilityTypeTag.IsValid(),
		TEXT("Make sure you have fill the \"AbilityTypeTag\" field for [%s] Ability in DA_AbilityInfo"),
		*AbilityInfo->AbilityTag.ToString());
	return AbilityTypeTag;
}
