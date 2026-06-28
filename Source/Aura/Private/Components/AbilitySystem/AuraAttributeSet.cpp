// @Copyright HaolunYuan

#include "Components/AbilitySystem/AuraAttributeSet.h"

#include "AbilitySystemBlueprintLibrary.h"
#include "AuraGameTagManager.h"
#include "Components/AbilitySystem/AuraAbilitySystemLibrary.h"
#include "GameFramework/Character.h"
#include "GameFramework/Controller.h"
#include "GameFramework/Pawn.h"
#include "GameplayEffect.h"
#include "GameplayEffectComponents/TargetTagsGameplayEffectComponent.h"
#include "GameplayEffectExtension.h"
#include "Interaction/CombatInterface.h"
#include "Interaction/PlayerInterface.h"
#include "Net/UnrealNetwork.h"
#include "Player/AuraPlayerController.h"

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

	/* Vital Attributes */
	DOREPLIFETIME_CONDITION_NOTIFY(UAuraAttributeSet, Health, COND_None, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(UAuraAttributeSet, Mana, COND_None, REPNOTIFY_Always);

	/* Primary Attributes */
	DOREPLIFETIME_CONDITION_NOTIFY(UAuraAttributeSet, Strength, COND_None, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(UAuraAttributeSet, Intelligence, COND_None, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(UAuraAttributeSet, Resilience, COND_None, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(UAuraAttributeSet, Vigor, COND_None, REPNOTIFY_Always);

	/* Secondary Attributes */
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

	/* Secondary Resistance Attributes */
	DOREPLIFETIME_CONDITION_NOTIFY(UAuraAttributeSet, FireResistance, COND_None, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(UAuraAttributeSet, LightningResistance, COND_None, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(UAuraAttributeSet, ArcaneResistance, COND_None, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(UAuraAttributeSet, PhysicalResistance, COND_None, REPNOTIFY_Always);
}

void UAuraAttributeSet::HandleIncomingHealth(const FGameplayEffectModCallbackData& Data)
{
	if (Data.EvaluatedData.Attribute == GetHealthAttribute())
	{
		SetHealth(FMath::Clamp(GetHealth(), 0.f, GetMaxHealth()));
	}
}

void UAuraAttributeSet::HandleIncomingMana(const FGameplayEffectModCallbackData& Data)
{
	if (Data.EvaluatedData.Attribute == GetManaAttribute())
	{
		SetMana(FMath::Clamp(GetMana(), 0.f, GetMaxMana()));
	}
}

void UAuraAttributeSet::HandleIncomingDamage(const FEffectProperties& Props)
{
	/*
	 * IncomingDamage is a meta attribute: it is only a mailbox for the ExecCalc result.
	 * The AttributeSet immediately copies and clears it so the next GameplayEffect starts with
	 * a clean mailbox, then translates that value into durable state such as Health and death.
	 */
	const float DamageLocalCopy = GetIncomingDamage();
	SetIncomingDamage(0.f);

	if (DamageLocalCopy > 0.f)
	{
		FGameplayEffectContextHandle AuraContextHandle = Props.GameplayEffectContextHandle;
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
				CombatInterface->Die(UAuraAbilitySystemLibrary::GetDeathImpulse(AuraContextHandle));
			}

			SendXPEvent(Props);
		}
		else 
		{
			// Non-fatal damage routes through the shared hit-react tag so movement, animation, and
			// gameplay abilities can all observe the same temporary combat state.
			ICombatInterface* TargetAvatarCombatInterface = Cast<ICombatInterface>(Props.TargetAvatarActor);
			if (UAuraAbilitySystemLibrary::ShouldHitReact(AuraContextHandle) 
				&& TargetAvatarCombatInterface && !ICombatInterface::Execute_IsBeingShocked(Props.TargetAvatarActor))
			{
				FGameplayTagContainer Tags;
				Tags.AddTag(FAuraGameTagManager::Get().Combat_HitReact);
				Props.TargetASC->TryActivateAbilitiesByTag(Tags);
			}
			
			const FVector KnockBackForce = UAuraAbilitySystemLibrary::GetKnockBackForce(AuraContextHandle);
			if (!KnockBackForce.IsNearlyZero(10.f))
			{
				Props.TargetCharacter->LaunchCharacter(KnockBackForce, true, true) ; 
			}
		}

		// The ExecCalc writes these booleans onto the custom effect context before outputting
		// IncomingDamage, so the AttributeSet can forward accurate result styling to the UI.
		const bool bBlockedHit = UAuraAbilitySystemLibrary::IsBlockedHit(Props.GameplayEffectContextHandle);
		const bool bCriticalHit = UAuraAbilitySystemLibrary::IsCriticalHit(Props.GameplayEffectContextHandle);
		ShowFloatingText(Props, DamageLocalCopy, bBlockedHit, bCriticalHit);

		if (UAuraAbilitySystemLibrary::IsSuccessfulDebuff(AuraContextHandle))
		{
			/*
			 * Debuffs are authored as data on the original damage spec, but the actual timed effect
			 * is created here after we know the hit landed and the debuff roll succeeded. The dynamic
			 * GameplayEffect applies periodic IncomingDamage back through this same AttributeSet path,
			 * which keeps damage/death/floating-text behavior centralized.
			 */
			check(Props.SourceASC);
			check(Props.TargetASC);

			const FAuraGameTagManager& TagManager = FAuraGameTagManager::Get();
			FGameplayEffectContextHandle EffectContextHandle = Props.SourceASC->MakeEffectContext();
			EffectContextHandle.AddSourceObject(Props.SourceAvatarActor);

			const FGameplayTag DamageTypeTag = UAuraAbilitySystemLibrary::GetDamageTypeTag(AuraContextHandle);
			const FGameplayTag DebuffTypeTag = TagManager.DamageTypeToDebuffType.FindRef(DamageTypeTag);
			const float DebuffDamage = UAuraAbilitySystemLibrary::GetDebuffDamage(AuraContextHandle);
			const float DebuffDuration = UAuraAbilitySystemLibrary::GetDebuffDuration(AuraContextHandle);
			const float DebuffFrequency = UAuraAbilitySystemLibrary::GetDebuffFrequency(AuraContextHandle);

			const FName DebuffEffectName = FName(*FString::Printf(TEXT("DynamicDebuffEffect_%s"), *DebuffTypeTag.ToString()));

			UGameplayEffect* Effect = NewObject<UGameplayEffect>(
				GetTransientPackage(),
				DebuffEffectName);

			Effect->DurationPolicy = EGameplayEffectDurationType::HasDuration;
			Effect->Period = DebuffFrequency;
			Effect->DurationMagnitude = FScalableFloat(DebuffDuration);
			Effect->StackingType = EGameplayEffectStackingType::AggregateBySource;
			Effect->StackLimitCount = 1;

			// UE 5.6 grants owned target tags through a GameplayEffectComponent instead of the
			// deprecated InheritableOwnedTagsContainer. The granted Debuff.* tag is what other
			// systems can query while this periodic effect is active on the target.
			FInheritedTagContainer GrantedDebuffTags;
			GrantedDebuffTags.AddTag(DebuffTypeTag);
			if (DebuffTypeTag.MatchesTagExact(TagManager.Debuff_Stun))
			{
				FGameplayTagContainer TagsToCancel;
				TagsToCancel.AddTag(TagManager.Ability_Fire_FireBolt);
				TagsToCancel.AddTag(TagManager.Ability_Lightning_Electrocute);
				Props.TargetASC->CancelAbilities(&TagsToCancel);
				
				GrantedDebuffTags.AddTag(TagManager.Player_BlockInputHeld);
				GrantedDebuffTags.AddTag(TagManager.PLayer_BlockCursorTrace);
				GrantedDebuffTags.AddTag(TagManager.PLayer_BlockInputPressed);
				GrantedDebuffTags.AddTag(TagManager.PLayer_BlockInputReleased);
			}
			UTargetTagsGameplayEffectComponent& TargetTagsComponent = Effect->FindOrAddComponent<UTargetTagsGameplayEffectComponent>();
			TargetTagsComponent.SetAndApplyTargetTagChanges(GrantedDebuffTags);
			

			// Periodic effects execute their modifier each tick. Writing to IncomingDamage here means
			// every burn/stun/arcane tick reuses the normal damage post-processing path.
			const int32 Index = Effect->Modifiers.Num();
			Effect->Modifiers.Add(FGameplayModifierInfo());
			FGameplayModifierInfo& ModifierInfo = Effect->Modifiers[Index];

			ModifierInfo.Attribute = GetIncomingDamageAttribute();
			ModifierInfo.ModifierOp = EGameplayModOp::Additive;
			ModifierInfo.ModifierMagnitude = FScalableFloat(DebuffDamage);

			FGameplayEffectSpec MutableSpec(Effect, EffectContextHandle, 1.0f);
			FGameplayEffectContextHandle MutableContextHandle = MutableSpec.GetContext();
			UAuraAbilitySystemLibrary::SetDamageTypeTag(MutableContextHandle, DamageTypeTag);

			Props.SourceASC->ApplyGameplayEffectSpecToTarget(MutableSpec, Props.TargetASC);
		}
	}
}

void UAuraAttributeSet::HandleIncomingXP(const FEffectProperties& Props)
{
	const float LocalIncomingXPRewardCopy = GetInComingXPReward();
	SetInComingXPReward(0);

	// XP rewards are resolved on the source character so PlayerState owns level/stat progression.
	if (Props.SourceCharacter && Props.SourceCharacter->Implements<UPlayerInterface>())
	{
		const int32 CurrentPlayerLevel = ICombatInterface::Execute_GetPlayerLevel(Props.SourceCharacter);
		const int32 CurrentXP = IPlayerInterface::Execute_GetXP(Props.SourceCharacter);
		const int32 UpdatedPlayerLevel = IPlayerInterface::Execute_FindLevelForXP(Props.SourceCharacter, CurrentXP + LocalIncomingXPRewardCopy);
		const int32 NumLevelsUp = UpdatedPlayerLevel - CurrentPlayerLevel;
		if (NumLevelsUp > 0)
		{
			IPlayerInterface::Execute_AddToPlayerLevel(Props.SourceCharacter, NumLevelsUp);
			
			int32 AttributePointsReward = 0;
			int32 SpellPointsReward = 0;
			
			for (int32 i = 0; i < NumLevelsUp; i++)
			{
				SpellPointsReward += IPlayerInterface::Execute_GetSpellPointsReward(Props.SourceCharacter, CurrentPlayerLevel + i);
				AttributePointsReward += IPlayerInterface::Execute_GetAttributePointsReward(Props.SourceCharacter, CurrentPlayerLevel + i);
			}

			IPlayerInterface::Execute_AddToAttributePoint(Props.SourceCharacter, AttributePointsReward);
			IPlayerInterface::Execute_AddToSpellPoint(Props.SourceCharacter, SpellPointsReward);

			// MaxHealth and MaxMana will be recalculated by follow-up GEs, so top off after those caps change.
			bTopOffHealth = true;
			bTopoffMana = true;

			IPlayerInterface::Execute_LevelUp(Props.SourceCharacter);
		}
		IPlayerInterface::Execute_AddToXP(Props.SourceCharacter, LocalIncomingXPRewardCopy);
	}
}

bool UAuraAttributeSet::CheckIfCharacterIsDead(const FEffectProperties& Props) const
{
	if (!Props.TargetCharacter || !Props.TargetCharacter->Implements<UCombatInterface>())
	{
		return false;
	}

	return ICombatInterface::Execute_IsDead(Props.TargetCharacter);
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
void UAuraAttributeSet::PostGameplayEffectExecute(const FGameplayEffectModCallbackData& Data)
{
	Super::PostGameplayEffectExecute(Data);

	FEffectProperties Props;
	SetEffectProperties(Data, Props);

	if (CheckIfCharacterIsDead(Props))
	{
		return;
	}

	HandleIncomingHealth(Data);
	HandleIncomingMana(Data);

	/*
	 * IncomingDamage is a meta attribute populated by the damage ExecCalc. We consume it here so the
	 * AttributeSet remains the single place that translates "damage landed" into health changes,
	 * death, hit react, and UI feedback.
	 */
	if (Data.EvaluatedData.Attribute == GetIncomingDamageAttribute())
	{
		HandleIncomingDamage(Props);
	}

	if (Data.EvaluatedData.Attribute == GetInComingXPRewardAttribute())
	{
		HandleIncomingXP(Props);
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

	if (!Props.TargetCharacter)
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
	if (Props.TargetCharacter && Props.TargetCharacter->Implements<UCombatInterface>())
	{
		const int32 TargetLevel = ICombatInterface::Execute_GetPlayerLevel(Props.TargetCharacter);
		const ECharacterClass TargetClass = ICombatInterface::Execute_GetCharacterClass(Props.TargetCharacter);
		const int32 XPReward = UAuraAbilitySystemLibrary::GetXPRewardForClassAndLevel(Props.TargetCharacter, TargetClass, TargetLevel);

		const FAuraGameTagManager& AuraGameplayTags = FAuraGameTagManager::Get();
		FGameplayEventData PayLoad;
		PayLoad.EventTag = AuraGameplayTags.Attributes_Meta_XP;
		PayLoad.EventMagnitude = XPReward;

		UAbilitySystemBlueprintLibrary::SendGameplayEventToActor(Props.SourceCharacter, AuraGameplayTags.Attributes_Meta_XP, PayLoad);
	}
}
