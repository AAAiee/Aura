// @Copyright HaolunYuan

#pragma once

#include "CoreMinimal.h"
#include "Components/WidgetComponent.h"
#include "DamageWidgetComponent.generated.h"

/**
 * Lightweight widget component used for transient floating combat text spawned at runtime.
 */
UCLASS()
class AURA_API UDamageWidgetComponent : public UWidgetComponent
{
	GENERATED_BODY()


public:
	// Blueprint implements the presentation, while C++ owns when/where the transient widget component is spawned.
	UFUNCTION(BlueprintImplementableEvent, BlueprintCallable)
	void SetDamageText(float Damage);

};
