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
	// Blueprint owns presentation variants; C++ passes the resolved combat result for styling.
	UFUNCTION(BlueprintImplementableEvent, BlueprintCallable)
	void SetDamageText(float Damage, bool bIsBlockedHit, bool bIsCriticalHit);
};
