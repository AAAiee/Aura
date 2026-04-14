// @Copyright HaolunYuan  https://github.com/AAAiee/Aura.git


#include "Character/AuraCharacterBase.h"
#include "AbilitySystemComponent.h"
#include "GameplayEffect.h"
#include "Components/AbilitySystem/AuraAbilitySystemComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "Components/CapsuleComponent.h"
#include "GameFramework/Character.h"
#include "../Aura.h"
#include "Engine/EngineTypes.h"

// Sets default values
AAuraCharacterBase::AAuraCharacterBase()
{
	// Base characters don't need to tick — subclasses can enable it if needed (e.g., enemy AI)
	PrimaryActorTick.bCanEverTick = false;

	/*Ignore the Camera*/
	GetCapsuleComponent()->SetCollisionResponseToChannel(ECC_Camera, ECR_Ignore);
	GetMesh()->SetCollisionResponseToChannel(ECC_Camera, ECR_Ignore); 

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

void AAuraCharacterBase::Die()
{
	// The authority-side entry point detaches the weapon first so the upcoming death presentation
	// is no longer driven by the living hand socket animation.
	FDetachmentTransformRules DetachmentRules(EDetachmentRule::KeepWorld, true);
	Weapon->DetachFromComponent(DetachmentRules);

	MulticastHandleDeath();
}

void AAuraCharacterBase::MulticastHandleDeath_Implementation()
{
	/*
	 * The multicast fans the visual death state out to every machine:
	 *   1. Break the weapon's socket attachment.
	 *   2. Turn on physics/gravity so the corpse settles naturally.
	 *   3. Disable the capsule so gameplay collision no longer treats the actor as alive.
	 *   4. Start the dissolve presentation used by the death cleanup flow.
	 */
	FDetachmentTransformRules DetachmentRules(EDetachmentRule::KeepWorld, true);
	Weapon->DetachFromComponent(DetachmentRules);

	Weapon->SetCollisionEnabled(ECollisionEnabled::PhysicsOnly); // easiest test
	Weapon->SetSimulatePhysics(true);
	Weapon->SetEnableGravity(true);

	GetMesh()->SetSimulatePhysics(true);
	GetMesh()->SetEnableGravity(true);
	GetMesh()->SetCollisionEnabled(ECollisionEnabled::PhysicsOnly);
	GetMesh()->SetCollisionResponseToChannel(ECC_WorldStatic, ECR_Block);

	GetCapsuleComponent()->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	Dissolve();
}

void AAuraCharacterBase::Dissolve()
{
	if (IsValid(CharacterDisolveMaterialInstance))
	{
		// Dynamic instances let the Blueprint timeline animate this specific actor's dissolve
		// parameters without mutating the shared material asset used by other characters.
		UMaterialInstanceDynamic* DynamicInstance  = UMaterialInstanceDynamic::Create(CharacterDisolveMaterialInstance, this);

		GetMesh()->SetMaterial(0, DynamicInstance);
		StartDissolveTimeline(DynamicInstance);
	}

	if (IsValid(WeaponDisolveMaterialInstance))
	{
		// The weapon dissolves independently so designers can author a slightly different material
		// response while still keeping the same death flow entry point.
		UMaterialInstanceDynamic* DynamicInstance = UMaterialInstanceDynamic::Create(WeaponDisolveMaterialInstance, this);

		Weapon->SetMaterial(0, DynamicInstance);
		StartWeaponDissolveTimeline(DynamicInstance);
	}
}

UAnimMontage* AAuraCharacterBase::GetHitReactMontage_Implementation()
{
	return HitReactMontage;
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
