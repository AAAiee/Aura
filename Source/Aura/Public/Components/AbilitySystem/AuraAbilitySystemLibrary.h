// @Copyright HaolunYuan

#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "Data/CharacterClassInfo.h"
#include "AuraAbilitySystemLibrary.generated.h"

class UAuraOverlayWidgetController;
class UAttributeMenuWidgetController;
class UAbilitySystemComponent;

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
	UFUNCTION(BlueprintCallable, Category = "AuraAbilitySystem|WidgetController")
	static UAuraOverlayWidgetController* GetOverlayWidgetController(const UObject* WorldContextObject);

	/** Returns (and lazily creates) the Attribute Menu widget controller for the local player. */
	UFUNCTION(BlueprintCallable, Category = "AuraAbilitySystem|WidgetController")
	static UAttributeMenuWidgetController* GetAttributeMenuWidgetController(const UObject* WorldContextObject);

	/**
	 * Applies the class-driven startup Gameplay Effects (primary / secondary / vital attributes).
	 *
	 * Important runtime rule:
	 *   - Attribute initialization must be authored by the server because Gameplay Effects are
	 *     authoritative state. Clients should never create their own default stats locally.
	 */
	UFUNCTION(BlueprintCallable, Category = "AuraAbilitySystem|Attributes")
	static void InitializeDefaultAttributes(const UObject* WorldContextObject, ECharacterClass CharacterClass, UAbilitySystemComponent* ASC, float Level);


	// Grants the shared startup abilities declared on CharacterClassInfo (for example, generic combat responses).
	UFUNCTION(BlueprintCallable, Category = "AuraAbilitySystem|CharacterClassDefaults")
	static void InitialzeDefaultAbilities(const UObject* WorldContextObject, ECharacterClass CharacterClass, UAbilitySystemComponent* ASC );

	// Convenience accessor for systems that need the authoritative class-info asset without each
	// caller repeating the GameMode lookup chain.
	UFUNCTION(BlueprintCallable, Category = "AuraAbilitySystem|CharacterClassDefaults")
	static UCharacterClassInfo* GetCharacterClassInfo(const UObject* WorldContextObject);
};
