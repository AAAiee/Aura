// @Copyright HaolunYuan

#include "AuraAbilityTypes.h"

UScriptStruct* FAuraGameplayEffectContext::GetScriptStruct() const
{
	return FAuraGameplayEffectContext::StaticStruct();
}

bool FAuraGameplayEffectContext::NetSerialize(FArchive& Ar, class UPackageMap* Map, bool& bOutSuccess)
{
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
	}

	// The bitmask is enough for booleans: presence means true, absence means false.
	// This keeps the custom payload compact and stable as it rides along with the base GAS context.
	Ar.SerializeBits(&RepBits, 3);

	if (Ar.IsLoading())
	{
		bIsCriticalHit = (RepBits & (1 << 0)) != 0;
		bIsBlockedHit = (RepBits & (1 << 1)) != 0;
		bShouldHitReact = (RepBits & (1 << 2)) != 0;
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
