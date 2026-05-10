#include "Effect/AuraProjectile.h"

#include "AbilitySystemBlueprintLibrary.h"
#include "AbilitySystemComponent.h"
#include "Aura/Aura.h"
#include "Components/AudioComponent.h"
#include "Components/PrimitiveComponent.h"
#include "GameFramework/ProjectileMovementComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Net/UnrealNetwork.h"
#include "NiagaraFunctionLibrary.h"

AAuraProjectile::AAuraProjectile()
{
	PrimaryActorTick.bCanEverTick = false;
	bReplicates = true;
	SetReplicateMovement(true);

	ProjectileMovement = CreateDefaultSubobject<UProjectileMovementComponent>(TEXT("ProjectileMovement"));
	ProjectileMovement->InitialSpeed = 550.f;
	ProjectileMovement->MaxSpeed = 550.f;
	ProjectileMovement->ProjectileGravityScale = 0.f;
	ProjectileMovement->bAutoActivate = false;
}

void AAuraProjectile::BeginPlay()
{
	Super::BeginPlay();
}

void AAuraProjectile::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	// Always notify when the pooled active flag changes so clients can apply local borrow/return state.
	DOREPLIFETIME_CONDITION_NOTIFY(AAuraProjectile, bReplicatedProjectileActive, COND_None, REPNOTIFY_Always);
}

void AAuraProjectile::LaunchInDirection(const FVector& Direction)
{
	if (!ensureMsgf(ProjectileMovement, TEXT("AuraProjectile::LaunchInDirection called without a valid ProjectileMovement component on %s."), *GetName()))
	{
		return;
	}

	if (!ensureMsgf(CollisionComponent, TEXT("AuraProjectile::LaunchInDirection called without a valid CollisionComponent on %s."), *GetName()))
	{
		return;
	}

	const FVector LaunchDirection = Direction.GetSafeNormal();
	if (!ensureMsgf(!LaunchDirection.IsNearlyZero(), TEXT("AuraProjectile::LaunchInDirection received a nearly zero direction for %s."), *GetName()))
	{
		return;
	}

	ProjectileMovement->Velocity = LaunchDirection * ProjectileMovement->InitialSpeed;
	ProjectileMovement->UpdateComponentVelocity();
	ForceNetUpdate();
}

void AAuraProjectile::HandleTakenFromPool()
{
	if (HasAuthority())
	{
		bHasResolvedImpact = false;
		bReplicatedProjectileActive = true;
		ForceNetUpdate();
		ApplyActiveState();
	}
}

void AAuraProjectile::ResetPooledState()
{
	if (HasAuthority())
	{
		bHasResolvedImpact = false;
		bReplicatedProjectileActive = false;
		ForceNetUpdate();
		ApplyInactiveState();
		SetOwner(nullptr);
		SetInstigator(nullptr);
	}
}

void AAuraProjectile::SetCollisionComponent(UPrimitiveComponent* InCollisionComponent)
{
	check(InCollisionComponent);

	CollisionComponent = InCollisionComponent;
	SetRootComponent(CollisionComponent);
	ProjectileMovement->SetUpdatedComponent(CollisionComponent);

	// Projectile collision overlaps gameplay targets and world blockers but leaves resolution to the server.
	CollisionComponent->SetCollisionObjectType(ECC_Projectile);
	CollisionComponent->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	CollisionComponent->SetGenerateOverlapEvents(true);

	CollisionComponent->SetCollisionResponseToAllChannels(ECR_Ignore);
	CollisionComponent->SetCollisionResponseToChannel(ECC_WorldDynamic, ECR_Overlap);
	CollisionComponent->SetCollisionResponseToChannel(ECC_WorldStatic, ECR_Overlap);
	CollisionComponent->SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap);
	CollisionComponent->SetCollisionResponseToChannel(ECC_EnemyCollision, ECR_Overlap);

	// Bind shared overlap logic once after the child class supplies its collision primitive.
	CollisionComponent->OnComponentBeginOverlap.AddDynamic(this, &AAuraProjectile::OnProjectileOverlap);
}

void AAuraProjectile::PlayImpactEffects()
{
	// Cosmetic playback is intentionally isolated from hit resolution so the same visuals can be
	// triggered by authority code and mirrored to simulated clients through a multicast.
	if (ImpactSound)
	{
		UGameplayStatics::PlaySoundAtLocation(this, ImpactSound, GetActorLocation(), FRotator::ZeroRotator, 0.6f);
	}

	if (ImpactEffect)
	{
		UNiagaraFunctionLibrary::SpawnSystemAtLocation(this, ImpactEffect, GetActorLocation(), FRotator::ZeroRotator);
	}

	if (LoopingSoundComponent)
	{
		LoopingSoundComponent->Stop();
		LoopingSoundComponent->DestroyComponent();
		LoopingSoundComponent = nullptr;
	}
}

