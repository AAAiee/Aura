// @Copyright HaolunYuan

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "AuraGameModeBase.generated.h"

class UCharacterClassInfo;

/**
 * Game Mode for the Aura project.
 *
 * The GameMode only exists on the SERVER - clients never have a GameMode instance.
 * It controls which default classes are spawned:
 *   - DefaultPawnClass (set in Blueprint: BP_AuraCharacter)
 *   - PlayerControllerClass (set in Blueprint: BP_AuraPlayerController)
 *   - PlayerStateClass (set in Blueprint: BP_AuraPlayerState)
 *   - HUDClass (set in Blueprint: BP_AuraHUD)
 *
 * Currently no custom logic - all configuration is done in the Blueprint subclass.
 */
UCLASS()
class AURA_API AAuraGameModeBase : public AGameModeBase
{
	GENERATED_BODY()

public:
	UPROPERTY(EditDefaultsOnly, Category = "ServerSide GameplayData")
	TObjectPtr<UCharacterClassInfo> CharacterClassInfo;
};
