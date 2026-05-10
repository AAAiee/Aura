// @Copyright HaolunYuan

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "LevelUpInfo.generated.h"


USTRUCT(BlueprintType)
struct FAuraLevelUpInfo
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly, EditAnywhere)
	int32 LevelUpRequirement = 0;

	UPROPERTY(BlueprintReadOnly, EditAnywhere)
	int32 AttributePointsGranted = 1;

	UPROPERTY(BlueprintReadOnly, EditAnywhere)
	int32 SpellPointsGranted = 1;
};

/**
 * Data asset containing XP thresholds and point rewards for each player level.
 */
UCLASS()
class AURA_API ULevelUpInfo : public UDataAsset
{
	GENERATED_BODY()

public:

	UFUNCTION(BlueprintCallable)
	int32 FindLevelForXP(int32 XP) const;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	TArray<FAuraLevelUpInfo> LevelUpInfos;
};
