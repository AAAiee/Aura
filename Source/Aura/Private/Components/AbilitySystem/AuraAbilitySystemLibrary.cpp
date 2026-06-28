// @Copyright HaolunYuan

#include "Components/AbilitySystem/AuraAbilitySystemLibrary.h"

#include "AbilitySystemBlueprintLibrary.h"
#include "AbilitySystemComponent.h"
#include "AuraAbilityTypes.h"
#include "AuraGameTagManager.h"
#include "AuraLogCategory.h"
#include "CollisionQueryParams.h"
#include "Components/AbilitySystem/AuraAbilitySystemComponent.h"
#include "Components/AbilitySystem/AuraAttributeSet.h"
#include "Components/AbilitySystem/Data/AbilityInfo.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "Game/AuraGameInstance.h"
#include "Game/AuraGameModeBase.h"
#include "GameFramework/PlayerController.h"
#include "Interaction/CombatInterface.h"
#include "Kismet/GameplayStatics.h"
#include "Player/AuraPlayerState.h"
#include "UI/HUD/AuraHUD.h"
#include "UI/WidgetController/AuraWidgetController.h"

namespace
{
	const FAuraGameplayEffectContext* GetAuraEffectContext(const FGameplayEffectContextHandle& EffectHandle, const TCHAR* CallerName)
	{
		const FGameplayEffectContext* Context = EffectHandle.Get();
		if (Context && Context->GetScriptStruct()->IsChildOf(FAuraGameplayEffectContext::StaticStruct()))
		{
			return static_cast<const FAuraGameplayEffectContext*>(Context);
		}

		UE_LOG(LogAura, Warning, TEXT("%s called without an Aura gameplay effect context."), CallerName);
		return nullptr;
	}

	FAuraGameplayEffectContext* GetMutableAuraEffectContext(FGameplayEffectContextHandle& EffectHandle, const TCHAR* CallerName)
	{
		FGameplayEffectContext* Context = EffectHandle.Get();
		if (Context && Context->GetScriptStruct()->IsChildOf(FAuraGameplayEffectContext::StaticStruct()))
		{
			return static_cast<FAuraGameplayEffectContext*>(Context);
		}

		UE_LOG(LogAura, Warning, TEXT("%s called without an Aura gameplay effect context."), CallerName);
		return nullptr;
	}
}

UAuraOverlayWidgetController* UAuraAbilitySystemLibrary::GetOverlayWidgetController(const UObject* WorldContextObject)
{
	FWidgetControllerParameters Params;
	AAuraHUD* AuraHUD = nullptr;
	if (!MakeWidgetControllerParameters(WorldContextObject, Params, AuraHUD))
	{
		return nullptr;
	}

	return AuraHUD->GetOverlayWidgetController(Params);
}

UAttributeMenuWidgetController* UAuraAbilitySystemLibrary::GetAttributeMenuWidgetController(const UObject* WorldContextObject)
{
	FWidgetControllerParameters Params;
	AAuraHUD* AuraHUD = nullptr;
	if (!MakeWidgetControllerParameters(WorldContextObject, Params, AuraHUD))
	{
		return nullptr;
	}

	return AuraHUD->GetAttributeMenuWidgetController(Params);
}

UAuraSpellMenuWidgetController* UAuraAbilitySystemLibrary::GetSpellMenuWidgetController(const UObject* WorldContextObject)
{
	FWidgetControllerParameters Params;
	AAuraHUD* AuraHUD = nullptr;
	if (!MakeWidgetControllerParameters(WorldContextObject, Params, AuraHUD))
	{
		return nullptr;
	}
	return AuraHUD->GetSpellMenuWidgetController(Params);
}

