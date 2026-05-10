// @Copyright HaolunYuan

#pragma once

#include "CoreMinimal.h"
#include "Components/WidgetComponent.h"
#include "ActorStatusWidgetComponent.generated.h"

class UAuraWidgetController;
struct FWidgetControllerParameters;

/**
 * World-space widget component used by non-player actors (for example enemies) that still need
 * a WidgetController-driven UI. This lets actor widgets reuse the same MVC-style data flow as HUD widgets.
 */
UCLASS()
class AURA_API UActorStatusWidgetComponent : public UWidgetComponent
{
	GENERATED_BODY()

public:
	UActorStatusWidgetComponent();

	// Creates (or reuses) the widget controller for this component's widget and wires the current
	// actor-facing data references into it.
	void InitializeWidgetController(const FWidgetControllerParameters& Params);

private:
	// Blueprint chooses which controller class should drive this widget instance.
	UPROPERTY(EditDefaultsOnly)
	TSubclassOf<UAuraWidgetController> WidgetControllerClass;

	// Cached controller so we keep one controller instance per widget component.
	UPROPERTY(BlueprintReadOnly, meta = (AllowPrivateAccess = true))
	UAuraWidgetController* WidgetController;
};
