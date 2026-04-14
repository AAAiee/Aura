// @Copyright HaolunYuan

#pragma once

#include "CoreMinimal.h"
#include "Character/AuraCharacterBase.h"
#include "Interaction//Highlightable.h"
#include "Components/AbilitySystem/Data/CharacterClassInfo.h"
#include "GameplayTagContainer.h"
#include "AuraEnemy.generated.h"

class UActorStatusWidgetComponent;

/**
 * Base class for AI-controlled enemies in Aura.
 *
 * ASC Ownership: Unlike the player character, the enemy OWNS its own ASC and AttributeSet
 * directly (created in the constructor). There is no PlayerState involved because enemies
 * are AI-controlled and don't need to persist attributes across respawns.
 *
 * Replication Mode: Minimal — enemies don't need GE prediction (no local player controls them).
 *
 * Implements IHighlightable so the player's cursor trace can highlight/unhighlight enemies
 * via Custom Depth rendering (post-process outline effect).
 */
UCLASS()
class AURA_API AAuraEnemy : public AAuraCharacterBase, public IHighlightable
{
	GENERATED_BODY()

public:
	AAuraEnemy();

	/* IHighlightable Interface — toggles Custom Depth for post-process outline*/
	virtual void HighLightActor() override;
	virtual void UnhighLightActor() override;
	/* ends IHighlightable Interface*/

	/* ICombatInterface*/
	FORCEINLINE virtual int32 GetPlayerLevel() const override { return EnemyLevel; }
	/* ends ICombatInterface*/

	// Enemy death keeps the actor alive for a short window so ragdoll + dissolve can finish.
	virtual void Die() override final;

protected:
	virtual void Tick(float DeltaTime) override;
	virtual void BeginPlay() override;

	/** Enemy owns ASC directly, so Owner=this, Avatar=this. */
	virtual void InitAbilityActorInfo() override;


	virtual void InitDefaultAttributes() override;


private:
	void InitializeStatusWidget();

	// Watches the replicated Combat.HitReact tag count so movement can mirror the current stagger state.
	void OnHitReactTagChanged(const FGameplayTag GameplayTag, int32 NewCount);


	/** Tracks highlight state to avoid redundant Custom Depth toggles. */
	bool bIsHighlighted = false;

	// Delay before the dead enemy is destroyed, giving the death visuals time to play out.
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Combat, meta=  (AllowPrivateAccess = true))
	float LifeSpan = 5.0f;

	UPROPERTY(EditAnywhere,BlueprintReadOnly, Category = "Character Class Default", meta = (AllowPrivateAccess = true))
	int32 EnemyLevel;

	// Exposed for animation / Blueprint logic that wants to know whether a hit react is active.
	UPROPERTY(BlueprintReadOnly, meta = (AllowPrivateAccess = true))
	bool bHitReacting = false;

	// Cached "alive" walk speed so hit react can temporarily stop movement and then restore it.
	UPROPERTY(BlueprintReadOnly, meta = (AllowPrivateAccess = true))
	float BaseSpeed = 250.f;

	UPROPERTY(EditAnywhere,BlueprintReadOnly, Category = "Character Class Default", meta = (AllowPrivateAccess = true))
	ECharacterClass CharacterClass = ECharacterClass::ECC_Warrior;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = UI, meta = (AllowPrivateAccess = true))
	TObjectPtr<UActorStatusWidgetComponent> HealthBarComponent;

};
