// @Copyright HaolunYuan

#pragma once

#include "CoreMinimal.h"
#include "Effect/AuraProjectile.h"
#include "Components/SphereComponent.h"
#include "AuraProjectileSphere.generated.h"

/**
 * 
 */
UCLASS()
class AURA_API AAuraProjectileSphere : public AAuraProjectile
{
	GENERATED_BODY()


public:
	AAuraProjectileSphere();


protected:
private:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Projectile", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<USphereComponent> SphereCollision;
	
};
