// @Copyright HaolunYuan

#pragma once

#include "CoreMinimal.h"
#include "UI/WidgetController/AuraWidgetController.h"
#include "Engine/DataTable.h"
#include "GameplayTagContainer.h"
#include "AuraOverlayWidgetController.generated.h"

struct FOnAttributeChangeData;
class UAuraUserWidget;

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
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnHealthChangedSignature, float, NewHealth);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnMaxHealthChangedSignature, float, NewMaxHealth);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnManaChangedSignature, float, NewMana);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnMaxManaChangedSignature, float, NewMaxMana);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnMessageWidgetRowSignature, FUIWidgetRow, Row);


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
	/*Attribute Change Callbacks ¡ª receive FOnAttributeChangeData from the ASC and relay to UI delegates*/
	void HealthChanged(const FOnAttributeChangeData& ChangedData) const;
	void MaxHealthChanged(const FOnAttributeChangeData& ChangedData) const;
	void ManaChanged(const FOnAttributeChangeData& ChangedData) const;
	void MaxManaChanged(const FOnAttributeChangeData& ChangedData) const;

private:
	/*Blueprint-Assignable Delegates ¡ª Blueprint widgets bind to these in WidgetControllerSet*/
	UPROPERTY(BlueprintAssignable, Category = "GAS|Attributes", meta = (AllowPrivateAccess = "true"))
	FOnHealthChangedSignature OnHealthChanged;

	UPROPERTY(BlueprintAssignable, Category = "GAS|Attributes", meta = (AllowPrivateAccess = "true"))
	FOnMaxHealthChangedSignature OnMaxHealthChanged;

	UPROPERTY(BlueprintAssignable, Category = "GAS|Attributes", meta = (AllowPrivateAccess = "true"))
	FOnManaChangedSignature OnManaChanged;

	UPROPERTY(BlueprintAssignable, Category = "GAS|Attributes", meta = (AllowPrivateAccess = "true"))
	FOnMaxManaChangedSignature OnMaxManaChanged;

	/** Fires when a GE with a "Message.*" tag is applied ¡ª Blueprint shows a popup widget. */
	UPROPERTY(BlueprintAssignable, Category = "GAS|Message", meta = (AllowPrivateAccess = "true"))
	FOnMessageWidgetRowSignature OnSendMessageWidgetRow;

	/** DataTable mapping GameplayTags to message text, widget class, and icon. Set in Blueprint. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Widget Data", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UDataTable> MessageWidgetDataTable;

};
