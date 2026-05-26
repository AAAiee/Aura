// @Copyright HaolunYuan

#include "AuraAbilityTypes.h"

UScriptStruct* FAuraGameplayEffectContext::GetScriptStruct() const
{
	return FAuraGameplayEffectContext::StaticStruct();
}

bool FAuraGameplayEffectContext::NetSerialize(FArchive& Ar, class UPackageMap* Map, bool& bOutSuccess)
{
	/*
	 * Custom context serialization follows the same pattern as the engine context:
	 *   1. Serialize the base FGameplayEffectContext first so instigator/source/hit-result data is intact.
	 *   2. Pack optional Aura fields into a bitmask so absent values cost only one bit.
	 *   3. Serialize the payloads whose bits are present on both save and load.
	 *
	 * This matters because ExecCalcs run on the server, but the UI on the owning client still needs
	 * the resolved hit facts (blocked, crit, debuff data) when the effect context is copied over.
	 */
	bool bBaseSuccess = true;
	FGameplayEffectContext::NetSerialize(Ar, Map, bBaseSuccess);

	uint8 RepBits = 0;
	if (Ar.IsSaving())
	{
		if (bIsCriticalHit)
		{
			RepBits |= 1 << 0;
		}

		if (bIsBlockedHit)
		{
			RepBits |= 1 << 1;
		}

		if (bShouldHitReact)
		{
			RepBits |= 1 << 2;
		}
		if (bIsSuccessfulDebuff)
		{
			RepBits |= 1 << 3;
		}
		if (DebuffDamage > 0.f)
		{
			RepBits |= 1 << 4;
		}
		if (DebuffDuration > 0.f)
		{
			RepBits |= 1 << 5;
		}
		if (DebuffFrequency > 0.f)
		{
			RepBits |= 1 << 6;
		}
		if (DebuffDamageTypeTag.IsValid())
		{
			RepBits |= 1 << 7;
		}
		if (!DeathImpulse.IsNearlyZero())
		{
			RepBits |= 1 << 8;
		}
		if (!KnockBackForce.IsNearlyZero())
		{
			RepBits |= 1 << 9;
		}
	}

	// The first four bits are booleans: presence means true, absence means false. Later bits gate
	// optional scalar/tag payloads so we do not serialize default debuff data for every hit.
	Ar.SerializeBits(&RepBits, 10);

	if (RepBits & (1 << 4))
	{
		Ar << DebuffDamage;
	}

	if (RepBits & (1 << 5))
	{
		Ar << DebuffDuration;
	}

	if (RepBits & (1 << 6))
	{
		Ar << DebuffFrequency;
	}

	if (RepBits & (1 << 7))
	{
		if (!DebuffDamageTypeTag.IsValid())
		{
			DebuffDamageTypeTag = MakeShared<FGameplayTag>();
		}
		DebuffDamageTypeTag->NetSerialize(Ar, Map, bOutSuccess);
	}
	if (RepBits & (1 << 8))
	{
		DeathImpulse.NetSerialize(Ar, Map, bOutSuccess);
	}
	if (RepBits & (1 << 9))
	{
		KnockBackForce.NetSerialize(Ar, Map, bOutSuccess);
	}

	if (Ar.IsLoading())
	{
		bIsCriticalHit = (RepBits & (1 << 0)) != 0;
		bIsBlockedHit = (RepBits & (1 << 1)) != 0;
		bShouldHitReact = (RepBits & (1 << 2)) != 0;
		bIsSuccessfulDebuff = (RepBits & (1 << 3)) != 0;
	}

	bOutSuccess = bBaseSuccess;
	return true;
}

FAuraGameplayEffectContext* FAuraGameplayEffectContext::Duplicate() const
{
	FAuraGameplayEffectContext* NewContext = new FAuraGameplayEffectContext();
	*NewContext = *this;

	if (GetHitResult())
	{
		// Hit results own nested data, so duplicate them deeply when GAS clones the context.
		NewContext->AddHitResult(*GetHitResult(), true);
	}

	return NewContext;
}
