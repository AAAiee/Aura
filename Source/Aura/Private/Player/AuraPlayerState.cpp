// @Copyright HaolunYuan


#include "Player/AuraPlayerState.h"
#include "Components/AbilitySystem/AuraAbilitySystemComponent.h"
#include "Components/AbilitySystem/AuraAttributeSet.h"

AAuraPlayerState::AAuraPlayerState()
{
	/**
	 * NetUpdateFrequency ¡ª how often (per second) this actor sends updates to clients.
	 * Default PlayerState is ~1 Hz. We raise it to 100 Hz because GAS attribute changes
	 * (Health, Mana) need to reach the client quickly for responsive UI updates.
	 * NOTE: actual update rate is still capped by the server's tick rate.
	 */
	SetNetUpdateFrequency(100.f);

	/*Create the ASC ¡ª SetIsReplicated(true) enables GE replication to clients.
	 * Mixed mode: server-authoritative for GEs, but allows client-predicted Gameplay Abilities.
	 * The ASC is CREATED here on the PlayerState but INITIALIZED later in
	 * AAuraCharacter::InitAbilityActorInfo() where the Owner/Avatar pair is set. */
	AbilitySystemComponent = CreateDefaultSubobject<UAuraAbilitySystemComponent>("AbilitySystemComponent");
	AbilitySystemComponent->SetIsReplicated(true);
	AbilitySystemComponent->SetReplicationMode(EGameplayEffectReplicationMode::Mixed);

	/*Create the AttributeSet ¡ª this automatically registers it with the ASC
	 * because UE discovers AttributeSets that are subobjects of the ASC's outer (PlayerState). */
	AttributeSet = CreateDefaultSubobject<UAuraAttributeSet>("AttributeSet");
}



