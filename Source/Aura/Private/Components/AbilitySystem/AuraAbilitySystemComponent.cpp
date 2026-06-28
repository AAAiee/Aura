// @Copyright HaolunYuan

#include "Components/AbilitySystem/AuraAbilitySystemComponent.h"

#include "AbilitySystemBlueprintLibrary.h"
#include "AuraGameTagManager.h"
#include "AuraLogCategory.h"
#include "Components/AbilitySystem/Ability/AuraGameplayAbility.h"
#include "Components/AbilitySystem/AuraAbilitySystemLibrary.h"
#include "Components/AbilitySystem/Data/AbilityInfo.h"
#include "GameplayAbilitySpec.h"
#include "Interaction/PlayerInterface.h"

void UAuraAbilitySystemComponent::AbilityActorInfoSet()
{
	/*Bind the engine's built-in "effect applied to self" delegate to our Client RPC.
	 * From this point on, every GE applied to this ASC will trigger Client_OnEffectAppliedToSelf,
	 * which runs on the owning client and rebroadcasts the tags for the UI. */
	OnGameplayEffectAppliedDelegateToSelf.AddUObject(this, &UAuraAbilitySystemComponent::Client_OnEffectAppliedToSelf);
}

void UAuraAbilitySystemComponent::AddCharacterAbilities(const TArray<TSubclassOf<UGameplayAbility>>& InAbilitiesClasses)
{
	for (const auto& AbilityClass : InAbilitiesClasses)
	{
		FGameplayAbilitySpec AbilitySpec = FGameplayAbilitySpec(AbilityClass, 1);
		if (const UAuraGameplayAbility* AuraAbility = Cast<UAuraGameplayAbility>(AbilitySpec.Ability))
		{
			// StartupGameTag doubles as the default input slot for tutorial-granted abilities.
			// The status tag is dynamic because the same ability spec can move through menu states.
			AbilitySpec.GetDynamicSpecSourceTags().AddTag(AuraAbility->StartupGameTag);
			AbilitySpec.GetDynamicSpecSourceTags().AddTag(FAuraGameTagManager::Get().Ability_Status_Equipped);
			GiveAbility(AbilitySpec);
		}
	}

	bStartUpAbilitiesGiven = true;
	OnAbilityGiven.Broadcast();
}

void UAuraAbilitySystemComponent::AddCharacterPassiveAbilities(const TArray<TSubclassOf<class UGameplayAbility>>& InPassiveAbilitiesClasses)
{
	for (const auto& AbilityClass : InPassiveAbilitiesClasses)
	{
		// Passive abilities are fire-and-forget startup grants: GiveAbilityAndActivateOnce lets the
		// ability apply persistent effects and then remain represented by those effects/tags.
		FGameplayAbilitySpec AbilitySpec = FGameplayAbilitySpec(AbilityClass, 1);
		GiveAbilityAndActivateOnce(AbilitySpec);
	}
}

bool UAuraAbilitySystemComponent::AbilityInputTagTriggered(const FGameplayTag& InputTag, bool bCanActivateInactiveAbility)
{
	if (!ensureMsgf(InputTag.IsValid(), TEXT("AbilityInputTagTriggered received an invalid input tag.")))
	{
		return false;
	}

	FScopedAbilityListLock AbilityListLock(*this);
	FGameplayAbilitySpec* MatchingAbilitySpec = nullptr;
	for (FGameplayAbilitySpec& GrantedAbilitySpec : GetActivatableAbilities())
	{
		if (GrantedAbilitySpec.GetDynamicSpecSourceTags().HasTagExact(InputTag))
		{
			if (!ensureMsgf(MatchingAbilitySpec == nullptr,
				TEXT("Multiple ability specs are bound to input tag [%s]. Equip logic should keep slots exclusive."),
				*InputTag.ToString()))
			{
				return false;
			}

			MatchingAbilitySpec = &GrantedAbilitySpec;
		}
	}

	if (!MatchingAbilitySpec)
	{
		return false;
	}

	if (MatchingAbilitySpec->IsActive())
	{
		NotifyAbilityInputPressed(*MatchingAbilitySpec);
		return true;
	}

	if (!bCanActivateInactiveAbility)
	{
		return false;
	}

	TryActivateAbility(MatchingAbilitySpec->Handle);
	return true;
}