void UAuraAbilitySystemLibrary::InitializeDefaultAttributes(const UObject* WorldContextObject, ECharacterClass CharacterClass, UAbilitySystemComponent* ASC, float Level)
{
	check(ASC);

	/*
	 * Default-attribute startup flow:
	 *   1. Resolve the avatar actor from the ASC.
	 *   2. Bail on clients because only the server is allowed to author Gameplay Effect state.
	 *   3. Read the class-info data asset from the authoritative GameMode.
	 *   4. Build the primary / secondary / vital specs.
	 *   5. Apply them to self so the replicated AttributeSet becomes the single source of truth.
	 *
	 * Keeping the flow here means both Aura-controlled pawns and enemy pawns can share the same
	 * initialization rules instead of duplicating startup effect code in multiple character classes.
	 */
	AActor* AvatarActor = ASC->GetAvatarActor();
	if (!ensureMsgf(AvatarActor, TEXT("AuraAbilitySystemLibrary::InitializeDefaultAttributes requires ASC %s to have a valid avatar actor."), *GetNameSafe(ASC)))
	{
		return;
	}

	// Gameplay Effects are authoritative state, so only the server should seed default attributes.
	if (!AvatarActor->HasAuthority())
	{
		return;
	}

	UCharacterClassInfo* ClassInfos = GetCharacterClassInfo(WorldContextObject);
	if (!ensureMsgf(ClassInfos, TEXT("AuraAbilitySystemLibrary::InitializeDefaultAttributes requires CharacterClassInfo to be assigned on %s."), *GetNameSafe(WorldContextObject)))
	{
		return;
	}

	const FCharacterClassDefaultInfo ClassInfoEntry = ClassInfos->GetDefaultInfoForClass(CharacterClass);
	FGameplayEffectContextHandle PrimaryEffectContextHandle = ASC->MakeEffectContext();
	PrimaryEffectContextHandle.AddSourceObject(AvatarActor);
	if (ensureMsgf(ClassInfoEntry.PrimaryAttributeEffect, TEXT("AuraAbilitySystemLibrary::InitializeDefaultAttributes is missing a primary attribute effect for class %d."), static_cast<uint8>(CharacterClass)))
	{
		// Primary attributes are the class-specific branch of the setup flow.
		const FGameplayEffectSpecHandle PrimarySpecHandle = ASC->MakeOutgoingSpec(ClassInfoEntry.PrimaryAttributeEffect, Level, PrimaryEffectContextHandle);
		if (ensureMsgf(PrimarySpecHandle.IsValid(), TEXT("AuraAbilitySystemLibrary::InitializeDefaultAttributes failed to build the primary attribute spec for class %d."), static_cast<uint8>(CharacterClass)))
		{
			ASC->ApplyGameplayEffectSpecToSelf(*PrimarySpecHandle.Data.Get());
		}
	}

	FGameplayEffectContextHandle SecondaryEffectContextHandle = ASC->MakeEffectContext();
	SecondaryEffectContextHandle.AddSourceObject(AvatarActor);
	if (ensureMsgf(ClassInfos->SecondaryAttributes, TEXT("AuraAbilitySystemLibrary::InitializeDefaultAttributes is missing the shared secondary attribute effect.")))
	{
		// Secondary attributes are shared by every class, so they come from the common asset fields.
		const FGameplayEffectSpecHandle SecondarySpecHandle = ASC->MakeOutgoingSpec(ClassInfos->SecondaryAttributes, Level, SecondaryEffectContextHandle);
		if (ensureMsgf(SecondarySpecHandle.IsValid(), TEXT("AuraAbilitySystemLibrary::InitializeDefaultAttributes failed to build the secondary attribute spec.")))
		{
			ASC->ApplyGameplayEffectSpecToSelf(*SecondarySpecHandle.Data.Get());
		}
	}

	FGameplayEffectContextHandle VitalEffectContextHandle = ASC->MakeEffectContext();
	VitalEffectContextHandle.AddSourceObject(AvatarActor);
	if (ensureMsgf(ClassInfos->VitalAttributes, TEXT("AuraAbilitySystemLibrary::InitializeDefaultAttributes is missing the shared vital attribute effect.")))
	{
		// Vital attributes are last so the final health / mana caps reflect any earlier startup effects.
		const FGameplayEffectSpecHandle VitalSpecHandle = ASC->MakeOutgoingSpec(ClassInfos->VitalAttributes, Level, VitalEffectContextHandle);
		if (ensureMsgf(VitalSpecHandle.IsValid(), TEXT("AuraAbilitySystemLibrary::InitializeDefaultAttributes failed to build the vital attribute spec.")))
		{
			ASC->ApplyGameplayEffectSpecToSelf(*VitalSpecHandle.Data.Get());
		}
	}
}

