// @Copyright HaolunYuan


#include "Components/AbilitySystem/AuraAbilitySystemComponent.h"
#include "GameplayAbilitySpec.h"
#include "Components/AbilitySystem/Ability/AuraGameplayAbility.h"

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

/**
 * Client RPC implementation ¡ª runs on the owning client.
 *
 * Flow:
 *   1. A GE is applied to self (server or predicted).
 *   2. Engine fires OnGameplayEffectAppliedDelegateToSelf ¡ú calls this RPC.
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
