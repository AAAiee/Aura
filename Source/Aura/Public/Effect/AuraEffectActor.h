// @Copyright HaolunYuan

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "GameplayEffectTypes.h"
#include "ActiveGameplayEffectHandle.h"
#include "AuraEffectActor.generated.h"

class USphereComponent;
class UStaticMeshComponent;
class UPrimitiveComponent;
struct FHitResult;

/**
 * Controls WHEN a Gameplay Effect is applied to a target.
 * Set per-effect type (Instant, Duration, Infinite) in the editor.
 */
UENUM(BlueprintType)
enum class EEffectApplicationPolicy:uint8
{
	ApplyOnOverlap,     // Apply the GE when the target enters the overlap volume
	ApplyOnEndOverlap,  // Apply the GE when the target leaves the overlap volume
	DoNotApply          // This effect type is not used on this actor
};

/**
 * Controls WHEN an Infinite effect is removed from a target.
 * Only relevant for Infinite effects ！ Instant and Duration handle their own lifecycle.
 */
UENUM(BlueprintType)
enum class EEffectRemovalPolicy: uint8
{
	RemoveOnEndOverlap, // Remove the Infinite GE when the target leaves the overlap volume
	DoNotRemove         // The Infinite GE persists forever (must be removed by other means)
};


/**
 * A world-placed actor that applies Gameplay Effects to overlapping actors.
 *
 * Supports three GE duration types, each with its own application policy:
 *   - Instant:  applied once, modifies attributes immediately (e.g., health pickup)
 *   - Duration: applied once, lasts for a set time, then auto-removes (e.g., speed buff)
 *   - Infinite: applied and tracked ！ must be manually removed (e.g., area slow field)
 *
 * Overlap events are NOT bound in C++ ！ they are connected in the Blueprint subclass
 * to OnOverlap/OnEndOverlap (BlueprintCallable). This gives designers flexibility
 * to use any collision component (Sphere, Box, etc.).
 *
 * Infinite effects require special tracking: the TMap stores each target's ASC ★ active handles
 * so we can remove exactly the right effects when the target leaves the volume.
 */
UCLASS()
class AURA_API AAuraEffectActor : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	AAuraEffectActor();

protected:
	virtual void BeginPlay() override;

	/**
	 * Core GE application logic. Creates a GE spec, applies it to the target, and
	 * tracks the handle if the effect is Infinite + RemoveOnEndOverlap.
	 * Server-only (HasAuthority check) ！ GEs should only be applied authoritatively.
	 */
	UFUNCTION(BlueprintCallable)
	void ApplyEffectToTarget(AActor* TargetActor, TSubclassOf<UGameplayEffect> GameplayEffectClass);

	/** Called from Blueprint when an actor enters the overlap volume. */
	UFUNCTION(BlueprintCallable)
	void OnOverlap(AActor* TargetActor);

	/** Called from Blueprint when an actor leaves the overlap volume. */
	UFUNCTION(BlueprintCallable)
	void OnEndOverlap(AActor* TargetActor);

private:

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Applied Effects", meta = (AllowPrivateAccess = true))
	bool bDestoryOnEffectRemoval = false;

	/** GE level ！ passed to MakeOutgoingSpec. Controls magnitude scaling in the GE curve tables. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Applied Effects", meta = (AllowPrivateAccess = true))
	float Level = 1.0f;

	/**
	 * Tracks active Infinite effect handles per target ASC.
	 * Key: target's ASC pointer.  Value: array of active GE handles applied by this actor.
	 * Cleaned up in OnEndOverlap after removal to prevent stale handles from accumulating.
	 */
	TMap<UAbilitySystemComponent*, TArray<FActiveGameplayEffectHandle>> AppliedEffects;

	/*Instant Effect ！ applied once, modifies attributes immediately*/
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Applied Effect", meta = (AllowPrivateAccess = true))
	TSubclassOf<UGameplayEffect> InstantGameplayEffectClass;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Applied Effect", meta = (AllowPrivateAccess = true))
	EEffectApplicationPolicy InstantEffectApplicationPolicy = EEffectApplicationPolicy::DoNotApply;

	/*Duration Effect ！ lasts for a set time, then auto-removes*/
	UPROPERTY(EditAnywhere,BlueprintReadOnly, Category ="Applied Effect", meta = (AllowPrivateAccess = true))
	TSubclassOf<UGameplayEffect> DurationGameplayEffectClass;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Applied Effect", meta = (AllowPrivateAccess = true))
	EEffectApplicationPolicy DurationEffectApplicationPolicy = EEffectApplicationPolicy::DoNotApply;

	/*Infinite Effect ！ persists until manually removed*/
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category= "Applied Effect", meta = (AllowPrivateAccess = true))
	TSubclassOf<UGameplayEffect> InfiniteGameplayEffectClass;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category= "Applied Effect", meta = (AllowPrivateAccess = true))
	EEffectApplicationPolicy InfiniteEffectApplicationPolicy = EEffectApplicationPolicy::DoNotApply;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category= "Applied Effect", meta = (AllowPrivateAccess = true))
	EEffectRemovalPolicy InfiniteEffectRemovalPolicy = EEffectRemovalPolicy::RemoveOnEndOverlap;

};
