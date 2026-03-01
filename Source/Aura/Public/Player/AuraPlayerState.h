// @Copyright HaolunYuan

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerState.h"
#include "AbilitySystemInterface.h"
#include "AuraPlayerState.generated.h"

class UAttributeSet;
class UAbilitySystemComponent;

/**
 * Aura's PlayerState ¡ª the authoritative owner of the player's ASC and AttributeSet.
 *
 * Why the PlayerState owns the ASC (instead of the Pawn):
 *   - PlayerState persists across pawn deaths/respawns, so attributes (Health, Mana) survive.
 *   - If the player ever switches pawns, the same ASC follows automatically.
 *   - This is the recommended GAS pattern for player-controlled characters.
 *
 * Implements IAbilitySystemInterface so any code that has a PlayerState pointer
 * can call GetAbilitySystemComponent() directly.
 *
 * Replication Mode: Mixed ¡ª supports both server-authoritative GEs and client-predicted GAs.
 * Compare with Enemy's Minimal mode (no prediction needed for AI).
 */
UCLASS()
class AURA_API AAuraPlayerState : public APlayerState, public IAbilitySystemInterface
{
	GENERATED_BODY()

public:
	AAuraPlayerState();

	/*IAbilitySystemInterface*/
	FORCEINLINE virtual UAbilitySystemComponent* GetAbilitySystemComponent() const override { return AbilitySystemComponent; }
	FORCEINLINE UAttributeSet* GetAttributeSet() const { return AttributeSet; }

protected:
	/*GAS Components ¡ª created here, but initialized in AAuraCharacter::InitAbilityActorInfo()*/
	UPROPERTY()
	TObjectPtr<UAbilitySystemComponent> AbilitySystemComponent;

	UPROPERTY()
	TObjectPtr<UAttributeSet> AttributeSet;
};