void UAuraAbilitySystemComponent::AbilityInputTagReleased(const FGameplayTag& InputTag)
{
	if (!ensureMsgf(InputTag.IsValid(), TEXT("AbilityInputTagReleased received an invalid input tag.")))
	{
		return;
	}

	FScopedAbilityListLock AbilityListLock(*this);
	FGameplayAbilitySpec* MatchingAbilitySpec = nullptr;
	for (FGameplayAbilitySpec& GrantedAbilitySpec : GetActivatableAbilities())
	{
		if (GrantedAbilitySpec.GetDynamicSpecSourceTags().HasTagExact(InputTag))
		{
			if (!ensureMsgf(MatchingAbilitySpec == nullptr,
				TEXT("Multiple ability specs are bound to input tag [%s]. Equip logic should keep slots exclusive."),
				*InputTag.ToString()))
			{
				return;
			}

			MatchingAbilitySpec = &GrantedAbilitySpec;
		}
	}

	if (MatchingAbilitySpec && MatchingAbilitySpec->IsActive())
	{
		NotifyAbilityInputReleased(*MatchingAbilitySpec);
	}
}

void UAuraAbilitySystemComponent::NotifyAbilityInputPressed(FGameplayAbilitySpec& AbilitySpec)
{
	if (!ensureMsgf(AbilitySpec.Ability.Get(), TEXT("NotifyAbilityInputPressed called for a spec with no ability.")))
	{
		return;
	}

	AbilitySpecInputPressed(AbilitySpec);
	if (const UGameplayAbility* PrimaryInstance = AbilitySpec.GetPrimaryInstance())
	{
		InvokeReplicatedEvent(EAbilityGenericReplicatedEvent::InputPressed, AbilitySpec.Handle, PrimaryInstance->GetCurrentActivationInfo().GetActivationPredictionKey());
	}
}

void UAuraAbilitySystemComponent::NotifyAbilityInputReleased(FGameplayAbilitySpec& AbilitySpec)
{
	if (!ensureMsgf(AbilitySpec.Ability.Get(), TEXT("NotifyAbilityInputReleased called for a spec with no ability.")))
	{
		return;
	}

	AbilitySpecInputReleased(AbilitySpec);
	if (const UGameplayAbility* PrimaryInstance = AbilitySpec.GetPrimaryInstance())
	{
		InvokeReplicatedEvent(EAbilityGenericReplicatedEvent::InputReleased, AbilitySpec.Handle, PrimaryInstance->GetCurrentActivationInfo().GetActivationPredictionKey());
	}
}

void UAuraAbilitySystemComponent::ForEachAbility(const TFunction<void(const FGameplayAbilitySpec&)>& Predicate)
{
	FScopedAbilityListLock AbilityListLock(*this);
	for (const FGameplayAbilitySpec& Spec : GetActivatableAbilities())
	{
		Predicate(Spec);
	}
}

FGameplayTag UAuraAbilitySystemComponent::GetAbilityTagFromSpec(const FGameplayAbilitySpec& Spec)
{
	if (Spec.Ability.Get())
	{
		for (const FGameplayTag& Tag : Spec.Ability.Get()->GetAssetTags())
		{
			if (Tag.MatchesTag(FGameplayTag::RequestGameplayTag(FName("Ability"))))
			{
				return Tag;
			}
		}
	}

	UE_LOG(LogAura, Warning, TEXT("GetAbilityTagFromSpec: No 'Ability.*' tag found for ability %s"),
		Spec.Ability.Get() ? *Spec.Ability.Get()->GetName() : TEXT("nullptr"));
	return FGameplayTag();
}

FGameplayTag UAuraAbilitySystemComponent::GetInputTagFromSpec(const FGameplayAbilitySpec& Spec)
{
	if (Spec.Ability.Get())
	{
		for (const FGameplayTag& Tag : Spec.GetDynamicSpecSourceTags())
		{
			if (Tag.MatchesTag(FGameplayTag::RequestGameplayTag(FName("InputTag"))))
			{
				return Tag;
			}
		}
	}

	UE_LOG(LogAura, Warning, TEXT("GetInputTagFromSpec: No 'InputTag.*' tag found for ability %s"),
		Spec.Ability.Get() ? *Spec.Ability.Get()->GetName() : TEXT("nullptr"));
	return FGameplayTag();
}

