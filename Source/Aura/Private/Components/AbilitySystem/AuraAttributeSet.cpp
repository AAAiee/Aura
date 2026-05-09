// @Copyright HaolunYuan

#include "Components/AbilitySystem/AuraAttributeSet.h"

#include "AuraGameTagManager.h"
#include "Components/AbilitySystem/AuraAbilitySystemLibrary.h"
#include "GameFramework/Character.h"
#include "GameFramework/Pawn.h"
#include "GameplayEffectExtension.h"
#include "Interaction/CombatInterface.h"
#include "Net/UnrealNetwork.h"
#include "Player/AuraPlayerController.h"
#include "AuraLogCategory.h"
#include "AbilitySystemBlueprintLibrary.h"
#include "Interaction/PlayerInterface.h"

/**
 * Constructor - use Init<Attribute>() to set both the Base and Current value.
 * Init functions should ONLY be called here; use Set<Attribute>() at runtime.
 */
UAuraAttributeSet::UAuraAttributeSet()
{
}

/**
 * PreAttributeChange is called BEFORE the attribute value is modified.
 * It receives a mutable reference to the new value, so we can clamp it.
 *
 * IMPORTANT: This only clamps the proposed value. If a Gameplay Effect uses an
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
 * - REPNOTIFY_Always: fire OnRep even if the value has not changed.
 *
 * The order mirrors the header groups so future attributes can be added in one predictable
 * place: vital, primary, secondary core stats, then secondary resistances.
 */
