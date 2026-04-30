// @Copyright HaolunYuan

#include "Components/AbilitySystem/Ability/AuraSummonAbility.h"

#include "Engine/World.h"
#include "GameFramework/Actor.h"

TArray<FVector> UAuraSummonAbility::GetSpawnLocations() const
{
	const AActor* AvatarActor = GetAvatarActorFromActorInfo();
	const FVector ForwardVector = AvatarActor->GetActorForwardVector();
	const FVector Location = AvatarActor->GetActorLocation();
	const float HalfSpread = SpawnSpread / 2.0f;
	const float DeltaSpread = SpawnSpread / NumSpawnLocations;
	TArray<FVector> SpawnLocations;

	const FVector LeftSpreadEdge = ForwardVector.RotateAngleAxis(-HalfSpread, FVector::UpVector);

	// Build evenly distributed directions, then randomize distance per slot for a less rigid group.
	for (int i = 0; i < NumSpawnLocations; ++i)
	{
		const FVector Direction = LeftSpreadEdge.RotateAngleAxis(DeltaSpread * i, FVector::ZAxisVector);
		const float RandomDistance = FMath::RandRange(MinSpawnDistance, MaxSpawnDistance);
		FVector SpawnLocation = Location + Direction * RandomDistance;

		// Project each candidate down to visible ground so spawned minions do not float at caster height.
		FHitResult HitGroundResult;
		GetWorld()->LineTraceSingleByChannel(HitGroundResult, SpawnLocation, SpawnLocation - FVector(0.0f, 0.0f, 400.f), ECollisionChannel::ECC_Visibility);
		if (HitGroundResult.bBlockingHit)
		{
			SpawnLocation = HitGroundResult.ImpactPoint;
		}

		SpawnLocations.Emplace(SpawnLocation);
	}

	return SpawnLocations;
}

TSubclassOf<APawn> UAuraSummonAbility::GetRandomMinionClass() const
{
	check(MinionClasses.Num() > 0);
	return MinionClasses[FMath::RandHelper(MinionClasses.Num())];
}
