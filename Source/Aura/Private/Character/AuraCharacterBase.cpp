// @Copyright HaolunYuan  https://github.com/AAAiee/Aura.git


#include "Character/AuraCharacterBase.h"
#include "AbilitySystemComponent.h"
#include "GameplayEffect.h"

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


void AAuraCharacterBase::ApplyGameEffectToSelf(TSubclassOf<UGameplayEffect> GameplayEffectClass, float Level) const
{
	checkf(GameplayEffectClass, TEXT("GameplayEffectClass should be set in blueprint"));
	check(AbilitySystemComponent);
	FGameplayEffectContextHandle ContextHandle = AbilitySystemComponent->MakeEffectContext();
	FGameplayEffectSpecHandle SpecHandle = AbilitySystemComponent->MakeOutgoingSpec(GameplayEffectClass, Level, ContextHandle);
	if (SpecHandle.IsValid())
	{
		AbilitySystemComponent->ApplyGameplayEffectSpecToSelf(*SpecHandle.Data.Get());
	}
}

void AAuraCharacterBase::InitDefaultAttributes()
{
	/* Follow the order, primary -> secondary -> vital */
	ApplyGameEffectToSelf(PrimaryAttributeInitGE, 1.0f);
	ApplyGameEffectToSelf(SecondaryAttributeInitGE, 1.0f);
	ApplyGameEffectToSelf(VitalAttributeInitGE, 1.0f); 
}