void UAuraAttributeSet::GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	/*Vital Attributes OnRep*/
	DOREPLIFETIME_CONDITION_NOTIFY(UAuraAttributeSet, Health, COND_None, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(UAuraAttributeSet, Mana, COND_None, REPNOTIFY_Always);

	/*Primary Attributes OnRep*/
	DOREPLIFETIME_CONDITION_NOTIFY(UAuraAttributeSet, Strength, COND_None, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(UAuraAttributeSet, Intelligence, COND_None, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(UAuraAttributeSet, Resilience, COND_None, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(UAuraAttributeSet, Vigor, COND_None, REPNOTIFY_Always);

	/*Secondary Attributes OnRep*/
	DOREPLIFETIME_CONDITION_NOTIFY(UAuraAttributeSet, MaxHealth, COND_None, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(UAuraAttributeSet, MaxMana, COND_None, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(UAuraAttributeSet, Armor, COND_None, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(UAuraAttributeSet, ArmorPenetration, COND_None, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(UAuraAttributeSet, BlockChance, COND_None, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(UAuraAttributeSet, CriticalHitChance, COND_None, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(UAuraAttributeSet, CriticalHitDamage, COND_None, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(UAuraAttributeSet, CriticalHitResilience, COND_None, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(UAuraAttributeSet, HealthRegeneration, COND_None, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(UAuraAttributeSet, ManaRegeneration, COND_None, REPNOTIFY_Always);

	/*Secondary Resistance Attributes OnRep*/
	DOREPLIFETIME_CONDITION_NOTIFY(UAuraAttributeSet, FireResistance, COND_None, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(UAuraAttributeSet, LightningResistance, COND_None, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(UAuraAttributeSet, ArcaneResistance, COND_None, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(UAuraAttributeSet, PhysicalResistance, COND_None, REPNOTIFY_Always);
}

/**
 * PostGameplayEffectExecute runs AFTER a GameplayEffect has modified an attribute.
 * The value is already committed at this point. This is the authoritative place to:
 *   - Re-clamp attributes (e.g., Health <= MaxHealth)
 *   - Check for death (Health <= 0)
 *   - Apply secondary effects (damage text, hit reactions, combat-result UI flags)
 *
 * We extract Source/Target info into FEffectProperties for easy access.
 */
void UAuraAttributeSet::PostGameplayEffectExecute(const struct FGameplayEffectModCallbackData& Data)
{
	Super::PostGameplayEffectExecute(Data);

	FEffectProperties Props;
	SetEffectProperties(Data, Props);

	if (Data.EvaluatedData.Attribute == GetHealthAttribute())
	{
		SetHealth(FMath::Clamp(GetHealth(), 0.f, GetMaxHealth()));
		UE_LOG(LogTemp, Warning, TEXT("Change of Health on %s, Changed to %f"), *GetNameSafe(Props.TargetAvatarActor), GetHealth());
	}

	if (Data.EvaluatedData.Attribute == GetManaAttribute())
	{
		SetMana(FMath::Clamp(GetMana(), 0.f, GetMaxMana()));
	}

	/*
	 * IncomingDamage is a meta attribute populated by the damage ExecCalc. We consume it here so the
	 * AttributeSet remains the single place that translates "damage landed" into health changes,
	 * death, hit react, and UI feedback.
	 */
	if (Data.EvaluatedData.Attribute == GetIncomingDamageAttribute())
	{
		const float DamageLocalCopy = GetIncomingDamage();
		SetIncomingDamage(0.f);

		if (DamageLocalCopy > 0.f)
		{
			const float NewHealth = GetHealth() - DamageLocalCopy;
			SetHealth(FMath::Clamp(NewHealth, 0.0f, GetMaxHealth()));

			const bool bFatal = NewHealth <= 0.f;

			if (bFatal)
			{
				ICombatInterface* CombatInterface = Cast<ICombatInterface>(Props.TargetAvatarActor);
				if (CombatInterface)
				{
					// Death stays polymorphic: the AttributeSet decides that the target died, while
					// the concrete combatant decides how its death sequence should play out.
					CombatInterface->Die();
				}

				SendXPEvent(Props); 
			}
			else if (UAuraAbilitySystemLibrary::ShouldHitReact(Props.GameplayEffectContextHandle))
			{
				// Non-fatal damage routes through the shared hit-react tag so movement, animation, and
				// gameplay abilities can all observe the same temporary combat state.
				FGameplayTagContainer Tags;
				Tags.AddTag(FAuraGameTagManager::Get().Combat_HitReact);
				Props.TargetASC->TryActivateAbilitiesByTag(Tags);
			}

			// The ExecCalc writes these booleans onto the custom effect context before outputting
			// IncomingDamage, so the AttributeSet can forward accurate result styling to the UI.
			const bool bBlockedHit = UAuraAbilitySystemLibrary::IsBlockedHit(Props.GameplayEffectContextHandle);
			const bool bCriticalHit = UAuraAbilitySystemLibrary::IsCriticalHit(Props.GameplayEffectContextHandle);
			ShowFloatingText(Props, DamageLocalCopy, bBlockedHit, bCriticalHit);
		}
	}

	if (Data.EvaluatedData.Attribute == GetInComingXPRewardAttribute())
	{
		const float LocalIncomingXPRewardCopy = GetInComingXPReward();
		SetInComingXPReward(0);

		//TODO::See If we shoud level up
		if (Props.SourceCharacter && Props.SourceCharacter->Implements<UPlayerInterface>())
		{

			const int32 CurrentPlayerLevel = ICombatInterface::Execute_GetPlayerLevel(Props.SourceCharacter); 
			const int32 CurrentXP = IPlayerInterface::Execute_GetXP(Props.SourceCharacter);
			const int32 PLayerLevel_Updated = IPlayerInterface::Execute_FindLevelForXP(Props.SourceCharacter, CurrentXP + LocalIncomingXPRewardCopy);
			const int32 NumLevelsUp = PLayerLevel_Updated - CurrentPlayerLevel;
			if (NumLevelsUp > 0)
			{
				const int32 AttributePointsReward = IPlayerInterface::Execute_GetAttributePointsReward(Props.SourceCharacter, PLayerLevel_Updated);
				const int32 SpellPointsReward = IPlayerInterface::Execute_GetSpellPointsReward(Props.SourceCharacter, PLayerLevel_Updated);	


				IPlayerInterface::Execute_AddToPlayerLevel(Props.SourceCharacter, NumLevelsUp); 
				IPlayerInterface::Execute_AddToAttributePoint(Props.SourceCharacter, AttributePointsReward);
				IPlayerInterface::Execute_AddToSpellPoint(Props.SourceCharacter, SpellPointsReward);
				

				/*Can't just topup the max health and mana here, level just changed + max health/mana depends on it*/
				bTopOffHealth = true;
				bTopoffMana = true;

				IPlayerInterface::Execute_LevelUp(Props.SourceCharacter);
			}
			IPlayerInterface::Execute_AddToXP(Props.SourceCharacter, LocalIncomingXPRewardCopy);
		}

	}

}


void UAuraAttributeSet::PostAttributeChange(const FGameplayAttribute& Attribute, float OldValue, float NewValue)
{
	Super::PostAttributeChange(Attribute, OldValue, NewValue);

	if (Attribute == GetMaxHealthAttribute() && bTopOffHealth)
	{
		SetHealth(GetMaxHealth());
		bTopOffHealth = false;
	}

	if (Attribute == GetMaxManaAttribute() && bTopoffMana)
	{
		SetMana(GetMaxMana());
		bTopoffMana = false; 
	}
}

/*
 * Rep Notify Callbacks
 * Called on the client when the server replicates a new attribute value.
 * GAMEPLAYATTRIBUTE_REPNOTIFY tells GAS to update its internal state so that
 * GetHealth() returns the replicated value and attribute-change delegates fire.
 */

/* Vital Attributes */
void UAuraAttributeSet::OnRep_Health(const FGameplayAttributeData& OldHealth) const
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UAuraAttributeSet, Health, OldHealth);
}

void UAuraAttributeSet::OnRep_Mana(const FGameplayAttributeData& OldMana) const
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UAuraAttributeSet, Mana, OldMana);
}

/* Primary Attributes */
void UAuraAttributeSet::OnRep_Strength(const FGameplayAttributeData& OldStrength) const
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UAuraAttributeSet, Strength, OldStrength);
}

void UAuraAttributeSet::OnRep_Intelligence(const FGameplayAttributeData& OldIntelligence) const
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UAuraAttributeSet, Intelligence, OldIntelligence);
}

void UAuraAttributeSet::OnRep_Resilience(const FGameplayAttributeData& OldResilience) const
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UAuraAttributeSet, Resilience, OldResilience);
}

void UAuraAttributeSet::OnRep_Vigor(const FGameplayAttributeData& OldVigor) const
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UAuraAttributeSet, Vigor, OldVigor);
}

/* Secondary Attributes */
void UAuraAttributeSet::OnRep_MaxHealth(const FGameplayAttributeData& OldMaxHealth) const
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UAuraAttributeSet, MaxHealth, OldMaxHealth);
}

void UAuraAttributeSet::OnRep_MaxMana(const FGameplayAttributeData& OldMaxMana) const
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UAuraAttributeSet, MaxMana, OldMaxMana);
}

