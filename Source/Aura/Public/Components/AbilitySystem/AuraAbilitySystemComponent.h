// @Copyright HaolunYuan

#pragma once

#include "CoreMinimal.h"
#include "AbilitySystemComponent.h"
#include "GameplayTagContainer.h"
#include "AuraAbilitySystemComponent.generated.h"

/**
 * Native (C++) multicast delegate that fires when a Gameplay Effect is applied to self.
 * Carries the effect's asset tags so listeners (e.g., OverlayWidgetController) can
 * react to specific tags (like "Message.HealthPotion") and update the UI.
 *
 * Why not a Dynamic delegate? This delegate is bound via AddLambda/AddUObject in C++,
 * so a native multicast is simpler and faster than a Dynamic (Blueprint-capable) one.
 */
DECLARE_MULTICAST_DELEGATE_OneParam(OnGatherEffectAssetTag, const FGameplayTagContainer& /*AssetTags*/);
DECLARE_MULTICAST_DELEGATE(FAbilityGiven);
DECLARE_MULTICAST_DELEGATE_ThreeParams(FAbilityStatusChanged, const FGameplayTag& /*AbilityTag*/, const FGameplayTag& /*AbilityStatusTag*/, int32 /*AbilityLevel*/);
DECLARE_MULTICAST_DELEGATE_FourParams(FAbilityEquipped, const FGameplayTag& /*AbilityTag*/, const FGameplayTag& /*StatusTag*/, const FGameplayTag& /*SlotTag*/, const FGameplayTag& /*PreviousSlotTag*/);

/**
 * Custom Ability System Component for the Aura project.
 *
 * Adds one key behavior on top of UAbilitySystemComponent:
 *   - Listens for OnGameplayEffectAppliedDelegateToSelf (engine delegate)
 *   - Extracts asset tags from the applied GE
 *   - Rebroadcasts them via OnGatherEffectAssetTags for the UI layer to consume
 *
 * This keeps the UI decoupled from GAS internals - widgets never touch FGameplayEffectSpec directly.
 */
UCLASS()
class AURA_API UAuraAbilitySystemComponent : public UAbilitySystemComponent
{
	GENERATED_BODY()

public:
	/**
	 * Called once after the ASC's AbilityActorInfo is set (Owner + Avatar are known).
	 * Binds the internal OnGameplayEffectAppliedDelegateToSelf -> Client_OnEffectAppliedToSelf.
	 *
	 * TIMING: Must be called AFTER the UI widget controller subscribes to OnGatherEffectAssetTags,
	 * otherwise the broadcast has 0 listeners. See AAuraCharacter::InitAbilityActorInfo() for ordering.
	 */
	void AbilityActorInfoSet();

	/** Grants startup active abilities and tags each spec with its input slot plus Equipped status. */
	void AddCharacterAbilities(const TArray<TSubclassOf<class UGameplayAbility>>& InAbilitiesClasses);

	/** Grants passive abilities and immediately activates them once so persistent effects can begin. */
	void AddCharacterPassiveAbilities(const TArray<TSubclassOf<class UGameplayAbility>>& InPassiveAbilitiesClasses);

	/* Input Routing */
	/** Marks a matching ability spec as pressed and activates it if it is not already running. */
	void AbilityInputTagPressed(FGameplayTag InputTag);

	/** Marks a matching ability spec as released so active abilities can end channels/holds cleanly. */
	void AbilityInputTagReleased(FGameplayTag InputTag);

	/** Reserved for abilities that should repeatedly respond while their input tag is held. */
	void AbilityInputTagHeld(FGameplayTag InputTag);

	/* Ability Spec Queries */
	/** Iterates granted abilities under GAS's scoped list lock so specs are not mutated mid-loop. */
	void ForEachAbility(const TFunction<void(const FGameplayAbilitySpec&)>& Predicate);

	/** Reads the authored Ability.* asset tag from a granted spec. */
	static FGameplayTag GetAbilityTagFromSpec(const FGameplayAbilitySpec& Spec);

	/** Reads the current InputTag.* dynamic spec tag, which represents the equipped slot. */
	static FGameplayTag GetInputTagFromSpec(const FGameplayAbilitySpec& Spec);

	/** Reads the Ability.Status.* dynamic spec tag used by spell menu unlock/equip UI. */
	static FGameplayTag GetStatusTagFromSpec(const FGameplayAbilitySpec& Spec);

