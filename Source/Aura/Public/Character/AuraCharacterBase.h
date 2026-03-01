// @Copyright HaolunYuan  https://github.com/AAAiee/Aura.git

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "AbilitySystemInterface.h"
#include "AuraCharacterBase.generated.h"

class UAbilitySystemComponent;
class UAttributeSet;

/**
 * Abstract base class for all Aura characters (player and enemies).
 *
 * Key design decisions:
 *   - Implements IAbilitySystemInterface so GAS can find the ASC on any Aura character.
 *   - Marked UCLASS(Abstract) so it cannot be placed in a level directly ¡ª only subclasses can.
 *   - ASC and AttributeSet pointers are declared here but CREATED in subclasses, because
 *     the player character stores them on the PlayerState (shared between pawns),
 *     while the enemy creates them directly on itself.
 *   - InitAbilityActorInfo() is virtual so each subclass can wire up the ASC with the correct
 *     Owner/Avatar pair (PlayerState+Pawn for players, self+self for enemies).
 */
UCLASS(Abstract)
class AURA_API AAuraCharacterBase : public ACharacter, public IAbilitySystemInterface
{
	GENERATED_BODY()

public:
	AAuraCharacterBase();

	/*IAbilitySystemInterface ¡ª allows GAS to find the ASC on this actor*/
	FORCEINLINE virtual UAbilitySystemComponent* GetAbilitySystemComponent() const override { return AbilitySystemComponent; }
	FORCEINLINE UAttributeSet* GetAttributeSet() const { return AttributeSet; }

protected:
	virtual void BeginPlay() override;

	/**
	 * Override in subclasses to initialize the ASC with the correct Owner/Avatar pair.
	 * - Player: Owner = PlayerState, Avatar = this Pawn
	 * - Enemy:  Owner = this, Avatar = this
	 * ORDER MATTERS: must be called before any GE is applied or UI is initialized.
	 */
	virtual void InitAbilityActorInfo() {};

protected:
	/*Combat Components Begins*/
	/** Weapon mesh attached to the character's hand socket. TObjectPtr provides lazy loading & tracking. */
	UPROPERTY(EditAnywhere, Category = Combat)
	TObjectPtr<USkeletalMeshComponent> Weapon;
	/*Combat Components Ends*/

	/*GAS References Begins ¡ª pointers only; subclasses create the actual objects*/
	UPROPERTY()
	TObjectPtr<UAbilitySystemComponent> AbilitySystemComponent;

	UPROPERTY()
	TObjectPtr<UAttributeSet> AttributeSet;
	/*GAS References Ends*/
};
