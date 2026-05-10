// @Copyright HaolunYuan

#pragma once

#include "CoreMinimal.h"
#include "GameplayModMagnitudeCalculation.h"
#include "MMC_MaxMana.generated.h"

/**
 * Calculates MaxMana from the character's level and Intelligence attribute.
 */
UCLASS()
class AURA_API UMMC_MaxMana : public UGameplayModMagnitudeCalculation
{
	GENERATED_BODY()

public:
	UMMC_MaxMana();

	virtual float CalculateBaseMagnitude_Implementation(const FGameplayEffectSpec& Spec) const override;

private:
	// Attribute capture used by the MMC when evaluating the target's Intelligence.
	FGameplayEffectAttributeCaptureDefinition IntelligenceDef;
};
