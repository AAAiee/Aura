// @Copyright HaolunYuan

#pragma once

#include "CoreMinimal.h"
#include "Engine/GameInstance.h"
#include "AuraGameInstance.generated.h"

class UAbilityInfo;

/** Game-wide data assets that need to be available on clients as well as the server. */
UCLASS()
class AURA_API UAuraGameInstance : public UGameInstance
{
	GENERATED_BODY()

public:
	/** Client-accessible spell metadata used by widget controllers for icons, descriptions, and unlock rows. */
	UPROPERTY(EditDefaultsOnly, Category = "Ability Data")
	TObjectPtr<UAbilityInfo> AbilityInfo;
};
