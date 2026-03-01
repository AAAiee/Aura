// @Copyright HaolunYuan

#include "Components/AbilitySystem/AuraAttributeSet.h"
#include "Net/UnrealNetwork.h"
#include "GameplayEffectExtension.h"
#include "GameFramework/Character.h"

/**
 * Constructor ¡ª use Init<Attribute>() to set both the Base and Current value.
 * Init functions should ONLY be called here; use Set<Attribute>() at runtime.
 */
UAuraAttributeSet::UAuraAttributeSet()
{
	InitHealth(50.f);
	InitMaxHealth(100.f);
	InitMana(10.f);
	InitMaxMana(50.f);
}

/**
 * PreAttributeChange is called BEFORE the attribute value is modified.
 * It receives a mutable reference to the new value, so we can clamp it.
 *
 * IMPORTANT: This only clamps the "proposed" value. If a Gameplay Effect uses an
 * Override modifier (sets the value directly), it bypasses this clamp.
 * Always re-clamp in PostGameplayEffectExecute for safety.
 */
void UAuraAttributeSet::PreAttributeChange(const FGameplayAttribute& Attribute, float& NewValue)
{
	Super::PreAttributeChange(Attribute, NewValue);

	if (Attribute == GetHealthAttribute())
	{
		NewValue = FMath::Clamp(NewValue, 0.f, GetMaxHealth());

	}

	if (Attribute == GetManaAttribute())
	{
		NewValue = FMath::Clamp(NewValue, 0.f, GetMaxMana());
	}
}

/**
 * Registers each attribute for network replication.
 * - COND_None: replicate to all clients (no condition).
 * - REPNOTIFY_Always: fire OnRep even if the value hasn't changed.
 *   This is important because GAS may set the same value during a prediction correction,
 *   and we still want the client to process the update.
 */
void UAuraAttributeSet::GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const
{
	DOREPLIFETIME_CONDITION_NOTIFY(UAuraAttributeSet, Health, COND_None, REPNOTIFY_Always); 

	DOREPLIFETIME_CONDITION_NOTIFY(UAuraAttributeSet, MaxHealth, COND_None, REPNOTIFY_Always); 

	DOREPLIFETIME_CONDITION_NOTIFY(UAuraAttributeSet, Mana, COND_None, REPNOTIFY_Always);

	DOREPLIFETIME_CONDITION_NOTIFY(UAuraAttributeSet, MaxMana, COND_None, REPNOTIFY_Always);
}

/**
 * PostGameplayEffectExecute runs AFTER a GameplayEffect has modified an attribute.
 * The value is already committed at this point ¡ª this is the authoritative place to:
 *   - Re-clamp attributes (e.g., Health <= MaxHealth)
 *   - Check for death (Health <= 0)
 *   - Apply secondary effects (e.g., damage numbers, hit reactions)
 *
 * We extract Source/Target info into FEffectProperties for easy access.
 */
void UAuraAttributeSet::PostGameplayEffectExecute(const struct FGameplayEffectModCallbackData& Data)
{
	Super::PostGameplayEffectExecute(Data);

	FEffectProperties Props;
	SetEffectProperties(Data, Props);

}

/*
 * Rep Notify Callbacks
 * Called on the CLIENT when the server replicates a new attribute value.
 * GAMEPLAYATTRIBUTE_REPNOTIFY tells GAS to update its internal state so that
 * GetHealth() returns the replicated value and attribute-change delegates fire.
 */

void UAuraAttributeSet::OnRep_Health(const FGameplayAttributeData& OldHealth) const
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UAuraAttributeSet, Health, OldHealth); 
}

void UAuraAttributeSet::OnRep_MaxHealth(const FGameplayAttributeData& OldMaxHealth) const
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UAuraAttributeSet, MaxHealth, OldMaxHealth); 
}

void UAuraAttributeSet::OnRep_Mana(const FGameplayAttributeData& OldMana) const
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UAuraAttributeSet, Mana, OldMana); 
}

void UAuraAttributeSet::OnRep_MaxMana(const FGameplayAttributeData& OldMaxMana) const
{

	GAMEPLAYATTRIBUTE_REPNOTIFY(UAuraAttributeSet, MaxMana, OldMaxMana);

}

/**
 * Extracts Source and Target information from a Gameplay Effect execution.
 *
 * The Data parameter contains everything about the effect that just executed.
 * We walk the chain:
 *   EffectSpec ¡ú Context ¡ú OriginalInstigator ASC ¡ú AbilityActorInfo ¡ú AvatarActor/Controller/Character
 *
 * Source = who applied the effect (e.g., the enemy that dealt damage)
 * Target = who received the effect (e.g., the player taking damage)
 *
 * This is useful for damage numbers, kill credits, knockback direction, etc.
 */
void UAuraAttributeSet::SetEffectProperties(const FGameplayEffectModCallbackData& Data, FEffectProperties& Props)
{ 
	// Source: the actor that caused/instigated this effect
	Props.GameplayEffectContextHandle = Data.EffectSpec.GetContext();
	Props.SourceASC = Props.GameplayEffectContextHandle.GetOriginalInstigatorAbilitySystemComponent(); 
	if (Props.SourceASC)
	{
		auto& AIF = Props.SourceASC->AbilityActorInfo;
		if (AIF.IsValid() && AIF->AvatarActor.IsValid())
		{
			Props.SourceAvatarActor = AIF->AvatarActor.Get();
			Props.SourceController = AIF->PlayerController.Get();

			// If no PlayerController (e.g., AI-controlled pawn), fall back to GetController()
			if (Props.SourceController == nullptr  && Props.SourceAvatarActor != nullptr)
			{ 
				Props.SourceController = (Cast<APawn>(Props.SourceAvatarActor))->GetController();
			}
		}
		
		if (Props.SourceController)
		{
			Props.SourceCharacter = Cast<ACharacter>(Props.SourceController->GetPawn());
		}

		// Target: the actor that received this effect (Data.Target is the target's ASC)
		if (Data.Target.AbilityActorInfo.IsValid()  && Data.Target.AbilityActorInfo->AvatarActor.IsValid())
		{
			Props.TargetAvatarActor = Data.Target.AbilityActorInfo->AvatarActor.Get(); 
			Props.TargetController = Data.Target.AbilityActorInfo->PlayerController.Get();
			ACharacter* TargetCharacter = Cast<ACharacter>(Props.TargetAvatarActor);
			Props.TargetASC = Data.Target.AbilityActorInfo->AbilitySystemComponent.Get();
		}
	}
}
