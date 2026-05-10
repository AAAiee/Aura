// @Copyright HaolunYuan

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "GameplayTagContainer.h"
#include "AbilityInfo.generated.h"


USTRUCT(BlueprintType)
struct FAuraAbilityInfo
{
	GENERATED_BODY()

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite)
	FGameplayTag AbilityTag;

	UPROPERTY(BlueprintReadWrite)
	FGameplayTag InputTag;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	FGameplayTag CooldownTag;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite)
	TObjectPtr<const UTexture2D> IconImage;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite)
	TObjectPtr<const UMaterialInterface> BackgroundImage;
};

/**
 * Data asset that maps ability tags to UI presentation and input/cooldown metadata.
 */
UCLASS()
class AURA_API UAbilityInfo : public UDataAsset
{
	GENERATED_BODY()

public:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	TArray<FAuraAbilityInfo> AbilitiesInfo;

	FAuraAbilityInfo* FindAbilityInfoByTag(const FGameplayTag& InAbilityTag, bool LogOnNotFound = false);

};
