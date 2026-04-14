// @Copyright HaolunYuan


#include "Components/AbilitySystem/ExecCalc/ExecCalc_Damage.h"
#include "AbilitySystemComponent.h"
#include "Components/AbilitySystem/AuraAttributeSet.h"
#include "AuraGameTagManager.h"
#include "Components/AbilitySystem/AuraAbilitySystemLibrary.h"
#include "Interaction/CombatInterface.h"

// Centralizes every captured attribute this execution needs so the constructor and Execute()
// stay in sync on which combat stats participate in damage resolution.
struct AuraDamageStatics
{

	DECLARE_ATTRIBUTE_CAPTUREDEF(Armor);
	DECLARE_ATTRIBUTE_CAPTUREDEF(ArmorPenetration);
	DECLARE_ATTRIBUTE_CAPTUREDEF(BlockChance);
	DECLARE_ATTRIBUTE_CAPTUREDEF(CriticalHitChance);
	DECLARE_ATTRIBUTE_CAPTUREDEF(CriticalHitResilience);
	DECLARE_ATTRIBUTE_CAPTUREDEF(CriticalHitDamage);

	AuraDamageStatics()
	{
		DEFINE_ATTRIBUTE_CAPTUREDEF(UAuraAttributeSet, Armor, Target, false);
		DEFINE_ATTRIBUTE_CAPTUREDEF(UAuraAttributeSet, BlockChance, Target, false);
		DEFINE_ATTRIBUTE_CAPTUREDEF(UAuraAttributeSet, CriticalHitResilience, Target, false);

		DEFINE_ATTRIBUTE_CAPTUREDEF(UAuraAttributeSet, ArmorPenetration, Source, true);
		DEFINE_ATTRIBUTE_CAPTUREDEF(UAuraAttributeSet, CriticalHitChance, Source, true);
		DEFINE_ATTRIBUTE_CAPTUREDEF(UAuraAttributeSet, CriticalHitDamage, Source, true);
	}


};

static const AuraDamageStatics& GetAuraDamageStatics()
{
	static AuraDamageStatics DStatics;
	return DStatics;
}




UExecCalc_Damage::UExecCalc_Damage()
{
	RelevantAttributesToCapture.Add(GetAuraDamageStatics().ArmorDef);
	RelevantAttributesToCapture.Add(GetAuraDamageStatics().BlockChanceDef);
	RelevantAttributesToCapture.Add(GetAuraDamageStatics().ArmorPenetrationDef);
	RelevantAttributesToCapture.Add(GetAuraDamageStatics().CriticalHitChanceDef);
	RelevantAttributesToCapture.Add(GetAuraDamageStatics().CriticalHitResilienceDef);
	RelevantAttributesToCapture.Add(GetAuraDamageStatics().CriticalHitDamageDef);
}



