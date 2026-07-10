// @Copyright HaolunYuan

#include "Components/AbilitySystem/ExecCalc/ExecCalc_Damage.h"

#include "AbilitySystemComponent.h"
#include "AuraGameTagManager.h"
#include "Components/AbilitySystem/AuraAbilitySystemLibrary.h"
#include "Components/AbilitySystem/AuraAttributeSet.h"
#include "Engine/CurveTable.h"
#include "Interaction/CombatInterface.h"

// Centralizes every captured attribute this execution needs so the constructor and Execute()
// stay in sync on which combat stats participate in damage resolution.
struct AuraDamageStatics;

static const AuraDamageStatics& GetAuraDamageStatics();

struct AuraDamageStatics
{
	/* Target mitigation captures */
	DECLARE_ATTRIBUTE_CAPTUREDEF(Armor);
	DECLARE_ATTRIBUTE_CAPTUREDEF(BlockChance);
	DECLARE_ATTRIBUTE_CAPTUREDEF(CriticalHitResilience);
	DECLARE_ATTRIBUTE_CAPTUREDEF(FireResistance);
	DECLARE_ATTRIBUTE_CAPTUREDEF(LightningResistance);
	DECLARE_ATTRIBUTE_CAPTUREDEF(ArcaneResistance);
	DECLARE_ATTRIBUTE_CAPTUREDEF(PhysicalResistance);

	/* Source offense captures */
	DECLARE_ATTRIBUTE_CAPTUREDEF(ArmorPenetration);
	DECLARE_ATTRIBUTE_CAPTUREDEF(CriticalHitChance);
	DECLARE_ATTRIBUTE_CAPTUREDEF(CriticalHitDamage);

	const TMap<FGameplayTag, FGameplayEffectAttributeCaptureDefinition>& GetTagsToCaptureDefs() const
	{
		static TMap<FGameplayTag, FGameplayEffectAttributeCaptureDefinition> TagsToCaptureDef;
		if (TagsToCaptureDef.IsEmpty())
		{
			TagsToCaptureDef.Add(FAuraGameTagManager::Get().Attribute_Secondary_Armor, ArmorDef);
			TagsToCaptureDef.Add(FAuraGameTagManager::Get().Attribute_Secondary_BlockChance, BlockChanceDef);
			TagsToCaptureDef.Add(FAuraGameTagManager::Get().Attribute_Secondary_CriticalHitResistance, CriticalHitResilienceDef);

			TagsToCaptureDef.Add(FAuraGameTagManager::Get().Attribute_Secondary_FireDamageResistance, FireResistanceDef);
			TagsToCaptureDef.Add(FAuraGameTagManager::Get().Attribute_Secondary_LightningDamageResistance, LightningResistanceDef);
			TagsToCaptureDef.Add(FAuraGameTagManager::Get().Attribute_Secondary_ArcaneDamageResistance, ArcaneResistanceDef);
			TagsToCaptureDef.Add(FAuraGameTagManager::Get().Attribute_Secondary_PhysicalDamageResistance, PhysicalResistanceDef);

			TagsToCaptureDef.Add(FAuraGameTagManager::Get().Attribute_Secondary_ArmorPenetration, ArmorPenetrationDef);
			TagsToCaptureDef.Add(FAuraGameTagManager::Get().Attribute_Secondary_CriticalHitChance, CriticalHitChanceDef);
			TagsToCaptureDef.Add(FAuraGameTagManager::Get().Attribute_Secondary_CriticalHitDamage, CriticalHitDamageDef);
		}

		return TagsToCaptureDef;
	}

private:
	AuraDamageStatics()
	{
		DEFINE_ATTRIBUTE_CAPTUREDEF(UAuraAttributeSet, Armor, Target, false);
		DEFINE_ATTRIBUTE_CAPTUREDEF(UAuraAttributeSet, BlockChance, Target, false);
		DEFINE_ATTRIBUTE_CAPTUREDEF(UAuraAttributeSet, CriticalHitResilience, Target, false);

		DEFINE_ATTRIBUTE_CAPTUREDEF(UAuraAttributeSet, FireResistance, Target, false);
		DEFINE_ATTRIBUTE_CAPTUREDEF(UAuraAttributeSet, LightningResistance, Target, false);
		DEFINE_ATTRIBUTE_CAPTUREDEF(UAuraAttributeSet, ArcaneResistance, Target, false);
		DEFINE_ATTRIBUTE_CAPTUREDEF(UAuraAttributeSet, PhysicalResistance, Target, false);

		DEFINE_ATTRIBUTE_CAPTUREDEF(UAuraAttributeSet, ArmorPenetration, Source, true);
		DEFINE_ATTRIBUTE_CAPTUREDEF(UAuraAttributeSet, CriticalHitChance, Source, true);
		DEFINE_ATTRIBUTE_CAPTUREDEF(UAuraAttributeSet, CriticalHitDamage, Source, true);
	}

