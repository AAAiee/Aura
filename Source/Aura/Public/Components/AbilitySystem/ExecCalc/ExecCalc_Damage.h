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
};
