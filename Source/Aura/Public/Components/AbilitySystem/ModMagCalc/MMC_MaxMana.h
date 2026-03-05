// @Copyright HaolunYuan

#pragma once

#include "CoreMinimal.h"
#include "GameplayModMagnitudeCalculation.h"
#include "MMC_MaxMana.generated.h"

/**
 * a Custom magnitude calculation for maximum mana. This class will be used to calculate the maximum mana of a character based on character level and intelligence
 */
UCLASS()
class AURA_API UMMC_MaxMana : public UGameplayModMagnitudeCalculation
{
	GENERATED_BODY()


public:
	UMMC_MaxMana();

	/* Override the base magnitude calculation to calculate max mana based on character level and intelligence */
	virtual float CalculateBaseMagnitude_Implementation(const FGameplayEffectSpec& Spec) const override;
private:
	// Capture definitions for the attributes we need to calculate max mana: Intelligence
	FGameplayEffectAttributeCaptureDefinition  IntelligenceDef;
};
