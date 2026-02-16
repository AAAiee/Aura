// @Copyright HaolunYuan

#pragma once

#include "CoreMinimal.h"
#include "UI/WidgetController/AuraWidgetController.h"
#include "AuraOverlayWidgetController.generated.h"

struct FOnAttributeChangeData;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnHealthChangedSignature, float, NewHealth);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnMaxHealthChangedSignature, float, NewMaxHealth);

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnManaChangedSignature, float, NewMana);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnMaxManaChangedSignature, float, NewMaxMana);


/**
 * 
 */
UCLASS(BlueprintType, Blueprintable)
class AURA_API UAuraOverlayWidgetController : public UAuraWidgetController
{
	GENERATED_BODY()

public:
	virtual void BroadcastInitialValues() override;
	virtual void BindAlldDependencies() override;

private:
	void HealthChanged(const FOnAttributeChangeData& ChangedData) const;
	void MaxHealthChanged(const FOnAttributeChangeData& ChangedData) const;
	void ManaChanged(const FOnAttributeChangeData& ChangedData) const;
	void MaxManaChanged(const FOnAttributeChangeData& ChangedData) const;

private:
	UPROPERTY(BlueprintAssignable, Category = "GAS|Attributes", meta = (AllowPrivateAccess = "true"))
	FOnHealthChangedSignature OnHealthChanged;

	UPROPERTY(BlueprintAssignable, Category = "GAS|Attributes", meta = (AllowPrivateAccess = "true"))
	FOnMaxHealthChangedSignature OnMaxHealthChanged;

	UPROPERTY(BlueprintAssignable, Category = "GAS|Attributes", meta = (AllowPrivateAccess = "true"))
	FOnManaChangedSignature OnManaChanged;

	UPROPERTY(BlueprintAssignable, Category = "GAS|Attributes", meta = (AllowPrivateAccess = "true"))
	FOnMaxManaChangedSignature OnMaxManaChanged;
};