void UAuraAbilitySystemLibrary::InitializeDefaultAbilities(const UObject* WorldContextObject, ECharacterClass CharacterClass, UAbilitySystemComponent* ASC)
{
	check(ASC);

	AActor* AvatarActor = ASC->GetAvatarActor();
	if (!ensureMsgf(AvatarActor, TEXT("AuraAbilitySystemLibrary::InitialzeDefaultAbilities requires ASC %s to have a valid avatar actor."), *GetNameSafe(ASC)))
	{
		return;
	}

	// Ability grants are authoritative gameplay state, so make the rule explicit instead of relying
	// on the server-only GameMode lookup to fail on clients.
	if (!AvatarActor->HasAuthority())
	{
		return;
	}

	UCharacterClassInfo* CharacterClassInfo = GetCharacterClassInfo(WorldContextObject);
	check(CharacterClassInfo);
	for (TSubclassOf<UGameplayAbility> AbilityClass : CharacterClassInfo->CommonAbilities)
	{
		// These are the shared "always available" combat abilities that every spawned combatant
		// should own before moment-to-moment game play begins.
		FGameplayAbilitySpec AbilitySpec = FGameplayAbilitySpec(AbilityClass, 1);
		ASC->GiveAbility(AbilitySpec);
	}

	const FCharacterClassDefaultInfo& DefaultInfo = CharacterClassInfo->GetDefaultInfoForClass(CharacterClass);
	for (TSubclassOf<UGameplayAbility> AbilityClass : DefaultInfo.ClassUniqueAbilities)
	{
		int32 PlayerLevel = 1;
		check(AvatarActor->Implements<UCombatInterface>());

		PlayerLevel = ICombatInterface::Execute_GetPlayerLevel(AvatarActor);
		FGameplayAbilitySpec AbilitySpec = FGameplayAbilitySpec(AbilityClass, PlayerLevel);
		ASC->GiveAbility(AbilitySpec);
	}
}

UCharacterClassInfo* UAuraAbilitySystemLibrary::GetCharacterClassInfo(const UObject* WorldContextObject)
{
	// CharacterClassInfo lives on GameMode because it is server-authored setup data; callers that
	// run on clients should expect this lookup to return nullptr.
	const AAuraGameModeBase* AuraGameMode = Cast<AAuraGameModeBase>(UGameplayStatics::GetGameMode(WorldContextObject));
	if (!AuraGameMode)
	{
		return nullptr;
	}

	return AuraGameMode->CharacterClassInfo;
}

UAbilityInfo* UAuraAbilitySystemLibrary::GetAbilityInfo(const UObject* WorldContextObject)
{
	const UAuraGameInstance* AuraGameInstance = Cast<UAuraGameInstance>(UGameplayStatics::GetGameInstance(WorldContextObject));
	if (!AuraGameInstance)
	{
		return nullptr;
	}

	return AuraGameInstance->AbilityInfo;
}

FGameplayEffectContextHandle UAuraAbilitySystemLibrary::ApplyDamageEffect(const FDamageEffectParameters& DamageParameters)
{
	check(DamageParameters.SourceAbilitySystemComponent);
	check(DamageParameters.TargetAbilitySystemComponent);
	check(DamageParameters.DamageGameplayEffectClass);

	/*
	 * Shared damage-spec authoring:
	 *   1. Source ASC creates the effect context so GAS knows the instigator/source object.
	 *   2. Ability-authored values are written as set-by-caller magnitudes. ExecCalc_Damage reads
	 *      the same tag keys later, which keeps abilities data-driven instead of hard-coded.
	 *   3. The finished spec is applied to the target ASC. The target AttributeSet consumes the
	 *      resulting IncomingDamage and any debuff metadata.
	 */
	const FAuraGameTagManager& TagManager = FAuraGameTagManager::Get();
	FGameplayEffectContextHandle ContextHandle = DamageParameters.SourceAbilitySystemComponent->MakeEffectContext();
	ContextHandle.AddSourceObject(DamageParameters.SourceAbilitySystemComponent->GetAvatarActor());

	const FGameplayEffectSpecHandle DamageEffectSpecHandle = DamageParameters.SourceAbilitySystemComponent->MakeOutgoingSpec(
		DamageParameters.DamageGameplayEffectClass,
		DamageParameters.AbilityLevel,
		ContextHandle);
	SetDeathImpulse(ContextHandle, DamageParameters.DeathImpulse);
	SetKnockBackForce(ContextHandle, DamageParameters.KnockBackForce);
	SetIsRadialDamage(ContextHandle, DamageParameters.bIsRadialDamage);
	SetRadialDamageInnerRadius(ContextHandle, DamageParameters.RadialDamageInnerRadius);
	SetRadialDamageOuterRadius(ContextHandle, DamageParameters.RadialDamageOuterRadius);
	SetRadialDamageOrigin(ContextHandle, DamageParameters.RadialDamageOrigin);
	
	UAbilitySystemBlueprintLibrary::AssignTagSetByCallerMagnitude(DamageEffectSpecHandle, TagManager.Debuff_Damage, DamageParameters.DebuffDamage);
	UAbilitySystemBlueprintLibrary::AssignTagSetByCallerMagnitude(DamageEffectSpecHandle, DamageParameters.DamageType, DamageParameters.BaseDamage);
	UAbilitySystemBlueprintLibrary::AssignTagSetByCallerMagnitude(DamageEffectSpecHandle, TagManager.Debuff_Chance, DamageParameters.DebuffChance);
	UAbilitySystemBlueprintLibrary::AssignTagSetByCallerMagnitude(DamageEffectSpecHandle, TagManager.Debuff_Frequency, DamageParameters.DebuffFrequency);
	UAbilitySystemBlueprintLibrary::AssignTagSetByCallerMagnitude(DamageEffectSpecHandle, TagManager.Debuff_Duration, DamageParameters.DebuffDuration);

	DamageParameters.SourceAbilitySystemComponent->ApplyGameplayEffectSpecToTarget(*DamageEffectSpecHandle.Data.Get(), DamageParameters.TargetAbilitySystemComponent);

	return ContextHandle;
}

