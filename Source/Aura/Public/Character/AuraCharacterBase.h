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
class UAnimMontage;
class UMaterialInstance;
class UMaterialInstanceDynamic;
class USkeletalMeshComponent;
class UNiagaraSystem;
class USoundBase;

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

	/* Ability System Access */
	// Required by GAS so generic systems can resolve the character's ASC through the interface.
	virtual UAbilitySystemComponent* GetAbilitySystemComponent() const override { return AbilitySystemComponent; }

	// Convenience accessor used by widgets and gameplay code that need the concrete AttributeSet pointer.
	UAttributeSet* GetAttributeSet() const { return AttributeSet; }

	/* Startup */
	void AddStartupGameAbilities();

	/* ICombatInterface */
	virtual FVector GetCombatSocketLocation_Implementation(const FGameplayTag& MontageTag) const override;
	virtual UAnimMontage* GetHitReactMontage_Implementation() override;
	virtual bool IsDead_Implementation() const override;
	virtual AActor* GetAvatar_Implementation() override;
	virtual TArray<FTaggedMontage> GetAttackMontages_Implementation() const override { return AttackMontages; }
	virtual UNiagaraSystem* GetBloodEffect_Implementation() const override { return BloodEffect; }
	virtual FTaggedMontage GetTaggedMontageForTag_Implementation(const FGameplayTag& MontageTag) const override;
	virtual int32 GetMinionCount_Implementation() const override { return MinionCount; };
	virtual void IncrementMinionCount_Implementation(int32 IncrementBy) override { MinionCount += IncrementBy; }
	virtual void Die() override;

	/* Death Presentation */
	UFUNCTION(NetMulticast, Reliable)
	virtual void MulticastHandleDeath();

	// Replaces authored materials with per-instance dynamics so each dead character can drive its
	// own dissolve timeline without mutating the shared material asset.
	void Dissolve();
	UFUNCTION(BlueprintImplementableEvent)
	void StartDissolveTimeline(UMaterialInstanceDynamic* MaterialInstance);
	UFUNCTION(BlueprintImplementableEvent)
	void StartWeaponDissolveTimeline(UMaterialInstanceDynamic* MaterialInstance);

protected:
	/* AActor Overrides */
	virtual void BeginPlay() override;

	/* Ability System Setup */
	/**
	 * Override in subclasses to initialize the ASC with the correct Owner/Avatar pair.
	 * - Player: Owner = PlayerState, Avatar = this Pawn
	 * - Enemy:  Owner = this, Avatar = this
	 * ORDER MATTERS: must be called before any GE is applied or UI is initialized.
	 */
	virtual void InitAbilityActorInfo() {}

	// Applies a Gameplay Effect to the character's own ASC at the requested level.
	void ApplyGameEffectToSelf(TSubclassOf<UGameplayEffect> GameplayEffectClass, float Level) const;

	// Applies the default startup attribute effects in primary -> secondary -> vital order.
	virtual void InitDefaultAttributes();

protected:
	/* Combat Authoring */
	UPROPERTY(EditAnywhere, Category = Combat)
	TArray<FTaggedMontage> AttackMontages;

	// Maps montage tags to the socket a combat trace or projectile should originate from.
	UPROPERTY()
	TMap<FGameplayTag, FName> MontageTagToSocketLocation;

	/** Weapon mesh attached to the character's hand socket. TObjectPtr provides lazy loading & tracking. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Combat)
	TObjectPtr<USkeletalMeshComponent> Weapon;

	UPROPERTY(EditAnywhere, Category = Combat)
	FName WeaponTipSocketName = FName("TipSocket");

	UPROPERTY(EditAnywhere, Category = Combat)
	FName LeftHandTipSocketName;

	UPROPERTY(EditAnywhere, Category = Combat)
	FName RightHandTipSocketName;

	UPROPERTY(EditAnywhere, Category = Combat)
	FName TailTipSocketName;

	/* Ability System State */
	// Created by subclasses, but cached here so shared character code can use the same access path.
	UPROPERTY()
	TObjectPtr<UAbilitySystemComponent> AbilitySystemComponent;

	UPROPERTY()
	TObjectPtr<UAttributeSet> AttributeSet;

	/* Default Attribute Effects */
	/**
	 * Game effects used to initialize all default attributes.
	 * Vital depends on secondary, which might depend on primary.
	 * Init order: primary -> secondary -> vital.
	 */
	UPROPERTY(EditAnywhere, Category = "Default Attributes GE")
	TSubclassOf<UGameplayEffect> PrimaryAttributeInitGE;

	UPROPERTY(EditAnywhere, Category = "Default Attributes GE")
	TSubclassOf<UGameplayEffect> SecondaryAttributeInitGE;

	UPROPERTY(EditAnywhere, Category = "Default Attributes GE")
	TSubclassOf<UGameplayEffect> VitalAttributeInitGE;

	/* Startup Abilities */
	// Gameplay abilities granted once the authoritative server finishes initializing this character.
	UPROPERTY(EditDefaultsOnly, Category = "Ability")
	TArray<TSubclassOf<UGameplayAbility>> StartupAbilityClasses;

	/* Animation */
	// Montage chosen by this character for non-fatal hit-react feedback.
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Combat Effect")
	TObjectPtr<UAnimMontage> HitReactMontage;

	/* Death Presentation Materials */
	// Authored base materials that are swapped to dynamic instances during the death dissolve.
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	TObjectPtr<UMaterialInstance> CharacterDisolveMaterialInstance;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	TObjectPtr<UMaterialInstance> WeaponDisolveMaterialInstance;

	/* Runtime State */
	bool bDead = false;

	// Cosmetic effect spawned when combat damage asks this character to show blood feedback.
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Combat Effect")
	TObjectPtr<UNiagaraSystem> BloodEffect;

	// One-shot sound used by the replicated death presentation.
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Combat Effect")
	TObjectPtr<USoundBase> DeathSound;

	// Number of active minions owned by this combatant, exposed through ICombatInterface.
	int32 MinionCount = 0;
};