void UAuraAttributeSet::OnRep_Armor(const FGameplayAttributeData& OldArmor) const
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UAuraAttributeSet, Armor, OldArmor);
}

void UAuraAttributeSet::OnRep_ArmorPenetration(const FGameplayAttributeData& OldArmorPenetration) const
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UAuraAttributeSet, ArmorPenetration, OldArmorPenetration);
}

void UAuraAttributeSet::OnRep_BlockChance(const FGameplayAttributeData& OldBlockChance) const
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UAuraAttributeSet, BlockChance, OldBlockChance);
}

void UAuraAttributeSet::OnRep_CriticalHitChance(const FGameplayAttributeData& OldCriticalHitChance) const
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UAuraAttributeSet, CriticalHitChance, OldCriticalHitChance);
}

void UAuraAttributeSet::OnRep_CriticalHitDamage(const FGameplayAttributeData& OldCriticalHitDamage) const
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UAuraAttributeSet, CriticalHitDamage, OldCriticalHitDamage);
}

void UAuraAttributeSet::OnRep_CriticalHitResilience(const FGameplayAttributeData& OldCriticalHitResilience) const
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UAuraAttributeSet, CriticalHitResilience, OldCriticalHitResilience);
}

void UAuraAttributeSet::OnRep_HealthRegeneration(const FGameplayAttributeData& OldHealthRegeneration) const
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UAuraAttributeSet, HealthRegeneration, OldHealthRegeneration);
}

void UAuraAttributeSet::OnRep_ManaRegeneration(const FGameplayAttributeData& OldManaRegeneration) const
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UAuraAttributeSet, ManaRegeneration, OldManaRegeneration);
}

/* Secondary Resistance Attributes */
void UAuraAttributeSet::OnRep_FireResistance(const FGameplayAttributeData& OldFireResistance) const
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UAuraAttributeSet, FireResistance, OldFireResistance);
}

void UAuraAttributeSet::OnRep_LightningResistance(const FGameplayAttributeData& OldLightningResistance) const
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UAuraAttributeSet, LightningResistance, OldLightningResistance);
}

void UAuraAttributeSet::OnRep_ArcaneResistance(const FGameplayAttributeData& OldArcaneResistance) const
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UAuraAttributeSet, ArcaneResistance, OldArcaneResistance);
}

void UAuraAttributeSet::OnRep_PhysicalResistance(const FGameplayAttributeData& OldPhysicalResistance) const
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UAuraAttributeSet, PhysicalResistance, OldPhysicalResistance);
}

