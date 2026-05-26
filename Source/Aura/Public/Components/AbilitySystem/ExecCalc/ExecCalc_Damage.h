// @Copyright HaolunYuan

#pragma once

#include "CoreMinimal.h"
#include "GameplayEffectExecutionCalculation.h"
#include "ExecCalc_Damage.generated.h"

/**
 * Server-side damage execution that turns typed set-by-caller damage plus captured combat
 * attributes into the final IncomingDamage meta-attribute consumed by UAuraAttributeSet.
 */
UCLASS()
class AURA_API UExecCalc_Damage : public UGameplayEffectExecutionCalculation
{
	GENERATED_BODY()

public:
	UExecCalc_Damage();

	/* UGameplayEffectExecutionCalculation */
	virtual void Execute_Implementation(const FGameplayEffectCustomExecutionParameters& ExecutionParams, FGameplayEffectCustomExecutionOutput& OutExecutionOutput) const override;

private:
	/**
	 * Reads debuff set-by-caller data from the owning damage spec, applies target resistance to
	 * the success chance, and writes the result metadata into FAuraGameplayEffectContext.
	 */
	void DetermineDebuff(
		const FGameplayEffectCustomExecutionParameters& ExecutionParams,
		const FGameplayEffectSpec& Spec,
		const FAggregatorEvaluateParameters& EvalParams) const;
};