FGameplayTag UAuraAbilitySystemComponent::GetStatusTagFromSpec(const FGameplayAbilitySpec& Spec)
{
	if (Spec.Ability.Get())
	{
		for (const FGameplayTag& Tag : Spec.GetDynamicSpecSourceTags())
		{
			if (Tag.MatchesTag(FGameplayTag::RequestGameplayTag(FName("Ability.Status"))))
			{
				return Tag;
			}
		}
	}

	UE_LOG(LogAura, Warning, TEXT("GetStatusTagFromSpec: No 'Ability.Status.*' tag found for ability %s"),
		Spec.Ability.Get() ? *Spec.Ability.Get()->GetName() : TEXT("nullptr"));
	return FGameplayTag();
}

FGameplayAbilitySpec* UAuraAbilitySystemComponent::GetAbilitySpecForAbilityTag(const FGameplayTag& InAbilityTag)
{
	if (!InAbilityTag.IsValid() || InAbilityTag.MatchesTagExact(FAuraGameTagManager::Get().Ability_None))
	{
		return nullptr;
	}

	FScopedAbilityListLock ActiveScopeLock(*this);
	for (FGameplayAbilitySpec& Spec : GetActivatableAbilities())
	{
		for (const FGameplayTag& Tag : Spec.Ability.Get()->GetAssetTags())
		{
			if (Tag.MatchesTag(InAbilityTag))
			{
				return &Spec;
			}
		}
	}

	return nullptr;
}

FGameplayTag UAuraAbilitySystemComponent::GetStatusTagForAbilityTag(const FGameplayTag& InAbilityTag)
{
	if (!InAbilityTag.IsValid() || InAbilityTag.MatchesTagExact(FAuraGameTagManager::Get().Ability_None))
	{
		return FGameplayTag();
	}

	if (const FGameplayAbilitySpec* AbilitySpec = GetAbilitySpecForAbilityTag(InAbilityTag))
	{
		return GetStatusTagFromSpec(*AbilitySpec);
	}

	return FGameplayTag();
}

FGameplayTag UAuraAbilitySystemComponent::GetAbilityInputTagFromAbilityTag(const FGameplayTag& InAbilityTag)
{
	if (!InAbilityTag.IsValid() || InAbilityTag.MatchesTagExact(FAuraGameTagManager::Get().Ability_None))
	{
		return FGameplayTag();
	}

	if (const FGameplayAbilitySpec* AbilitySpec = GetAbilitySpecForAbilityTag(InAbilityTag))
	{
		return GetInputTagFromSpec(*AbilitySpec);
	}

	return FGameplayTag();
}

bool UAuraAbilitySystemComponent::GetDescriptionsByAbilityTag(const FGameplayTag& AbilityTag, FString& OutDescription, FString& OutNextLevelDescription)
{
	if (const FGameplayAbilitySpec* AbilitySpec = GetAbilitySpecForAbilityTag(AbilityTag))
	{
		if (const UAuraGameplayAbility* AuraAbility = Cast<UAuraGameplayAbility>(AbilitySpec->Ability))
		{
			OutDescription = AuraAbility->GetDescription(AbilitySpec->Level);
			OutNextLevelDescription = AuraAbility->GetNextLevelDescription(AbilitySpec->Level + 1);
			return true;
		}
	}

	UAbilityInfo* AbilityInfos = UAuraAbilitySystemLibrary::GetAbilityInfo(GetAvatarActor());
	if (!AbilityTag.IsValid() || AbilityTag.MatchesTagExact(FAuraGameTagManager::Get().Ability_None))
	{
		OutDescription = FString();
	}
	else
	{
		const FAuraAbilityInfo* AbilityInfo = AbilityInfos->FindAbilityInfoByTag(AbilityTag);
		OutDescription = UAuraGameplayAbility::GetLockedDescription(AbilityInfo->LevelRequirement);
	}

	OutNextLevelDescription = FString();
	return false;
}