	/** Finds the mutable spec for a specific Ability.* tag, or nullptr if the player has not received it yet. */
	FGameplayAbilitySpec* GetAbilitySpecForAbilityTag(const FGameplayTag& InAbilityTag);

	/** Convenience lookup for the status tag of a specific ability. */
	FGameplayTag GetStatusTagForAbilityTag(const FGameplayTag& InAbilityTag);

	/** Convenience lookup for the input slot tag of a specific ability. */
	FGameplayTag GetAbilityInputTagFromAbilityTag(const FGameplayTag& InAbilityTag);

	/** Builds current and next-level description strings for the spell menu. */
	bool GetDescriptionsByAbilityTag(const FGameplayTag& AbilityTag, FString& OutDescription, FString& OutNextLevelDescription);

	/* Player Progression */
	/** Client-facing request that forwards an attribute upgrade to the authoritative server RPC. */
	UFUNCTION(BlueprintCallable)
	void UpgradeAttribute(const FGameplayTag& AttributeTag, int32 Delta);

	/** Grants newly eligible spell specs when the player reaches a level requirement. */
	UFUNCTION(BlueprintCallable)
	void UpdateAbilityStatus(int32 Level);

	/* Spell Menu RPCs */
	/** Server spends one spell point to unlock or level the requested ability. */
	UFUNCTION(Server, Reliable)
	void Server_SpendSpellPoints(const FGameplayTag& AbilityTag);

	/** Server equips an unlocked ability into the requested input slot and clears slot conflicts. */
	UFUNCTION(Server, Reliable)
	void Server_EquipAbility(const FGameplayTag& AbilityTag, const FGameplayTag& SlotInputTag);

protected:
	virtual void OnRep_ActivateAbilities() override;

public:
	/** UI layer (OverlayWidgetController) subscribes to this for applied GE asset tags. */
	OnGatherEffectAssetTag OnGatherEffectAssetTags;

	/** Fired when initial ability specs are available, covering both server grant and client replication order. */
	FAbilityGiven OnAbilityGiven;

	/** Fired when an ability moves between Locked/Eligible/Unlocked/Equipped or changes level. */
	FAbilityStatusChanged OnAbilityStatusChanged;

	/** Fired after equipment changes so UI slots can clear the previous slot and fill the new one. */
	FAbilityEquipped OnAbilityEquipped;

	/** True once startup abilities have been granted locally or replicated from the server. */
	bool bStartUpAbilitiesGiven = false;

private:
	/**
	 * Client RPC - called when a GE is applied to self.
	 * Extracts all asset tags from the GE spec and broadcasts them via OnGatherEffectAssetTags.
	 * Marked Client+Reliable so the broadcast always reaches the owning client for UI updates.
	 */
	UFUNCTION(Client, Reliable)
	void Client_OnEffectAppliedToSelf(UAbilitySystemComponent* AbilitySystemComponent, const FGameplayEffectSpec& GameEffectSpec, FActiveGameplayEffectHandle ActiveGameEffectHandle);

	/** Authoritative attribute upgrade path; sends a gameplay event and spends player attribute points. */
	UFUNCTION(Server, Reliable)
	void Server_UpgradeAttribute(const FGameplayTag& AttributeTag, int32 Delta);

	/** Mirrors ability status/level changes to the owning client for spell menu refresh. */
	UFUNCTION(Client, Reliable)
	void Client_UpdateAbilitySpecStatus(const FGameplayTag& AbilityTag, const FGameplayTag& StatusTag, int32 AbilityLevel);

	/** Mirrors equipment changes to the owning client for overlay and spell menu refresh. */
	UFUNCTION(Client, Reliable)
	void Client_EquipAbility(const FGameplayTag& AbilityTag, const FGameplayTag& Status, const FGameplayTag& SlotInputTag, const FGameplayTag& PreviousSlotTag);

	/** Removes the currently assigned InputTag.* from one spec. */
	void ClearSlot(FGameplayAbilitySpec* Spec);

	/** Clears any ability currently occupying a requested input slot. */
	void ClearAbilityOfSlot(const FGameplayTag& SlotInputTag);

	/** Returns true when a spec already owns the requested InputTag.* slot. */
	static bool AbilityHasSlot(FGameplayAbilitySpec* Spec, const FGameplayTag& SlotInputTag);
};
