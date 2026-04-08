// @Copyright HaolunYuan


#include "Components/AbilitySystem/Data/CharacterClassInfo.h"

FCharacterClassDefaultInfo UCharacterClassInfo::GetDefaultInfoForClass(ECharacterClass CharacterClass) const
{
	// Startup class data is required authoring, so FindChecked gives us a loud failure if a new
	// enum entry is added but the corresponding asset data was never populated.
	return CharacterClassDefaultInfoMap.FindChecked(CharacterClass); 
}
