// @Copyright HaolunYuan

#pragma once

#include "CoreMinimal.h"
#include "Components/AbilitySystem/Ability/AuraDamageGameplayAbility.h"
#include "AuraBeamSpell.generated.h"

/**
 * 
 */
UCLASS()
class AURA_API UAuraBeamSpell : public UAuraDamageGameplayAbility
{
	GENERATED_BODY()
	
public:
	UFUNCTION(BlueprintCallable)
	void StoreMouseDataInfo(const FHitResult& HitResult);
	
	UFUNCTION(BlueprintCallable)
	void StoreOwnerCachedVariables();
	
	UFUNCTION(BlueprintCallable)
	void TraceFirstTarget(const FVector& BeamTargetLocation); 
	
	UFUNCTION(BlueprintCallable)
	void StoreAdditionalTarget(TArray<AActor*>& OutAdditionalTargets);
	
	UFUNCTION(BlueprintImplementableEvent)
	void PrimaryTargetDied(AActor* DeadActor);
	
	UFUNCTION(BlueprintImplementableEvent)
	void AdditionalTargetDied(AActor* DeadActor);
	
	/** Builds the current-level rich-text tooltip shown by the spell menu. */
	virtual FString GetDescription(int32 Level) const override;

	/** Builds the preview tooltip for what the next spell level will change. */
	virtual FString GetNextLevelDescription(int32 Level) const override;
	
protected:
	UPROPERTY(BlueprintReadWrite, Category = "CachedRef")
	FVector MouseHitLocation;
	
	UPROPERTY(BlueprintReadWrite, Category = "CachedRef")
	TObjectPtr<AActor> MouseHitActor;
	
	UPROPERTY(BlueprintReadWrite, Category = "CachedRef")
	TObjectPtr<APlayerController> OwnerPlayerController;
	
	UPROPERTY(BlueprintReadWrite, Category = "CachedRef")
	TObjectPtr<ACharacter> OwnerCharacter;
	
	UPROPERTY(EditDefaultsOnly, Category = "Beam")
	int32 MaxNumShockTarget = 5;
	
};