bool UAuraAbilitySystemLibrary::IsBlockedHit(const FGameplayEffectContextHandle& EffectHandle)
{
	if (const FAuraGameplayEffectContext* AuraContext = GetAuraEffectContext(EffectHandle, TEXT("AuraAbilitySystemLibrary::IsBlockedHit")))
	{
		return AuraContext->IsBlockedHit();
	}

	return false;
}

bool UAuraAbilitySystemLibrary::IsCriticalHit(const FGameplayEffectContextHandle& EffectHandle)
{
	if (const FAuraGameplayEffectContext* AuraContext = GetAuraEffectContext(EffectHandle, TEXT("AuraAbilitySystemLibrary::IsCriticalHit")))
	{
		return AuraContext->IsCriticalHit();
	}

	return false;
}

bool UAuraAbilitySystemLibrary::IsRadialDamage(const FGameplayEffectContextHandle& EffectHandle)
{
	if (const FAuraGameplayEffectContext* AuraContext = GetAuraEffectContext(EffectHandle, TEXT("AuraAbilitySystemLibrary::IsCriticalHit")))
	{
		return AuraContext->GetIsRadialDamage();
	}
	
	return false;
}

float UAuraAbilitySystemLibrary::GetRadialDamageInnerRadius(const FGameplayEffectContextHandle& EffectHandle)
{
	if (const FAuraGameplayEffectContext* AuraContext = GetAuraEffectContext(EffectHandle, TEXT("AuraAbilitySystemLibrary::IsCriticalHit")))
	{
		return AuraContext->GetRadialDamageInnerRadius();
	}
	return  0.0f;
}

float UAuraAbilitySystemLibrary::GetRadialDamageOuterRadius(const FGameplayEffectContextHandle& EffectHandle)
{
	if (const FAuraGameplayEffectContext* AuraContext = GetAuraEffectContext(EffectHandle, TEXT("AuraAbilitySystemLibrary::IsCriticalHit")))
	{
		return AuraContext->GetRadialDamageOuterRadius();
	}
	return  0.0f;
}

FVector UAuraAbilitySystemLibrary::GetRadialDamageOrigin(const FGameplayEffectContextHandle& EffectHandle)
{
	if (const FAuraGameplayEffectContext* AuraContext = GetAuraEffectContext(EffectHandle, TEXT("AuraAbilitySystemLibrary::IsCriticalHit")))
	{
		return AuraContext->GetRadialDamageOrigin();
	}
	return  FVector::ZeroVector;
}

bool UAuraAbilitySystemLibrary::ShouldHitReact(UPARAM(ref)FGameplayEffectContextHandle& EffectHandle)
{
	if (const FAuraGameplayEffectContext* AuraContext = GetAuraEffectContext(EffectHandle, TEXT("AuraAbilitySystemLibrary::ShouldHitReact")))
	{
		return AuraContext->ShouldHitReact();
	}

	return false;
}

bool UAuraAbilitySystemLibrary::IsSuccessfulDebuff(FGameplayEffectContextHandle& EffectHandle)
{
	if (const FAuraGameplayEffectContext* AuraContext = GetAuraEffectContext(EffectHandle, TEXT("AuraAbilitySystemLibrary::IsSuccessfulDebuff")))
	{
		return AuraContext->IsSuccessfulDebuff();
	}

	return false;
}

