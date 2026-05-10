// @Copyright HaolunYuan

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "ScalableFloat.h"
#include "CharacterClassInfo.generated.h"

class UCurveTable;
class UGameplayAbility;
class UGameplayEffect;

UENUM(BlueprintType)
enum class ECharacterClass : uint8
{
	ECC_Elementalist UMETA(DisplayName = "Elementalist"),
	ECC_Warrior UMETA(DisplayName = "Warrior"),
	ECC_Ranger UMETA(DisplayName = "Ranger")
};


/**
 * Bundle of startup effects for one playable / enemy class archetype.
 *
 * We keep the struct intentionally small right now because only the class-specific primary
 * attributes vary per archetype. Shared secondary / vital effects live on the data asset itself.
 */
USTRUCT(BlueprintType)
struct FCharacterClassDefaultInfo
{
	GENERATED_BODY()

	// Primary attributes are class-specific (for example, warriors and rangers scale differently).
	UPROPERTY(EditDefaultsOnly, Category = "Class Defaults")
	TSubclassOf<UGameplayEffect> PrimaryAttributeEffect;

	// Startup abilities granted to this class in addition to the shared common abilities.
	UPROPERTY(EditDefaultsOnly, Category = "Class Defaults")
	TArray<TSubclassOf<UGameplayAbility>> ClassUniqueAbilities;

	UPROPERTY(EditDefaultsOnly, Category = "Class Defaults")
	FScalableFloat XPReward = FScalableFloat();
};


/**
 * Central data asset that answers the question:
 *   "Which Gameplay Effects should seed a character's stats when it first spawns?"
 *
 * This keeps startup attributes data-driven so designers can tune classes in assets without
 * changing gameplay code every time a balance pass happens.
 */
UCLASS()
class AURA_API UCharacterClassInfo : public UDataAsset
{
	GENERATED_BODY()

public:
	/* Startup Effect Data */
	// Per-class startup effects keyed by the enum that gameplay code passes in at spawn time.
	UPROPERTY(EditDefaultsOnly, Category = "Character Class Defaults")
	TMap<ECharacterClass, FCharacterClassDefaultInfo> CharacterClassDefaultInfoMap;

	// Shared secondary stats (armor, crit, regen, etc.) that every class receives.
	UPROPERTY(EditDefaultsOnly, Category = "Common Class Defaults")
	TSubclassOf<UGameplayEffect> SecondaryAttributes;

	// Shared vital stats (health / mana style values) that every class receives.
	UPROPERTY(EditDefaultsOnly, Category = "Common Class Defaults")
	TSubclassOf<UGameplayEffect> VitalAttributes;

	// Shared startup abilities granted to every combatant that uses this data asset.
	UPROPERTY(EditDefaultsOnly, Category = "Common Class Defaults")
	TArray<TSubclassOf<UGameplayAbility>> CommonAbilities;

	// Curve table used by damage execution to convert raw attributes into level-scaled combat math.
	UPROPERTY(EditDefaultsOnly, Category = "Common Class Defaults | Combat")
	TObjectPtr<UCurveTable> DefaultCalculationCoeffcient;

	/* Queries */
	// FindChecked is intentional here: missing class entries are authoring errors we want to catch
	// immediately during setup instead of silently creating partially initialized characters.
	FCharacterClassDefaultInfo GetDefaultInfoForClass(ECharacterClass CharacterClass) const;
};
