// @Copyright HaolunYuan

#include "Character/AuraEnemy.h"
#include "Aura/Aura.h"
#include "Components/AbilitySystem/AuraAbilitySystemComponent.h"
#include "Components/AbilitySystem/AuraAttributeSet.h"



AAuraEnemy::AAuraEnemy()
{
	PrimaryActorTick.bCanEverTick = true;

	/*Block trace by player's cursor */
	GetMesh()->SetCollisionResponseToChannel(ECC_Visibility, ECR_Block);

	/*Ability System*/
	AbilitySystemComponent = CreateDefaultSubobject<UAuraAbilitySystemComponent>("AbilitySystemComponent");
	AbilitySystemComponent->SetIsReplicated(true); 

	// Minimal for AI Controlled Enemy
	AbilitySystemComponent->SetReplicationMode(EGameplayEffectReplicationMode::Minimal);
	AttributeSet = CreateDefaultSubobject<UAuraAttributeSet>("AttributeSet");
}

void AAuraEnemy::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}


void AAuraEnemy::BeginPlay()
{
	Super::BeginPlay();

	// Enemy owns ASC , so we initialize it here in the constructor
	check(AbilitySystemComponent); 
	AbilitySystemComponent->InitAbilityActorInfo(this, this);

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
