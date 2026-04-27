// @Copyright HaolunYuan

#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "Data/CharacterClassInfo.h"
#include "GameplayEffectTypes.h"
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
	/* Widget Controller Lookup */

	/** Returns (and lazily creates) the Overlay widget controller for the local player. */
	UFUNCTION(BlueprintCallable, Category = "AuraAbilitySystem|WidgetController")
	static UAuraOverlayWidgetController* GetOverlayWidgetController(const UObject* WorldContextObject);

	/** Returns (and lazily creates) the Attribute Menu widget controller for the local player. */
	UFUNCTION(BlueprintCallable, Category = "AuraAbilitySystem|WidgetController")
	static UAttributeMenuWidgetController* GetAttributeMenuWidgetController(const UObject* WorldContextObject);

	/* Character Class Defaults */

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
	static void InitializeDefaultAbilities(const UObject* WorldContextObject, ECharacterClass CharacterClass, UAbilitySystemComponent* ASC );

	// Convenience accessor for systems that need the authoritative class-info asset without each
	// caller repeating the GameMode lookup chain.
	UFUNCTION(BlueprintCallable, Category = "AuraAbilitySystem|CharacterClassDefaults")
	static UCharacterClassInfo* GetCharacterClassInfo(const UObject* WorldContextObject);

	/* Gameplay Effect Context Metadata */

	// Read the custom combat-result flags carried by FAuraGameplayEffectContext.
	UFUNCTION(BlueprintPure, Category = "AuraAbilitySystem|GameplayEffects")
	static bool IsBlockedHit(const FGameplayEffectContextHandle& EffectHandle);

	UFUNCTION(BlueprintPure, Category = "AuraAbilitySystem|GameplayEffects")
	static bool IsCriticalHit(const FGameplayEffectContextHandle& EffectHandle);

	UFUNCTION(BlueprintCallable, Category = "AuraAbilitySystem|GameplayEffects")
	static bool ShouldHitReact(UPARAM(ref)FGameplayEffectContextHandle& EffectHandle);


	// Write combat-result flags during server-side ExecCalc resolution so UI can read them later.
	UFUNCTION(BlueprintCallable, Category = "AuraAbilitySystem|GameplayEffects")
	static void SetIsBlockedHit(UPARAM(ref)FGameplayEffectContextHandle& EffectHandle, bool bInIsBlocked);

	UFUNCTION(BlueprintCallable, Category = "AuraAbilitySystem|GameplayEffects")
	static void SetIsCriticalHit(UPARAM(ref)FGameplayEffectContextHandle& EffectHandle, bool bInIsCritical);

	UFUNCTION(BlueprintCallable, Category = "AuraAbilitySystem|GameplayEffects")
	static void SetShouldHitReact(UPARAM(ref)FGameplayEffectContextHandle& EffectHandle, bool bInShouldHitReact);

	/* Gameplay Utilities */
	// Collects living combatants inside a radius while respecting an ignore list supplied by the caller.
	UFUNCTION(BlueprintCallable, Category = "AuraAbilitySystem|GameplayUtilities")
	static void GetLivePlayersWithinRadius(const UObject* WorldContextObject, TArray<AActor*>& OutOverlapActors, const TArray<AActor*>& ActorToIgnore, float Radius, const FVector& SphereOrigin);

	// Returns one montage entry at random from the authored list. Caller must provide a non-empty array.
	UFUNCTION(BlueprintCallable, Category ="AuraAbilitySystem|GameplayUtilities")
	static const FTaggedMontage& GetRandomMontageInArray(const TArray<FTaggedMontage>& MontageArray);

	// Lightweight team check used by projectiles and AoE utilities to ignore friendly targets.
	UFUNCTION(BlueprintCallable, BlueprintPure, Category ="AuraAbilitySystem|GameplayUtilities")
	static bool  IsNotFriend(AActor* FirstActor, AActor* SecondActor);
};
