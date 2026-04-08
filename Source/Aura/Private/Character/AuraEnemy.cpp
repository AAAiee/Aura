// @Copyright HaolunYuan

#include "Character/AuraEnemy.h"
#include "Aura/Aura.h"
#include "Components/AbilitySystem/AuraAbilitySystemComponent.h"
#include "Components/AbilitySystem/AuraAttributeSet.h"
#include "Components/CapsuleComponent.h"
#include "Components/WidgetComponent.h"
#include "UI/WidgetController/AuraWidgetController.h"
#include "UI/WidgetComponent/ActorStatusWidgetComponent.h"
#include "Components/AbilitySystem/AuraAbilitySystemLibrary.h"


AAuraEnemy::AAuraEnemy()
{
	PrimaryActorTick.bCanEverTick = true;

	/*Collision Setup — make the mesh block the Visibility channel so the player's
	 * cursor trace (ECC_Visibility) can detect this enemy for highlighting.
	 * Ignore Camera so the spring arm doesn't collide with enemy meshes. */
	/*moved to base*/

	/*use capsule to as it's more versatile in terms of heights*/
	GetCapsuleComponent()->SetGenerateOverlapEvents(true);
	GetCapsuleComponent()->SetCollisionResponseToChannel(ECC_Projectile, ECR_Overlap);

	GetMesh()->SetGenerateOverlapEvents(false);
	GetMesh()->SetCollisionResponseToChannel(ECC_Visibility, ECR_Block);

	/*GAS Setup — enemy owns ASC and AttributeSet directly (no PlayerState).
	 * Replication Mode: Minimal — no GE prediction needed for AI-controlled pawns.
	 * Compare with player's Mixed mode in AuraPlayerState. */
	AbilitySystemComponent = CreateDefaultSubobject<UAuraAbilitySystemComponent>("AbilitySystemComponent");
	AbilitySystemComponent->SetIsReplicated(true); 
	AbilitySystemComponent->SetReplicationMode(EGameplayEffectReplicationMode::Minimal);
	AttributeSet = CreateDefaultSubobject<UAuraAttributeSet>("AttributeSet");

	HealthBarComponent = CreateDefaultSubobject<UActorStatusWidgetComponent>(TEXT("EnemyHealthBar"));
	HealthBarComponent->SetupAttachment(GetRootComponent()); 
}

void AAuraEnemy::HighLightActor()
{
	if (bIsHighlighted) return;
	bIsHighlighted = true;

	/*Enable Custom Depth Rendering for Highlighting Effect*/
	GetMesh()->SetRenderCustomDepth(true);
	GetMesh()->SetCustomDepthStencilValue(CUSTOM_DEPTH_RED);

	/*Also highlight the weapon if any*/
	if (Weapon)
	{
		Weapon->SetRenderCustomDepth(true);
		Weapon->SetCustomDepthStencilValue(CUSTOM_DEPTH_RED);
	}

}

void AAuraEnemy::UnhighLightActor()
{
	if (!bIsHighlighted) return;
	bIsHighlighted = false;

	/*Disable Custom Depth Rendering*/
	GetMesh()->SetRenderCustomDepth(false);

	/*Also Disable highlight for weapon if any*/
	if (Weapon)
	{
		Weapon->SetRenderCustomDepth(false); 
	}
}


void AAuraEnemy::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}


void AAuraEnemy::BeginPlay()
{
	Super::BeginPlay();

	/*
	 * Enemy startup flow:
	 *   1. Build the ASC's ActorInfo so ability / attribute queries know Owner + Avatar.
	 *   2. On the server, apply the class-driven startup attributes.
	 *   3. On every instance, initialize the status widget so the replicated AttributeSet can drive UI.
	 *
	 * The split between server-only stat setup and shared widget setup keeps authority rules clear
	 * while still allowing clients to render the enemy's health bar from replicated state.
	 */
	InitAbilityActorInfo();

	// Enemy stats are authoritative gameplay state, so only the server should seed them.
	if (HasAuthority())
	{
		InitDefaultAttributes();
	}

	InitializeStatusWidget();

}

/**
 * Enemy's ASC init — Owner and Avatar are both "this" (the enemy itself).
 * Compare with AAuraCharacter where Owner=PlayerState and Avatar=Pawn.
 * AbilityActorInfoSet() binds the OnGameplayEffectApplied delegate on the ASC.
 */
void AAuraEnemy::InitAbilityActorInfo()
{
	check(AbilitySystemComponent);
	AbilitySystemComponent->InitAbilityActorInfo(this, this);
	Cast<UAuraAbilitySystemComponent>(AbilitySystemComponent)->AbilityActorInfoSet();
}

void AAuraEnemy::InitDefaultAttributes()
{
	// Class-based defaults are now centralized in the ability-system library so both Aura and
	// enemy archetypes can pull from the same data-driven startup pipeline.
	UAuraAbilitySystemLibrary::InitializeDefaultAttributes(this, CharacterClass, AbilitySystemComponent, EnemyLevel);
}

void AAuraEnemy::InitializeStatusWidget()
{
	check(HealthBarComponent);

	/*
	 * Enemy widgets do not have a player controller or player state like HUD widgets do.
	 * Instead, we pass only the actor-facing ASC + AttributeSet references so the status bar
	 * can subscribe directly to the replicated gameplay data it needs.
	 */
	// Enemy does not have state /player controller references, so we pass in nullptr for those parameters. The widget controller should be designed to handle null references for non-player characters.
	const FWidgetControllerParameters Parameters(nullptr, nullptr, AbilitySystemComponent, AttributeSet);
	HealthBarComponent->InitializeWidgetController(Parameters); 

}
