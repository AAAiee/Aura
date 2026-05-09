// @Copyright HaolunYuan

#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintAsyncActionBase.h"
#include "GameplayTagContainer.h"
#include "ActiveGameplayEffectHandle.h"
#include "WaitCooldownChange.generated.h"

class UAbilitySystemComponent;
struct FGameplayEffectSpec;


DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FWaitCooldownChangeSignature, float, CooldownRemaining);

/**
 * 
 */
UCLASS(BlueprintType, meta = (ExposedAsyncProxy = "AsyncTask"))
class AURA_API UWaitCooldownChange : public UBlueprintAsyncActionBase
{
	GENERATED_BODY()

public:

	UPROPERTY(BlueprintAssignable)
	FWaitCooldownChangeSignature CooldownStart;


	UPROPERTY(BlueprintAssignable)
	FWaitCooldownChangeSignature CooldownEnd;


	UFUNCTION(BlueprintCallable, meta = (BlueprintInternalUseOnly = "true"))
	static UWaitCooldownChange* WaitCooldownChange(UAbilitySystemComponent* InAbilitySystemComponent, const FGameplayTag& InCooldownTag);

	UFUNCTION(BlueprintCallable)
	void EndTask();

protected:
	void CooldownTagChanged(const FGameplayTag CallbackTag, int32 NewCount);
	void OnActiveGameplayEffectAdded(UAbilitySystemComponent* TargetASC, const FGameplayEffectSpec& SpecApplied, FActiveGameplayEffectHandle ActiveHandle);


	UPROPERTY()
	TObjectPtr<UAbilitySystemComponent> ASC;

	FGameplayTag CooldownTag;


};
