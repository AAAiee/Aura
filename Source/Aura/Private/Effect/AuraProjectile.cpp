#include "Effect/AuraProjectile.h"
#include "Components/PrimitiveComponent.h"
#include "GameFramework/ProjectileMovementComponent.h"

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
	Super::HandleTakenFromPool();

	bHasRegisteredHit = false;

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
}

void AAuraProjectile::ResetPooledState()
{
	Super::ResetPooledState();

	bHasRegisteredHit = false;

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

	SetOwner(nullptr);
	SetInstigator(nullptr);
	ForceNetUpdate();
}

void AAuraProjectile::SetCollisionComponent(UPrimitiveComponent* InCollisionComponent)
{
	check(InCollisionComponent);

	CollisionComponent = InCollisionComponent;
	SetRootComponent(CollisionComponent);
	ProjectileMovement->SetUpdatedComponent(CollisionComponent);

	// Example collision setup
	CollisionComponent->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	CollisionComponent->SetGenerateOverlapEvents(true);

	CollisionComponent->SetCollisionResponseToAllChannels(ECR_Ignore);
	CollisionComponent->SetCollisionResponseToChannel(ECC_WorldDynamic, ECR_Overlap);
	CollisionComponent->SetCollisionResponseToChannel(ECC_WorldStatic, ECR_Overlap);
	CollisionComponent->SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap);

	// Bind shared overlap logic once here
	CollisionComponent->OnComponentBeginOverlap.AddDynamic(this, &AAuraProjectile::OnProjectileOverlap);
}

void AAuraProjectile::OnProjectileOverlap(
	UPrimitiveComponent* OverlappedComponent,
	AActor* OtherActor,
	UPrimitiveComponent* OtherComp,
	int32 OtherBodyIndex,
	bool bFromSweep,
	const FHitResult& SweepResult)
{
	if (!IsActiveInPool() || bHasRegisteredHit || !bReturnToPoolOnAnyOverlap)
	{
		return;
	}

	if (OtherActor == nullptr || OtherActor == this || OtherActor == GetOwner())
	{
		return;
	}

	bHasRegisteredHit = true;
	RequestReturnToPool();
}
