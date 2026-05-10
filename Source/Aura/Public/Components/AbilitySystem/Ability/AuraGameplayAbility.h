// @Copyright HaolunYuan

#pragma once

#include "CoreMinimal.h"
#include "Abilities/GameplayAbility.h"
#include "AuraGameplayAbility.generated.h"

/**
 * Base Aura gameplay ability carrying the startup input/ability tag authored by designers.
 */
UCLASS()
class AURA_API UAuraGameplayAbility : public UGameplayAbility
{
	GENERATED_BODY()


public:

	UPROPERTY(EditDefaultsOnly, Category = StartUpProperties)
	FGameplayTag StartupGameTag;
};

