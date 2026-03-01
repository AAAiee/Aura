// @Copyright HaolunYuan  https://github.com/AAAiee/Aura.git


#include "Character/AuraCharacterBase.h"

// Sets default values
AAuraCharacterBase::AAuraCharacterBase()
{
	// Base characters don't need to tick ¡ª subclasses can enable it if needed (e.g., enemy AI)
	PrimaryActorTick.bCanEverTick = false;

	/*Weapon Setup ¡ª attach a skeletal mesh to the hand socket defined on the character's skeleton*/
	Weapon = CreateDefaultSubobject<USkeletalMeshComponent>("Weapon");
	Weapon->SetupAttachment(GetMesh(), FName("WeaponHandSocket"));
	Weapon->SetCollisionEnabled(ECollisionEnabled::NoCollision); // Weapon visuals only; combat uses traces/overlaps
}

// Called when the game starts or when spawned
void AAuraCharacterBase::BeginPlay()
{
	Super::BeginPlay();
	
}




