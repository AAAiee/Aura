// @Copyright HaolunYuan


#include "Components/AbilitySystem/Ability/AuraProjectileSpell.h"
#include "Effect/AuraProjectile.h"
#include "GameFramework/Pawn.h"
#include "Interaction/CombatInterface.h"
#include "ObjectPoolConfigDataAsset.h"
#include "ObjectPoolSubsystem.h"

void UAuraProjectileSpell::SpawnProjectile()
{

	bool bIsOnServer = GetAvatarActorFromActorInfo()->HasAuthority();
	if (!bIsOnServer)
	{
		return;
	}

	AActor* AvatarActor = GetAvatarActorFromActorInfo();
	UWorld* World = AvatarActor ? AvatarActor->GetWorld() : nullptr;
	if (!ensureMsgf(World, TEXT("AuraProjectileSpell::ActivateAbility could not resolve a valid World from AvatarActor %s."), AvatarActor ? *AvatarActor->GetName() : TEXT("None")))
	{
		return;
	}

	UObjectPoolSubsystem* PoolSubsystem = World->GetSubsystem<UObjectPoolSubsystem>();
	if (!ensureMsgf(PoolSubsystem, TEXT("AuraProjectileSpell::ActivateAbility could not resolve ObjectPoolSubsystem for world %s."), *World->GetName()))
	{
		return;
	}

	if (!PoolSubsystem->EnsurePoolInitializedFromConfig(ProjectileClass))
	{
		ensureMsgf(false, TEXT("AuraProjectileSpell::ActivateAbility failed to initialize a pool for projectile class %s. Check Simple Object Pool developer settings and config asset entries."), *ProjectileClass->GetName());
		return;
	}

	ICombatInterface* CombatInterface = Cast<ICombatInterface>(AvatarActor);
	if (CombatInterface)
	{
		const FVector SocketLocation = CombatInterface->GetCombatSocketLocation();

		FTransform SpawnTransform;
		SpawnTransform.SetLocation(SocketLocation);

		const FPoolRecyclePolicy RecyclePolicy = PoolSubsystem->GetRecyclePolicyForClass(ProjectileClass);

		AAuraProjectile* Projectile = PoolSubsystem->GetPooledActorTyped<AAuraProjectile>(
			ProjectileClass,
			SpawnTransform,
			RecyclePolicy.bShouldAutomaticallyReturn,
			RecyclePolicy.RecycleDelay);

		if (Projectile)
		{
			Projectile->SetOwner(AvatarActor);
			Projectile->SetInstigator(Cast<APawn>(AvatarActor));
			Projectile->LaunchInDirection(SpawnTransform.GetRotation().GetForwardVector());
		}

	}

}

void UAuraProjectileSpell::ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData)
{
	Super::ActivateAbility(Handle, ActorInfo, ActivationInfo, TriggerEventData);


}

