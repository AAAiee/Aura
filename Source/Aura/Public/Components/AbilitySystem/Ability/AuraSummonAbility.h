// @Copyright HaolunYuan

#pragma once

#include "CoreMinimal.h"
#include "Components/AbilitySystem/Ability/AuraGameplayAbility.h"
#include "AuraSummonAbility.generated.h"

class APawn;

/**
 * Ability helper for selecting minion classes and candidate spawn points around the caster.
 */
UCLASS()
class AURA_API UAuraSummonAbility : public UAuraGameplayAbility
{
	GENERATED_BODY()

public:
	/* Summon Placement */
	// Returns candidate ground locations spread across an arc in front of the caster.
	UFUNCTION(BlueprintCallable, Category = "Summon")
	TArray<FVector> GetSpawnLocations() const;

	// Picks one authored minion class for this summon activation.
	UFUNCTION(BlueprintCallable, Category = "Summon")
	TSubclassOf<APawn> GetRandomMinionClass() const;

	/* Summon Authoring */
	// Number of candidate locations to generate. Keep above zero to avoid an empty spread.
	UPROPERTY(EditAnywhere, Category = "Summon")
	int32 NumSpawnLocations = 5;

	// Blueprint-authored pool of minion pawn classes this ability can choose from.
	UPROPERTY(EditAnywhere, Category = "Summon")
	TArray<TSubclassOf<APawn>> MinionClasses;

	// Minimum distance from the caster when choosing a random point in the spread.
	UPROPERTY(EditAnywhere, Category = "Summon")
	float MinSpawnDistance = 200.f;

	// Maximum distance from the caster when choosing a random point in the spread.
	UPROPERTY(EditAnywhere, Category = "Summon")
	float MaxSpawnDistance = 500.f;

	// Width of the spawn arc centered on the caster's forward vector.
	UPROPERTY(EditAnywhere, Category = "Summon")
	float SpawnSpread = 90.f;
};