void AAuraProjectile::ApplyActiveState()
{
	// The pool subsystem already unhides/positions the authoritative actor. We still apply the
	// same activation state locally so replicated copies behave like a freshly borrowed projectile.
	SetActorHiddenInGame(false);
	SetActorTickEnabled(true);
	SetActorEnableCollision(true);

	if (CollisionComponent)
	{
		CollisionComponent->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
		CollisionComponent->SetGenerateOverlapEvents(true);
	}

	if (ProjectileMovement)
	{
		ProjectileMovement->SetUpdatedComponent(CollisionComponent);
		ProjectileMovement->StopMovementImmediately();
		ProjectileMovement->Activate(true);
	}

	// Borrowed projectiles can be reused many times, so make sure we do not leave a previous
	// activation's looping audio component attached before we spawn a new one.
	if (LoopingSoundComponent)
	{
		LoopingSoundComponent->Stop();
		LoopingSoundComponent->DestroyComponent();
		LoopingSoundComponent = nullptr;
	}

	if (LoopingSound)
	{
		LoopingSoundComponent = UGameplayStatics::SpawnSoundAttached(LoopingSound, GetRootComponent());
	}
}

void AAuraProjectile::ApplyInactiveState()
{
	// Clients do not own the actual pool bookkeeping, but they still need the same local visual/
	// collision shutdown when the authoritative server returns the projectile to the pool.
	if (ProjectileMovement)
	{
		ProjectileMovement->StopMovementImmediately();
		ProjectileMovement->Deactivate();
	}

	if (CollisionComponent)
	{
		CollisionComponent->SetGenerateOverlapEvents(false);
		CollisionComponent->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	}

	SetActorEnableCollision(false);
	SetActorTickEnabled(false);
	SetActorHiddenInGame(true);

	if (LoopingSoundComponent)
	{
		LoopingSoundComponent->Stop();
		LoopingSoundComponent->DestroyComponent();
		LoopingSoundComponent = nullptr;
	}

	DamageEffectHandle.Clear();
}

void AAuraProjectile::OnRep_ReplicatedProjectileActive()
{
	if (bReplicatedProjectileActive)
	{
		ApplyActiveState();
	}
	else
	{
		ApplyInactiveState();
	}
}

void AAuraProjectile::MulticastPlayImpactEffects_Implementation()
{
	// Dedicated servers do not need local cosmetic playback.
	if (GetNetMode() != NM_DedicatedServer)
	{
		PlayImpactEffects();
	}
}

void AAuraProjectile::OnProjectileOverlap(
	UPrimitiveComponent* OverlappedComponent,
	AActor* OtherActor,
	UPrimitiveComponent* OtherComp,
	int32 OtherBodyIndex,
	bool bFromSweep,
	const FHitResult& SweepResult)
{
	/*
	 * Impact resolution flow:
	 *   1. Ignore pooled / invalid / self-overlaps.
	 *   2. Let the server be the only authority that decides whether damage should be applied.
	 *   3. Apply the prebuilt damage spec to the target ASC.
	 *   4. Multicast impact cosmetics so every relevant machine sees / hears the hit once.
	 *   5. Return the projectile to the pool so the next cast can reuse it.
	 */

	// The server owns hit resolution. Clients keep their collision for local copies, but they do
	// not decide projectile lifetime or trigger impact effects directly.
	if (!HasAuthority())
	{
		return;
	}

	// Ignore overlaps from parked/inactive pooled actors and from projectiles that should not auto-return on hit.
	if (!IsActiveInPool() || !bReturnToPoolOnAnyOverlap)
	{
		return;
	}

	// Ignore meaningless overlaps that can happen with our own components or owning actor.
	if (OtherActor == nullptr || OtherActor == this || OtherActor == GetOwner())
	{
		return;
	}
	// Protect against duplicate overlap callbacks before collision is fully disabled/returned.
	if (bHasResolvedImpact)
	{
		return;
	}

	bHasResolvedImpact = true;

	if (DamageEffectHandle.IsValid())
	{
		// The projectile itself does not "calculate" damage. It simply carries the already-authored
		// outgoing spec from the ability and hands that spec to the target ASC on impact.
		if (UAbilitySystemComponent* SourceASC = UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(GetOwner()))
		{
			if (UAbilitySystemComponent* TargetASC = UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(OtherActor))
			{
				SourceASC->ApplyGameplayEffectSpecToTarget(*DamageEffectHandle.Data.Get(), TargetASC);
			}
		}
	}
	else
	{
		ensureMsgf(false, TEXT("AuraProjectile::OnProjectileOverlap hit %s without a valid damage effect spec on projectile %s."), *GetNameSafe(OtherActor), *GetName());
	}

	// One authoritative multicast tells every relevant machine to play the impact locally once.
	MulticastPlayImpactEffects();
	RequestReturnToPool();
}
