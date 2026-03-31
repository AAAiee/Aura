// @Copyright HaolunYuan  https://github.com/AAAiee/Aura.git

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "AbilitySystemInterface.h"
#include "Interaction/CombatInterface.h"
#include "AuraCharacterBase.generated.h"

class UAbilitySystemComponent;
class UAttributeSet;
class UGameplayEffect;
class UGameplayAbility;
class USkeletalMeshComponent;

/**
 * Abstract base class for all Aura characters (player and enemies).
 *
 * Key design decisions:
 *   - Implements IAbilitySystemInterface so GAS can find the ASC on any Aura character.
 *   - Marked UCLASS(Abstract) so it cannot be placed in a level directly — only subclasses can.
 *   - ASC and AttributeSet pointers are declared here but CREATED in subclasses, because
 *     the player character stores them on the PlayerState (shared between pawns),
 *     while the enemy creates them directly on itself.
 *   - InitAbilityActorInfo() is virtual so each subclass can wire up the ASC with the correct
 *     Owner/Avatar pair (PlayerState+Pawn for players, self+self for enemies).
 */
UCLASS(Abstract)
class AURA_API AAuraCharacterBase : public ACharacter, public IAbilitySystemInterface, public ICombatInterface
{
	GENERATED_BODY()

public:
	AAuraCharacterBase();

	/*IAbilitySystemInterface — allows GAS to find the ASC on this actor*/
	FORCEINLINE virtual UAbilitySystemComponent* GetAbilitySystemComponent() const override { return AbilitySystemComponent; }
	FORCEINLINE UAttributeSet* GetAttributeSet() const { return AttributeSet; }
	/*ends IAbilitySystemInterface*/

	void AddStartupGameAbilities();
	virtual FVector GetCombatSocketLocation() const override;

protected:
	virtual void BeginPlay() override;

	/**
	 * Override in subclasses to initialize the ASC with the correct Owner/Avatar pair.
	 * - Player: Owner = PlayerState, Avatar = this Pawn
	 * - Enemy:  Owner = this, Avatar = this
	 * ORDER MATTERS: must be called before any GE is applied or UI is initialized.
	 */
	virtual void InitAbilityActorInfo(){};

	/*Helpers that apply a game effect to the character itself*/
	void ApplyGameEffectToSelf(TSubclassOf<UGameplayEffect> GameplayEffectClass, float Level) const;

	/*Helper function to apply default attributes GE (Primary and Secondary) to the character */
	void InitDefaultAttributes();

protected:
	/** Weapon mesh attached to the character's hand socket. TObjectPtr provides lazy loading & tracking. */
	UPROPERTY(EditAnywhere, Category = Combat)
	TObjectPtr<USkeletalMeshComponent> Weapon;

	UPROPERTY(EditDefaultsOnly, Category = Combat)
	FName WeaponTipSocketName = FName("TipSocket");

	/*Ability System Component & Attribute Set pointers — created and initialized in subclasses, but declared here for easy access in C++ and Blueprints*/
	UPROPERTY()
	TObjectPtr<UAbilitySystemComponent> AbilitySystemComponent;

	UPROPERTY()
	TObjectPtr<UAttributeSet> AttributeSet;

	/** 
	* Game effects used to initialize all default attributes
	* Vital depends on secondary, which might depend on primary
	* Init order-> Primary -> Secondary -> Vital
	*/
	UPROPERTY(EditAnywhere, Category = "Default Attributes GE")
	TSubclassOf<UGameplayEffect> PrimaryAttributeInitGE;
	UPROPERTY(EditAnywhere, Category = "Default Attributes GE")
	TSubclassOf<UGameplayEffect>  SecondaryAttributeInitGE;
	UPROPERTY(EditAnywhere, Category = "Default Attributes GE")
	TSubclassOf<UGameplayEffect> VitalAttributeInitGE;

	/**
	 * 
	 */
	UPROPERTY(EditDefaultsOnly, Category= "Ability")
	TArray < TSubclassOf<UGameplayAbility> > StartupAbilityClasses;
};
