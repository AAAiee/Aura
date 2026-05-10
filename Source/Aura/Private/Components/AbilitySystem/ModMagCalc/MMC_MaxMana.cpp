// @Copyright HaolunYuan


#include "Components/AbilitySystem/ModMagCalc/MMC_MaxMana.h"

#include "Components/AbilitySystem/AuraAttributeSet.h"
#include "Interaction/CombatInterface.h"

UMMC_MaxMana::UMMC_MaxMana()
{
	// MaxMana scales from the target's Intelligence so runtime stat changes affect the result.
	IntelligenceDef.AttributeToCapture = UAuraAttributeSet::GetIntelligenceAttribute();
	IntelligenceDef.AttributeSource = EGameplayEffectAttributeCaptureSource::Target;

	// Evaluate at application time instead of snapshotting when the GE spec was created.
	IntelligenceDef.bSnapshot = false;

	RelevantAttributesToCapture.Add(IntelligenceDef);
}

float UMMC_MaxMana::CalculateBaseMagnitude_Implementation(const FGameplayEffectSpec& Spec) const
{
	// Aggregated tags let captured attributes respect conditional modifiers from source or target.
	const FGameplayTagContainer* SourceTags = Spec.CapturedSourceTags.GetAggregatedTags();
	const FGameplayTagContainer* TargetTags = Spec.CapturedTargetTags.GetAggregatedTags();

	FAggregatorEvaluateParameters EvaluationParameters;
	EvaluationParameters.SourceTags = SourceTags;
	EvaluationParameters.TargetTags = TargetTags;

	float IntelligenceMag = 0.0f;
	GetCapturedAttributeMagnitude(IntelligenceDef, Spec, EvaluationParameters, IntelligenceMag);
	IntelligenceMag = FMath::Max<float>(IntelligenceMag, 0.0f);

	// Level comes from the source object because this MMC is used by character-authored startup GEs.
	float Level = 1.0f;
	UObject* SourceCharacterObject = Spec.GetContext().GetSourceObject();
	if (SourceCharacterObject && SourceCharacterObject->Implements<UCombatInterface>())
	{
		Level = ICombatInterface::Execute_GetPlayerLevel(SourceCharacterObject);
	}

	return 50.f + 2.5f * IntelligenceMag + 15.f * Level;
}

