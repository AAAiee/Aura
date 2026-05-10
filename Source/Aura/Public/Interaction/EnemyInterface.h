// @Copyright HaolunYuan

#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "EnemyInterface.generated.h"

// Reflection shell for actors that expose enemy combat targeting to Blueprint and C++.
UINTERFACE(MinimalAPI)
class UEnemyInterface : public UInterface
{
	GENERATED_BODY()
};

/**
 * Small combat-target contract implemented by enemies and systems that steer enemy combat logic.
 */
class AURA_API IEnemyInterface
{
	GENERATED_BODY()

public:
	/* Combat Target Access */
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable)
	void SetCombatTarget(AActor* CombatTarget);

	UFUNCTION(BlueprintNativeEvent, BlueprintCallable)
	AActor* GetCombatTarget() const;
};
