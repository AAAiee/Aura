// @Copyright HaolunYuan

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "NiagaraComponent.h"
#include "PassiveNiagaraComponent.generated.h"

/**
 * UPassiveNiagaraComponent
 *
 * Niagara component that turns passive spell visuals on and off from Aura ability-system events.
 *
 * The component listens for passive activation broadcasts from UAuraAbilitySystemComponent and compares
 * the broadcast ability tag against its configured PassiveSpellTag. This lets character Blueprints attach
 * passive visual effects once while the ability system controls when the effect is visible and active.
 *
 * Important functions:
 *   - UPassiveNiagaraComponent() - Disables auto-activation so the passive event owns visibility.
 *   - BeginPlay() - Binds to the Aura ASC immediately or waits for ASC registration.
 *   - OnPassiveActivate() - Activates or deactivates the Niagara effect for the matching passive tag.
 */
UCLASS()
class AURA_API UPassiveNiagaraComponent : public UNiagaraComponent
{
	GENERATED_BODY()

public:
	/* Passive Visual Effects begins */

	/**
	 * @brief Creates the passive Niagara component with auto-activation disabled.
	 *
	 * Passive visuals should only activate after the Aura ability system broadcasts that the matching
	 * passive spell is active.
	 */
	UPassiveNiagaraComponent();

	/* UNiagaraComponent begins */

	/**
	 * @brief Subscribes this component to passive activation events.
	 *
	 * If the owner already has an Aura ability system component, the component binds immediately. Otherwise
	 * it waits for owners implementing ICombatInterface to report ASC registration.
	 *
	 * @note The component does not activate itself during BeginPlay.
	 */
	virtual void BeginPlay() override;

	/* UNiagaraComponent ends */

	/**
	 * @brief Applies a passive activation state when the broadcast tag matches this component.
	 *
	 * @param AbilityTag Passive ability tag broadcast by the Aura ability system component.
	 * @param bActivate Whether the matching passive effect should activate or deactivate.
	 */
	void OnPassiveActivate(const FGameplayTag& AbilityTag, bool bActivate);

	/* Passive Visual Effects ends */

	// Passive visual tuning
	/** Passive spell tag that owns this Niagara visual. */
	UPROPERTY(EditDefaultsOnly)
	FGameplayTag PassiveSpellTag;
};
