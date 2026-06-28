// @Copyright HaolunYuan

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "NiagaraComponent.h"
#include "DebuffNiagaraComponent.generated.h"

/**
 * UDebuffNiagaraComponent
 *
 * Niagara component that visualizes a specific GameplayTag-driven debuff on its owning actor.
 *
 * The component listens for NewOrRemoved events for DebuffTag on the owner's ability system component and
 * activates only while the tag is present and the owner is alive. It also listens to Aura combat death
 * notifications so lingering debuff visuals shut down immediately when the owning actor dies.
 *
 * Important functions:
 *   - UDebuffNiagaraComponent() - Starts the visual hidden and inactive.
 *   - BeginPlay() - Binds gameplay tag and death delegates.
 *   - OnDebuffTagChangedCallback() - Mirrors debuff tag state into Niagara activation.
 *   - OnOwnerDeath() - Stops the visual when the owning actor dies.
 */
UCLASS()
class AURA_API UDebuffNiagaraComponent : public UNiagaraComponent
{
	GENERATED_BODY()

public:
	/**
	 * @brief Creates the debuff Niagara component hidden and inactive.
	 *
	 * Debuff visuals are controlled by GameplayTag counts and owner death events rather than automatic
	 * component activation.
	 */
	UDebuffNiagaraComponent();

	/* UNiagaraComponent begins */

	/**
	 * @brief Subscribes this component to the owner's debuff tag and death events.
	 *
	 * Binds immediately when the owner already has an ASC, or waits for ASC registration through
	 * ICombatInterface when the ASC is not available yet.
	 */
	virtual void BeginPlay() override;

	/* UNiagaraComponent ends */

	// Debuff visual tuning
	/** Gameplay tag that activates this debuff visual while present on the owner's ASC. */
	UPROPERTY(EditDefaultsOnly)
	FGameplayTag DebuffTag;

private:
	/* Debuff Visual State begins */

	/**
	 * @brief Mirrors the debuff tag count into Niagara visibility and activation.
	 *
	 * Activates the component only when the owner is valid, alive, implements Aura combat behavior, and the
	 * tracked debuff tag count is greater than zero.
	 *
	 * @param CallbackTag Gameplay tag that triggered the callback.
	 * @param NewCount New number of active instances for the tracked debuff tag.
	 */
	void OnDebuffTagChangedCallback(const FGameplayTag CallbackTag, int32 NewCount);

	/**
	 * @brief Stops the debuff visual when the owning combat actor dies.
	 *
	 * @param DeadActor Actor reported by the combat death delegate.
	 */
	UFUNCTION()
	void OnOwnerDeath(AActor* DeadActor);

	/* Debuff Visual State ends */
};