void UAuraAbilitySystemComponent::UpgradeAttribute(const FGameplayTag& AttributeTag, int32 Delta)
{
	if (GetAvatarActor()->Implements<UPlayerInterface>())
	{
		Server_UpgradeAttribute(AttributeTag, Delta);
	}
}

void UAuraAbilitySystemComponent::UpdateAbilityStatus(int32 Level)
{
	UAbilityInfo* AbilityInfo = UAuraAbilitySystemLibrary::GetAbilityInfo(GetAvatarActor());
	for (const FAuraAbilityInfo& Info : AbilityInfo->AbilitiesInfos)
	{
		if (!Info.AbilityTag.IsValid())
		{
			continue;
		}
		if (Level < Info.LevelRequirement)
		{
			continue;
		}

		// Missing specs represent spells the player has just become eligible to unlock.
		if (GetAbilitySpecForAbilityTag(Info.AbilityTag) == nullptr)
		{
			FGameplayAbilitySpec AbilitySpec = FGameplayAbilitySpec(Info.Ability, 1);
			const FGameplayTag& EligibleStatusTag = FAuraGameTagManager::Get().Ability_Status_Eligible;
			// Eligible abilities are granted without an input slot. The player must spend a spell
			// point and equip the spell before the normal input path can activate it.
			AbilitySpec.GetDynamicSpecSourceTags().AddTag(EligibleStatusTag);
			GiveAbility(AbilitySpec);
			MarkAbilitySpecDirty(AbilitySpec);
			Client_UpdateAbilitySpecStatus(Info.AbilityTag, EligibleStatusTag, 1);
		}
	}
}

void UAuraAbilitySystemComponent::Server_SpendSpellPoints_Implementation(const FGameplayTag& AbilityTag)
{
	const FAuraGameTagManager& TagManager = FAuraGameTagManager::Get();
	if (FGameplayAbilitySpec* AbilitySpec = GetAbilitySpecForAbilityTag(AbilityTag))
	{
		if (GetAvatarActor()->Implements<UPlayerInterface>())
		{
			IPlayerInterface::Execute_AddToSpellPoint(GetAvatarActor(), -1);
		}
		FGameplayTag Status = GetStatusTagFromSpec(*AbilitySpec);
		if (Status.MatchesTagExact(TagManager.Ability_Status_Eligible))
		{
			// First spend unlocks the ability: Eligible -> UnLocked. Later spends level the same spec.
			AbilitySpec->GetDynamicSpecSourceTags().RemoveTag(Status);
			AbilitySpec->GetDynamicSpecSourceTags().AddTag(TagManager.Ability_Status_UnLocked);
		}
		else if (Status.MatchesTagExact(TagManager.Ability_Status_UnLocked) || Status.MatchesTagExact(TagManager.Ability_Status_Equipped))
		{
			AbilitySpec->Level += 1;
		}

		Client_UpdateAbilitySpecStatus(AbilityTag, TagManager.Ability_Status_UnLocked, AbilitySpec->Level);
		MarkAbilitySpecDirty(*AbilitySpec);
	}
}

