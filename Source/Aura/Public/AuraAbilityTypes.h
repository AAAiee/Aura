// @Copyright HaolunYuan

#pragma once

#include "CoreMinimal.h"
#include "GameplayEffectTypes.h"
#include "AuraAbilityTypes.generated.h"

/**
 * Aura's custom GameplayEffect context.
 *
 * GAS copies this context into outgoing specs, so it is a good home for small pieces of
 * per-hit metadata that should travel from the server-side ExecCalc to later consumers such
 * as AttributeSets, GameplayCues, and client UI. The actual damage still lives in attributes;
 * these flags only describe how that damage was resolved.
 */
USTRUCT()
struct FAuraGameplayEffectContext : public FGameplayEffectContext
{
	GENERATED_BODY()

public:
	/* FGameplayEffectContext */
	virtual UScriptStruct* GetScriptStruct() const override;
	virtual bool NetSerialize(FArchive& Ar, class UPackageMap* Map, bool& bOutSuccess) override;
	virtual FAuraGameplayEffectContext* Duplicate() const override;

	/* Combat Result Flags */
	bool IsCriticalHit() const { return bIsCriticalHit; }
	bool IsBlockedHit() const { return bIsBlockedHit; }
	bool ShouldHitReact() const { return bShouldHitReact; }

	void SetCriticalHit(bool bInIsCriticalHit) { bIsCriticalHit = bInIsCriticalHit; }
	void SetBlockedHit(bool bInIsBlockedHit) { bIsBlockedHit = bInIsBlockedHit; }
	void SetShouldHitReact(bool bInShouldHitReact) { bShouldHitReact = bInShouldHitReact; }

protected:
	/* Replicated Metadata */
	UPROPERTY()
	bool bIsCriticalHit = false;

	UPROPERTY()
	bool bIsBlockedHit = false;

	UPROPERTY()
	bool bShouldHitReact = false;
};

template<>
struct TStructOpsTypeTraits<FAuraGameplayEffectContext> : public TStructOpsTypeTraitsBase2<FAuraGameplayEffectContext>
{
	enum
	{
		WithNetSerializer = true,
		WithCopy = true
	};
};
