// @Copyright HaolunYuan  https://github.com/AAAiee/Aura.git

#include "Character/AuraCharacterBase.h"

#include "AbilitySystemComponent.h"
#include "Aura/Aura.h"
#include "AuraGameTagManager.h"
#include "Components/AbilitySystem/AuraAbilitySystemComponent.h"
#include "Components/CapsuleComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "GameplayEffect.h"
#include "Kismet/GameplayStatics.h"
#include "Materials/MaterialInstanceDynamic.h"

AAuraCharacterBase::AAuraCharacterBase()
{
	// Base characters do not need ticking by default. Subclasses opt in only when they have
	// per-frame responsibilities such as AI state updates.
	PrimaryActorTick.bCanEverTick = false;

	// Camera collision is disabled so the spring arm and camera can move through friendly meshes
	// without snapping or zooming when the player fights in close quarters.
	GetCapsuleComponent()->SetCollisionResponseToChannel(ECC_Camera, ECR_Ignore);
	GetMesh()->SetCollisionResponseToChannel(ECC_Camera, ECR_Ignore);

	// Weapon visuals live on a dedicated mesh component so combat sockets and dissolve materials can
	// be authored independently from the main skeletal mesh.
	Weapon = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("Weapon"));
	Weapon->SetupAttachment(GetMesh(), FName("WeaponHandSocket"));
	Weapon->SetCollisionEnabled(ECollisionEnabled::NoCollision);
}

void AAuraCharacterBase::AddStartupGameAbilities()
{
	// Startup abilities are authoritative gameplay state, so only the server grants them.
	if (!HasAuthority())
	{
		return;
	}

	UAuraAbilitySystemComponent* ASC = CastChecked<UAuraAbilitySystemComponent>(GetAbilitySystemComponent());
	ASC->AddCharacterAbilities(StartupAbilityClasses);
	ASC->AddCharacterPassiveAbilities(StartupPassiveAbilityClasses); 
}

FVector AAuraCharacterBase::GetCombatSocketLocation_Implementation(const FGameplayTag& MontageTag) const
{
	const FName* SocketName = MontageTagToSocketLocation.Find(MontageTag);
	check(SocketName);

	if (MontageTag.MatchesTagExact(FAuraGameTagManager::Get().CombatSocket_Weapon))
	{
		check(Weapon);
		return Weapon->GetSocketLocation(*SocketName);
	}

	return GetMesh()->GetSocketLocation(*SocketName);
}

UAnimMontage* AAuraCharacterBase::GetHitReactMontage_Implementation()
{
	return HitReactMontage;
}

bool AAuraCharacterBase::IsDead_Implementation() const
{
	return bDead;
}

AActor* AAuraCharacterBase::GetAvatar_Implementation()
{
	return this;
}

FTaggedMontage AAuraCharacterBase::GetTaggedMontageForTag_Implementation(const FGameplayTag& MontageTag) const
{
	for (const FTaggedMontage& TaggedMontage : AttackMontages)
	{
		if (TaggedMontage.MontageTag.MatchesTagExact(MontageTag))
		{
			return TaggedMontage;
		}
	}

	checkf(false, TEXT("No tagged montage found for tag: %s"), *MontageTag.ToString());
	return FTaggedMontage();
}

void AAuraCharacterBase::Die()
{
	// The authority-side entry point detaches the weapon first so the death presentation is no
	// longer driven by living hand-socket animation.
	FDetachmentTransformRules DetachmentRules(EDetachmentRule::KeepWorld, true);
	Weapon->DetachFromComponent(DetachmentRules);

	MulticastHandleDeath();
}

void AAuraCharacterBase::MulticastHandleDeath_Implementation()
{
	/*
	 * The multicast fans the visual death state out to every machine:
	 *   1. Break the weapon's socket attachment.
	 *   2. Turn on physics and gravity so the corpse settles naturally.
	 *   3. Disable the capsule so gameplay collision no longer treats the actor as alive.
	 *   4. Start the dissolve presentation used by the death cleanup flow.
	 */

	UGameplayStatics::PlaySoundAtLocation(this, DeathSound, GetActorLocation(), GetActorRotation());

	FDetachmentTransformRules DetachmentRules(EDetachmentRule::KeepWorld, true);
	Weapon->DetachFromComponent(DetachmentRules);

	Weapon->SetCollisionEnabled(ECollisionEnabled::PhysicsOnly);
	Weapon->SetSimulatePhysics(true);
	Weapon->SetEnableGravity(true);

	GetMesh()->SetSimulatePhysics(true);
	GetMesh()->SetEnableGravity(true);
	GetMesh()->SetCollisionEnabled(ECollisionEnabled::PhysicsOnly);
	GetMesh()->SetCollisionResponseToChannel(ECC_WorldStatic, ECR_Block);

	// Keep the capsule blocking world static for attached client UI while ignoring gameplay collision.
	GetCapsuleComponent()->SetCollisionResponseToAllChannels(ECR_Ignore);
	GetCapsuleComponent()->SetCollisionResponseToChannel(ECC_WorldStatic, ECR_Block);
	Dissolve();

	bDead = true;
}

void AAuraCharacterBase::Dissolve()
{
	if (IsValid(CharacterDisolveMaterialInstance))
	{
		// Dynamic instances let the Blueprint timeline animate this specific actor's dissolve
		// parameters without mutating the shared material asset used by other characters.
		UMaterialInstanceDynamic* DynamicInstance = UMaterialInstanceDynamic::Create(CharacterDisolveMaterialInstance, this);
		GetMesh()->SetMaterial(0, DynamicInstance);
		StartDissolveTimeline(DynamicInstance);
	}

	if (IsValid(WeaponDisolveMaterialInstance))
	{
		// The weapon dissolves independently so designers can author a slightly different material
		// response while still keeping the same death-flow entry point.
		UMaterialInstanceDynamic* DynamicInstance = UMaterialInstanceDynamic::Create(WeaponDisolveMaterialInstance, this);
		Weapon->SetMaterial(0, DynamicInstance);
		StartWeaponDissolveTimeline(DynamicInstance);
	}
}

void AAuraCharacterBase::BeginPlay()
{
	Super::BeginPlay();

	// Combat socket selection stays data-driven through montage tags so melee traces and projectile
	// casts can share the same authored montage metadata.
	MontageTagToSocketLocation.Add(FAuraGameTagManager::Get().CombatSocket_Weapon, WeaponTipSocketName);
	MontageTagToSocketLocation.Add(FAuraGameTagManager::Get().CombatSocket_RightHand, RightHandTipSocketName);
	MontageTagToSocketLocation.Add(FAuraGameTagManager::Get().CombatSocket_LeftHand, LeftHandTipSocketName);
	MontageTagToSocketLocation.Add(FAuraGameTagManager::Get().CombatSocket_TailTip, TailTipSocketName);
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
	// Default attribute effects are ordered so later sets can depend on values initialized earlier.
	ApplyGameEffectToSelf(PrimaryAttributeInitGE, 1.0f);
	ApplyGameEffectToSelf(SecondaryAttributeInitGE, 1.0f);
	ApplyGameEffectToSelf(VitalAttributeInitGE, 1.0f);
}
