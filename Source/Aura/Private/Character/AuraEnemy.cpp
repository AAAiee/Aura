// @Copyright HaolunYuan

#include "Character/AuraEnemy.h"
#include "Aura/Aura.h"
#include "Components/AbilitySystem/AuraAbilitySystemComponent.h"
#include "Components/AbilitySystem/AuraAttributeSet.h"



AAuraEnemy::AAuraEnemy()
{
	PrimaryActorTick.bCanEverTick = true;

	/*Collision Setup ！ make the mesh block the Visibility channel so the player's
	 * cursor trace (ECC_Visibility) can detect this enemy for highlighting.
	 * Ignore Camera so the spring arm doesn't collide with enemy meshes. */
	GetMesh()->SetCollisionResponseToChannel(ECC_Visibility, ECR_Block);
	GetMesh()->SetCollisionResponseToChannel(ECC_Camera, ECR_Ignore); 

	/*GAS Setup ！ enemy owns ASC and AttributeSet directly (no PlayerState).
	 * Replication Mode: Minimal ！ no GE prediction needed for AI-controlled pawns.
	 * Compare with player's Mixed mode in AuraPlayerState. */
	AbilitySystemComponent = CreateDefaultSubobject<UAuraAbilitySystemComponent>("AbilitySystemComponent");
	AbilitySystemComponent->SetIsReplicated(true); 
	AbilitySystemComponent->SetReplicationMode(EGameplayEffectReplicationMode::Minimal);
	AttributeSet = CreateDefaultSubobject<UAuraAttributeSet>("AttributeSet");
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

}

/**
 * Enemy's ASC init ！ Owner and Avatar are both "this" (the enemy itself).
 * Compare with AAuraCharacter where Owner=PlayerState and Avatar=Pawn.
 * AbilityActorInfoSet() binds the OnGameplayEffectApplied delegate on the ASC.
 */
void AAuraEnemy::InitAbilityActorInfo()
{
	check(AbilitySystemComponent);
	AbilitySystemComponent->InitAbilityActorInfo(this, this);
	Cast<UAuraAbilitySystemComponent>(AbilitySystemComponent)->AbilityActorInfoSet();
}


