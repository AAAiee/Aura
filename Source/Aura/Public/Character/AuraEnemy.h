// @Copyright HaolunYuan

#pragma once

#include "CoreMinimal.h"
#include "Character/AuraCharacterBase.h"
#include "GameplayTagContainer.h"
#include "Interaction/EnemyInterface.h"
#include "Interaction/Highlightable.h"
#include "Interaction/Summonable.h"
#include "AuraEnemy.generated.h"

class AAuraAIController;
class UActorStatusWidgetComponent;
class UBehaviorTree;

/**
 * Base class for AI-controlled enemies in Aura.
 *
 * Unlike the player character, enemies own their ASC and AttributeSet directly because they do
 * not rely on PlayerState persistence. They also use Minimal replication because no local player
 * ever predicts their Gameplay Effects.
 */
UCLASS()
class AURA_API AAuraEnemy : public AAuraCharacterBase, public IHighlightable, public IEnemyInterface, public ISummonable
{
	GENERATED_BODY()

public:
	AAuraEnemy();

	/* IHighlightable */
	// Toggles Custom Depth on the mesh (and weapon, when present) for the post-process outline.
	virtual void HighLightActor() override;
	virtual void UnhighLightActor() override;

	/* ICombatInterface */
	FORCEINLINE virtual int32 GetPlayerLevel_Implementation() const override { return EnemyLevel; }
	virtual void Die(const FVector& DeathImpulse) override final;

	/* APawn / AActor */
	virtual void PossessedBy(AController* NewController) override;

	/* IEnemyInterface */
	virtual void SetCombatTarget_Implementation(AActor* InCombatTarget) override;
	virtual AActor* GetCombatTarget_Implementation() const override;

	/* ISummonable */
	// Provides the vertical placement offset used when this enemy is spawned as a summon.
	virtual float GetZOffset() const override;

protected:
	/* AActor Overrides */
	virtual void Tick(float DeltaTime) override;
	virtual void BeginPlay() override;

	/* Ability System Setup */
	/** Enemy owns ASC directly, so Owner=this and Avatar=this. */
	virtual void InitAbilityActorInfo() override;
	virtual void InitDefaultAttributes() override;

private:
	/* Internal Helpers */
	void InitializeStatusWidget();

	// Watches the replicated Combat.HitReact tag count so movement can mirror the current stagger state.
	void OnHitReactTagChanged(const FGameplayTag GameplayTag, int32 NewCount);

private:
	/* Highlight State */
	/** Tracks highlight state to avoid redundant Custom Depth toggles. */
	bool bIsHighlighted = false;

	/* Combat State */
	// Delay before the dead enemy is destroyed, giving the death visuals time to play out.
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Combat, meta = (AllowPrivateAccess = true))
	float LifeSpan = 5.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Character Class Default", meta = (AllowPrivateAccess = true))
	int32 EnemyLevel;

	// Exposed for animation and Blueprint logic that wants to know whether a hit react is active.
	UPROPERTY(BlueprintReadOnly, meta = (AllowPrivateAccess = true))
	bool bHitReacting = false;

	// Cached "alive" walk speed so hit react can temporarily stop movement and then restore it.
	UPROPERTY(EditAnywhere, BlueprintReadOnly, meta = (AllowPrivateAccess = true))
	float BaseSpeed = 250.f;

	// Current combat target exposed to AI and Blueprint combat logic through IEnemyInterface.
	UPROPERTY(BlueprintReadWrite, Category = Combat, meta = (AllowPrivateAccess = true))
	TObjectPtr<AActor> CombatTarget;

	/* UI */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = UI, meta = (AllowPrivateAccess = true))
	TObjectPtr<UActorStatusWidgetComponent> HealthBarComponent;

	/* AI */
	UPROPERTY(EditAnywhere, Category = AI)
	TObjectPtr<UBehaviorTree> BehaviourTree;

	UPROPERTY()
	TObjectPtr<AAuraAIController> AuraAIController;
};