float UAuraAbilitySystemLibrary::GetDebuffDamage(FGameplayEffectContextHandle& EffectHandle)
{
	if (const FAuraGameplayEffectContext* AuraContext = GetAuraEffectContext(EffectHandle, TEXT("AuraAbilitySystemLibrary::GetDebuffDamage")))
	{
		return AuraContext->GetDebuffDamage();
	}

	return 0.f;
}

float UAuraAbilitySystemLibrary::GetDebuffDuration(FGameplayEffectContextHandle& EffectHandle)
{
	if (const FAuraGameplayEffectContext* AuraContext = GetAuraEffectContext(EffectHandle, TEXT("AuraAbilitySystemLibrary::GetDebuffDuration")))
	{
		return AuraContext->GetDebuffDuration();
	}

	return 0.f;
}

float UAuraAbilitySystemLibrary::GetDebuffFrequency(FGameplayEffectContextHandle& EffectHandle)
{
	if (const FAuraGameplayEffectContext* AuraContext = GetAuraEffectContext(EffectHandle, TEXT("AuraAbilitySystemLibrary::GetDebuffFrequency")))
	{
		return AuraContext->GetDebuffFrequency();
	}

	return 0.f;
}

FGameplayTag UAuraAbilitySystemLibrary::GetDamageTypeTag(FGameplayEffectContextHandle& EffectHandle)
{
	if (const FAuraGameplayEffectContext* AuraContext = GetAuraEffectContext(EffectHandle, TEXT("AuraAbilitySystemLibrary::GetDamageTypeTag")))
	{
		if (AuraContext->GetDamageTypeTag().IsValid())
		{
			return *AuraContext->GetDamageTypeTag();
		}
	}

	return FGameplayTag();
}

FVector UAuraAbilitySystemLibrary::GetDeathImpulse(FGameplayEffectContextHandle& EffectHandle)
{
	if (const FAuraGameplayEffectContext* AuraContext = GetAuraEffectContext(EffectHandle, TEXT("AuraAbilitySystemLibrary::GetDeathImpulse")))
	{
		return AuraContext->GetDeathImpulse();
	}
	return FVector::ZeroVector;
}

FVector UAuraAbilitySystemLibrary::GetKnockBackForce(FGameplayEffectContextHandle& EffectHandle)
{
	if (const FAuraGameplayEffectContext* AuraContext = GetAuraEffectContext(EffectHandle, TEXT("AuraAbilitySystemLibrary::GetKnockBackForce")))
	{
		return AuraContext->GetKnockBackForce();
	}
	return FVector::ZeroVector;
}

void UAuraAbilitySystemLibrary::SetIsBlockedHit(UPARAM(ref)FGameplayEffectContextHandle& EffectHandle, bool bInIsBlocked)
{
	if (FAuraGameplayEffectContext* AuraContext = GetMutableAuraEffectContext(EffectHandle, TEXT("AuraAbilitySystemLibrary::SetIsBlockedHit")))
	{
		AuraContext->SetBlockedHit(bInIsBlocked);
	}
}

void UAuraAbilitySystemLibrary::SetIsCriticalHit(UPARAM(ref)FGameplayEffectContextHandle& EffectHandle, bool bInIsCritical)
{
	if (FAuraGameplayEffectContext* AuraContext = GetMutableAuraEffectContext(EffectHandle, TEXT("AuraAbilitySystemLibrary::SetIsCriticalHit")))
	{
		AuraContext->SetCriticalHit(bInIsCritical);
	}
}

void UAuraAbilitySystemLibrary::SetShouldHitReact(UPARAM(ref)FGameplayEffectContextHandle& EffectHandle, bool bInShouldHitReact)
{
	if (FAuraGameplayEffectContext* AuraContext = GetMutableAuraEffectContext(EffectHandle, TEXT("AuraAbilitySystemLibrary::SetShouldHitReact")))
	{
		AuraContext->SetShouldHitReact(bInShouldHitReact);
	}
}

void UAuraAbilitySystemLibrary::SetSuccessfulDebuff(FGameplayEffectContextHandle& EffectHandle, bool bInIsSuccessful)
{
	if (FAuraGameplayEffectContext* AuraContext = GetMutableAuraEffectContext(EffectHandle, TEXT("AuraAbilitySystemLibrary::SetSuccessfulDebuff")))
	{
		AuraContext->SetSuccessfulDebuff(bInIsSuccessful);
	}
}

