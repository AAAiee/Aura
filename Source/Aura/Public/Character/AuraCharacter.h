// @Copyright HaolunYuan

#pragma once

#include "CoreMinimal.h"
#include "Character/AuraCharacterBase.h"
#include "AuraCharacter.generated.h"


/**
 * The player-controlled character in Aura.
 *
 * ASC Ownership: The ASC and AttributeSet live on the PlayerState (not this pawn).
 * This is the standard GAS pattern for player characters because:
 *   - The PlayerState persists across pawn respawns, so attributes survive death.
 *   - Multiple pawns can share the same ASC if the player changes characters.
 *
 * Initialization flow (ORDER MATTERS):
 *   1. PossessedBy / OnRep_PlayerState ¡ú InitAbilityActorInfo()
 *   2. InitAbilityActorInfo() caches ASC/AS from PlayerState
 *   3. HUD creates the Overlay Widget and its Controller, which subscribes to ASC delegates
 *   4. AbilityActorInfoSet() binds the ASC's OnGameplayEffectApplied delegate
 *      (must be LAST so the UI listeners are already registered before any GE broadcasts)
 */
UCLASS()
class AURA_API AAuraCharacter : public AAuraCharacterBase
{
	GENERATED_BODY()

public:
	AAuraCharacter(); 

	/** Called on the SERVER when this pawn is possessed. Initializes ASC for the server. */
	virtual void PossessedBy(AController* NewController) override;

	/** Called on the CLIENT when PlayerState replicates. Initializes ASC for the client. */
	virtual void OnRep_PlayerState() override;

protected:
	/**
	 * Wires up ASC, AttributeSet, HUD, and ASC delegates.
	 * Called from both PossessedBy (server) and OnRep_PlayerState (client) to cover all cases.
	 */
	virtual void InitAbilityActorInfo() override;

};
