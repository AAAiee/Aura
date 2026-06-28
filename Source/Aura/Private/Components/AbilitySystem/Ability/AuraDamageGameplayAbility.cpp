// @Copyright HaolunYuan

#include "Components/AbilitySystem/Ability/AuraDamageGameplayAbility.h"

#include "AbilitySystemBlueprintLibrary.h"
#include "AbilitySystemComponent.h"
#include "NiagaraMeshRendererProperties.h"

void UAuraDamageGameplayAbility::CauseDamage(AActor* TargetActor)
{
	// Damage application is authoritative. Clients can predict ability activation, but the outgoing
	// damage spec must still be authored and applied on the server.
	if (!GetAvatarActorFromActorInfo()->HasAuthority())
	{
		return;
	}

	FGameplayEffectSpecHandle SpecHandle = MakeOutgoingGameplayEffectSpec(DamageEffectClass, 1.0f);

	// Each authored DamageType entry becomes one set-by-caller magnitude on the outgoing spec so
	// ExecCalc_Damage can resolve typed resistance without knowing the ability that produced it.
	UAbilitySystemBlueprintLibrary::AssignTagSetByCallerMagnitude(SpecHandle, DamageTypeTag, DamageMagnitude.GetValueAtLevel(GetAbilityLevel()));

	UAbilitySystemComponent* OwnerASC = GetAbilitySystemComponentFromActorInfo_Ensured();
	if (!OwnerASC)
	{
		return;
	}

	UAbilitySystemComponent* TargetASC = UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(TargetActor);
	OwnerASC->ApplyGameplayEffectSpecToTarget(*SpecHandle.Data.Get(), TargetASC);
}

FDamageEffectParameters UAuraDamageGameplayAbility::MakeDamageEffectParametersFromClassDefault(AActor* TargetActor) 
{
	/*
	 * Projectiles need to know "what damage should I apply?" before they know "who did I hit?".
	 * This bundle captures the ability's authored defaults now, then the projectile fills the
	 * target ASC at impact and hands everything to UAuraAbilitySystemLibrary::ApplyDamageEffect.
	 */
	FDamageEffectParameters Parameters;
	Parameters.WorldContextObject = GetAvatarActorFromActorInfo();
	Parameters.DamageGameplayEffectClass = DamageEffectClass;
	Parameters.SourceAbilitySystemComponent = GetAbilitySystemComponentFromActorInfo();
	Parameters.TargetAbilitySystemComponent = UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(TargetActor);
	Parameters.BaseDamage = DamageMagnitude.GetValueAtLevel(GetAbilityLevel());
	Parameters.AbilityLevel = GetAbilityLevel();
	Parameters.DamageType = DamageTypeTag;
	Parameters.DebuffDamage = DebuffDamage;
	Parameters.DebuffChance = DebuffChance;
	Parameters.DebuffFrequency = DebuffFrequency;
	Parameters.DebuffDuration = DebuffDuration;
	Parameters.DeathImpulseMagnitude = DeathImpulseMagnitude;
	Parameters.KnockBackMagnitude = KnockbackForceMagnitude;
	Parameters.KnockBackChance = KnockbackChance;
	
	if (IsValid(TargetActor))
	{
		FRotator DirToTarget = (TargetActor->GetActorLocation() - GetAvatarActorFromActorInfo()->GetActorLocation()).Rotation();
		if (bOverrideDirPitch)
		{
			DirToTarget.Pitch = DirPitchOverride;
		}
		FVector ToTarget = DirToTarget.Vector();
		if (!bDeathImpulseDirectionOverride)
		{
			Parameters.DeathImpulse = ToTarget * DeathImpulseMagnitude;
		}
		if (!bOverrideKnockbackDirection)
		{
			Parameters.KnockBackForce = ToTarget * KnockbackForceMagnitude;
		}
	}
	
	if (bOverrideKnockbackDirection)
	{
		KnockBackDirectionOverride.Normalize();
		Parameters.KnockBackForce = KnockBackDirectionOverride * KnockbackForceMagnitude;
		if (bOverrideDirPitch)
		{
			FRotator KnockbackRotation = KnockBackDirectionOverride.Rotation();
			KnockbackRotation.Pitch = DirPitchOverride; 
			Parameters.KnockBackForce = KnockbackRotation.Vector() * KnockbackForceMagnitude;
		}
	}
	
	if (bDeathImpulseDirectionOverride)
	{
		DeathImpulseDirectionOverride.Normalize();
		Parameters.DeathImpulse = DeathImpulseDirectionOverride * DeathImpulseMagnitude;
		if (bOverrideDirPitch)
		{
			FRotator DeathImpulseRotation = DeathImpulseDirectionOverride.Rotation();
			DeathImpulseRotation.Pitch = DirPitchOverride; 
			Parameters.DeathImpulse = DeathImpulseRotation.Vector() * DeathImpulseMagnitude;
		}
	}
	
	
	/* Calculate a default value for death impulse and knockback force if target is valid*/
	if (bIsRadialDamage)
	{
		Parameters.bIsRadialDamage = bIsRadialDamage;
		Parameters.RadialDamageInnerRadius = RadialDamageInnerRadius;
		Parameters.RadialDamageOuterRadius = RadialDamageOuterRadius;
		Parameters.RadialDamageOrigin = RadialDamageOrigin;
	}
	
	return Parameters;
}
