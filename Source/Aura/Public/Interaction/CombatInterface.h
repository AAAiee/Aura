// @Copyright HaolunYuan

#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "CombatInterface.generated.h"

class UAnimMontage;

// This class does not need to be modified.
UINTERFACE(MinimalAPI, BlueprintType)
class UCombatInterface : public UInterface
{
	GENERATED_BODY()
};

/**
 * 
 */
class AURA_API ICombatInterface
{
	GENERATED_BODY()

public:
	virtual int32 GetPlayerLevel() const;
	virtual FVector GetCombatSocketLocation() const;

	UFUNCTION(BlueprintCallable, BlueprintImplementableEvent)
	void SetWarpingFacingTarget(const FVector& TargetLocation);


	// Lets generic combat abilities ask the current target which reaction montage to play without
	// depending on a concrete Aura character subclass.
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable)
	UAnimMontage* GetHitReactMontage();

	// Called by authoritative damage resolution when Health reaches zero. Implementers own the
	// actual death presentation (ragdoll, dissolve, lifespan cleanup, etc.).
	virtual void Die() = 0;
};
