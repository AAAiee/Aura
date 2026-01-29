// @Copyright HaolunYuan

#include "Character/AuraEnemy.h"
#include "Aura/Aura.h"


AAuraEnemy::AAuraEnemy()
{
	PrimaryActorTick.bCanEverTick = true;

	/*Block trace by player's cursor */
	GetMesh()->SetCollisionResponseToChannel(ECC_Visibility, ECR_Block);
}

void AAuraEnemy::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
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