void UAuraAbilitySystemComponent::Server_EquipAbility_Implementation(const FGameplayTag& InAbilityTag,
	const FGameplayTag& InSlotInputTag)
{
	const FAuraGameTagManager& AuraGameTagManager = FAuraGameTagManager::Get();
	if (FGameplayAbilitySpec* AbilitySpec = GetAbilitySpecForAbilityTag(InAbilityTag))
	{
		const FGameplayTag& PrevInputTag = GetInputTagFromSpec(*AbilitySpec);
		const FGameplayTag& Status = GetStatusTagFromSpec(*AbilitySpec);

		const bool bStatusValid = Status == AuraGameTagManager.Ability_Status_Equipped
			|| Status == FAuraGameTagManager::Get().Ability_Status_UnLocked;

		if (bStatusValid)
		{
			//Handle passive ability activation/deactivation
			if (!SlotIsEmpty(InSlotInputTag))
			{
				//GetSpecFromSlotInputTag
				if (FGameplayAbilitySpec* CurrentAbilitySpecInSlot = GetSpecWithSlotInputTag(InSlotInputTag))
				{
					// if player is trying to equip the same ability to the slot again
					const FGameplayTag CurrentAbilityTagInSlot = GetAbilityTagFromSpec(*CurrentAbilitySpecInSlot);
					if (InAbilityTag.MatchesTagExact(CurrentAbilityTagInSlot))
					{
						Client_EquipAbility(InAbilityTag, AuraGameTagManager.Ability_Status_Equipped, InSlotInputTag, PrevInputTag);
						return;
					}
					
					if (IsPassiveAbility(*CurrentAbilitySpecInSlot))
					{
						MultiCast_ActivatePassiveEffect(CurrentAbilityTagInSlot, false);
						OnDeactivatePassiveAbility.Broadcast(CurrentAbilityTagInSlot);
					}
					
					ClearSlot(CurrentAbilitySpecInSlot);
				}
			}
			
			// if ability does not have a input tag, (it is not active(if it's a passive ability)
			if (!AbilityHasAnySlot(*AbilitySpec))
			{
				if (IsPassiveAbility(*AbilitySpec))
				{
					TryActivateAbility(AbilitySpec->Handle);
					MultiCast_ActivatePassiveEffect(InAbilityTag, true);
				}
			}
			AssignSlotToAbility(*AbilitySpec, InSlotInputTag);
			MarkAbilitySpecDirty(*AbilitySpec);
		}
		Client_EquipAbility(InAbilityTag, AuraGameTagManager.Ability_Status_Equipped, InSlotInputTag, PrevInputTag);
	}
}

void UAuraAbilitySystemComponent::OnRep_ActivateAbilities()
{
	Super::OnRep_ActivateAbilities();

	if (!bStartUpAbilitiesGiven)
	{
		bStartUpAbilitiesGiven = true;
		OnAbilityGiven.Broadcast();
	}
}

void UAuraAbilitySystemComponent::Client_EquipAbility_Implementation(const FGameplayTag& AbilityTag,
	const FGameplayTag& Status, const FGameplayTag& SlotInputTag, const FGameplayTag& PreviousSlotTag)
{
	OnAbilityEquipped.Broadcast(AbilityTag, Status, SlotInputTag, PreviousSlotTag);
}

void UAuraAbilitySystemComponent::MultiCast_ActivatePassiveEffect_Implementation(const FGameplayTag& AbilityTag,
	bool bActivate)
{
	ActivatePassiveEffects.Broadcast(AbilityTag, bActivate);
}

void UAuraAbilitySystemComponent::ClearSlot(FGameplayAbilitySpec* Spec)
{
	// A spec can have at most one InputTag.*. Removing it leaves the ability unlocked/equipped
	// status intact while freeing the UI slot for another spell.
	const FGameplayTag Slot = GetInputTagFromSpec(*Spec);
	Spec->GetDynamicSpecSourceTags().RemoveTag(Slot);
}

void UAuraAbilitySystemComponent::ClearAbilityOfSlot(const FGameplayTag& SlotInputTag)
{
	FScopedAbilityListLock AbilityListLock(*this);
	for (FGameplayAbilitySpec& Spec : GetActivatableAbilities())
	{
		if (AbilityHasSlot(Spec, SlotInputTag))
		{
			ClearSlot(&Spec);
		}
	}
}


bool UAuraAbilitySystemComponent::AbilityHasSlot(FGameplayAbilitySpec& Spec, const FGameplayTag& SlotInputTag)
{
	for (const FGameplayTag& Tag : Spec.GetDynamicSpecSourceTags())
	{
		if (Tag.MatchesTagExact(SlotInputTag))
		{
			return true;
		}
	}

	return false;
}

bool UAuraAbilitySystemComponent::AbilityHasAnySlot(FGameplayAbilitySpec& Spec)
{
	return Spec.GetDynamicSpecSourceTags().HasTag(FGameplayTag::RequestGameplayTag(FName("InputTag")));
}

