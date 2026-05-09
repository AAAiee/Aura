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

private:
	/** Helper used by both initial push and live updates to avoid duplicate logic. */
	void BroadcastAttributeDataEntry(const FAuraAttributeTagMetadatas& AttributeTagInfo);

	/** Blueprint binds to this to update each row in the attribute menu. */
	UPROPERTY(BlueprintAssignable, Category = "Attribute Menu Delegate", meta = (AllowPrivateAccess = "true"))
	FOnAttributeMenuChangeSignature OnAttributeMenuChange;

	UPROPERTY(BlueprintAssignable, Category = "Attribute Menu Delegate", meta = (AllowPrivateAccess = "true"))
	FOnPlayerStatChangedSignature OnAttributePointsChanged;

	/**
	 * DataAsset listing all attributes shown in the menu.
	 * Bug-prone if null: always assign in BP_AttributeMenuWidgetController defaults.
	 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Attribute Menu", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UAttributeDataAsset> AttributeTagsDataAsset = nullptr;
};

