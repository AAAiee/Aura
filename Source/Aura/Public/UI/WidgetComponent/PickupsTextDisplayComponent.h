// @Copyright HaolunYuan

#pragma once

#include "CoreMinimal.h"
#include "Components/WidgetComponent.h"
#include "PickupsTextDisplayComponent.generated.h"

/**
 * World-space widget component that displays pickup item text while highlighted.
 *
 * PickupEffectActor drives this component from C++ highlight events, while Blueprint implements the
 * actual display animation and text layout.
 */
UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class AURA_API UPickupsTextDisplayComponent : public UWidgetComponent
{
	GENERATED_BODY()

public:
	UPickupsTextDisplayComponent();
	
	UFUNCTION(BlueprintImplementableEvent, BlueprintCallable)
	void ShowDisplayText(const FText& Text);
	
	UFUNCTION(BlueprintCallable)
	void HideText();
};
