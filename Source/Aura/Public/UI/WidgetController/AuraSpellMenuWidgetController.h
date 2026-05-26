// @Copyright HaolunYuan

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "UI/WidgetController/AuraWidgetController.h"
#include "UI/WidgetController/ControllerDelegates.h"
#include "AuraSpellMenuWidgetController.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_FourParams(FSpellGlobeSelectedSignature, bool, bSpendPointsButtonEnabled, bool, bEquippedButtonEnabled, const FString&, Description, const FString&, NextLevelDescription);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FWaitingForEquipSlotSelection, const FGameplayTag&, AbilityTypeTag);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnAbilityInputTagReassigned, const FGameplayTag&, AbilityTag);

/** Lightweight selection state cached while the player is inspecting or equipping one spell. */
struct FSelectedAbility
{
	FSelectedAbility() = default;

	FSelectedAbility(const FGameplayTag& InAbilityTag, const FGameplayTag& InStatusTag)
		: AbilityTag(InAbilityTag)
		, StatusTag(InStatusTag)
	{
	}

	FGameplayTag AbilityTag;
	FGameplayTag StatusTag;
};

/** Widget controller for spell unlock, upgrade, and equipment UI. */
UCLASS(BlueprintType, Blueprintable)
class AURA_API UAuraSpellMenuWidgetController : public UAuraWidgetController
{
	GENERATED_BODY()

public:
	UAuraSpellMenuWidgetController();

	/** Broadcasts current spell rows and current spell-point count when the menu opens. */
	virtual void BroadcastInitialValues() override;

	/** Binds to ASC ability-status/equipment delegates and PlayerState spell-point changes. */
	virtual void BindAllDependencies() override;

	/** Updates the selected spell and broadcasts button enablement plus description text. */
	UFUNCTION(BlueprintCallable)
	void SpellGlobeSelected(const FGameplayTag& AbilityTag);

	/** Spends one spell point on the selected ability through the authoritative ASC server RPC. */
	UFUNCTION(BlueprintCallable)
	void OnSpentPointsButtonPressed();

	/** Clears the current selection and disables spell-menu action buttons. */
	UFUNCTION(BlueprintCallable)
	void GlobeDeselect();

	/** Starts slot-selection mode for the currently selected spell. */
	UFUNCTION(BlueprintCallable)
	void OnEquipButtonPressed();

	/** Attempts to equip the selected spell into a clicked compatible input slot. */
	UFUNCTION(BlueprintCallable)
	void OnEquipRowSpellGlobePressed(const FGameplayTag& SlotAbilityTypeTag, const FGameplayTag& SlotInputTag);

	/** Broadcasts whether the spend/equip buttons are enabled plus current and next-level descriptions. */
	UPROPERTY(BlueprintAssignable)
	FSpellGlobeSelectedSignature SpellGlobeSelectedDelegate;

	/** Broadcasts the current spell-point count for menu widgets. */
	UPROPERTY(BlueprintAssignable)
	FOnPlayerStatChangedSignature OnSpellPointsChanged;

	/** Tells compatible slot widgets to enter "waiting for equip target" state. */
	UPROPERTY(BlueprintAssignable)
	FWaitingForEquipSlotSelection OnWaitingForEquipSlotSelection;

	/** Tells compatible slot widgets to leave "waiting for equip target" state. */
	UPROPERTY(BlueprintAssignable)
	FWaitingForEquipSlotSelection OnStopWaitingForEquipSlotSelection;

	/** Notifies slot widgets that an ability was reassigned and any stale visual selection should clear. */
	UPROPERTY(BlueprintAssignable)
	FOnAbilityInputTagReassigned OnAbilityReassigned;

private:
	/** Computes button enablement from ability status and available spell points. */
	static void ShouldEnableButton(const FGameplayTag& AbilityStatus, int32 SpellPoints, bool& bShouldEnableSpellPointsButton, bool& bShouldEnableEquipButton);

	/** Finds the Ability.Type.* tag for an ability so it can only be equipped into compatible rows. */
	FGameplayTag GetGameplayTypeTagForAbilityTag(const FGameplayTag& AbilityTag) const;

	/** Exits equip-slot selection mode and tells the UI to clear compatible slot highlights. */
	void StopWaitingForEquipSelection();

	/** Shared helper for pushing selected ability descriptions and button states to Blueprint. */
	void BroadcastAbilityGlobeEnabledStatusAndAbilityDescriptions(const FGameplayTag& AbilityTag, const FGameplayTag& AbilityStatusTag);

	/** Handles the client-side result of a successful server equipment request. */
	void OnAbilityEquippedCallback(const FGameplayTag& AbilityTag, const FGameplayTag& StatusTag, const FGameplayTag& SlotInputTag, const FGameplayTag& PreviousSlotTag);

	/** Ability currently selected in the spell menu. */
	FSelectedAbility CurrentlySelectedAbility;

	/** Cached spell points so button enablement can refresh without querying PlayerState every click. */
	int32 CurrentSpellPoints = 0;

	/** True while the menu waits for the player to click a compatible input slot. */
	bool bWaitingForEquipSelection = false;

	/** Previous slot for an equipped ability while it is being reassigned. */
	FGameplayTag CachedAbilityInputTag;
};
