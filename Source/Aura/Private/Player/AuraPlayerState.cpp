// @Copyright HaolunYuan


#include "Player/AuraPlayerState.h"
#include "Components/AbilitySystem/AuraAbilitySystemComponent.h"
#include "Components/AbilitySystem/AuraAttributeSet.h"
#include "Net/UnrealNetwork.h"

AAuraPlayerState::AAuraPlayerState()
{
	/**
	 * NetUpdateFrequency ¡ª how often (per second) this actor sends updates to clients.
	 * Default PlayerState is ~1 Hz. We raise it to 100 Hz because GAS attribute changes
	 * (Health, Mana) need to reach the client quickly for responsive UI updates.
	 * NOTE: actual update rate is still capped by the server's tick rate.
	 */
	SetNetUpdateFrequency(100.f);

	/*Create the ASC ¡ª SetIsReplicated(true) enables GE replication to clients.
	 * Mixed mode: server-authoritative for GEs, but allows client-predicted Gameplay Abilities.
	 * The ASC is CREATED here on the PlayerState but INITIALIZED later in
	 * AAuraCharacter::InitAbilityActorInfo() where the Owner/Avatar pair is set. */
	AbilitySystemComponent = CreateDefaultSubobject<UAuraAbilitySystemComponent>("AbilitySystemComponent");
	AbilitySystemComponent->SetIsReplicated(true);
	AbilitySystemComponent->SetReplicationMode(EGameplayEffectReplicationMode::Mixed);

	/*Create the AttributeSet ¡ª this automatically registers it with the ASC
	 * because UE discovers AttributeSets that are subobjects of the ASC's outer (PlayerState). */
	AttributeSet = CreateDefaultSubobject<UAuraAttributeSet>("AttributeSet");
}

void AAuraPlayerState::AddPlayerLevel(const int32 LevelsToAdd)
{
	PlayerLevel += LevelsToAdd;
	OnLevelChanged.Broadcast(PlayerLevel);
}

void AAuraPlayerState::SetPlayerLevel(const int32 LevelToSet)
{
	PlayerLevel = LevelToSet; 
	OnLevelChanged.Broadcast(PlayerLevel);
}

void AAuraPlayerState::AddPlayerXP(const int32 XPToAdd)
{
	PlayerXP += XPToAdd;
	OnXPChanged.Broadcast(PlayerXP);
}

void AAuraPlayerState::SetPlayerXP(const int32 XPToSet)
{
	PlayerXP = XPToSet;
	OnXPChanged.Broadcast(PlayerXP);
}

void AAuraPlayerState::AddAttributePoints(const int32 PointsToAdd)
{
	PlayerAttributePoints += PointsToAdd;
	OnAttributePointsChanged.Broadcast(PlayerAttributePoints);
}

void AAuraPlayerState::SetAttributePoints(const int32 PointsToSet)
{
	PlayerAttributePoints = PointsToSet;
	OnAttributePointsChanged.Broadcast(PlayerAttributePoints);
}

void AAuraPlayerState::AddSpellPoints(const int32 PointsToAdd)
{
	PlayerSpellPoints += PointsToAdd; 
	OnSpellPointsChanged.Broadcast(PlayerSpellPoints);
}

void AAuraPlayerState::SetSpellPoints(const int32 PointsToSet)
{
	PlayerSpellPoints = PointsToSet;
	OnSpellPointsChanged.Broadcast(PlayerSpellPoints);
}

void AAuraPlayerState::OnRep_PlayerXP(const int32 OldXP)
{
	OnXPChanged.Broadcast(PlayerXP);
}

void AAuraPlayerState::OnRep_PlayerLevel(const int32 OldPlayerLevel)
{
	OnLevelChanged.Broadcast(PlayerLevel);
}

void AAuraPlayerState::OnRep_PlayerAttributePoints(const int32 OldPlayerAttributePoints)
{
	OnAttributePointsChanged.Broadcast(PlayerAttributePoints);
}

void AAuraPlayerState::OnRep_PlayerSpellPoints(const int32 OldPlayerSpellPoints)
{
	OnSpellPointsChanged.Broadcast(PlayerSpellPoints);
}

void AAuraPlayerState::GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const
{
	DOREPLIFETIME_CONDITION_NOTIFY(AAuraPlayerState, PlayerXP, COND_None, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(AAuraPlayerState, PlayerLevel, COND_None, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(AAuraPlayerState, PlayerAttributePoints, COND_None, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(AAuraPlayerState, PlayerSpellPoints, COND_None, REPNOTIFY_Always);
}

