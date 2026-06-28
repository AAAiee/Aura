// @Copyright HaolunYuan

#pragma once

#include "CoreMinimal.h"
#include "Components/WidgetComponent.h"
#include "PickupsTextDisplayComponent.generated.h"


UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class AURA_API UPickupsTextDisplayComponent : public UWidgetComponent
{
	GENERATED_BODY()

public:
	// Sets default values for this component's properties
	UPickupsTextDisplayComponent();
	
	UFUNCTION(BlueprintImplementableEvent, BlueprintCallable)
	void ShowDisplayText(const FText& Text);
	
	UFUNCTION(BlueprintCallable)
	void HideText();
};
