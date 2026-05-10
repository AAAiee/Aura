// @Copyright HaolunYuan


#include "Components/AbilitySystem/AuraAbilitySystemComponent.h"

#include "AbilitySystemBlueprintLibrary.h"
#include "AuraLogCategory.h"
#include "Components/AbilitySystem/Ability/AuraGameplayAbility.h"
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
			AbilitySpec.DynamicAbilityTags.AddTag(AuraAbility->StartupGameTag);
			GiveAbility(AbilitySpec);
		}
	}

	bStartUpAbilitiesGiven = true;
	AbilityGivenDelegate.Broadcast(this);
}

void UAuraAbilitySystemComponent::AddCharacterPassiveAbilities(const TArray<TSubclassOf<class UGameplayAbility>>& InPassiveAbilitiesClasses)
{
	for (const auto& AbilityClass : InPassiveAbilitiesClasses)
	{
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

	for (auto& GrantedAbilitySpec : GetActivatableAbilities())
	{
		if (GrantedAbilitySpec.DynamicAbilityTags.HasTagExact(InputTag))
		{
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

	for (auto& GrantedAbilitySpec : GetActivatableAbilities())
	{
		if (GrantedAbilitySpec.DynamicAbilityTags.HasTagExact(InputTag))
		{
			AbilitySpecInputReleased(GrantedAbilitySpec);
		}
	}
}

void UAuraAbilitySystemComponent::AbilityInputTagHeld(FGameplayTag InputTag)
{
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
		for (const FGameplayTag& Tag : Spec.Ability.Get()->AbilityTags)
		{
			if (Tag.MatchesTag(FGameplayTag::RequestGameplayTag(FName("Ability"))))
			{
				return Tag;
			}
		}
	}

	check(false);
	return FGameplayTag();
}

FGameplayTag UAuraAbilitySystemComponent::GetInputTagFromSpec(const FGameplayAbilitySpec& Spec)
{
	if (Spec.Ability.Get())
	{
		for (const FGameplayTag& Tag : Spec.DynamicAbilityTags)
		{
			if (Tag.MatchesTag(FGameplayTag::RequestGameplayTag(FName("InputTag"))))
			{
				return Tag;
			}
		}
	}

	check(false);
	return FGameplayTag();
}

void UAuraAbilitySystemComponent::UpgradeAttribute(const FGameplayTag& AttributeTag, int32 Delta)
{
	if (GetAvatarActor()->Implements<UPlayerInterface>())
	{
		Server_UpgradeAttribute(AttributeTag, Delta);
	}
}

void UAuraAbilitySystemComponent::OnRep_ActivateAbilities()
{
	Super::OnRep_ActivateAbilities();

	if (!bStartUpAbilitiesGiven)
	{
		bStartUpAbilitiesGiven = true;
		AbilityGivenDelegate.Broadcast(this);
	}
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