void UAuraAttributeSet::ShowFloatingText(const FEffectProperties& Props, const float Damage, const bool bBlockedHit, const bool bCriticalHit)
{
	if (!Props.SourceAvatarActor || !Props.TargetAvatarActor || Props.SourceAvatarActor == Props.TargetAvatarActor)
	{
		return;
	}

	if (!Props.TargetCharacter )
	{
		return;
	}

	if (AAuraPlayerController* PC = Cast<AAuraPlayerController>(Props.SourceController))
	{
		// Damage numbers are cosmetic attacker feedback, so we send them through the
		// attacking player's controller instead of storing extra UI state on the target.
		PC->Client_ShowDamageNumber(Damage, Props.TargetCharacter, bBlockedHit, bCriticalHit);
		return;
	}
	if (AAuraPlayerController* PC = Cast<AAuraPlayerController>(Props.TargetController))
	{
		// Damage numbers are cosmetic attacker feedback, so we send them through the
		// attacking player's controller instead of storing extra UI state on the target.
		PC->Client_ShowDamageNumber(Damage, Props.TargetCharacter, bBlockedHit, bCriticalHit);
	}

}

/**
 * Extracts Source and Target information from a Gameplay Effect execution.
 *
 * The Data parameter contains everything about the effect that just executed.
 * We walk the chain:
 *   EffectSpec -> Context -> OriginalInstigator ASC -> AbilityActorInfo -> AvatarActor/Controller/Character
 *
 * Source = who applied the effect (e.g., the enemy that dealt damage)
 * Target = who received the effect (e.g., the player taking damage)
 *
 * This is useful for damage numbers, kill credits, knockback direction, etc.
 */
void UAuraAttributeSet::SetEffectProperties(const FGameplayEffectModCallbackData& Data, FEffectProperties& Props)
{
	// Source: the actor that caused/instigated this effect.
	Props.GameplayEffectContextHandle = Data.EffectSpec.GetContext();
	Props.SourceASC = Props.GameplayEffectContextHandle.GetOriginalInstigatorAbilitySystemComponent();
	if (Props.SourceASC)
	{
		const TSharedPtr<FGameplayAbilityActorInfo>& SourceActorInfo = Props.SourceASC->AbilityActorInfo;
		if (SourceActorInfo.IsValid() && SourceActorInfo->AvatarActor.IsValid())
		{
			Props.SourceAvatarActor = SourceActorInfo->AvatarActor.Get();
			Props.SourceController = SourceActorInfo->PlayerController.Get();

			// If no PlayerController exists (for example, AI-controlled pawns), fall back to the pawn controller.
			if (!Props.SourceController)
			{
				if (APawn* SourcePawn = Cast<APawn>(Props.SourceAvatarActor))
				{
					Props.SourceController = SourcePawn->GetController();
				}
			}
		}

		if (Props.SourceController)
		{
			Props.SourceCharacter = Cast<ACharacter>(Props.SourceController->GetPawn());
		}
	}

	// Target: the actor that received this effect (Data.Target is the target's ASC).
	if (Data.Target.AbilityActorInfo.IsValid() && Data.Target.AbilityActorInfo->AvatarActor.IsValid())
	{
		Props.TargetAvatarActor = Data.Target.AbilityActorInfo->AvatarActor.Get();
		Props.TargetController = Data.Target.AbilityActorInfo->PlayerController.Get();
		Props.TargetCharacter = Cast<ACharacter>(Props.TargetAvatarActor);
		Props.TargetASC = Data.Target.AbilityActorInfo->AbilitySystemComponent.Get();
	}
}

void UAuraAttributeSet::SendXPEvent(const FEffectProperties& Props)
{
	if (Props.TargetCharacter->Implements<UCombatInterface>())
	{
		const int32 TargetLevel = ICombatInterface::Execute_GetPlayerLevel(Props.TargetCharacter);
		const ECharacterClass TargetClass = ICombatInterface::Execute_GetCharacterClass(Props.TargetCharacter); 
		const int32 XPReward = UAuraAbilitySystemLibrary::GetXPRewardForClassAndLevel(Props.TargetCharacter,TargetClass, TargetLevel);	
			

		const FAuraGameTagManager& AuraGameplayTags = FAuraGameTagManager::Get();
		FGameplayEventData PayLoad;
		PayLoad.EventTag = AuraGameplayTags.Attributes_Meta_XP;
		PayLoad.EventMagnitude = XPReward; 

		UAbilitySystemBlueprintLibrary::SendGameplayEventToActor(Props.SourceCharacter, AuraGameplayTags.Attributes_Meta_XP, PayLoad);
	}
 

}
