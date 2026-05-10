// @Copyright HaolunYuan

#pragma once

#include "CoreMinimal.h"
#include "Effect/AuraProjectile.h"
#include "AuraProjectileSphere.generated.h"

class USphereComponent;

/**
 * Projectile implementation that supplies a sphere collision component to the shared projectile base.
 */
UCLASS()
class AURA_API AAuraProjectileSphere : public AAuraProjectile
{
	GENERATED_BODY()


public:
	AAuraProjectileSphere();

private:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Projectile", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<USphereComponent> SphereCollision;
};
