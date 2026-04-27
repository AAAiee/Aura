// @Copyright HaolunYuan

#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "GameplayTagContainer.h"
#include "CombatInterface.generated.h"

class UAnimMontage;

USTRUCT(BlueprintType)
struct FTaggedMontage
{
	GENERATED_BODY()

	/** Animation montage to play for this attack entry. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	UAnimMontage* Montage;

	/** Tag that identifies which combat socket this montage should use. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	FGameplayTag MontageTag;
};

// This class does not need to be modified.
UINTERFACE(MinimalAPI, BlueprintType)
class UCombatInterface : public UInterface
{
	GENERATED_BODY()
};

/**
 * Shared combat contract implemented by player and enemy combatants.
 *
 * The interface keeps combat abilities data-driven by exposing sockets, level information,
 * death handling, and montage data without requiring casts to a concrete character class.
 */
class AURA_API ICombatInterface
{
	GENERATED_BODY()

public:
	// Returns the level used by startup ability grants and damage-scaling curves.
	virtual int32 GetPlayerLevel() const;

	/* Combat Socket Queries */
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent)
	FVector GetCombatSocketLocation(const FGameplayTag& MontageTag) const;

	/* Orientation */
	UFUNCTION(BlueprintCallable, BlueprintImplementableEvent)
	void SetWarpingFacingTarget(const FVector& TargetLocation);

	/* Combat Reaction */
	// Lets generic combat abilities ask the current target which reaction montage to play without
	// depending on a concrete Aura character subclass.
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable)
	UAnimMontage* GetHitReactMontage();

	/* Life Cycle */
	// Called by authoritative damage resolution when Health reaches zero. Implementers own the
	// actual death presentation (ragdoll, dissolve, lifespan cleanup, etc.).
	virtual void Die() = 0;

	UFUNCTION(BlueprintCallable, BlueprintNativeEvent)
	bool IsDead() const;

	UFUNCTION(BlueprintCallable, BlueprintNativeEvent)
	AActor* GetAvatar();

	/* Attack Presentation */
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent)
	TArray<FTaggedMontage> GetTaggedMontages() const;
};
