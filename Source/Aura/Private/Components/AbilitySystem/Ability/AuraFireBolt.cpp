// @Copyright HaolunYuan

#include "Components/AbilitySystem/Ability/AuraFireBolt.h"

#include "AuraGameTagManager.h"
#include "ObjectPoolSubsystem.h"
#include "Components/AbilitySystem/AuraAbilitySystemLibrary.h"
#include "Effect/AuraProjectile.h"
#include "GameFramework/ProjectileMovementComponent.h"
#include "Interaction/CombatInterface.h"

class UObjectPoolSubsystem;

FString UAuraFireBolt::GetDescription(int32 Level) const
{
	const float FireDamage = DamageMagnitude.GetValueAtLevel(Level);
	const float ManaCost = GetManaCost(Level) * -1.f;
	const float Cooldown = GetCooldown(Level);
	if (Level == 1)
	{
		return FString::Printf(TEXT(
			"<Title>FIRE BOLT</>\n\n"
			"<Small>Level: %d</>\n"
			"<Small>ManaCost: %.1f</>\n"
			"<Cooldown>Cooldown: %.1f Seconds</>\n\n"
			"<Default>Launched a bolt of fire, exploding on impact and dealing: </><Damage>%d</>"
			"<Default> fire damage with a chance to burn</>\n\n"),
			Level,
			ManaCost,
			Cooldown,
			FMath::RoundToInt(FireDamage));
	}
	else
	{
		return FString::Printf(TEXT(
			"<Title>FIRE BOLT</>\n\n"
			"<Small>Level: %d</>\n"
			"<Small>ManaCost: %.1f</>\n"
			"<Cooldown>Cooldown: %.1f Seconds</>\n\n"
			"<Default>Launched</>"
			"<Small> %d </>"
			"<Default>bolts of fire, exploding on impact and dealing: "
			"</><Damage>%d</>"
			"<Default> fire damage with a chance to burn</>\n\n"),
			Level,
			ManaCost,
			Cooldown,
			FMath::Min(MaxNumProjectiles, Level),
			FMath::RoundToInt(FireDamage));
	}
}

FString UAuraFireBolt::GetNextLevelDescription(int32 Level) const
{
	const float FireDamage = DamageMagnitude.GetValueAtLevel(Level);
	const float ManaCost = GetManaCost(Level) * -1.f;
	const float Cooldown = GetCooldown(Level);
	return FString::Printf(TEXT(
		"<Title>NEXT LEVEL</>\n\n"
		"<Small>Level: %d</>\n"
		"<Small>ManaCost: %.1f</>\n"
		"<Cooldown>Cooldown: %.1f Seconds</>\n\n"
		"<Default>Launched</>"
		"<Small> %d </>"
		"<Default>bolts of fire, exploding on impact and dealing: "
		"</><Damage>%d</>"
		"<Default> fire damage with a chance to burn</>\n\n"),
		Level,
		ManaCost,
		Cooldown,
		FMath::Min(MaxNumProjectiles, Level),
		FMath::RoundToInt(FireDamage));
}

void UAuraFireBolt::SpawnProjectiles(const FVector& ProjectileTargetLocation, const FGameplayTag& SocketTag,
	bool bOverridePitch, float PitchOverride, AActor* HomingTarget)
{
	AActor* AvatarActor = GetAvatarActorFromActorInfo();
	checkf(AvatarActor, TEXT("AuraProjectileSpell::SpawnProjectile requires a valid avatar actor."));

	if (!AvatarActor->HasAuthority())
	{
		return;
	}

	UWorld* World = AvatarActor ? AvatarActor->GetWorld() : nullptr;
	UObjectPoolSubsystem* PoolSubsystem = World->GetSubsystem<UObjectPoolSubsystem>();
	
	const FVector SocketLocation = ICombatInterface::Execute_GetCombatSocketLocation(AvatarActor, SocketTag);
	const FVector AvatarPosition = AvatarActor->GetActorLocation();
	FRotator TargetRotation = (ProjectileTargetLocation - AvatarPosition).Rotation();
	if (bOverridePitch)
	{
		TargetRotation.Pitch = PitchOverride;
	}
	
	const int32 EffectiveNumProjectiles = FMath::Min(MaxNumProjectiles, GetAbilityLevel()); 
	const FVector Forward = TargetRotation.Vector();
	const TArray<FRotator> DirectionRotators = UAuraAbilitySystemLibrary::EvenlySpacedRotators(Forward, FVector::UpVector, 180.f, EffectiveNumProjectiles);
	
	for (const FRotator& DirRotator: DirectionRotators)
	{
		FTransform SpawnTransform;
		SpawnTransform.SetLocation(SocketLocation);
		SpawnTransform.SetRotation(DirRotator.Quaternion());

		AAuraProjectile* Projectile = PoolSubsystem->GetPooledActorTyped<AAuraProjectile>(
			ProjectileClass,
			SpawnTransform);
		checkf(Projectile, TEXT("AuraProjectileSpell::SpawnProjectile failed to borrow a pooled projectile for class %s."), *ProjectileClass->GetName());
		
		Projectile->DamageEffectParameters = MakeDamageEffectParametersFromClassDefault();
		
		if (HomingTarget && HomingTarget->Implements<UCombatInterface>())
		{
			Projectile->ProjectileMovement->HomingTargetComponent = HomingTarget->GetRootComponent();
		}
		else
		{
			Projectile->HomingTargetComponent = NewObject<USceneComponent>(USceneComponent::StaticClass());
			Projectile->HomingTargetComponent->SetWorldLocation(ProjectileTargetLocation);
			Projectile->ProjectileMovement->HomingTargetComponent =  Projectile->HomingTargetComponent;
		}
		
		Projectile->ProjectileMovement->HomingAccelerationMagnitude = FMath::RandRange(HomingAccelerationMin, HomingAccelerationMax);
		Projectile->ProjectileMovement->bIsHomingProjectile = bLaunchHomingProjectiles;
		
		Projectile->SetOwner(AvatarActor);
		Projectile->SetInstigator(Cast<APawn>(AvatarActor));
		Projectile->LaunchInDirection(SpawnTransform.GetRotation().GetForwardVector());
	}
}
