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

FString UAuraBeamSpell::GetDescription(int32 Level) const
{
	const float Damage = DamageMagnitude.GetValueAtLevel(Level);
	const float ManaCost = GetManaCost(Level) * -1.f;
	const float Cooldown = GetCooldown(Level);
	const int32 NumAdditionalTargets = FMath::Min(MaxNumShockTarget, FMath::Max(Level - 1, 0));
	if (NumAdditionalTargets == 0)
	{
		return FString::Printf(TEXT(
			"<Title>ELECTROCUTE</>\n\n"
			"<Small>Level: %d</>\n"
			"<Small>ManaCost: %.1f</>\n"
			"<Cooldown>Cooldown: %.1f Seconds</>\n\n"
			"<Default>Launches a beam of lightning at the target enemy, dealing </><Damage>%d</><Default> lightning damage.</>\n\n"),
			Level,
			ManaCost,
			Cooldown,
			FMath::RoundToInt(Damage));
	}

	return FString::Printf(TEXT(
		"<Title>ELECTROCUTE</>\n\n"
		"<Small>Level: %d</>\n"
		"<Small>ManaCost: %.1f</>\n"
		"<Cooldown>Cooldown: %.1f Seconds</>\n\n"
		"<Default>Launches a beam of lightning at the target enemy, then chains to up to </>"
		"<Small>%d</>"
		"<Default> nearby enemies, dealing </><Damage>%d</><Default> lightning damage to each target.</>\n\n"),
		Level,
		ManaCost,
		Cooldown,
		NumAdditionalTargets,
		FMath::RoundToInt(Damage));
}

FString UAuraBeamSpell::GetNextLevelDescription(int32 Level) const
{
	const float Damage = DamageMagnitude.GetValueAtLevel(Level);
	const float ManaCost = GetManaCost(Level) * -1.f;
	const float Cooldown = GetCooldown(Level);
	const int32 NumAdditionalTargets = FMath::Min(MaxNumShockTarget, FMath::Max(Level - 1, 0));
	if (NumAdditionalTargets == 0)
	{
		return FString::Printf(TEXT(
			"<Title>NEXT LEVEL</>\n\n"
			"<Small>Level: %d</>\n"
			"<Small>ManaCost: %.1f</>\n"
			"<Cooldown>Cooldown: %.1f Seconds</>\n\n"
			"<Default>Launches a beam of lightning at the target enemy, dealing </><Damage>%d</><Default> lightning damage.</>\n\n"),
			Level,
			ManaCost,
			Cooldown,
			FMath::RoundToInt(Damage));
	}

	return FString::Printf(TEXT(
		"<Title>NEXT LEVEL</>\n\n"
		"<Small>Level: %d</>\n"
		"<Small>ManaCost: %.1f</>\n"
		"<Cooldown>Cooldown: %.1f Seconds</>\n\n"
		"<Default>Launches a beam of lightning at the target enemy, then chains to up to </>"
		"<Small>%d</>"
		"<Default> nearby enemies, dealing </><Damage>%d</><Default> lightning damage to each target.</>\n\n"),
		Level,
		ManaCost,
		Cooldown,
		NumAdditionalTargets,
		FMath::RoundToInt(Damage));
}

