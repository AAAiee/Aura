// @Copyright HaolunYuan

#include "Effect/Projectile/AuraProjectileSphere.h"

#include "Components/SphereComponent.h"

AAuraProjectileSphere::AAuraProjectileSphere()
{
	SphereCollision = CreateDefaultSubobject<USphereComponent>(TEXT("SphereCollision"));
	SetCollisionComponent(SphereCollision);
}
