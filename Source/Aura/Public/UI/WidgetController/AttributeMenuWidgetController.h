// @Copyright HaolunYuan

#pragma once

#include "CoreMinimal.h"
#include "UI/WidgetController/AuraWidgetController.h"
#include "Components/AbilitySystem/Data/AttributeDataAsset.h"
#include "ControllerDelegates.h"
#include "AttributeMenuWidgetController.generated.h"

struct FAuraAttributeTagMetadatas;
struct FGameplayTag;

/**
 * Broadcasts one row update to the Attribute Menu UI.
 * - AttributeTagMetadatas: static metadata from DataAsset (name/description/tag/attribute handle)
 * - NewValue: runtime value read from the current AttributeSet
 */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnAttributeMenuChangeSignature, FAuraAttributeTagMetadatas, AttributeTagMetadatas, float, NewValue);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnAttributeDeltaChangedSignature, const FGameplayTag&, AttributeTag, int32, CurrentDelta);

/**
 * Widget Controller responsible for the full Attribute Menu panel.
 *
 * Data flow:
 *   1) Iterate all attribute rows configured in AttributeTagsDataAsset.
 *   2) Read each runtime value from CachedAttributeSet.
 *   3) Broadcast to UI widgets via OnAttributeMenuChange.
 *
 * Important runtime dependencies:
 *   - AttributeTagsDataAsset must be assigned in BP defaults.
 *   - CachedAbilitySystemComponent / CachedAttributeSet must already be set.
 */
UCLASS(BlueprintType, Blueprintable)
class AURA_API UAttributeMenuWidgetController : public UAuraWidgetController
{
	GENERATED_BODY()

public:
	/** Pushes all initial attribute values when the menu is opened/initialized. */
	virtual void BroadcastInitialValues() override;

	/** Subscribes to ASC attribute-value-change delegates for live UI updates. */
	virtual void BindAllDependencies() override;

	UFUNCTION(BlueprintCallable)
	void UpgradeAttribute(const FGameplayTag& AttributeTag);

	/** Removes one unconfirmed assignment delta and refunds it to the current session pool. */
	UFUNCTION(BlueprintCallable)
	void DeductAttribute(const FGameplayTag& AttributeTag);

	/** Captures the current primary attribute values and starts a local preview/assignment session. */
	void BeginAssignmentSession();

	/** Ends the local preview session and restores unconfirmed preview values in the UI. */
	void EndAssignmentSession();

	/** Sends all pending primary attribute deltas to the ASC/server and clears the local preview deltas. */
	UFUNCTION(BlueprintCallable)
	void ConfirmAttributeAssignments();

private:
	/** Helper used by both initial push and live updates to avoid duplicate logic. */
	void BroadcastAttributeDataEntry(const FAuraAttributeTagMetadatas& AttributeTagInfo);

	/** Blueprint binds to this to update each row in the attribute menu. */
	UPROPERTY(BlueprintAssignable, Category = "Attribute Menu Delegate", meta = (AllowPrivateAccess = "true"))
	FOnAttributeMenuChangeSignature OnAttributeMenuChange;

	UPROPERTY(BlueprintAssignable, Category = "Attribute Menu Delegate", meta = (AllowPrivateAccess = "true"))
	FOnPlayerStatChangedSignature OnAttributePointsChanged;

	UPROPERTY(BlueprintAssignable, Category = "Attribute Menu Delegate", meta = (AllowPrivateAccess = "true"))
	FOnPlayerStatChangedSignature OnSessionAttributePointsAvailableChanged;

	UPROPERTY(BlueprintAssignable, Category = "Attribute Menu Delegate", meta = (AllowPrivateAccess = "true"))
	FOnAttributeDeltaChangedSignature OnSessionAttributeDeltaChanged;

	/**
	 * DataAsset listing all attributes shown in the menu.
	 * Bug-prone if null: always assign in BP_AttributeMenuWidgetController defaults.
	 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Attribute Menu", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UAttributeDataAsset> AttributeTagsDataAsset = nullptr;

	/** Baseline attribute values captured when an assignment session begins, used to rollback previews. */
	UPROPERTY(Transient)
	TMap<FGameplayTag, float> SessionBasePrimaryAttributeValues;

	/** Local pending point deltas keyed by primary attribute tag until the player confirms. */
	UPROPERTY(Transient)
	TMap<FGameplayTag, int32> SessionPrimaryAttributeDeltas;

	/** Points still available for local preview assignment in the current session. */
	UPROPERTY(Transient)
	int32 SessionAttributePointsAvailable = 0;

	/** True while the menu is showing local preview deltas instead of only committed AttributeSet values. */
	UPROPERTY(Transient)
	bool bAssignmentSessionInProgress = false;

	/** True once the player has changed at least one delta that still needs confirm/cancel handling. */
	UPROPERTY(Transient)
	bool bWaitingForConfirmation = false;
};

