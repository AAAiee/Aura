// @Copyright HaolunYuan

#pragma once

#include "CoreMinimal.h"
#include "AttributeSet.h"
#include "AbilitySystemComponent.h"
#include "AuraAttributeSet.generated.h"

/**
 * ATTRIBUTE_ACCESSORS is a convenience macro provided by GAS.
 * For each attribute (e.g. Health), it generates four helper functions:
 *   - GetHealthAttribute()  ！ returns the FGameplayAttribute handle (used to bind delegates)
 *   - GetHealth()           ！ returns the current float value
 *   - SetHealth(float)      ！ sets the value directly (server-only, skips clamping)
 *   - InitHealth(float)     ！ sets the base AND current value (used in constructors)
 */
#define ATTRIBUTE_ACCESSORS(ClassName, PropertyName) \
	GAMEPLAYATTRIBUTE_PROPERTY_GETTER(ClassName, PropertyName) \
	GAMEPLAYATTRIBUTE_VALUE_GETTER(PropertyName) \
	GAMEPLAYATTRIBUTE_VALUE_SETTER(PropertyName) \
	GAMEPLAYATTRIBUTE_VALUE_INITTER(PropertyName)


/**
 * Holds Source and Target information extracted from a Gameplay Effect execution.
 * Populated in PostGameplayEffectExecute so we know WHO applied the effect and WHO received it.
 * Useful for damage calculations, kill credits, UI feedback, etc.
 */
USTRUCT()
struct FEffectProperties
{
	GENERATED_BODY()

	FEffectProperties() = default;

	/*Target ！ the actor receiving the effect*/
	UPROPERTY()
	TObjectPtr<UAbilitySystemComponent> TargetASC;

	/*Source ！ the actor that caused/applied the effect*/
	UPROPERTY()
	TObjectPtr<UAbilitySystemComponent> SourceASC;

	UPROPERTY()
    TObjectPtr<AActor> SourceAvatarActor; 

	UPROPERTY()
	TObjectPtr<AActor> TargetAvatarActor;

	UPROPERTY()
	TObjectPtr<AController> SourceController;

	UPROPERTY()
	TObjectPtr<AController> TargetController;

	UPROPERTY()
	TObjectPtr<ACharacter> SourceCharacter;

	UPROPERTY()
	TObjectPtr<ACharacter> TargetCharacter;

	UPROPERTY()
    FGameplayEffectContextHandle  GameplayEffectContextHandle;
};


/**
 * Defines the gameplay attributes (Health, Mana, etc.) for Aura characters.
 *
 * Key GAS lifecycle callbacks used here:
 *   - PreAttributeChange  ！ clamp values BEFORE they are applied (preview only, can be overridden)
 *   - PostGameplayEffectExecute ！ runs AFTER an effect modifies an attribute (final, authoritative)
 *   - OnRep_<Attribute>   ！ client-side replication callback, keeps client in sync with server
 *
 * Replication: each attribute uses REPNOTIFY_Always so the client is notified even if the
 * new value equals the old value (important for UI refresh on respawn/reset).
 */
UCLASS()
class AURA_API UAuraAttributeSet : public UAttributeSet
{
	GENERATED_BODY()
	
public:
	UAuraAttributeSet();

	/*Attribute Accessors ！ generated getters, setters, and initializers for each attribute*/
	ATTRIBUTE_ACCESSORS(UAuraAttributeSet, Health);
	ATTRIBUTE_ACCESSORS(UAuraAttributeSet, MaxHealth); 
	ATTRIBUTE_ACCESSORS(UAuraAttributeSet, Mana); 
	ATTRIBUTE_ACCESSORS(UAuraAttributeSet, MaxMana); 

public:
	/**
	 * Called before an attribute value is changed. Use this to clamp incoming values.
	 * NOTE: This only clamps the "proposed" value ！ if a GE overrides the attribute,
	 * you must re-clamp in PostGameplayEffectExecute as well.
	 */
	virtual void PreAttributeChange(const FGameplayAttribute& Attribute, float& NewValue);

	/** Registers all attributes for replication. Required for any replicated UPROPERTY in an AttributeSet. */
	virtual void GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const override;

	/**
	 * Called after a GameplayEffect has been executed and the attribute value has been committed.
	 * This is the authoritative place to react to attribute changes (e.g., death check, final clamping).
	 */
	virtual void PostGameplayEffectExecute(const struct FGameplayEffectModCallbackData& Data);

private:
	/*Rep Notify Callbacks ！ called on the client when the server replicates a new value*/
	UFUNCTION()
	void OnRep_Health(const FGameplayAttributeData& OldHealth) const;

	UFUNCTION()
	void OnRep_MaxHealth(const FGameplayAttributeData& OldMaxHealth) const;

	UFUNCTION()
	void OnRep_Mana(const FGameplayAttributeData& OldMana) const;

	UFUNCTION()
	void OnRep_MaxMana(const FGameplayAttributeData& OldMaxMana) const;

private:
	/** Extracts Source/Target actor info from a GE execution into an FEffectProperties struct. */
	void SetEffectProperties(const FGameplayEffectModCallbackData& Data, FEffectProperties& Props);

private:
	/*Vital Attributes ！ replicated with REPNOTIFY_Always so the client UI always refreshes*/
	UPROPERTY(BlueprintReadOnly, ReplicatedUsing = OnRep_Health, Category = "Vital Attributes", meta = (AllowPrivateAccess = "true"))
	FGameplayAttributeData Health;

	UPROPERTY(BlueprintReadOnly, ReplicatedUsing = OnRep_MaxHealth, Category = "Vital Attributes", meta = (AllowPrivateAccess = "true"))
	FGameplayAttributeData MaxHealth;

	UPROPERTY(BlueprintReadOnly, ReplicatedUsing = OnRep_Mana, Category = "Vital Attributes", meta = (AllowPrivateAccess = "true"))
	FGameplayAttributeData Mana;

	UPROPERTY(BlueprintReadOnly, ReplicatedUsing = OnRep_MaxMana, Category = "Vital Attributes", meta = (AllowPrivateAccess = "true"))
	FGameplayAttributeData MaxMana;
};

