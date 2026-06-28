#include "Item/InvSS_ItemGameplayTag.h"

namespace GameItems
{
	namespace Equippable
	{
		namespace Weapon
		{
			UE_DEFINE_GAMEPLAY_TAG(Wand, "GameItems.Equippable.Weapon.Wand")
		}
	}

	namespace Consumables
	{
		namespace Potion
		{
			UE_DEFINE_GAMEPLAY_TAG(HealthPotion, "GameItems.Consumables.Potion.HeathPotion");
			UE_DEFINE_GAMEPLAY_TAG(ManaPotion, "GameItems.Consumables.Potion.ManaPotion");
		}

		namespace Crystal
		{
			UE_DEFINE_GAMEPLAY_TAG(HealthCrystal, "GameItems.Consumables.Crystal.HealthCrystal");
			UE_DEFINE_GAMEPLAY_TAG(ManaCrystal, "GameItems.Consumables.Crystal.ManaCrystal");
		}
	}
}
