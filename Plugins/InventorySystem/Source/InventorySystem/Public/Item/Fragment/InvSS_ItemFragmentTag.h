#pragma once

#include "NativeGameplayTags.h"

/**
 * Gameplay tags used to identify item manifest fragment types.
 */
namespace ItemFragmentTag
{
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(GridFragment);
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(IconFragment);
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(StackableFragment);
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(ConsumableFragment);
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(ItemNameFraghment);
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(PrimaryStatFragment);
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(ItemTypeFragment);
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(SellValueFragment);
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(RequiredLevelFragment);
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(ItemFlavourText);
}
