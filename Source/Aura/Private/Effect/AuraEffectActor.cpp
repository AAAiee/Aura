// @Copyright HaolunYuan


#include "Effect/AuraEffectActor.h"

#include "AbilitySystemBlueprintLibrary.h"
#include "AbilitySystemComponent.h"
#include "Components/AbilitySystem/AuraAbilitySystemLibrary.h"
#include "Components/SceneComponent.h"

AAuraEffectActor::AAuraEffectActor()
{
	PrimaryActorTick.bCanEverTick = false;

	// Overlap components (Sphere, Box, etc.) are added in the Blueprint subclass.
	SetRootComponent(CreateDefaultSubobject<USceneComponent>("SceneRoot"));
}

void AAuraEffectActor::BeginPlay()
{
	Super::BeginPlay();
}

/**
 * Applies a Gameplay Effect to the target actor.
 *
 * Flow:
 *   1. Server-only check - GEs must be applied authoritatively to avoid desync.
 *   2. Get the target's ASC (safe method that doesn't assume IAbilitySystemInterface).
 *   3. Create an Effect Context (carries metadata like "who caused this effect").
 *   4. Create an Effect Spec from the GE class + level + context.
 *   5. Apply the spec to the target.
 *   6. If the effect is Infinite AND removable, store the handle for later removal.
 */
void AAuraEffectActor::ApplyEffectToTarget(AActor* TargetActor, TSubclassOf<UGameplayEffect> GameplayEffectClass)
{
	// GEs should only be applied on the server to stay authoritative.
	if (!HasAuthority())
	{
		return;
	}

	// UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent is safer than casting to
	// IAbilitySystemInterface - it works even if the actor doesn't implement the interface.
	UAbilitySystemComponent* TargetAbilitySystemComponent = UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(TargetActor);

	if (TargetAbilitySystemComponent == nullptr)
	{
		return;
	}
	check(GameplayEffectClass);

	// Effect Context - stores "source" info (this actor) so PostGameplayEffectExecute
	// can trace back who caused the effect (useful for damage credits, VFX origin, etc.)
	FGameplayEffectContextHandle EffectContextHandle = TargetAbilitySystemComponent->MakeEffectContext();
	UAuraAbilitySystemLibrary::SetShouldHitReact(EffectContextHandle, false);

	// Effect Spec - combines the GE class + level + context into an applicable package
	FGameplayEffectSpecHandle EffectSpecHandle = TargetAbilitySystemComponent->MakeOutgoingSpec(GameplayEffectClass, Level, EffectContextHandle);

	// Apply and keep the handle so removable Infinite effects can be cleaned up later.
	FActiveGameplayEffectHandle ActiveHandle = TargetAbilitySystemComponent->ApplyGameplayEffectSpecToSelf(*EffectSpecHandle.Data.Get());

	/* Track Infinite effects that need removal on end overlap.
	 * We store the ASC pointer -> array of handles so OnEndOverlap can remove exactly
	 * the effects this actor applied (not effects from other sources). */
	const UGameplayEffect* GamePlayEffect = EffectSpecHandle.Data.Get()->Def.Get();
	const bool bIsInfiniteEffect = GamePlayEffect->DurationPolicy == EGameplayEffectDurationType::Infinite;
	const bool bIsRemovable = InfiniteEffectRemovalPolicy == EEffectRemovalPolicy::RemoveOnEndOverlap;
	if (bIsInfiniteEffect && bIsRemovable)
	{
		TArray<FActiveGameplayEffectHandle>& GameEffectsPool = AppliedEffects.FindOrAdd(TargetAbilitySystemComponent);
		GameEffectsPool.Add(ActiveHandle);
	}
}

/**
 * Called from Blueprint when an actor enters the overlap volume.
 * Checks each effect type's application policy and applies if set to ApplyOnOverlap.
 */
void AAuraEffectActor::OnOverlap(AActor* TargetActor)
{
	if (InstantEffectApplicationPolicy == EEffectApplicationPolicy::ApplyOnOverlap)
	{
		ApplyEffectToTarget(TargetActor, InstantGameplayEffectClass);
	}

	if (DurationEffectApplicationPolicy == EEffectApplicationPolicy::ApplyOnOverlap)
	{
		ApplyEffectToTarget(TargetActor, DurationGameplayEffectClass);
	}

	if (InfiniteEffectApplicationPolicy == EEffectApplicationPolicy::ApplyOnOverlap)
	{
		ApplyEffectToTarget(TargetActor, InfiniteGameplayEffectClass);
	}
}

/**
 * Called from Blueprint when an actor leaves the overlap volume.
 *
 * Two responsibilities:
 *   1. Apply effects that are configured for ApplyOnEndOverlap.
 *   2. Remove tracked Infinite effects if RemoveOnEndOverlap is set.
 *      After removal, clean up the TMap entry to prevent stale handles from accumulating.
 */
void AAuraEffectActor::OnEndOverlap(AActor* TargetActor)
{
	if (InstantEffectApplicationPolicy == EEffectApplicationPolicy::ApplyOnEndOverlap)
	{
		ApplyEffectToTarget(TargetActor, InstantGameplayEffectClass);
	}

	if (DurationEffectApplicationPolicy == EEffectApplicationPolicy::ApplyOnEndOverlap)
	{
		ApplyEffectToTarget(TargetActor, DurationGameplayEffectClass);
	}

	if (InfiniteEffectApplicationPolicy == EEffectApplicationPolicy::ApplyOnEndOverlap)
	{
		ApplyEffectToTarget(TargetActor, InfiniteGameplayEffectClass);
	}

	// Remove tracked Infinite effects and clean up the map entry.
	if (InfiniteEffectRemovalPolicy == EEffectRemovalPolicy::RemoveOnEndOverlap)
	{
		UAbilitySystemComponent* TargetASC = UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(TargetActor);
		if (!IsValid(TargetASC))
		{
			return;
		}

		TArray<FActiveGameplayEffectHandle>* TargetAppliedInfiniteRemovalEffectsPool = AppliedEffects.Find(TargetASC);
		if (!TargetAppliedInfiniteRemovalEffectsPool)
		{
			return;
		}

		for (const FActiveGameplayEffectHandle& Handles : *TargetAppliedInfiniteRemovalEffectsPool)
		{
			TargetASC->RemoveActiveGameplayEffect(Handles, 1);
		}

		// Clean up the map entry so stale handles do not interfere with future overlaps.
		AppliedEffects.Remove(TargetASC);
	}
}
