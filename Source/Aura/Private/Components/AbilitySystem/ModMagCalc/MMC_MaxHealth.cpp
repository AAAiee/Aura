// @Copyright HaolunYuan


#include "Components/AbilitySystem/ModMagCalc/MMC_MaxHealth.h"

#include "Components/AbilitySystem/AuraAttributeSet.h"
#include "Interaction/CombatInterface.h"

UMMC_MaxHealth::UMMC_MaxHealth()
{
	// MaxHealth scales from the target's Vigor so buffs/debuffs on the owning character are reflected.
	VigorDef.AttributeToCapture = UAuraAttributeSet::GetVigorAttribute();
	VigorDef.AttributeSource = EGameplayEffectAttributeCaptureSource::Target;

	// Evaluate at application time instead of snapshotting when the GE spec was created.
	VigorDef.bSnapshot = false;

	RelevantAttributesToCapture.Add(VigorDef);
}

float UMMC_MaxHealth::CalculateBaseMagnitude_Implementation(const FGameplayEffectSpec& Spec) const
{
	// Aggregated tags let captured attributes respect conditional modifiers from source or target.
	const FGameplayTagContainer* SourceTags = Spec.CapturedSourceTags.GetAggregatedTags();
	const FGameplayTagContainer* TargetTags = Spec.CapturedTargetTags.GetAggregatedTags();

	FAggregatorEvaluateParameters EvaluationParameters;
	EvaluationParameters.SourceTags = SourceTags;
	EvaluationParameters.TargetTags = TargetTags;

	float VigorMag = 0.0f;
	GetCapturedAttributeMagnitude(VigorDef, Spec, EvaluationParameters, VigorMag);
	VigorMag = FMath::Max<float>(VigorMag, 0.0f);

	// Level comes from the source object because this MMC is used by character-authored startup GEs.
	float Level = 1.0f;
	UObject* SourceCharacterObject = Spec.GetContext().GetSourceObject();
	if (SourceCharacterObject && SourceCharacterObject->Implements<UCombatInterface>())
	{
		Level = ICombatInterface::Execute_GetPlayerLevel(SourceCharacterObject);
	}

	return 80 + 2.5 * VigorMag + 10 * Level;
}
