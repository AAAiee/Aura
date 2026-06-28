#include "Effect/AuraPooledGameplayActor.h"

#include "Engine/World.h"
#include "ObjectPoolSubsystem.h"

AAuraPooledGameplayActor::AAuraPooledGameplayActor()
{
	PrimaryActorTick.bCanEverTick = false;
}


void AAuraPooledGameplayActor::RequestReturnToPool()
{
	if (UObjectPoolSubsystem* PoolSubsystem = GetObjectPoolSubsystem())
	{
		PoolSubsystem->ReturnActorToPool(this);
		return;
	}

	UE_LOG(LogTemp, Warning, TEXT("AuraPooledGameplayActor::RequestReturnToPool called without a valid pool subsystem for %s"), *GetName());
}

void AAuraPooledGameplayActor::OnTakenFromPool()
{
	bIsActiveInPool = true;
	HandleTakenFromPool();
}

void AAuraPooledGameplayActor::OnReturnedToPool()
{
	ResetPooledState();
	bIsActiveInPool = false;
	HandleReturnedToPool();
}

void AAuraPooledGameplayActor::HandleTakenFromPool()
{
}

void AAuraPooledGameplayActor::HandleReturnedToPool()
{
}

void AAuraPooledGameplayActor::ResetPooledState()
{
}

UObjectPoolSubsystem* AAuraPooledGameplayActor::GetObjectPoolSubsystem() const
{
	const UWorld* World = GetWorld();
	return World ? World->GetSubsystem<UObjectPoolSubsystem>() : nullptr;
}
