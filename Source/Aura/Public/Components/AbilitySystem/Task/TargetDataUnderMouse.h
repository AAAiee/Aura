// @Copyright HaolunYuan

#pragma once

#include "CoreMinimal.h"
#include "Abilities/Tasks/AbilityTask.h"
#include "TargetDataUnderMouse.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnValidateDataSingnature, const FGameplayAbilityTargetDataHandle&, Data);


/**
 * Ability task that captures the player's cursor hit result and packages it as target data.
 *
 * Why use a task instead of a plain helper:
 *   - tasks fit naturally into GAS activation graphs
 *   - they handle prediction / replicated callbacks for us
 *   - Blueprint abilities can wait for the data through a familiar delegate output pin
 */
UCLASS()
class AURA_API UTargetDataUnderMouse : public UAbilityTask
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable, Category = "Ability|Tasks", meta = (DisplayName = "Get Target Data Under Mouse", HidePin = "OwningAbility", DefaultToSelf = "OwningAbility",BlueprintInternalUseOnly = true))
	static UTargetDataUnderMouse* GetTargetDataUnderMouse(UGameplayAbility* OwningAbility);

	// Fires once the task has built or received the target data payload for the current activation.
	UPROPERTY(BlueprintAssignable)
	FOnValidateDataSingnature OnValidateData;

protected:
	// Entry point invoked by GAS when the task is activated inside the ability flow.
	virtual void Activate() override;

	void OnTargetDataReplicatedCallback(const FGameplayAbilityTargetDataHandle& DataHandle, FGameplayTag GameplayTag);

private:
	// Local-player helper that samples the cursor hit and sends it to the server using the
	// current prediction window / activation keys.
	void SendMouseCursorData(); 
};
