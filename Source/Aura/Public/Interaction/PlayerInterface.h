// @Copyright HaolunYuan

#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "PlayerInterface.generated.h"

// Reflection shell for player-owned progression state exposed to Blueprint and GAS code.
UINTERFACE(MinimalAPI)
class UPlayerInterface : public UInterface
{
	GENERATED_BODY()
};

/**
 * Player progression contract used by combat rewards, UI, and level-up effects.
 */
class AURA_API IPlayerInterface
{
	GENERATED_BODY()

public:
	/* Progression Values */
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable)
	int32 GetXP() const;

	UFUNCTION(BlueprintNativeEvent, BlueprintCallable)
	int32 GetAttributePointsReward(int32 Level) const;

	UFUNCTION(BlueprintNativeEvent, BlueprintCallable)
	int32 GetSpellPointsReward(int32 Level) const;

	UFUNCTION(BlueprintNativeEvent, BlueprintCallable)
	int32 GetSpellPoints() const;

	UFUNCTION(BlueprintNativeEvent, BlueprintCallable)
	int32 GetAttributePoints() const;

	/* Progression Mutators */
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable)
	void AddToXP(int32 InXP);

	UFUNCTION(BlueprintNativeEvent, BlueprintCallable)
	void AddToPlayerLevel(int32 InPlayerLevel);

	UFUNCTION(BlueprintNativeEvent, BlueprintCallable)
	void AddToAttributePoint(int32 InAttributePoint);

	UFUNCTION(BlueprintNativeEvent, BlueprintCallable)
	void AddToSpellPoint(int32 InSpellPoint);

	/* Level Queries and Presentation */
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable)
	int32 FindLevelForXP(int32 InXP) const;

	UFUNCTION(BlueprintNativeEvent, BlueprintCallable)
	void LevelUp();
};
