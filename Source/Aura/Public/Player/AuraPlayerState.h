// @Copyright HaolunYuan

#pragma once

#include "CoreMinimal.h"
#include "AbilitySystemInterface.h"
#include "GameFramework/PlayerState.h"
#include "AuraPlayerState.generated.h"

DECLARE_MULTICAST_DELEGATE_OneParam(FOnPlayerStatChanged, int32 /*StatValue*/)

class UAbilitySystemComponent;
class UAttributeSet;
class ULevelUpInfo;

/**
 * Aura PlayerState - the authoritative owner of the player's ASC and AttributeSet.
 *
 * Why the PlayerState owns the ASC (instead of the Pawn):
 *   - PlayerState persists across pawn deaths/respawns, so attributes (Health, Mana) survive.
 *   - If the player ever switches pawns, the same ASC follows automatically.
 *   - This is the recommended GAS pattern for player-controlled characters.
 *
 * Implements IAbilitySystemInterface so any code that has a PlayerState pointer
 * can call GetAbilitySystemComponent() directly.
 *
 * Replication Mode: Mixed - supports both server-authoritative GEs and client-predicted GAs.
 * Compare with Enemy's Minimal mode (no prediction needed for AI).
 */
UCLASS()
class AURA_API AAuraPlayerState : public APlayerState, public IAbilitySystemInterface
{
	GENERATED_BODY()

public:
	AAuraPlayerState();

	/* IAbilitySystemInterface */
	FORCEINLINE virtual UAbilitySystemComponent* GetAbilitySystemComponent() const override { return AbilitySystemComponent; }

	FORCEINLINE UAttributeSet* GetAttributeSet() const { return AttributeSet; }

	/* Player Progression Access */
	FORCEINLINE int32 GetPlayerXP() const { return PlayerXP; }
	FORCEINLINE int32 GetPlayerLevel() const { return PlayerLevel; }
	FORCEINLINE int32 GetAttributePoints() const { return PlayerAttributePoints; }
	FORCEINLINE int32 GetSpellPoints() const { return PlayerSpellPoints; }

	void AddPlayerLevel(const int32 LevelsToAdd);
	void SetPlayerLevel(const int32 LevelToSet);

	void AddPlayerXP(const int32 XPToAdd);
	void SetPlayerXP(const int32 XPToSet);

	void AddAttributePoints(const int32 PointsToAdd);
	void SetAttributePoints(const int32 PointsToSet);

	void AddSpellPoints(const int32 PointsToAdd);
	void SetSpellPoints(const int32 PointsToSet);

	FOnPlayerStatChanged OnXPChanged;
	FOnPlayerStatChanged OnLevelChanged;
	FOnPlayerStatChanged OnAttributePointsChanged;
	FOnPlayerStatChanged OnSpellPointsChanged;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	TObjectPtr<ULevelUpInfo> LevelUpInfo;

protected:
	UFUNCTION()
	void OnRep_PlayerXP(const int32 OldPlayerXP);

	UFUNCTION()
	void OnRep_PlayerLevel(const int32 OldPlayerLevel);

	UFUNCTION()
	void OnRep_PlayerAttributePoints(const int32 OldPlayerAttributePoints);

	UFUNCTION()
	void OnRep_PlayerSpellPoints(const int32 OldPlayerSpellPoints);

	virtual void GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const override;

	/* GAS components: created here, but initialized in AAuraCharacter::InitAbilityActorInfo(). */
	UPROPERTY()
	TObjectPtr<UAbilitySystemComponent> AbilitySystemComponent;

	UPROPERTY()
	TObjectPtr<UAttributeSet> AttributeSet;

private:
	UPROPERTY(BlueprintReadOnly, ReplicatedUsing = OnRep_PlayerLevel, meta = (AllowPrivateAccess = true))
	int32 PlayerLevel = 1;

	UPROPERTY(BlueprintReadOnly, ReplicatedUsing = OnRep_PlayerXP, meta = (AllowPrivateAccess = true))
	int32 PlayerXP = 0;

	UPROPERTY(BlueprintReadOnly, ReplicatedUsing = OnRep_PlayerAttributePoints, meta = (AllowPrivateAccess = true))
	int32 PlayerAttributePoints = 0;

	UPROPERTY(BlueprintReadOnly, ReplicatedUsing = OnRep_PlayerSpellPoints, meta = (AllowPrivateAccess = true))
	int32 PlayerSpellPoints = 0;
};

