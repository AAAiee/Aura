// @Copyright HaolunYuan


#include "Components/AbilitySystem/ModMagCalc/MMC_MaxHealth.h"
#include "Components/AbilitySystem/AuraAttributeSet.h"
#include "Interaction/CombatInterface.h"

UMMC_MaxHealth::UMMC_MaxHealth()
{
	// Capture the Vigor attribute from the target (the character whose max health we are calculating)
	VigorDef.AttributeToCapture = UAuraAttributeSet::GetVigorAttribute();
	VigorDef.AttributeSource = EGameplayEffectAttributeCaptureSource::Target;

	/* Snapshot: get the attribute at when the effect spec was created vs at applying time*/
	VigorDef.bSnapshot = false;

	RelevantAttributesToCapture.Add(VigorDef);
}

float UMMC_MaxHealth::CalculateBaseMagnitude_Implementation(const FGameplayEffectSpec& Spec) const
{
	//TODO:: These steps can be in Utili
	// Gather tags from source and target
	const FGameplayTagContainer* SourceTags = Spec.CapturedSourceTags.GetAggregatedTags();
	const FGameplayTagContainer* TargetTags = Spec.CapturedTargetTags.GetAggregatedTags();

	FAggregatorEvaluateParameters EvaluationParameters;
	EvaluationParameters.SourceTags = SourceTags;
	EvaluationParameters.TargetTags = TargetTags;

	// Get Vigor
	float VigorMag = 0.0f;
	GetCapturedAttributeMagnitude(VigorDef, Spec, EvaluationParameters, VigorMag); 
	VigorMag = FMath::Max<float>(VigorMag, 0.0f);

	//Get PlayerLevel
	float Level = 1.0f;
	UObject* SourceCharacterObject = Spec.GetContext().GetSourceObject();
	if (SourceCharacterObject && SourceCharacterObject->Implements<UCombatInterface>())
	{
		Level = ICombatInterface::Execute_GetPlayerLevel(SourceCharacterObject);
	}

	return 80 + 2.5 * VigorMag + 10 * Level;
}
