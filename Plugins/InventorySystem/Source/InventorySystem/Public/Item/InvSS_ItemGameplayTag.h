#pragma once

#include "NativeGameplayTags.h"

namespace GameItems
{
	namespace Equippable
	{
		namespace Weapon
		{
			UE_DECLARE_GAMEPLAY_TAG_EXTERN(Wand);
		}
	}

	namespace Consumables
	{
		namespace Potion
		{
			UE_DECLARE_GAMEPLAY_TAG_EXTERN(HealthPotion);
			UE_DECLARE_GAMEPLAY_TAG_EXTERN(ManaPotion);
		}

		namespace Crystal
		{
			UE_DECLARE_GAMEPLAY_TAG_EXTERN(HealthCrystal);
			UE_DECLARE_GAMEPLAY_TAG_EXTERN(ManaCrystal);
		}
	}
}
