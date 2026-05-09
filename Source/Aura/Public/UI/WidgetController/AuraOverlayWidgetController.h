// @Copyright HaolunYuan

#pragma once

#include "CoreMinimal.h"
#include "UI/WidgetController/AuraWidgetController.h"
#include "ControllerDelegates.h"
#include "Engine/DataTable.h"
#include "GameplayTagContainer.h"
#include "AuraOverlayWidgetController.generated.h"

struct FOnAttributeChangeData;
class UAuraUserWidget;
class UAbilityInfo;
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
	UPROPERTY(EditAnywhere,  BlueprintReadOnly)
	FGameplayTag MessageTag = FGameplayTag();

	UPROPERTY(EditAnywhere,  BlueprintReadOnly)
	FText Message = FText();

	/** Optional: a specific widget class to display for this message. */
	UPROPERTY(EditAnywhere,  BlueprintReadOnly)
	TSubclassOf<UAuraUserWidget> MessageWidgetClass = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	UTexture2D* Image = nullptr;
};

/*Dynamic Multicast Delegates ¡ª Blueprint widgets bind to these to receive attribute updates*/
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnMessageWidgetRowSignature, const FUIWidgetRow&, Row);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FAbilityInfoSignature, const FAuraAbilityInfo&, AbilityInfo);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnPlayerStateStatChanged, const float, NewStatValue);


/**
 * Helper template ¡ª looks up a DataTable row by GameplayTag name.
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
 *      ASC attribute change delegate ¡ú C++ callback ¡ú Dynamic delegate ¡ú Blueprint widget
 *
 *   2. Effect Messages (e.g., "Health Potion Collected"):
 *      GE applied ¡ú ASC fires OnGatherEffectAssetTags ¡ú lambda filters for "Message.*" tags
 *      ¡ú DataTable lookup ¡ú OnSendMessageWidgetRow ¡ú Blueprint widget
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
	void OnInitializeStartupAbilities(UAuraAbilitySystemComponent* AuraASC);
	void OnXpChanged(const int32 NewXP) const ;
	void OnLevelChanged(const int32 NewLevel) const;


	/*Blueprint-Assignable Delegates ¡ª Blueprint widgets bind to these in WidgetControllerSet*/
	UPROPERTY(BlueprintAssignable, Category = "GAS|Delegate", meta = (AllowPrivateAccess = "true"))
	FOnAttributeChangeSignature OnHealthChanged;

	UPROPERTY(BlueprintAssignable, Category = "GAS|Delegate", meta = (AllowPrivateAccess = "true"))
	FOnAttributeChangeSignature OnMaxHealthChanged;

	UPROPERTY(BlueprintAssignable, Category = "GAS|Delegate", meta = (AllowPrivateAccess = "true"))
	FOnAttributeChangeSignature OnManaChanged;

	UPROPERTY(BlueprintAssignable, Category = "GAS|Delegate", meta = (AllowPrivateAccess = "true"))
	FOnAttributeChangeSignature OnMaxManaChanged;

	/** Fires when a GE with a "Message.*" tag is applied ¡ª Blueprint shows a popup widget. */
	UPROPERTY(BlueprintAssignable, Category = "GAS|Delegate", meta = (AllowPrivateAccess = "true"))
	FOnMessageWidgetRowSignature OnSendMessageWidgetRow;

	UPROPERTY(BlueprintAssignable, Category = "GAS|Delegate", meta = (AllowPrivateAccess = "true"))
	FAbilityInfoSignature AbilityInfoDelegate;

	/* this might not be attributes, but they also broadcast a float, that's why it just uses the existing signature*/
	UPROPERTY(BlueprintAssignable, Category = "GAS|Delegate", meta = (AllowPrivateAccess = "true"))
	FOnAttributeChangeSignature OnPlayerXPChanged;

	UPROPERTY(BlueprintAssignable, Category = "GAS|Delegate", meta = (AllowPrivateAccess = "true"))
	FOnPlayerStatChangedSignature OnPlayerLevelChanged;

	/** DataTable mapping GameplayTags to message text, widget class, and icon. Set in Blueprint. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Data", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UDataTable> MessageWidgetDataTable;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Data", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UAbilityInfo> AbilityInfoDataAsset;



};
