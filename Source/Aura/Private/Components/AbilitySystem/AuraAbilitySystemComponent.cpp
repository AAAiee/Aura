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

void UAuraAbilitySystemComponent::AbilityInputTagPressed(FGameplayTag InputTag)
{
	if (!InputTag.IsValid())
	{
		return;
	}

	FScopedAbilityListLock AbilityListLock(*this);
	for (auto& GrantedAbilitySpec : GetActivatableAbilities())
	{
		if (GrantedAbilitySpec.GetDynamicSpecSourceTags().HasTagExact(InputTag))
		{
			// Notify the spec first so active abilities receive their input-pressed event. Only then
			// try activation for abilities that are currently idle.
			AbilitySpecInputPressed(GrantedAbilitySpec);
			if (!GrantedAbilitySpec.IsActive())
			{
				TryActivateAbility(GrantedAbilitySpec.Handle);
			}
		}
	}
}

void UAuraAbilitySystemComponent::AbilityInputTagReleased(FGameplayTag InputTag)
{
	if (!InputTag.IsValid())
	{
		return;
	}

	FScopedAbilityListLock AbilityListLock(*this);
	for (auto& GrantedAbilitySpec : GetActivatableAbilities())
	{
		if (GrantedAbilitySpec.GetDynamicSpecSourceTags().HasTagExact(InputTag))
		{
			AbilitySpecInputReleased(GrantedAbilitySpec);
			if (GrantedAbilitySpec.IsActive())
			{
				// Primary Instance is only valid for instance per actor
				if (const UGameplayAbility* PrimaryInstance = GrantedAbilitySpec.GetPrimaryInstance())
				{
					InvokeReplicatedEvent(EAbilityGenericReplicatedEvent::InputReleased, GrantedAbilitySpec.Handle,PrimaryInstance->GetCurrentActivationInfo().GetActivationPredictionKey());
				}
			}
		}
	}
}

void UAuraAbilitySystemComponent::AbilityInputTagHeld(FGameplayTag InputTag)
{
	// Kept as a routing hook for future held/channelled abilities. Pressed/released currently
	// carry the active gameplay behavior.
	if (!InputTag.IsValid())
	{
		return;
	}

	FScopedAbilityListLock AbilityListLock(*this);
	for (auto& GrantedAbilitySpec : GetActivatableAbilities())
	{
		if (GrantedAbilitySpec.GetDynamicSpecSourceTags().HasTagExact(InputTag))
		{
			if (GrantedAbilitySpec.IsActive())
			{
				// Primary Instance is only valid for instance per actor
				if (const UGameplayAbility* PrimaryInstance = GrantedAbilitySpec.GetPrimaryInstance())
				{
					InvokeReplicatedEvent(EAbilityGenericReplicatedEvent::InputPressed, GrantedAbilitySpec.Handle,PrimaryInstance->GetCurrentActivationInfo().GetActivationPredictionKey());
				}
			}
		}
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
	FAuraGameTagManager& TagManager = FAuraGameTagManager::Get();
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

void UAuraAbilitySystemComponent::Server_EquipAbility_Implementation(const FGameplayTag& AbilityTag,
	const FGameplayTag& SlotInputTag)
{
	const FAuraGameTagManager& AuraGameTagManager = FAuraGameTagManager::Get();
	if (FGameplayAbilitySpec* AbilitySpec = GetAbilitySpecForAbilityTag(AbilityTag))
	{
		const FGameplayTag& PrevInputTag = GetInputTagFromSpec(*AbilitySpec);
		const FGameplayTag& Status = GetStatusTagFromSpec(*AbilitySpec);

		const bool bStatusValid = Status == AuraGameTagManager.Ability_Status_Equipped
			|| Status == FAuraGameTagManager::Get().Ability_Status_UnLocked;

		if (bStatusValid)
		{
			// Keep each input slot exclusive across equipped abilities.
			ClearAbilityOfSlot(SlotInputTag);
			ClearSlot(AbilitySpec);

			AbilitySpec->GetDynamicSpecSourceTags().AddTag(SlotInputTag);
			if (Status.MatchesTagExact(AuraGameTagManager.Ability_Status_UnLocked))
			{
				AbilitySpec->GetDynamicSpecSourceTags().RemoveTag(AuraGameTagManager.Ability_Status_UnLocked);
				AbilitySpec->GetDynamicSpecSourceTags().AddTag(AuraGameTagManager.Ability_Status_Equipped);
			}
			MarkAbilitySpecDirty(*AbilitySpec);
		}

		Client_EquipAbility(AbilityTag, AuraGameTagManager.Ability_Status_Equipped, SlotInputTag, PrevInputTag);
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

void UAuraAbilitySystemComponent::ClearSlot(FGameplayAbilitySpec* Spec)
{
	// A spec can have at most one InputTag.*. Removing it leaves the ability unlocked/equipped
	// status intact while freeing the UI slot for another spell.
	const FGameplayTag Slot = GetInputTagFromSpec(*Spec);
	Spec->GetDynamicSpecSourceTags().RemoveTag(Slot);
	MarkAbilitySpecDirty(*Spec);
}

void UAuraAbilitySystemComponent::ClearAbilityOfSlot(const FGameplayTag& SlotInputTag)
{
	FScopedAbilityListLock AbilityListLock(*this);
	for (FGameplayAbilitySpec& Spec : GetActivatableAbilities())
	{
		if (AbilityHasSlot(&Spec, SlotInputTag))
		{
			ClearSlot(&Spec);
		}
	}
}

bool UAuraAbilitySystemComponent::AbilityHasSlot(FGameplayAbilitySpec* Spec, const FGameplayTag& SlotInputTag)
{
	for (const FGameplayTag& Tag : Spec->GetDynamicSpecSourceTags())
	{
		if (Tag.MatchesTagExact(SlotInputTag))
		{
			return true;
		}
	}

	return false;
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
