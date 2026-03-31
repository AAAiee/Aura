// @Copyright HaolunYuan

#pragma once

#include "CoreMinimal.h"
#include "AbilitySystemComponent.h"
#include "AuraAbilitySystemComponent.generated.h"

/**
 * Native (C++) multicast delegate that fires when a Gameplay Effect is applied to self.
 * Carries the effect's asset tags so listeners (e.g., OverlayWidgetController) can
 * react to specific tags (like "Message.HealthPotion") and update the UI.
 *
 * Why not a Dynamic delegate? This delegate is bound via AddLambda/AddUObject in C++,
 * so a native multicast is simpler and faster than a Dynamic (Blueprint-capable) one.
 */
DECLARE_MULTICAST_DELEGATE_OneParam(OnGatherEffectAssetTag, const FGameplayTagContainer& /*AssetTags*/)


/**
 * Custom Ability System Component for the Aura project.
 *
 * Adds one key behavior on top of UAbilitySystemComponent:
 *   - Listens for OnGameplayEffectAppliedDelegateToSelf (engine delegate)
 *   - Extracts asset tags from the applied GE
 *   - Rebroadcasts them via OnGatherEffectAssetTags for the UI layer to consume
 *
 * This keeps the UI decoupled from GAS internals ¡ª widgets never touch FGameplayEffectSpec directly.
 */
UCLASS()
class AURA_API UAuraAbilitySystemComponent : public UAbilitySystemComponent
{
	GENERATED_BODY()
	
public:
	/**
	 * Called once after the ASC's AbilityActorInfo is set (Owner + Avatar are known).
	 * Binds the internal OnGameplayEffectAppliedDelegateToSelf ¡ú Client_OnEffectAppliedToSelf.
	 *
	 * TIMING: Must be called AFTER the UI widget controller subscribes to OnGatherEffectAssetTags,
	 * otherwise the broadcast has 0 listeners. See AAuraCharacter::InitAbilityActorInfo() for ordering.
	 */
	void AbilityActorInfoSet();
	void AddCharacterAbilities(const TArray<TSubclassOf<class UGameplayAbility>>& InAbilitiesClasses);

	void AbilityInputTagPressed(FGameplayTag InputTag);
	void AbilityInputTagReleased(FGameplayTag InputTag);
	void AbilityInputTagHeld(FGameplayTag InputTag);


public:
	/** Public delegate ¡ª UI layer (OverlayWidgetController) subscribes to this. */
	OnGatherEffectAssetTag OnGatherEffectAssetTags;


private:
	/**
	 * Client RPC ¡ª called when a GE is applied to self.
	 * Extracts all asset tags from the GE spec and broadcasts them via OnGatherEffectAssetTags.
	 * Marked Client+Reliable so the broadcast always reaches the owning client for UI updates.
	 */
	UFUNCTION(Client, Reliable)
	void Client_OnEffectAppliedToSelf(UAbilitySystemComponent* AbilitySystemComponent, const FGameplayEffectSpec& GameEffectSpec, FActiveGameplayEffectHandle ActiveGameEffectHandle);

};
