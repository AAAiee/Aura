// @Copyright HaolunYuan

#include "Interaction/CombatInterface.h"

int32 ICombatInterface::GetPlayerLevel() const
{
	// Default zero keeps lightweight test actors safe if they opt into the interface incrementally.
	return 0;
}
