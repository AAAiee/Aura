// @Copyright HaolunYuan

#pragma once

#include "CoreMinimal.h"
#include "Character/AuraCharacterBase.h"
#include "Interaction//Highlightable.h"
#include "AuraEnemy.generated.h"

/**
 * Base class for AI-controlled enemies in Aura.
 *
 * ASC Ownership: Unlike the player character, the enemy OWNS its own ASC and AttributeSet
 * directly (created in the constructor). There is no PlayerState involved because enemies
 * are AI-controlled and don't need to persist attributes across respawns.
 *
 * Replication Mode: Minimal ¡ª enemies don't need GE prediction (no local player controls them).
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

	/* IHighlightable Interface ¡ª toggles Custom Depth for post-process outline*/
	virtual void HighLightActor() override;
	virtual void UnhighLightActor() override;
	/* ends IHighlightable Interface*/

	/* ICombatInterface*/
	FORCEINLINE virtual int32 GetPlayerLevel() const override { return EnemyLevel; }
	/* ends ICombatInterface*/

protected:
	virtual void Tick(float DeltaTime) override;
	virtual void BeginPlay() override;

	/** Enemy owns ASC directly, so Owner=this, Avatar=this. */
	virtual void InitAbilityActorInfo() override;

private:
	/** Tracks highlight state to avoid redundant Custom Depth toggles. */
	bool bIsHighlighted = false;

	UPROPERTY(EditAnywhere,BlueprintReadOnly, Category = "Character Class Default", meta = (AllowPrivateAccess = true))
	int32 EnemyLevel; 
};
