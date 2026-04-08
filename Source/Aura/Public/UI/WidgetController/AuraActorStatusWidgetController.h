// @Copyright HaolunYuan

#pragma once

#include "CoreMinimal.h"
#include "UI/WidgetController/AuraWidgetController.h"
#include "UI/WidgetController/ControllerDelegates.h"
#include "AuraActorStatusWidgetController.generated.h"

struct FWidgetControllerParameters;
class UAuraWidgetController;

/**
 * Widget controller for actor-facing status bars (enemy health / mana style UI).
 *
 * Unlike the player HUD controllers, this controller is intentionally lightweight:
 *   - it only needs the actor ASC + AttributeSet
 *   - it pushes values into a world-space widget instead of a screen HUD
 */
UCLASS(BlueprintType, Blueprintable)
class AURA_API UAuraActorStatusWidgetController : public UAuraWidgetController
{
	GENERATED_BODY()


public:
	// Pushes the actor's current values immediately so the bar is correct on first render.
	virtual void BroadcastInitialValues() override;

	// Subscribes to ASC attribute change delegates so the widget updates as the actor takes damage.
	virtual void BindAllDependencies() override;

private:
	// Blueprint widgets bind to these multicast delegates and redraw their bars when values change.
	UPROPERTY(BlueprintAssignable, Category = "Actor Status", meta = (AllowPrivateAccess = true))
	FOnAttributeChangeSignature OnHealthChangedDelegate;

	UPROPERTY(BlueprintAssignable, Category = "Actor Status", meta = (AllowPrivateAccess = true))
	FOnAttributeChangeSignature OnManaChangedDelegate;

	UPROPERTY(BlueprintAssignable, Category = "Actor Status", meta = (AllowPrivateAccess = true))
	FOnAttributeChangeSignature OnMaxHealthChangeDelegate;

	UPROPERTY(BlueprintAssignable, Category = "Actor Status", meta = (AllowPrivateAccess = true))
	FOnAttributeChangeSignature OnMaxManaChangeDelegate;
};
