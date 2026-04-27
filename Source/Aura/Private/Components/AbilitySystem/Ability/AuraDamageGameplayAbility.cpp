// @Copyright HaolunYuan


#include "Components/AbilitySystem/Ability/AuraDamageGameplayAbility.h"
#include "AbilitySystemBlueprintLibrary.h"
#include "AbilitySystemComponent.h"

void UAuraDamageGameplayAbility::CauseDamage(AActor* TargetActor)
{
	// Damage application is authoritative. Clients can predict ability activation, but the outgoing
	// damage spec must still be authored and applied on the server.
	if (!GetAvatarActorFromActorInfo()->HasAuthority())
	{
		return;
	}

	FGameplayEffectSpecHandle SpecHandle = MakeOutgoingGameplayEffectSpec(DamageEffect, 1.0f);

	// Each authored DamageType entry becomes one set-by-caller magnitude on the outgoing spec so
	// ExecCalc_Damage can resolve typed resistance without knowing the ability that produced it.
	for (const TPair<FGameplayTag, FScalableFloat>& Pair : DamageType)
	{
		UAbilitySystemBlueprintLibrary::AssignTagSetByCallerMagnitude(SpecHandle, Pair.Key, Pair.Value.GetValueAtLevel(GetAbilityLevel()));
	}

	UAbilitySystemComponent* OwnerASC = GetAbilitySystemComponentFromActorInfo_Ensured();
	if (!OwnerASC)
	{
		return;
	}

	UAbilitySystemComponent* TargetASC = UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(TargetActor);

	if (TargetASC)
	{
		OwnerASC->ApplyGameplayEffectSpecToTarget(*SpecHandle.Data.Get(), TargetASC);
	}
}