void UAuraAbilitySystemLibrary::SetDebuffDamage(FGameplayEffectContextHandle& EffectHandle, float InDamage)
{
	if (FAuraGameplayEffectContext* AuraContext = GetMutableAuraEffectContext(EffectHandle, TEXT("AuraAbilitySystemLibrary::SetDebuffDamage")))
	{
		AuraContext->SetDebuffDamage(InDamage);
	}
}

void UAuraAbilitySystemLibrary::SetDebuffDuration(FGameplayEffectContextHandle& EffectHandle, float InDuration)
{
	if (FAuraGameplayEffectContext* AuraContext = GetMutableAuraEffectContext(EffectHandle, TEXT("AuraAbilitySystemLibrary::SetDebuffDuration")))
	{
		AuraContext->SetDebuffDuration(InDuration);
	}
}

void UAuraAbilitySystemLibrary::SetDebuffFrequency(FGameplayEffectContextHandle& EffectHandle, float InDebuffFrequency)
{
	if (FAuraGameplayEffectContext* AuraContext = GetMutableAuraEffectContext(EffectHandle, TEXT("AuraAbilitySystemLibrary::SetDebuffFrequency")))
	{
		AuraContext->SetDebuffFrequency(InDebuffFrequency);
	}
}

void UAuraAbilitySystemLibrary::SetDamageTypeTag(FGameplayEffectContextHandle& EffectHandle,
	const FGameplayTag& InDamageTypeTag)
{
	FAuraGameplayEffectContext* AuraContext = GetMutableAuraEffectContext(EffectHandle, TEXT("AuraAbilitySystemLibrary::SetDamageTypeTag"));
	if (AuraContext)
	{
		AuraContext->SetDamageTypeTag(InDamageTypeTag);
	}
}

void UAuraAbilitySystemLibrary::SetDeathImpulse(FGameplayEffectContextHandle& EffectHandle, const FVector& InDeathImpulse)
{
	if (FAuraGameplayEffectContext* AuraContext = GetMutableAuraEffectContext(EffectHandle, TEXT("AuraAbilitySystemLibrary::SetDeathImpulse")))
	{
		AuraContext->SetDeathImpulse(InDeathImpulse);
	}
}

void UAuraAbilitySystemLibrary::SetKnockBackForce(FGameplayEffectContextHandle& EffectHandle,
	const FVector& InKnockBackForce)
{
	if (FAuraGameplayEffectContext* AuraContext = GetMutableAuraEffectContext(EffectHandle, TEXT("AuraAbilitySystemLibrary::SetKnockBackForce")))
	{
		AuraContext->SetKnockBackForce(InKnockBackForce);
	}
}

void UAuraAbilitySystemLibrary::SetIsRadialDamage(FGameplayEffectContextHandle& EffectHandle, bool bInIsRadialDamage)
{
	if (FAuraGameplayEffectContext* AuraContext = GetMutableAuraEffectContext(EffectHandle, TEXT("AuraAbilitySystemLibrary::SetIsRadialDamage")))
	{
		AuraContext->SetIsRadialDamage(bInIsRadialDamage);
	}
}

void UAuraAbilitySystemLibrary::SetRadialDamageInnerRadius(FGameplayEffectContextHandle& EffectHandle,
                                                           float InInnerRadius)
{
	if (FAuraGameplayEffectContext* AuraContext = GetMutableAuraEffectContext(EffectHandle, TEXT("AuraAbilitySystemLibrary::SetKnockBackForce")))
	{
		AuraContext->SetRadialDamageInnerRadius(InInnerRadius);
	}
}

void UAuraAbilitySystemLibrary::SetRadialDamageOuterRadius(FGameplayEffectContextHandle& EffectHandle,
	float InOuterRadius)
{
	if (FAuraGameplayEffectContext* AuraContext = GetMutableAuraEffectContext(EffectHandle, TEXT("AuraAbilitySystemLibrary::SetKnockBackForce")))
	{
		AuraContext->SetRadialDamageOuterRadius(InOuterRadius);
	}
}

void UAuraAbilitySystemLibrary::SetRadialDamageOrigin(FGameplayEffectContextHandle& EffectHandle,
	const FVector& InOrigin)
{
	FAuraGameplayEffectContext* AuraContext = GetMutableAuraEffectContext(EffectHandle, TEXT("AuraAbilitySystemLibrary::SetRadialDamageOrigin"));
		if (AuraContext)
		{
			AuraContext->SetRadialDamageOrigin(InOrigin);
		}
}

