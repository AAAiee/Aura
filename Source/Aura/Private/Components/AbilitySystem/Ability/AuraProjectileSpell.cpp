// @Copyright HaolunYuan


#include "Components/AbilitySystem/Ability/AuraProjectileSpell.h"
#include "AbilitySystemBlueprintLibrary.h"
#include "AbilitySystemComponent.h"
#include "Effect/AuraProjectile.h"
#include "GameFramework/Pawn.h"
#include "Interaction/CombatInterface.h"
#include "ObjectPoolSubsystem.h"
#include "AuraGameTagManager.h"

void UAuraProjectileSpell::SpawnProjectile(const FVector& ProjectileTargetLocation)
{
	/*
	 * Projectile spawn flow:
	 *   1. Server validates that the ability has a caster + projectile class.
	 *   2. Resolve the combat socket through ICombatInterface so the spell works for any caster type.
	 *   3. Borrow a projectile from the pool instead of spawning a fresh actor.
	 *   4. Build the outgoing damage spec while we still have direct access to the casting ASC.
	 *   5. Assign owner / instigator and launch the projectile toward the requested target point.
	 */
	AActor* AvatarActor = GetAvatarActorFromActorInfo();
	checkf(AvatarActor, TEXT("AuraProjectileSpell::SpawnProjectile requires a valid avatar actor."));
	checkf(ProjectileClass, TEXT("AuraProjectileSpell::SpawnProjectile requires ProjectileClass to be set."));

	if (!AvatarActor->HasAuthority())
	{
		return;
	}

	UWorld* World = AvatarActor ? AvatarActor->GetWorld() : nullptr;
	if (!ensureMsgf(World, TEXT("AuraProjectileSpell::SpawnProjectile could not resolve a valid World from AvatarActor %s."), AvatarActor ? *AvatarActor->GetName() : TEXT("None")))
	{
		return;
	}

	UObjectPoolSubsystem* PoolSubsystem = World->GetSubsystem<UObjectPoolSubsystem>();
	if (!ensureMsgf(PoolSubsystem, TEXT("AuraProjectileSpell::SpawnProjectile could not resolve ObjectPoolSubsystem for world %s."), *World->GetName()))
	{
		return;
	}

	// CombatInterface abstracts "where should projectiles originate?" so both player and enemy
	// casters can reuse the same spawn logic without hard-coding socket lookups here.
	const FVector SocketLocation = ICombatInterface::Execute_GetCombatSocketLocation(AvatarActor, FAuraGameTagManager::Get().Montage_Attack_Weapon);
	const FVector AvatarPosition = AvatarActor->GetActorLocation();
	FRotator TargetRotation = (ProjectileTargetLocation - AvatarPosition).Rotation();

	FTransform SpawnTransform;
	SpawnTransform.SetLocation(SocketLocation);
	TargetRotation.Pitch = 0.0;
	SpawnTransform.SetRotation(TargetRotation.Quaternion()); 

	AAuraProjectile* Projectile = PoolSubsystem->GetPooledActorTyped<AAuraProjectile>(
		ProjectileClass,
		SpawnTransform);
	checkf(Projectile, TEXT("AuraProjectileSpell::SpawnProjectile failed to borrow a pooled projectile for class %s."), *ProjectileClass->GetName());

	if (UAbilitySystemComponent* AbilitySystemComponent = UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(GetAvatarActorFromActorInfo()))
	{
		check(DamageEffect);

		// Build the outgoing spec once here while we still know the owning ability level / source ASC.
		// The projectile simply carries this spec until its overlap callback resolves the impact.
		// Typed damage is authored on the ability as a tag-to-scalable-float map, then converted to
		// set-by-caller magnitudes so ExecCalc_Damage can resolve resistance by DamageType.* tag.
		FGameplayEffectContextHandle ContextHandle = AbilitySystemComponent->MakeEffectContext();
		ContextHandle.SetAbility(this);
		ContextHandle.AddSourceObject(Projectile);

		FGameplayEffectSpecHandle SpecHandle = AbilitySystemComponent->MakeOutgoingSpec(DamageEffect, GetAbilityLevel(), ContextHandle);
		for (const TPair<FGameplayTag, FScalableFloat>& Pair : DamageType)
		{
			const float ScaledDamage = Pair.Value.GetValueAtLevel(GetAbilityLevel());
			UAbilitySystemBlueprintLibrary::AssignTagSetByCallerMagnitude(SpecHandle, Pair.Key, ScaledDamage);
		}

		Projectile->DamageEffectHandle = SpecHandle;
	}

	Projectile->SetOwner(AvatarActor);
	Projectile->SetInstigator(Cast<APawn>(AvatarActor));
	Projectile->LaunchInDirection(SpawnTransform.GetRotation().GetForwardVector());

}

void UAuraProjectileSpell::ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData)
{
	Super::ActivateAbility(Handle, ActorInfo, ActivationInfo, TriggerEventData);


}