bool UAuraAbilitySystemComponent::AbilityHasSlot(const FGameplayAbilitySpec& AbilitySpec, const FGameplayTag& SlotTag)
{
	return AbilitySpec.GetDynamicSpecSourceTags().HasTagExact(SlotTag);
}

bool UAuraAbilitySystemComponent::SlotIsEmpty(const FGameplayTag& Slot)
{
	FScopedAbilityListLock AbilityListLock(*this);
	for (FGameplayAbilitySpec& Spec : GetActivatableAbilities())
	{
		if (AbilityHasSlot(Spec, Slot))
		{
			return false;
		}
	}
	
	return true;
}

bool UAuraAbilitySystemComponent::IsPassiveAbility(const FGameplayAbilitySpec& AbilitySpec) const
{
	UAbilityInfo* AbilityInfos = UAuraAbilitySystemLibrary::GetAbilityInfo(GetAvatarActor());
	const FGameplayTag AbilityTag = GetAbilityTagFromSpec(AbilitySpec);
	FGameplayTag AbilityType = FGameplayTag();
	if (const FAuraAbilityInfo* AbilityInfo =  AbilityInfos->FindAbilityInfoByTag(AbilityTag))
	{
		AbilityType = AbilityInfo->AbilityTypeTag;
	}
	
	return (AbilityType.MatchesTagExact(FAuraGameTagManager::Get().Ability_Type_Passive));
}

void UAuraAbilitySystemComponent::AssignSlotToAbility(FGameplayAbilitySpec& Spec, const FGameplayTag& SlotInputTag)
{
	ClearSlot(&Spec);
	Spec.GetDynamicSpecSourceTags().AddTag(SlotInputTag);
}

FGameplayAbilitySpec* UAuraAbilitySystemComponent::GetSpecWithSlotInputTag(const FGameplayTag& SlotInputTag)
{
	FScopedAbilityListLock AbilityListLock(*this);
	for (FGameplayAbilitySpec& Spec : GetActivatableAbilities())
	{
		if (AbilityHasSlot(Spec, SlotInputTag))
		{
			return &Spec;
		}
	}
	return nullptr;
}


/**
 * Client RPC implementation - runs on the owning client.
 *
 * Flow:
 *   1. A GE is applied to self (server or predicted).
 *   2. Engine fires OnGameplayEffectAppliedDelegateToSelf -> calls this RPC.
 *   3. We extract all asset tags from the GE spec (tags set in the GE Blueprint asset).
 *   4. Broadcast them via OnGatherEffectAssetTags.
 *   5. OverlayWidgetController's lambda receives the tags, filters for "Message.*",
 *      looks up the DataTable row, and broadcasts the widget row to the UI.
 */
void UAuraAbilitySystemComponent::Client_OnEffectAppliedToSelf_Implementation(UAbilitySystemComponent* AbilitySystemComponent, const FGameplayEffectSpec& GameEffectSpec, FActiveGameplayEffectHandle ActiveGameEffectHandle)
{
	FGameplayTagContainer TagContainer;
	GameEffectSpec.GetAllAssetTags(TagContainer);
	OnGatherEffectAssetTags.Broadcast(TagContainer);
}

void UAuraAbilitySystemComponent::Server_UpgradeAttribute_Implementation(const FGameplayTag& AttributeTag, int32 Delta)
{
	// Send a Gameplay Event for passive gameplay abilities that listen for attribute upgrades.
	FGameplayEventData PayLoad;
	PayLoad.EventMagnitude = Delta;
	PayLoad.EventTag = AttributeTag;
	UAbilitySystemBlueprintLibrary::SendGameplayEventToActor(GetAvatarActor(), AttributeTag, PayLoad);

	// Attribute points live on the player progression interface, not the ASC.
	check(GetAvatarActor()->Implements<UPlayerInterface>());

	IPlayerInterface::Execute_AddToAttributePoint(GetAvatarActor(), -Delta);
}

void UAuraAbilitySystemComponent::Client_UpdateAbilitySpecStatus_Implementation(const FGameplayTag& AbilityTag, const FGameplayTag& StatusTag, int32 AbilityLevel)
{
	OnAbilityStatusChanged.Broadcast(AbilityTag, StatusTag, AbilityLevel);
}
