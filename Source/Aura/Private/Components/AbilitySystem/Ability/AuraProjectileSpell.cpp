// @Copyright HaolunYuan

#include "Components/AbilitySystem/Ability/AuraProjectileSpell.h"

#include "AbilitySystemBlueprintLibrary.h"
#include "AbilitySystemComponent.h"
#include "Effect/AuraProjectile.h"
#include "GameFramework/Pawn.h"
#include "Interaction/CombatInterface.h"
#include "ObjectPoolSubsystem.h"

void UAuraProjectileSpell::SpawnProjectile(const FVector& ProjectileTargetLocation, const FGameplayTag& CombatSocket)
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
	const FVector SocketLocation = ICombatInterface::Execute_GetCombatSocketLocation(AvatarActor, CombatSocket);
	const FVector AvatarPosition = AvatarActor->GetActorLocation();
	FRotator TargetRotation = (ProjectileTargetLocation - AvatarPosition).Rotation();

	FTransform SpawnTransform;
	SpawnTransform.SetLocation(SocketLocation);
	SpawnTransform.SetRotation(TargetRotation.Quaternion());

	AAuraProjectile* Projectile = PoolSubsystem->GetPooledActorTyped<AAuraProjectile>(
		ProjectileClass,
		SpawnTransform);
	checkf(Projectile, TEXT("AuraProjectileSpell::SpawnProjectile failed to borrow a pooled projectile for class %s."), *ProjectileClass->GetName());

	// The target is unknown until overlap, so the projectile stores a source-authored payload now
	// and fills TargetAbilitySystemComponent when it actually hits something.
	Projectile->DamageEffectParameters = MakeDamageEffectParametersFromClassDefault();

	Projectile->SetOwner(AvatarActor);
	Projectile->SetInstigator(Cast<APawn>(AvatarActor));
	Projectile->LaunchInDirection(SpawnTransform.GetRotation().GetForwardVector());
}

void UAuraProjectileSpell::ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData)
{
	Super::ActivateAbility(Handle, ActorInfo, ActivationInfo, TriggerEventData);
}
