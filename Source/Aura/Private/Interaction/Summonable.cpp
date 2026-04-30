// @Copyright HaolunYuan

#include "Interaction/Summonable.h"

float ISummonable::GetZOffset() const
{
	// Default keeps generic summonables grounded at their actor origin.
	return 0.f;
}
