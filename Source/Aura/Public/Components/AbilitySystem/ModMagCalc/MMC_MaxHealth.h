// @Copyright HaolunYuan

#pragma once

#include "CoreMinimal.h"
#include "GameplayModMagnitudeCalculation.h"
#include "MMC_MaxHealth.generated.h"

/**
 * Custom magnitude calculation for maximum health. This class will be used to calculate the maximum health of a character based on character level and vigor
 */
UCLASS()
class AURA_API UMMC_MaxHealth : public UGameplayModMagnitudeCalculation
{
	GENERATED_BODY()

public:

	UMMC_MaxHealth();

	virtual float CalculateBaseMagnitude_Implementation(const FGameplayEffectSpec& Spec) const override;

private:
	// Capture definitions for the attributes we need to calculate max health: Vigor
	FGameplayEffectAttributeCaptureDefinition  VigorDef;
	
};