void UAuraAbilitySystemLibrary::GetLivePlayersWithinRadius(const UObject* WorldContextObject, TArray<AActor*>& OutOverlapActors, const TArray<AActor*>& ActorToIgnore, float Radius, const FVector& SphereOrigin)
{
	FCollisionQueryParams SphereParams;
	SphereParams.AddIgnoredActors(ActorToIgnore);

	TArray<FOverlapResult> Overlaps;
	if (const UWorld* World = GEngine->GetWorldFromContextObject(WorldContextObject, EGetWorldErrorMode::LogAndReturnNull))
	{
		World->OverlapMultiByObjectType(Overlaps, SphereOrigin, FQuat::Identity, FCollisionObjectQueryParams(FCollisionObjectQueryParams::InitType::AllDynamicObjects), FCollisionShape::MakeSphere(Radius), SphereParams);

		for (FOverlapResult& Overlap : Overlaps)
		{
			AActor* OverlapActor = Overlap.GetActor();
			const bool bImplementCombatInterface = OverlapActor->Implements<UCombatInterface>();

			if (bImplementCombatInterface && !ICombatInterface::Execute_IsDead(OverlapActor))
			{
				OutOverlapActors.AddUnique(OverlapActor);
			}
		}
	}
}

void UAuraAbilitySystemLibrary::GetClosestTargets(
	int32 MaxTargets,
	const TArray<AActor*>& Actors,
	TArray<AActor*>& OutClosestTargets,
	const FVector& Origin)
{
	OutClosestTargets.Reset();

	if (MaxTargets <= 0 || Actors.Num() == 0)
	{
		return;
	}

	if (MaxTargets >= Actors.Num())
	{
		OutClosestTargets = Actors;
		return;
	}
	
	// Maintain a max heap of size MaxTargets.
	// Scans all actors and keeps only the closest MaxTargets.
	// Time: O(N * log(MaxTargets))
	// Space: O(MaxTargets)
	TArray<AActor*> MaxHeap;
	MaxHeap.Reserve(MaxTargets);
	
	auto Predicate_IsFarther =  [&Origin](const AActor& Actor, const AActor& OtherActor)
	{
		const float DistanceSq = FVector::DistSquared(Actor.GetActorLocation(), Origin);
		const float OtherDistanceSq = FVector::DistSquared(OtherActor.GetActorLocation(), Origin);
		return DistanceSq > OtherDistanceSq;
	};
	
	for (AActor* Actor : Actors)
	{
		if (!Actor) continue;
		
		if (MaxHeap.Num() < MaxTargets)
		{
			MaxHeap.HeapPush(Actor, Predicate_IsFarther);
		}else if (!Predicate_IsFarther(*Actor, *MaxHeap.HeapTop()))
		{
			MaxHeap.HeapPopDiscard(Predicate_IsFarther,EAllowShrinking::No);
			MaxHeap.HeapPush(Actor, Predicate_IsFarther);
		}
	}

	OutClosestTargets.Reserve(MaxTargets);
	OutClosestTargets.Append(MaxHeap.GetData(), MaxTargets);
}

void  UAuraAbilitySystemLibrary::CalculateRadialDamage(const FGameplayEffectContextHandle& EffectContextHandle,
	 float& OutDamage, const AActor* TargetActor)
{
	check(OutDamage > 0.f);
	FVector TargetLocation = TargetActor->GetActorLocation(); 
	const FVector Origin = UAuraAbilitySystemLibrary::GetRadialDamageOrigin(EffectContextHandle);
	const float InnerRadius = UAuraAbilitySystemLibrary::GetRadialDamageInnerRadius(EffectContextHandle);
	const float OuterRadius = UAuraAbilitySystemLibrary::GetRadialDamageOuterRadius(EffectContextHandle);
	TargetLocation.Z = Origin.Z;  // 
	
	const float SquaredDistance = FVector::DistSquared(Origin, TargetLocation);
	const float InnerRadiusSq = InnerRadius * InnerRadius;
	const float OuterRadiusSq = OuterRadius * OuterRadius;
	
	if (SquaredDistance <= InnerRadiusSq)
	{
		return;
	}
	
	const TRange<float> DistanceRange = TRange<float>(InnerRadiusSq, OuterRadiusSq);
	const TRange<float> DamageScaledRange(1.0f, 0.f);
	const float DamageScale = FMath::GetMappedRangeValueClamped(DistanceRange, DamageScaledRange, SquaredDistance);
	OutDamage *= DamageScale;
}

