// @Copyright HaolunYuan

#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "AuraAbilitySystemLibrary.generated.h"

class UAuraOverlayWidgetController;
class UAttributeMenuWidgetController;

/**
 * Blueprint utility library for resolving Aura-specific widget controllers.
 *
 * Why this exists:
 *   - Widgets often only have a `WorldContextObject`.
 *   - This helper centralizes the chain: World -> Local PC -> HUD -> WidgetController.
 */
UCLASS()
class AURA_API UAuraAbilitySystemLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:
	/** Returns (and lazily creates) the Overlay widget controller for the local player. */
	UFUNCTION(BlueprintCallable, Category = "AbilitySystem|WidgetController")
	static UAuraOverlayWidgetController* GetOverlayWidgetController(const UObject* WorldContextObject);

	/** Returns (and lazily creates) the Attribute Menu widget controller for the local player. */
	UFUNCTION(BlueprintCallable, Category = "AbilitySystem|WidgetController")
	static UAttributeMenuWidgetController* GetAttributeMenuWidgetController(const UObject* WorldContextObject);
};
