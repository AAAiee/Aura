// @Copyright HaolunYuan

#include "Effect/Projectile/AuraProjectileSphere.h"

AAuraProjectileSphere::AAuraProjectileSphere()
{
	SphereCollision = CreateDefaultSubobject<USphereComponent>(TEXT("SphereCollision"));
	SetCollisionComponent(SphereCollision);
}
