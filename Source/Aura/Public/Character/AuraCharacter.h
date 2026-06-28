// @Copyright HaolunYuan

#pragma once

#include "CoreMinimal.h"
#include "Character/AuraCharacterBase.h"
#include "Interaction/PlayerInterface.h"
#include "AuraCharacter.generated.h"

class UCameraComponent;
class UNiagaraComponent;
class USpringArmComponent;

/**
 * The player-controlled character in Aura.
 *
 * ASC Ownership: The ASC and AttributeSet live on the PlayerState (not this pawn).
 * This is the standard GAS pattern for player characters because:
 *   - The PlayerState persists across pawn respawns, so attributes survive death.
 *   - Multiple pawns can share the same ASC if the player changes characters.
 *
 * Initialization flow (ORDER MATTERS):
 *   1. PossessedBy / OnRep_PlayerState -> InitAbilityActorInfo()
 *   2. InitAbilityActorInfo() caches ASC/AS from PlayerState
 *   3. HUD creates the Overlay Widget and its Controller, which subscribes to ASC delegates
 *   4. AbilityActorInfoSet() binds the ASC's OnGameplayEffectApplied delegate
 *      (must be LAST so the UI listeners are already registered before any GE broadcasts)
 */
UCLASS()
class AURA_API AAuraCharacter : public AAuraCharacterBase, public IPlayerInterface
{
	GENERATED_BODY()

public:
	AAuraCharacter();

	/** Called on the SERVER when this pawn is possessed. Initializes ASC for the server. */
	virtual void PossessedBy(AController* NewController) override;

	/** Called on the CLIENT when PlayerState replicates. Initializes ASC for the client. */
	virtual void OnRep_PlayerState() override;

	/* ICombatInterface */
	virtual int32 GetPlayerLevel_Implementation() const override;

	/* IPlayerInterface */
	virtual int32 GetXP_Implementation() const override;
	virtual int32 GetAttributePointsReward_Implementation(int32 Level) const override;
	virtual int32 GetSpellPointsReward_Implementation(int32 Level) const override;
	virtual int32 GetAttributePoints_Implementation() const override;
	virtual int32 GetSpellPoints_Implementation() const override;
	virtual void AddToXP_Implementation(int32 InXP) override;
	virtual void AddToPlayerLevel_Implementation(int32 InPlayerLevel) override;
	virtual void AddToAttributePoint_Implementation(int32 InAttributePoint) override;
	virtual void AddToSpellPoint_Implementation(int32 InSpellPoint) override;
	virtual int32 FindLevelForXP_Implementation(int32 InXP) const override;
	virtual void LevelUp_Implementation() override;
	virtual void ShowMagicCircle_Implementation(UMaterialInterface* DecalMaterial = nullptr) override;
	virtual void HideMagicCircle_Implementation() override;

protected:
	/**
	 * Wires up ASC, AttributeSet, HUD, and ASC delegates.
	 * Called from both PossessedBy (server) and OnRep_PlayerState (client) to cover all cases.
	 */
	virtual void InitAbilityActorInfo() override;
	
	virtual void OnRep_Stunned() override;
	virtual void OnRep_Burned() override;

	UPROPERTY(VisibleAnywhere)
	TObjectPtr<UNiagaraComponent> LevelUpNiagaraComponent;

	UPROPERTY(VisibleAnywhere)
	TObjectPtr<UCameraComponent> TopDownCameraComponent;

	UPROPERTY(VisibleAnywhere)
	TObjectPtr<USpringArmComponent> CameraBoom;

private:
	UFUNCTION(NetMulticast, Reliable)
	void MultiCastLevelUpEffect();
};