	friend const AuraDamageStatics& GetAuraDamageStatics();
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
	RelevantAttributesToCapture.Add(GetAuraDamageStatics().CriticalHitResilienceDef);

	RelevantAttributesToCapture.Add(GetAuraDamageStatics().FireResistanceDef);
	RelevantAttributesToCapture.Add(GetAuraDamageStatics().LightningResistanceDef);
	RelevantAttributesToCapture.Add(GetAuraDamageStatics().ArcaneResistanceDef);
	RelevantAttributesToCapture.Add(GetAuraDamageStatics().PhysicalResistanceDef);

	RelevantAttributesToCapture.Add(GetAuraDamageStatics().ArmorPenetrationDef);
	RelevantAttributesToCapture.Add(GetAuraDamageStatics().CriticalHitChanceDef);
	RelevantAttributesToCapture.Add(GetAuraDamageStatics().CriticalHitDamageDef);
}

void UExecCalc_Damage::DetermineDebuff(
	const FGameplayEffectCustomExecutionParameters& ExecutionParams,
	const FGameplayEffectSpec& Spec,
	const FAggregatorEvaluateParameters& EvalParams) const
{
	/*
	 * Debuff resolution happens beside damage calculation because it needs the same captured
	 * target resistance attributes and the same authored set-by-caller payload.
	 *
	 * The ExecCalc does not apply the timed debuff itself. It records the result on the custom
	 * GameplayEffectContext, and UAuraAttributeSet creates the dynamic periodic effect only after
	 * IncomingDamage is consumed. That keeps all "what happens when a hit lands?" work in one place.
	 */
	const FAuraGameTagManager& TagManager = FAuraGameTagManager::Get();
	for (const TTuple<FGameplayTag, FGameplayTag>& Pair : TagManager.DamageTypeToDebuffType)
	{
		const FGameplayTag& DamageTypeTag = Pair.Key;
		const float TypeDamage = Spec.GetSetByCallerMagnitude(DamageTypeTag, false, -1.0f);
		if (TypeDamage > -1.0f)
		{
			const float SourceDebuffSuccessChance = Spec.GetSetByCallerMagnitude(TagManager.Debuff_Chance, false, -1.f);
			float TargetDebuffResistance = 0.f;
			const FGameplayTag& TargetResistanceTag = TagManager.DamageTypesToResistance[DamageTypeTag];
			ExecutionParams.AttemptCalculateCapturedAttributeMagnitude(GetAuraDamageStatics().GetTagsToCaptureDefs()[TargetResistanceTag], EvalParams, TargetDebuffResistance);
			TargetDebuffResistance = FMath::Clamp(TargetDebuffResistance, 0.f, 100.f);

			// Resistance reduces the authored debuff chance by percentage. A target with 25 fire
			// resistance only faces 75% of a fire ability's authored burn chance.
			const float EffectiveDebuffChance = SourceDebuffSuccessChance * (100.f - TargetDebuffResistance) / 100.f;
			const bool bDebuffSuccess = FMath::FRandRange(1.0f, 100.f) < EffectiveDebuffChance;
			if (bDebuffSuccess)
			{
				FGameplayEffectContextHandle ContextHandle = Spec.GetContext();
				UAuraAbilitySystemLibrary::SetSuccessfulDebuff(ContextHandle, bDebuffSuccess);
				UAuraAbilitySystemLibrary::SetDamageTypeTag(ContextHandle, DamageTypeTag);
				
				const float DebuffDamage = Spec.GetSetByCallerMagnitude(TagManager.Debuff_Damage, false, -1.f);
				UAuraAbilitySystemLibrary::SetDebuffDamage(ContextHandle, DebuffDamage);
				const float DebuffDuration = Spec.GetSetByCallerMagnitude(TagManager.Debuff_Duration, false, -1.f);
				const float DebuffFrequency = Spec.GetSetByCallerMagnitude(TagManager.Debuff_Frequency, false, -1.f);
				UAuraAbilitySystemLibrary::SetDebuffDuration(ContextHandle, DebuffDuration);
				UAuraAbilitySystemLibrary::SetDebuffFrequency(ContextHandle, DebuffFrequency);
			}
		}
	}
}

