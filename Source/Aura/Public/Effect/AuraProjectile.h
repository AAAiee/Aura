#pragma once

#include "CoreMinimal.h"
#include "Effect/AuraPooledGameplayActor.h"
#include "AuraProjectile.generated.h"

class UProjectileMovementComponent;
class UPrimitiveComponent;

UCLASS(Abstract)
class AURA_API AAuraProjectile : public AAuraPooledGameplayActor
{
	GENERATED_BODY()

public:
	AAuraProjectile();

	void LaunchInDirection(const FVector& Direction);

protected:
	virtual void BeginPlay() override;

	virtual void HandleTakenFromPool() override;
	virtual void ResetPooledState() override;

	// Shared overlap logic for all projectile types
	UFUNCTION()
	virtual void OnProjectileOverlap(
		UPrimitiveComponent* OverlappedComponent,
		AActor* OtherActor,
		UPrimitiveComponent* OtherComp,
		int32 OtherBodyIndex,
		bool bFromSweep,
		const FHitResult& SweepResult);

	// Child classes pass in whatever collision component they created
	void SetCollisionComponent(UPrimitiveComponent* InCollisionComponent);

protected:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Projectile")
	bool bReturnToPoolOnAnyOverlap = false ;

	UPROPERTY(Transient)
	bool bHasRegisteredHit = false;

protected:

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Projectile")
	TObjectPtr<UProjectileMovementComponent> ProjectileMovement;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Projectile")
	TObjectPtr<UPrimitiveComponent> CollisionComponent;
};
