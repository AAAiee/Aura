// @Copyright HaolunYuan

#pragma once

#include "CoreMinimal.h"
#include "UI/WidgetController/AuraWidgetController.h"
#include "AuraOverlayWidgetController.generated.h"

struct FOnAttributeChangeData;

/*Delegate Declarations ¡ª used by Widgets to listen for attribute value changes*/
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnHealthChangedSignature, float, NewHealth);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnMaxHealthChangedSignature, float, NewMaxHealth);

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnManaChangedSignature, float, NewMana);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnMaxManaChangedSignature, float, NewMaxMana);


/**
 * Overlay Widget Controller is responsible for broadcasting Health and Mana attribute data to the
 * Overlay UI. It binds to the Ability System Component's attribute change delegates so that widgets
 * are notified in real time whenever these values change.
 */
UCLASS(BlueprintType, Blueprintable)
class AURA_API UAuraOverlayWidgetController : public UAuraWidgetController
{
	GENERATED_BODY()

public:
	/** Reads current Health, MaxHealth, Mana, and MaxMana from the AttributeSet and broadcasts them. */
	virtual void BroadcastInitialValues() override;

	/** Binds to Health, MaxHealth, Mana, and MaxMana attribute change delegates on the ASC. */
	virtual void BindAlldDependencies() override;

private:
	/*Attribute Change Callbacks Begins*/
	/** Called when the Health attribute changes. Broadcasts the new value to the UI. */
	void HealthChanged(const FOnAttributeChangeData& ChangedData) const;

	/** Called when the MaxHealth attribute changes. Broadcasts the new value to the UI. */
	void MaxHealthChanged(const FOnAttributeChangeData& ChangedData) const;

	/** Called when the Mana attribute changes. Broadcasts the new value to the UI. */
	void ManaChanged(const FOnAttributeChangeData& ChangedData) const;

	/** Called when the MaxMana attribute changes. Broadcasts the new value to the UI. */
	void MaxManaChanged(const FOnAttributeChangeData& ChangedData) const;
	/*Attribute Change Callbacks Ends*/

private:
	/*Blueprint-Assignable Delegates Begins ¡ª Widgets bind to these in Blueprint*/
	UPROPERTY(BlueprintAssignable, Category = "GAS|Attributes", meta = (AllowPrivateAccess = "true"))
	FOnHealthChangedSignature OnHealthChanged;

	UPROPERTY(BlueprintAssignable, Category = "GAS|Attributes", meta = (AllowPrivateAccess = "true"))
	FOnMaxHealthChangedSignature OnMaxHealthChanged;

	UPROPERTY(BlueprintAssignable, Category = "GAS|Attributes", meta = (AllowPrivateAccess = "true"))
	FOnManaChangedSignature OnManaChanged;

	UPROPERTY(BlueprintAssignable, Category = "GAS|Attributes", meta = (AllowPrivateAccess = "true"))
	FOnMaxManaChangedSignature OnMaxManaChanged;
	/*Blueprint-Assignable Delegates Ends*/
};
