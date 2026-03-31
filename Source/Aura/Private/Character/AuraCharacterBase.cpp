// @Copyright HaolunYuan  https://github.com/AAAiee/Aura.git


#include "Character/AuraCharacterBase.h"
#include "AbilitySystemComponent.h"
#include "GameplayEffect.h"
#include "Components/AbilitySystem/AuraAbilitySystemComponent.h"
#include "Components/SkeletalMeshComponent.h"

// Sets default values
AAuraCharacterBase::AAuraCharacterBase()
{
	// Base characters don't need to tick — subclasses can enable it if needed (e.g., enemy AI)
	PrimaryActorTick.bCanEverTick = false;

	/*Weapon Setup — attach a skeletal mesh to the hand socket defined on the character's skeleton*/
	Weapon = CreateDefaultSubobject<USkeletalMeshComponent>("Weapon");
	Weapon->SetupAttachment(GetMesh(), FName("WeaponHandSocket"));
	Weapon->SetCollisionEnabled(ECollisionEnabled::NoCollision); // Weapon visuals only; combat uses traces/overlaps
}

void AAuraCharacterBase::AddStartupGameAbilities()
{
	// Game Abilities should only being added to the server version
	if (!HasAuthority()) return;

	UAuraAbilitySystemComponent* ASC = CastChecked<UAuraAbilitySystemComponent>(GetAbilitySystemComponent());

	ASC->AddCharacterAbilities(StartupAbilityClasses);
}

// Called when the game starts or when spawned
void AAuraCharacterBase::BeginPlay()
{
	Super::BeginPlay();
}

FVector AAuraCharacterBase::GetCombatSocketLocation() const
{
	check(Weapon);
	return Weapon->GetSocketLocation(WeaponTipSocketName);
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
