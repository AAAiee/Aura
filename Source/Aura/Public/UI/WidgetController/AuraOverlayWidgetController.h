// @Copyright HaolunYuan

#pragma once

#include "CoreMinimal.h"
#include "ControllerDelegates.h"
#include "Engine/DataTable.h"
#include "GameplayTagContainer.h"
#include "UI/WidgetController/AuraWidgetController.h"
#include "AuraOverlayWidgetController.generated.h"

class UAuraUserWidget;
class UAuraAbilitySystemComponent;

/**
 * Data Table row struct for message widgets.
 * Each row maps a GameplayTag (e.g., "Message.HealthPotion") to a message, widget class, and icon.
 * The DataTable is set in the Blueprint (EditDefaultsOnly) on the WidgetController.
 */
USTRUCT(BlueprintType)
struct FUIWidgetRow : public FTableRowBase
{
	GENERATED_BODY()

	/** Tag that identifies this message (must be a child of "Message" tag). */
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FGameplayTag MessageTag = FGameplayTag();

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FText Message = FText();
	
	/** Optional: a specific widget class to display for this message. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	TSubclassOf<UAuraUserWidget> MessageWidgetClass = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	UTexture2D* Image = nullptr;
};

/* Dynamic multicast delegates that Blueprint widgets bind to for overlay updates. */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnMessageWidgetRowSignature, const FUIWidgetRow&, Row);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnInventoryMessageSignature, const FText&, Message);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnPlayerStateStatChanged, const float, NewStatValue);


/**
 * Helper template - looks up a DataTable row by GameplayTag name.
 * Returns nullptr if the tag has no matching row (always null-check the result!).
 */
template<typename T>
T* GetDataTableRowFromTag(const UDataTable* DataTable, const FGameplayTag& Tag)
{
	return DataTable->FindRow<T>(Tag.GetTagName(), FString(""));
}


/**
 * Widget Controller for the main Overlay HUD (health bar, mana bar, message popups).
 *
 * Two data pathways:
 *
 *   1. Attribute Changes (Health, Mana):
 *      ASC attribute change delegate -> C++ callback -> Dynamic delegate -> Blueprint widget
 *
 *   2. Effect Messages (e.g., "Health Potion Collected"):
 *      GE applied -> ASC fires OnGatherEffectAssetTags -> lambda filters for "Message.*" tags
 *      -> DataTable lookup -> OnSendMessageWidgetRow -> Blueprint widget
 *
 * BlueprintType + Blueprintable: can be subclassed in Blueprint for additional customization.
 */
UCLASS(BlueprintType, Blueprintable)
class AURA_API UAuraOverlayWidgetController : public UAuraWidgetController
{
	GENERATED_BODY()

public:
	/** Reads current attribute values from the AttributeSet and broadcasts them to the UI. */
	virtual void BroadcastInitialValues() override;

	/**
	 * Subscribes to:
	 *   - ASC attribute change delegates (Health, MaxHealth, Mana, MaxMana)
	 *   - AuraASC's OnGatherEffectAssetTags (for message popup system)
	 */
	virtual void BindAllDependencies() override;

private:
	/** Converts absolute XP into a normalized progress bar value for the current level. */
	void OnXpChanged(const int32 NewXP);

	/** Broadcasts level text updates to Blueprint widgets. */
	void OnLevelChanged(const int32 NewLevel) const;

	/** Mirrors equipped ability changes into overlay action slots. */
	void OnAbilityEquippedCallback(const FGameplayTag& AbilityTag, const FGameplayTag& StatusTag, const FGameplayTag& SlotInputTag, const FGameplayTag& PreviousSlotTag) const;

	/* Blueprint-assignable delegates: Blueprint widgets bind to these in WidgetControllerSet. */
	UPROPERTY(BlueprintAssignable, Category = "GAS|Delegate", meta = (AllowPrivateAccess = "true"))
	FOnAttributeChangeSignature OnHealthChanged;

	UPROPERTY(BlueprintAssignable, Category = "GAS|Delegate", meta = (AllowPrivateAccess = "true"))
	FOnAttributeChangeSignature OnMaxHealthChanged;

	UPROPERTY(BlueprintAssignable, Category = "GAS|Delegate", meta = (AllowPrivateAccess = "true"))
	FOnAttributeChangeSignature OnManaChanged;

	UPROPERTY(BlueprintAssignable, Category = "GAS|Delegate", meta = (AllowPrivateAccess = "true"))
	FOnAttributeChangeSignature OnMaxManaChanged;

	/** Fires when a GE with a "Message.*" tag is applied - Blueprint shows a popup widget. */
	UPROPERTY(BlueprintAssignable, Category = "GAS|Delegate", meta = (AllowPrivateAccess = "true"))
	FOnMessageWidgetRowSignature OnSendMessageWidgetRow;

	UPROPERTY(BlueprintAssignable, Category = "Inventory|Delegate", meta = (AllowPrivateAccess = "true"))
	FOnInventoryMessageSignature OnInventoryMessageRequestedDelegate;

	// XP is not a GAS attribute, but the overlay only needs the same float broadcast shape.
	UPROPERTY(BlueprintAssignable, Category = "GAS|Delegate", meta = (AllowPrivateAccess = "true"))
	FOnAttributeChangeSignature OnPlayerXPChanged;

	UPROPERTY(BlueprintAssignable, Category = "GAS|Delegate", meta = (AllowPrivateAccess = "true"))
	FOnPlayerStatChangedSignature OnPlayerLevelChanged;

	/** DataTable mapping GameplayTags to message text, widget class, and icon. Set in Blueprint. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Data", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UDataTable> MessageWidgetDataTable;
};
