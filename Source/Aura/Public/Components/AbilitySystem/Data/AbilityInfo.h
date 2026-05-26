// @Copyright HaolunYuan

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "GameplayTagContainer.h"
#include "AbilityInfo.generated.h"

class UGameplayAbility;

USTRUCT(BlueprintType)
struct FAuraAbilityInfo
{
	GENERATED_BODY()

	/** Stable Ability.* tag that identifies this row and matches the ability asset tag. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite)
	FGameplayTag AbilityTag;

	/** Runtime InputTag.* slot currently assigned to this ability; empty/None means not equipped. */
	UPROPERTY(BlueprintReadWrite)
	FGameplayTag InputTag;

	/** Cooldown.* tag used by UI to observe cooldown state for this ability. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	FGameplayTag CooldownTag;

	/** Ability.Type.* tag used to restrict equipment to compatible offensive/passive rows. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	FGameplayTag AbilityTypeTag;

	/** Runtime Ability.Status.* tag broadcast by the ASC for locked/unlocked/equipped UI state. */
	UPROPERTY(BlueprintReadOnly)
	FGameplayTag AbilityStatusTag;

	/** Icon texture shown in the spell menu and equipped ability slots. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite)
	TObjectPtr<const UTexture2D> IconImage;

	/** Optional background material used by UI slots to theme this ability. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite)
	TObjectPtr<const UMaterialInterface> BackgroundImage;

	/** Minimum player level required before the ASC can grant this ability as Eligible. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	int32 LevelRequirement = 1;

	/** Gameplay ability class granted when this row becomes eligible/unlocked. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	TSubclassOf<UGameplayAbility> Ability;
};

/**
 * Data asset that maps ability tags to UI presentation and input/cooldown metadata.
 */
UCLASS()
class AURA_API UAbilityInfo : public UDataAsset
{
	GENERATED_BODY()

public:
	/** All spell-menu rows authored in the data asset; each row should have a unique AbilityTag. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	TArray<FAuraAbilityInfo> AbilitiesInfos;

	/** Finds mutable row data by Ability.* tag so widget controllers can stamp runtime status/input data. */
	FAuraAbilityInfo* FindAbilityInfoByTag(const FGameplayTag& InAbilityTag, bool LogOnNotFound = false);
};
