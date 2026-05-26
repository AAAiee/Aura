// @Copyright HaolunYuan

#pragma once

#include "CoreMinimal.h"
#include "Components/AbilitySystem/Ability/AuraProjectileSpell.h"
#include "AuraFireBolt.generated.h"

/** Fire projectile spell that builds rich-text descriptions for the spell menu. */
UCLASS()
class AURA_API UAuraFireBolt : public UAuraProjectileSpell
{
	GENERATED_BODY()

public:
	/** Builds the current-level rich-text tooltip shown by the spell menu. */
	virtual FString GetDescription(int32 Level) const override;

	/** Builds the preview tooltip for what the next spell level will change. */
	virtual FString GetNextLevelDescription(int32 Level) const override;
	
	UFUNCTION(BlueprintCallable)
	void SpawnProjectiles(const FVector& ProjectileTargetLocation, const FGameplayTag& SocketTag, bool bOverridePitch, float PitchOverride, AActor* HomingTarget);
	
protected:
	UPROPERTY(EditDefaultsOnly)
	float ProjectileSpread = 90.f;
	
	UPROPERTY(EditDefaultsOnly)
	int32 MaxNumProjectiles = 5;
	
	UPROPERTY(EditDefaultsOnly)
	float HomingAccelerationMin = 600.f;
	
	UPROPERTY(EditDefaultsOnly)
	float HomingAccelerationMax = 1200.f;
	
	UPROPERTY(EditDefaultsOnly)
	bool bLaunchHomingProjectiles = false;
	
};