void UExecCalc_Damage::Execute_Implementation(const FGameplayEffectCustomExecutionParameters& ExecutionParams, FGameplayEffectCustomExecutionOutput& OutExecutionOutput) const
{
	const UAbilitySystemComponent* TargetAbilityComponent = ExecutionParams.GetTargetAbilitySystemComponent();
	const UAbilitySystemComponent* SourceAbilityComponent = ExecutionParams.GetSourceAbilitySystemComponent();

	const AActor* SourceAvatar = SourceAbilityComponent ? SourceAbilityComponent->GetAvatarActor() : nullptr;
	const AActor* TargetAvatar = TargetAbilityComponent ? TargetAbilityComponent->GetAvatarActor() : nullptr;

	const ICombatInterface* SourceCombatInterface = Cast<ICombatInterface>(SourceAvatar);
	const ICombatInterface* TargetCombatInterface = Cast<ICombatInterface>(TargetAvatar);


	const FGameplayEffectSpec& Spec = ExecutionParams.GetOwningSpec();

	const FGameplayTagContainer& SourceTag = *Spec.CapturedSourceTags.GetAggregatedTags();
	const FGameplayTagContainer& TargetTag = *Spec.CapturedTargetTags.GetAggregatedTags();

	FAggregatorEvaluateParameters EvalPrams;
	EvalPrams.SourceTags = &SourceTag;
	EvalPrams.TargetTags = &TargetTag;

	UCharacterClassInfo* ClassInfo = UAuraAbilitySystemLibrary::GetCharacterClassInfo(SourceAvatar);
	auto GetClampedAttributeMagnitude = [&EvalPrams, &ExecutionParams](const FGameplayEffectAttributeCaptureDefinition& Def) -> float
		{
			// Every captured attribute is clamped to a sane non-negative range before we use it in
			// combat math so authored data cannot accidentally flip damage behavior.
			float Value = 0.f;
			ExecutionParams.AttemptCalculateCapturedAttributeMagnitude(Def, EvalPrams, Value);
			Value = FMath::Max<float>(0.f, Value);
			return Value;
		};


	/*
	 * Damage resolution order:
	 *   1. Read the base damage authored by the ability via set-by-caller.
	 *   2. Apply crit logic.
	 *   3. Apply block mitigation.
	 *   4. Apply armor / armor-penetration scaling.
	 *   5. Output the final amount into IncomingDamage for the AttributeSet to consume.
	 */
	float Damage = Spec.GetSetByCallerMagnitude(FAuraGameTagManager::Get().Combat_Damage);

	//Critical hit chance
	const float SourceCriticalHitChance = GetClampedAttributeMagnitude(GetAuraDamageStatics().CriticalHitChanceDef);
	const float TargetCritcalHitResistence = GetClampedAttributeMagnitude(GetAuraDamageStatics().CriticalHitResilienceDef);
	const float SourceCriticalDamage = GetClampedAttributeMagnitude(GetAuraDamageStatics().CriticalHitDamageDef);
	// Curve tables let combat scaling evolve with character level without hard-coding balancing
	// constants directly into the execution logic.
	const FRealCurve* HitResilienceCurve = ClassInfo->DefaultCalculationCoeffcient->FindCurve("CriticalHitResilience", FString());
	const float HitResilienceScalingFactor = HitResilienceCurve->Eval(TargetCombatInterface->GetPlayerLevel());

	const float AdjustedCritcalHitChance = SourceCriticalHitChance - TargetCritcalHitResistence * HitResilienceScalingFactor;
	bool bCriticalHit = FMath::FRandRange(1.0f, 100.f) < AdjustedCritcalHitChance;
	if (bCriticalHit)
	{
		Damage = Damage * 2.0f + SourceCriticalDamage;
	}

	// Capture BlockChance on target, and determine if there was a successful block
	float TargetBlockChance = GetClampedAttributeMagnitude(GetAuraDamageStatics().BlockChanceDef);
	// If block, halve the damage
	const bool bBlocked = FMath::FRandRange(1.0f, 100.f) < TargetBlockChance;
	if (bBlocked)
	{
		Damage *= 0.5f;
	}

	//Armor penetration reduces the effective armor of the target, which reduces the damage reduction provided by armor.
	float TargetArmor = 0.f;
	ExecutionParams.AttemptCalculateCapturedAttributeMagnitude(GetAuraDamageStatics().ArmorDef, EvalPrams, TargetArmor);
	TargetArmor = FMath::Max<float>(0.f, TargetArmor);

	float SourceArmorPenetration = 0.f;
	ExecutionParams.AttemptCalculateCapturedAttributeMagnitude(GetAuraDamageStatics().ArmorPenetrationDef, EvalPrams, SourceArmorPenetration);
	SourceArmorPenetration = FMath::Max<float>(0.f, SourceArmorPenetration);

	const FRealCurve* ArmorPenetrationCurve = ClassInfo->DefaultCalculationCoeffcient->FindCurve("ArmorPenetration", FString());
	const float ArmorPenetrationScalingValue = ArmorPenetrationCurve->Eval(SourceCombatInterface->GetPlayerLevel());

	const FRealCurve* ArmorCurve = ClassInfo->DefaultCalculationCoeffcient->FindCurve("EffectiveArmor", FString());
	const float ArmorScalingValue = ArmorCurve->Eval(TargetCombatInterface->GetPlayerLevel());

	const float EffectiveArmor = TargetArmor * (100 - SourceArmorPenetration * ArmorPenetrationScalingValue) / 100.0f;
	Damage *= (100.f - EffectiveArmor * ArmorScalingValue) / 100.f;

	// ExecCalcs do not write Health directly. They output into a meta attribute so the AttributeSet
	// can centralize the "what happens when damage lands?" flow in one place.
	const FGameplayModifierEvaluatedData EvaluateData(UAuraAttributeSet::GetIncomingDamageAttribute(), EGameplayModOp::Additive, Damage);
	OutExecutionOutput.AddOutputModifier(EvaluateData);
}
