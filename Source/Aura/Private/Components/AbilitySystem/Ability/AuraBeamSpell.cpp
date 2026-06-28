// @Copyright HaolunYuan


#include "Components/AbilitySystem/Ability/AuraBeamSpell.h"

#include "Components/AbilitySystem/AuraAbilitySystemLibrary.h"
#include "GameFramework/Character.h"
#include "Interaction/CombatInterface.h"
#include "Kismet/KismetSystemLibrary.h"


void UAuraBeamSpell::StoreMouseDataInfo(const FHitResult& HitResult)
{
	if (HitResult.bBlockingHit)
	{
		MouseHitLocation = HitResult.ImpactPoint;
		MouseHitActor = HitResult.GetActor();
	} 
	else
	{
		EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo,true, false);
	}
}

void UAuraBeamSpell::StoreOwnerCachedVariables()
{
	if (CurrentActorInfo)
	{
		/*Persistant ref to player controller*/
		OwnerPlayerController = CurrentActorInfo->PlayerController.Get();
		OwnerCharacter = Cast<ACharacter>(CurrentActorInfo->AvatarActor.Get());
	}
}

void UAuraBeamSpell::TraceFirstTarget(const FVector& BeamTargetLocation)
{
	check(OwnerCharacter);
	if (OwnerCharacter->Implements<UCombatInterface>())
	{
		if (const USkeletalMeshComponent* Weapon = ICombatInterface::Execute_GetWeaponMesh(OwnerCharacter))
		{
			TArray<AActor*> ActorsToIgnore;
			ActorsToIgnore.Add(OwnerCharacter);
			FHitResult HitResult;
			const FVector SocketLocation = Weapon->GetSocketLocation("TipSocket");
			
			UKismetSystemLibrary::SphereTraceSingle(
				OwnerCharacter,
				SocketLocation,
				BeamTargetLocation,
				20.f,
				TraceTypeQuery1,
				false ,
				ActorsToIgnore,
				EDrawDebugTrace::None,
				HitResult, 
				true);
			
			if (HitResult.bBlockingHit)
			{
				MouseHitLocation = HitResult.ImpactPoint;
				MouseHitActor = HitResult.GetActor();
			}
		}
	}
	
	// mouse hit actor is the primary target, bind delegate to know when they die
	if (ICombatInterface* CombatInterface = Cast<ICombatInterface>(MouseHitActor))
	{
		if (!CombatInterface->GetOnCharacterDieDelegate().IsAlreadyBound(this, &UAuraBeamSpell::PrimaryTargetDied))
		{
			CombatInterface->GetOnCharacterDieDelegate().AddDynamic(this, &UAuraBeamSpell::PrimaryTargetDied); 
		}
	}
		
}

void UAuraBeamSpell::StoreAdditionalTarget(TArray<AActor*>& OutAdditionalTargets)
{
	TArray<AActor*> OverlappedTargets;
	const FVector HitActorLocation = MouseHitActor->GetActorLocation();
	UAuraAbilitySystemLibrary::GetLivePlayersWithinRadius(
		GetAvatarActorFromActorInfo(), 
		OverlappedTargets, 
		{ MouseHitActor, OwnerCharacter },
		800.f, HitActorLocation );

	const int32 NumAdditionalTargets = FMath::Min(GetAbilityLevel() - 1, MaxNumShockTarget);
	UAuraAbilitySystemLibrary::GetClosestTargets(NumAdditionalTargets,MoveTemp(OverlappedTargets), OutAdditionalTargets,HitActorLocation);
	
	// If additional targets do not bind to the delegate yet, bind them.
	for (AActor* Target : OutAdditionalTargets)
	{
		if (ICombatInterface* CombatInterface = Cast<ICombatInterface>(Target))
		{
			if (!CombatInterface->GetOnCharacterDieDelegate().IsAlreadyBound(this, &UAuraBeamSpell::AdditionalTargetDied))
			{
				CombatInterface->GetOnCharacterDieDelegate().AddDynamic(this, &UAuraBeamSpell::AdditionalTargetDied); 
			}
		}
	}
}