void UExecCalc_Damage::Execute_Implementation(const FGameplayEffectCustomExecutionParameters& ExecutionParams, FGameplayEffectCustomExecutionOutput& OutExecutionOutput) const
{
	const UAbilitySystemComponent* TargetAbilityComponent = ExecutionParams.GetTargetAbilitySystemComponent();
	const UAbilitySystemComponent* SourceAbilityComponent = ExecutionParams.GetSourceAbilitySystemComponent();

	AActor* SourceAvatar = SourceAbilityComponent ? SourceAbilityComponent->GetAvatarActor() : nullptr;
	AActor* TargetAvatar = TargetAbilityComponent ? TargetAbilityComponent->GetAvatarActor() : nullptr;

	int32 SourcePlayerLevel = 1;
	if (SourceAvatar && SourceAvatar->Implements<UCombatInterface>())
	{
		SourcePlayerLevel = ICombatInterface::Execute_GetPlayerLevel(SourceAvatar);
	}

	int32 TargetPlayerLevel = 1;
	if (TargetAvatar && TargetAvatar->Implements<UCombatInterface>())
	{
		TargetPlayerLevel = ICombatInterface::Execute_GetPlayerLevel(TargetAvatar);
	}

	const UCharacterClassInfo* ClassInfo = UAuraAbilitySystemLibrary::GetCharacterClassInfo(SourceAvatar);
	if (!ensureMsgf(ClassInfo && ClassInfo->DefaultCalculationCoeffcient, TEXT("ExecCalc_Damage requires CharacterClassInfo with a calculation coefficient curve table.")))
	{
		return;
	}

	const FGameplayEffectSpec& Spec = ExecutionParams.GetOwningSpec();
	FGameplayEffectContextHandle ContextHandle = Spec.GetContext();

	const FGameplayTagContainer& SourceTag = *Spec.CapturedSourceTags.GetAggregatedTags();
	const FGameplayTagContainer& TargetTag = *Spec.CapturedTargetTags.GetAggregatedTags();

	FAggregatorEvaluateParameters EvalParams;
	EvalParams.SourceTags = &SourceTag;
	EvalParams.TargetTags = &TargetTag;

	auto GetClampedAttributeMagnitude = [&EvalParams, &ExecutionParams](const FGameplayEffectAttributeCaptureDefinition& Def) -> float
	{
		// Every captured attribute is clamped to a sane non-negative range before we use it in
		// combat math so authored data cannot accidentally flip damage behavior.
		float Value = 0.f;
		ExecutionParams.AttemptCalculateCapturedAttributeMagnitude(Def, EvalParams, Value);
		return FMath::Max(0.f, Value);
	};

	DetermineDebuff(ExecutionParams, Spec, EvalParams);

	/*
	 * Damage resolution order:
	 *   1. Aggregate each authored DamageType.* set-by-caller magnitude.
	 *   2. Mitigate each damage type by its paired resistance attribute.
	 *   3. Resolve crit and block, writing those result flags into the custom effect context.
	 *   4. Apply armor / armor-penetration scaling as the final generic mitigation layer.
	 *   5. Output IncomingDamage for the AttributeSet to consume.
	 */
	float Damage = 0.f;
	for (const TPair<FGameplayTag, FGameplayTag>& DamageTypeToResistancePair : FAuraGameTagManager::Get().DamageTypesToResistance)
	{
		const FGameplayTag DamageType = DamageTypeToResistancePair.Key;
		const FGameplayTag ResistanceTag = DamageTypeToResistancePair.Value;
		const TMap<FGameplayTag, FGameplayEffectAttributeCaptureDefinition>& TagsToCaptureDefs = GetAuraDamageStatics().GetTagsToCaptureDefs();
		checkf(TagsToCaptureDefs.Contains(ResistanceTag), TEXT("TagsToCaptureDefs does not contain tag [%s] in ExecCalc_Damage."), *ResistanceTag.ToString());

		const FGameplayEffectAttributeCaptureDefinition& CaptureDef = TagsToCaptureDefs[ResistanceTag];
		float DamageTypeValue = Spec.GetSetByCallerMagnitude(DamageType, false, 0.f);
		if (DamageTypeValue <= 0.f) continue; 
		const float TargetResistanceValue = FMath::Clamp(GetClampedAttributeMagnitude(CaptureDef), 0.f, 100.f);

		// Typed resistance is applied per bucket before all buckets are recombined, which lets a
		// mixed-damage ability be partially resisted instead of treating it as one flat number.
		DamageTypeValue *= (100.f - TargetResistanceValue) / 100.f;
		
		// nerf the damage based on distance if it's radial  
		if (UAuraAbilitySystemLibrary::IsRadialDamage(ContextHandle))
		{
			 UAuraAbilitySystemLibrary::CalculateRadialDamage(ContextHandle, DamageTypeValue, TargetAvatar);
		}
		
		Damage += DamageTypeValue;
	}

	const float SourceCriticalHitChance = GetClampedAttributeMagnitude(GetAuraDamageStatics().CriticalHitChanceDef);
	const float TargetCriticalHitResilience = GetClampedAttributeMagnitude(GetAuraDamageStatics().CriticalHitResilienceDef);
	const float SourceCriticalDamage = GetClampedAttributeMagnitude(GetAuraDamageStatics().CriticalHitDamageDef);

	// Curve tables let combat scaling evolve with character level without hard-coding balance
	// constants directly into the execution logic.
	const FRealCurve* CriticalHitResilienceCurve = ClassInfo->DefaultCalculationCoeffcient->FindCurve("CriticalHitResilience", FString());
	check(CriticalHitResilienceCurve);
	const float CriticalHitResilienceScalingFactor = CriticalHitResilienceCurve->Eval(TargetPlayerLevel);

	const float AdjustedCriticalHitChance = SourceCriticalHitChance - TargetCriticalHitResilience * CriticalHitResilienceScalingFactor;
	const bool bCriticalHit = FMath::FRandRange(1.0f, 100.f) < AdjustedCriticalHitChance;
	UAuraAbilitySystemLibrary::SetIsCriticalHit(ContextHandle, bCriticalHit);

	if (bCriticalHit)
	{
		Damage = Damage * 2.0f + SourceCriticalDamage;
	}

	const float TargetBlockChance = GetClampedAttributeMagnitude(GetAuraDamageStatics().BlockChanceDef);
	const bool bBlockedHit = FMath::FRandRange(1.0f, 100.f) < TargetBlockChance;
	UAuraAbilitySystemLibrary::SetIsBlockedHit(ContextHandle, bBlockedHit);

	if (bBlockedHit)
	{
		Damage *= 0.5f;
	}

	const float TargetArmor = GetClampedAttributeMagnitude(GetAuraDamageStatics().ArmorDef);
	const float SourceArmorPenetration = GetClampedAttributeMagnitude(GetAuraDamageStatics().ArmorPenetrationDef);

	const FRealCurve* ArmorPenetrationCurve = ClassInfo->DefaultCalculationCoeffcient->FindCurve("ArmorPenetration", FString());
	check(ArmorPenetrationCurve);
	const float ArmorPenetrationScalingValue = ArmorPenetrationCurve->Eval(SourcePlayerLevel);

	const FRealCurve* ArmorCurve = ClassInfo->DefaultCalculationCoeffcient->FindCurve("EffectiveArmor", FString());
	check(ArmorCurve);
	const float ArmorScalingValue = ArmorCurve->Eval(TargetPlayerLevel);

	// Armor is intentionally the last mitigation layer: elemental resistance answers "what kind of
	// damage was this?", while armor answers "how well did the defender absorb the remaining hit?".
	const float EffectiveArmor = TargetArmor * (100.f - SourceArmorPenetration * ArmorPenetrationScalingValue) / 100.f;
	Damage *= (100.f - EffectiveArmor * ArmorScalingValue) / 100.f;

	UAuraAbilitySystemLibrary::SetShouldHitReact(ContextHandle, true);

	// ExecCalcs do not write Health directly. They output into a meta attribute so the AttributeSet
	// can centralize the "what happens when damage lands?" flow in one place.
	const FGameplayModifierEvaluatedData EvaluatedData(UAuraAttributeSet::GetIncomingDamageAttribute(), EGameplayModOp::Additive, Damage);
	OutExecutionOutput.AddOutputModifier(EvaluatedData);
}
