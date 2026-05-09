// @Copyright HaolunYuan


#include "Components/AbilitySystem/ModMagCalc/MMC_MaxMana.h"
#include "Components/AbilitySystem/AuraAttributeSet.h"
#include "Interaction/CombatInterface.h"

UMMC_MaxMana::UMMC_MaxMana()
{
	IntelligenceDef.AttributeToCapture = UAuraAttributeSet::GetIntelligenceAttribute();
	IntelligenceDef.AttributeSource = EGameplayEffectAttributeCaptureSource::Target;

	/* Snapshot: get the attribute at when the effect spec was created vs at applying time*/
	IntelligenceDef.bSnapshot = false;

	RelevantAttributesToCapture.Add(IntelligenceDef);
}
float UMMC_MaxMana::CalculateBaseMagnitude_Implementation(const FGameplayEffectSpec& Spec)  const
{
	//TODO:: These steps can be in Utili
// Gather tags from source and target
	const FGameplayTagContainer* SourceTags = Spec.CapturedSourceTags.GetAggregatedTags();
	const FGameplayTagContainer* TargetTags = Spec.CapturedTargetTags.GetAggregatedTags();

	FAggregatorEvaluateParameters EvaluationParameters;
	EvaluationParameters.SourceTags = SourceTags;
	EvaluationParameters.TargetTags = TargetTags;

	// Get Intelligence
	float IntelligenceMag = 0.0f;
	GetCapturedAttributeMagnitude(IntelligenceDef, Spec, EvaluationParameters, IntelligenceMag);
	IntelligenceMag = FMath::Max<float>(IntelligenceMag, 0.0f);

	//Get PlayerLevel
	float Level = 1.0f;
	UObject* SourceCharacterObject = Spec.GetContext().GetSourceObject();
	if (SourceCharacterObject && SourceCharacterObject->Implements<UCombatInterface>())
	{
		Level = ICombatInterface::Execute_GetPlayerLevel(SourceCharacterObject);
	}

	return 50.f + 2.5f * IntelligenceMag + 15.f * Level;
}

