// @Copyright HaolunYuan

#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "Data/CharacterClassInfo.h"
#include "GameplayEffectTypes.h"
#include "AuraAbilitySystemLibrary.generated.h"

struct FDamageEffectParameters;
class UAuraOverlayWidgetController;
class UAttributeMenuWidgetController;
class UAbilitySystemComponent;
class UAbilityInfo;
class UAuraSpellMenuWidgetController;
class AAuraHUD;
struct FWidgetControllerParameters;

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
	UFUNCTION(BlueprintCallable, Category = "AuraAbilitySystem|WidgetController", meta = (DefaultToSelf = "WorldContextObject"))
	static UAuraOverlayWidgetController* GetOverlayWidgetController(const UObject* WorldContextObject);

	/** Returns (and lazily creates) the Attribute Menu widget controller for the local player. */
	UFUNCTION(BlueprintCallable, Category = "AuraAbilitySystem|WidgetController", meta = (DefaultToSelf = "WorldContextObject"))
	static UAttributeMenuWidgetController* GetAttributeMenuWidgetController(const UObject* WorldContextObject);

	/** Returns (and lazily creates) the Spell Menu widget controller for the local player. */
	UFUNCTION(BlueprintCallable, Category = "AuraAbilitySystem|WidgetController", meta = (DefaultToSelf = "WorldContextObject"))
	static UAuraSpellMenuWidgetController* GetSpellMenuWidgetController(const UObject* WorldContextObject);

	/* Character Class Defaults */

	/**
	 * Applies the class-driven startup Gameplay Effects (primary / secondary / vital attributes).
	 *
	 * Important runtime rule:
	 *   - Attribute initialization must be authored by the server because Gameplay Effects are
	 *     an authoritative state. Clients should never create their own default stats locally.
	 */
	UFUNCTION(BlueprintCallable, Category = "AuraAbilitySystem|Attributes")
	static void InitializeDefaultAttributes(const UObject* WorldContextObject, ECharacterClass CharacterClass, UAbilitySystemComponent* ASC, float Level);

	// Grants the shared startup abilities declared on CharacterClassInfo (for example, generic combat responses).
	UFUNCTION(BlueprintCallable, Category = "AuraAbilitySystem|CharacterClassDefaults")
	static void InitializeDefaultAbilities(const UObject* WorldContextObject, ECharacterClass CharacterClass, UAbilitySystemComponent* ASC);

	// Convenience accessor for systems that need the authoritative class-info asset without each
	// caller repeating the GameMode lookup chain.
	UFUNCTION(BlueprintCallable, Category = "AuraAbilitySystem|CharacterClassDefaults")
	static UCharacterClassInfo* GetCharacterClassInfo(const UObject* WorldContextObject);

	UFUNCTION(BlueprintCallable, Category = "AuraAbilitySystem|CharacterClassDefaults")
	static UAbilityInfo* GetAbilityInfo(const UObject* WorldContextObject);

	/* Damage Effect Construction */

	/**
	 * Builds and applies the shared Aura damage GameplayEffect from an ability-authored data bundle.
	 *
	 * The returned context handle is the same context placed on the outgoing spec, so callers can
	 * inspect server-authored hit metadata such as block/crit/debuff results after execution.
	 */
	UFUNCTION(BlueprintCallable, Category = "AuraAbilitySystem|DamageEffect")
	static FGameplayEffectContextHandle ApplyDamageEffect(const FDamageEffectParameters& DamageParameters);

	/* Gameplay Effect Context Metadata */

	// Read the custom combat-result flags carried by FAuraGameplayEffectContext. These helpers keep
	// Blueprint and C++ call sites from needing to cast the base GAS context manually.
	UFUNCTION(BlueprintPure, Category = "AuraAbilitySystem|GameplayEffects")
	static bool IsBlockedHit(const FGameplayEffectContextHandle& EffectHandle);

	UFUNCTION(BlueprintPure, Category = "AuraAbilitySystem|GameplayEffects")
	static bool IsCriticalHit(const FGameplayEffectContextHandle& EffectHandle);

	UFUNCTION(BlueprintCallable, Category = "AuraAbilitySystem|GameplayEffects")
	static bool ShouldHitReact(UPARAM(ref)FGameplayEffectContextHandle& EffectHandle);

	UFUNCTION(BlueprintCallable, Category = "AuraAbilitySystem|GameplayEffects")
	static bool IsSuccessfulDebuff(UPARAM(ref)FGameplayEffectContextHandle& EffectHandle);

	UFUNCTION(BlueprintCallable, Category = "AuraAbilitySystem|GameplayEffects")
	static float GetDebuffDamage(UPARAM(ref)FGameplayEffectContextHandle& EffectHandle);

	UFUNCTION(BlueprintCallable, Category = "AuraAbilitySystem|GameplayEffects")
	static float GetDebuffDuration(UPARAM(ref)FGameplayEffectContextHandle& EffectHandle);

	UFUNCTION(BlueprintCallable, Category = "AuraAbilitySystem|GameplayEffects")
	static float GetDebuffFrequency(UPARAM(ref)FGameplayEffectContextHandle& EffectHandle);

	UFUNCTION(BlueprintCallable, Category = "AuraAbilitySystem|GameplayEffects")
	static FGameplayTag GetDamageTypeTag(UPARAM(ref)FGameplayEffectContextHandle& EffectHandle);
	
	UFUNCTION(BlueprintCallable, Category = "AuraAbilitySystem|GameplayEffects")
	static FVector GetDeathImpulse(UPARAM(ref)FGameplayEffectContextHandle& EffectHandle);
	
	UFUNCTION(BlueprintCallable, Category = "AuraAbilitySystem|GameplayEffects")
	static FVector GetKnockBackForce(UPARAM(ref)FGameplayEffectContextHandle& EffectHandle);
	

	// Write combat-result flags during server-side ExecCalc resolution so AttributeSets and UI can
	// read the resolved result without recomputing combat math.
	UFUNCTION(BlueprintCallable, Category = "AuraAbilitySystem|GameplayEffects")
	static void SetIsBlockedHit(UPARAM(ref)FGameplayEffectContextHandle& EffectHandle, bool bInIsBlocked);

	UFUNCTION(BlueprintCallable, Category = "AuraAbilitySystem|GameplayEffects")
	static void SetIsCriticalHit(UPARAM(ref)FGameplayEffectContextHandle& EffectHandle, bool bInIsCritical);

	UFUNCTION(BlueprintCallable, Category = "AuraAbilitySystem|GameplayEffects")
	static void SetShouldHitReact(UPARAM(ref)FGameplayEffectContextHandle& EffectHandle, bool bInShouldHitReact);

	UFUNCTION(BlueprintCallable, Category = "AuraAbilitySystem|GameplayEffects")
	static void SetSuccessfulDebuff(UPARAM(ref)FGameplayEffectContextHandle& EffectHandle, bool bInIsSuccessful);

	UFUNCTION(BlueprintCallable, Category = "AuraAbilitySystem|GameplayEffects")
	static void SetDebuffDamage(UPARAM(ref)FGameplayEffectContextHandle& EffectHandle, float InDamage);

	UFUNCTION(BlueprintCallable, Category = "AuraAbilitySystem|GameplayEffects")
	static void SetDebuffDuration(UPARAM(ref)FGameplayEffectContextHandle& EffectHandle, float InDuration);

	UFUNCTION(BlueprintCallable, Category = "AuraAbilitySystem|GameplayEffects")
	static void SetDebuffFrequency(UPARAM(ref)FGameplayEffectContextHandle& EffectHandle, float InDebuffFrequency);

	UFUNCTION(BlueprintCallable, Category = "AuraAbilitySystem|GameplayEffects")
	static void SetDamageTypeTag(UPARAM(ref)FGameplayEffectContextHandle& EffectHandle, const FGameplayTag& InDamageTypeTag);
	
	UFUNCTION(BlueprintCallable, Category = "AuraAbilitySystem|GameplayEffects")
	static void SetDeathImpulse(UPARAM(ref)FGameplayEffectContextHandle& EffectHandle, const FVector& InDeathImpulse);
	
	UFUNCTION(BlueprintCallable, Category = "AuraAbilitySystem|GameplayEffects")
	static void SetKnockBackForce(UPARAM(ref)FGameplayEffectContextHandle& EffectHandle, const FVector& InKnockBackForce);
	

	/* Gameplay Utilities */
	// Collects living combatants inside a radius while respecting an ignore list supplied by the caller.
	UFUNCTION(BlueprintCallable, Category = "AuraAbilitySystem|GameplayUtilities")
	static void GetLivePlayersWithinRadius(const UObject* WorldContextObject, TArray<AActor*>& OutOverlapActors, const TArray<AActor*>& ActorToIgnore, float Radius, const FVector& SphereOrigin);
	
	UFUNCTION(BlueprintCallable, Category = "AuraAbilitySystem|GameplayUtilities")
	static void GetClosestTargets(int32 MaxTargets, const TArray<AActor*>& Actors,  TArray<AActor*>& OutClosestTargets, const FVector& Origin);

	// Returns one montage entry at random from the authored list. Caller must provide a non-empty array.
	UFUNCTION(BlueprintCallable, Category = "AuraAbilitySystem|GameplayUtilities")
	static const FTaggedMontage& GetRandomMontageInArray(const TArray<FTaggedMontage>& MontageArray);

	// Lightweight team check used by projectiles and AoE utilities to ignore friendly targets.
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "AuraAbilitySystem|GameplayUtilities")
	static bool IsNotFriend(AActor* FirstActor, AActor* SecondActor);

	/* XP */
	static int32 GetXPRewardForClassAndLevel(const UObject* WorldContextObject, ECharacterClass CharacterClass, int32 CharacterLevel);
	
	UFUNCTION(BlueprintPure,  Category = "AuraAbilitySystem|GameplayUtilities")
	static TArray<FRotator> EvenlySpacedRotators(const FVector& Forward, const FVector& Axis, float Spread, int32 NumRotators);
	
	UFUNCTION(BlueprintPure,  Category = "AuraAbilitySystem|GameplayUtilities")
	static TArray<FVector> EvenlyRotatedVectors(const FVector& Forward, const FVector& Axis, float Spread, int32 NumVectors);
	
	

private:
	/**
	 * Fills OutParams and OutAuraHUD for widget controller initialization.
	 * @return true if all required objects were found, false otherwise.
	 */
	static bool MakeWidgetControllerParameters(
		const UObject* WorldContextObject,
		FWidgetControllerParameters& OutParams,
		AAuraHUD*& OutAuraHUD
	);
};
