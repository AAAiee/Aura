// @Copyright HaolunYuan

#pragma once

#include "CoreMinimal.h"
#include "GameplayModMagnitudeCalculation.h"
#include "MMC_MaxHealth.generated.h"

/**
 * Calculates MaxHealth from the character's level and Vigor attribute.
 */
UCLASS()
class AURA_API UMMC_MaxHealth : public UGameplayModMagnitudeCalculation
{
	GENERATED_BODY()

public:
	UMMC_MaxHealth();

	virtual float CalculateBaseMagnitude_Implementation(const FGameplayEffectSpec& Spec) const override;

private:
	// Attribute capture used by the MMC when evaluating the target's Vigor.
	FGameplayEffectAttributeCaptureDefinition VigorDef;
};