const FTaggedMontage& UAuraAbilitySystemLibrary::GetRandomMontageInArray(const TArray<FTaggedMontage>& MontageArray)
{
	check(MontageArray.Num() > 0);

	const int32 RandomIndex = FMath::RandRange(0, MontageArray.Num() - 1);
	return MontageArray[RandomIndex];
}

bool UAuraAbilitySystemLibrary::IsNotFriend(AActor* FirstActor, AActor* SecondActor)
{
	check(FirstActor && SecondActor);
	const bool bFirstIsPlayer = FirstActor->ActorHasTag("Player");
	const bool bSecondIsPlayer = SecondActor->ActorHasTag("Player");

	const bool bIsFriend = (bFirstIsPlayer && bSecondIsPlayer) || (!bFirstIsPlayer && !bSecondIsPlayer);

	return !bIsFriend;
}

int32 UAuraAbilitySystemLibrary::GetXPRewardForClassAndLevel(const UObject* WorldContextObject, ECharacterClass CharacterClass, int32 CharacterLevel)
{
	const UCharacterClassInfo* CharacterClassInfo = GetCharacterClassInfo(WorldContextObject);
	check(CharacterClassInfo);

	const FCharacterClassDefaultInfo CharacterClassDefaultInfo = CharacterClassInfo->GetDefaultInfoForClass(CharacterClass);
	const float XPReward = CharacterClassDefaultInfo.XPReward.GetValueAtLevel(CharacterLevel);
	return FMath::RoundToInt(XPReward);
}

TArray<FRotator> UAuraAbilitySystemLibrary::EvenlySpacedRotators(const FVector& Forward, const FVector& Axis, float Spread, int32 NumRotators)
{
	TArray<FRotator> Rotators;
	if (NumRotators <= 1)
	{
		Rotators.Add(Forward.Rotation());
		return Rotators;
	}
	
	if (NumRotators > 1)
	{
		const FVector LeftOfSpread =  Forward.RotateAngleAxis(-Spread / 2.0f, FVector::UpVector);
		const float SpreadDelta = Spread / (NumRotators - 1);
		
		for (int32 i = 0; i < NumRotators; ++i)
		{
			const FRotator Rotator = LeftOfSpread.RotateAngleAxis(SpreadDelta * i, Axis).Rotation();
			Rotators.Add(Rotator);
		}
	}
	return Rotators;
}

TArray<FVector> UAuraAbilitySystemLibrary::EvenlyRotatedVectors(const FVector& Forward, const FVector& Axis,  float Spread, int32 NumVectors)
{
	TArray<FVector> Vectors;
	if (NumVectors <= 1)
	{
		Vectors.Add(Forward);
		return Vectors;
	}
	
	if (NumVectors > 1)
	{
		const FVector LeftOfSpread =  Forward.RotateAngleAxis(-Spread / 2.0f, FVector::UpVector);
		const float SpreadDelta = Spread / (NumVectors - 1);
		
		for (int32 i = 0; i < NumVectors; ++i)
		{
			const FVector Vector = LeftOfSpread.RotateAngleAxis(SpreadDelta * i, Axis);
			Vectors.Add(Vector);
		}
	}
	return Vectors;
}

bool UAuraAbilitySystemLibrary::MakeWidgetControllerParameters(const UObject* WorldContextObject, FWidgetControllerParameters& OutParams, AAuraHUD*& OutAuraHUD)
{
	check(WorldContextObject);

	check(GEngine);
	APlayerController* LocalPlayerController = GEngine->GetFirstLocalPlayerController(WorldContextObject->GetWorld());

	if (!LocalPlayerController)
	{
		return false;
	}

	OutAuraHUD = Cast<AAuraHUD>(LocalPlayerController->GetHUD());
	AAuraPlayerState* PS = LocalPlayerController->GetPlayerState<AAuraPlayerState>();
	if (!OutAuraHUD || !PS)
	{
		return false;
	}

	UAuraAbilitySystemComponent* ASC = Cast<UAuraAbilitySystemComponent>(PS->GetAbilitySystemComponent());
	UAuraAttributeSet* AttributeSet = Cast<UAuraAttributeSet>(PS->GetAttributeSet());
	if (!ASC || !AttributeSet)
	{
		return false;
	}

	OutParams = FWidgetControllerParameters(LocalPlayerController, PS, ASC, AttributeSet);
	return true;
}
