// @Copyright HaolunYuan


#include "Character/AuraCharacter.h"

#include "AuraGameTagManager.h"
#include "Aura/Aura.h"
#include "Camera/CameraComponent.h"
#include "Components/AbilitySystem/AuraAbilitySystemComponent.h"
#include "Components/AbilitySystem/Data/LevelUpInfo.h"
#include "Components/CapsuleComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "NiagaraComponent.h"
#include "Components/AbilitySystem/Debuff/DebuffNiagaraComponent.h"
#include "Player/AuraPlayerController.h"
#include "Player/AuraPlayerState.h"
#include "UI/HUD/AuraHUD.h"

AAuraCharacter::AAuraCharacter()
{
	CameraBoom = CreateDefaultSubobject<USpringArmComponent>(TEXT("CameraBoom"));
	CameraBoom->SetupAttachment(RootComponent);
	CameraBoom->bDoCollisionTest = false;
	CameraBoom->SetUsingAbsoluteRotation(true);

	TopDownCameraComponent = CreateDefaultSubobject<UCameraComponent>(TEXT("TopDownCamera"));
	TopDownCameraComponent->SetupAttachment(CameraBoom, USpringArmComponent::SocketName);
	TopDownCameraComponent->bUsePawnControlRotation = false;

	LevelUpNiagaraComponent = CreateDefaultSubobject<UNiagaraComponent>(TEXT("LevelUpNiagaraComponent"));
	LevelUpNiagaraComponent->SetupAttachment(RootComponent);
	LevelUpNiagaraComponent->SetAutoActivate(false);

	GetCapsuleComponent()->SetGenerateOverlapEvents(false);
	GetMesh()->SetGenerateOverlapEvents(true);
	GetMesh()->SetCollisionResponseToChannel(ECC_Projectile, ECR_Overlap);

	/* Movement Configuration - top-down ARPG style:
	 * bOrientRotationToMovement: character faces the direction of movement automatically.
	 * bConstrainToPlane + bSnapToPlaneAtStart: keeps movement on a flat plane (no flying). */
	UCharacterMovementComponent* AuraMovementComponent = GetCharacterMovement();
	check(AuraMovementComponent);
	AuraMovementComponent->bOrientRotationToMovement = true;
	AuraMovementComponent->RotationRate = FRotator(0.0f, 400.f, 0.f);
	AuraMovementComponent->bConstrainToPlane = true;
	AuraMovementComponent->bSnapToPlaneAtStart = true;

	/* Fixed Camera - disable controller rotation so the camera stays fixed (top-down view).
	 * The character rotates via bOrientRotationToMovement instead. */
	bUseControllerRotationPitch = false;
	bUseControllerRotationRoll = false;
	bUseControllerRotationYaw = false;

	CharacterClass = ECharacterClass::ECC_Elementalist;
}

void AAuraCharacter::PossessedBy(AController* NewController)
{
	Super::PossessedBy(NewController);

	// Server-side: PlayerState is already valid at this point
	InitAbilityActorInfo();

	// Server-side: apply the GE that initializes primary attributes (Strength, Intellect, etc).
	// This will replicate to clients and trigger UI updates.
	InitDefaultAttributes();

	AddStartupGameAbilities();
}

void AAuraCharacter::OnRep_PlayerState()
{
	Super::OnRep_PlayerState();

	// Client-side: PlayerState just replicated, safe to read ASC/AS from it now
	InitAbilityActorInfo();
}

int32 AAuraCharacter::GetPlayerLevel_Implementation() const
{
	AAuraPlayerState* AuraPlayerState = GetPlayerState<AAuraPlayerState>();
	check(AuraPlayerState);
	return AuraPlayerState->GetPlayerLevel();
}

int32 AAuraCharacter::GetXP_Implementation() const
{
	AAuraPlayerState* AuraPlayerState = GetPlayerState<AAuraPlayerState>();
	check(AuraPlayerState);
	return AuraPlayerState->GetPlayerXP();
}

int32 AAuraCharacter::GetAttributePointsReward_Implementation(int32 Level) const
{
	AAuraPlayerState* AuraPlayerState = GetPlayerState<AAuraPlayerState>();
	check(AuraPlayerState);
	ULevelUpInfo* AuraLevelUpInfo = AuraPlayerState->LevelUpInfo;

	check(Level > 0 && Level < AuraLevelUpInfo->LevelUpInfos.Num() - 1);
	return AuraLevelUpInfo->LevelUpInfos[Level].AttributePointsGranted;
}

int32 AAuraCharacter::GetSpellPointsReward_Implementation(int32 Level) const
{
	AAuraPlayerState* AuraPlayerState = GetPlayerState<AAuraPlayerState>();
	check(AuraPlayerState);
	ULevelUpInfo* AuraLevelUpInfo = AuraPlayerState->LevelUpInfo;

	check(Level > 0 && Level < AuraLevelUpInfo->LevelUpInfos.Num() - 1);
	return AuraLevelUpInfo->LevelUpInfos[Level].SpellPointsGranted;
}

int32 AAuraCharacter::GetAttributePoints_Implementation() const
{
	AAuraPlayerState* AuraPlayerState = GetPlayerState<AAuraPlayerState>();
	check(AuraPlayerState);
	return AuraPlayerState->GetAttributePoints();
}

int32 AAuraCharacter::GetSpellPoints_Implementation() const
{
	AAuraPlayerState* AuraPlayerState = GetPlayerState<AAuraPlayerState>();
	check(AuraPlayerState);
	return AuraPlayerState->GetSpellPoints();
}

void AAuraCharacter::AddToXP_Implementation(int32 InXP)
{
	AAuraPlayerState* AuraPlayerState = GetPlayerState<AAuraPlayerState>();
	check(AuraPlayerState);
	AuraPlayerState->AddPlayerXP(InXP);
}

void AAuraCharacter::AddToPlayerLevel_Implementation(int32 InPlayerLevel)
{
	AAuraPlayerState* AuraPlayerState = GetPlayerState<AAuraPlayerState>();
	check(AuraPlayerState);
	AuraPlayerState->AddPlayerLevel(InPlayerLevel);

	if (UAuraAbilitySystemComponent* AuraASC = Cast<UAuraAbilitySystemComponent>(GetAbilitySystemComponent()))
	{
		AuraASC->UpdateAbilityStatus(AuraPlayerState->GetPlayerLevel());
	}
}

void AAuraCharacter::AddToAttributePoint_Implementation(int32 InAttributePoint)
{
	AAuraPlayerState* AuraPlayerState = GetPlayerState<AAuraPlayerState>();
	check(AuraPlayerState);
	AuraPlayerState->AddAttributePoints(InAttributePoint);
}

void AAuraCharacter::AddToSpellPoint_Implementation(int32 InSpellPoint)
{
	AAuraPlayerState* AuraPlayerState = GetPlayerState<AAuraPlayerState>();
	check(AuraPlayerState);
	AuraPlayerState->AddSpellPoints(InSpellPoint);
}

int32 AAuraCharacter::FindLevelForXP_Implementation(int32 InXP) const
{
	AAuraPlayerState* AuraPlayerState = GetPlayerState<AAuraPlayerState>();
	check(AuraPlayerState);
	return AuraPlayerState->LevelUpInfo->FindLevelForXP(InXP);
}

void AAuraCharacter::LevelUp_Implementation()
{
	MultiCastLevelUpEffect();
}

void AAuraCharacter::MultiCastLevelUpEffect_Implementation()
{
	if (LevelUpNiagaraComponent)
	{
		const FVector CameraLocation = TopDownCameraComponent->GetComponentLocation();
		const FVector NiagaraSystemLocation = LevelUpNiagaraComponent->GetComponentLocation();
		const FRotator ToCameraRotation = (CameraLocation - NiagaraSystemLocation).Rotation();
		LevelUpNiagaraComponent->SetWorldRotation(ToCameraRotation);
		LevelUpNiagaraComponent->Activate(true);
	}
}

void AAuraCharacter::ShowMagicCircle_Implementation(UMaterialInterface* DecalMaterial)
{
	if (AAuraPlayerController* AuraPC = GetController<AAuraPlayerController>())
	{
		AuraPC->ShowMagicCircle(DecalMaterial);
		AuraPC->bShowMouseCursor = false;
	}
}

void AAuraCharacter::HideMagicCircle_Implementation()
{
	if (AAuraPlayerController* AuraPC = GetController<AAuraPlayerController>())
	{
		AuraPC->HideMagicCircle();
		AuraPC->bShowMouseCursor = true;
	}
}

/**
 * Initialization flow - ORDER MATTERS:
 *
 * Step 1: Tell the ASC who owns it (PlayerState) and who the physical avatar is (this pawn).
 * Step 2: Cache ASC and AttributeSet on the base class so other systems can find them.
 * Step 3: Initialize the HUD -> creates OverlayWidget -> WidgetController subscribes to ASC delegates.
 * Step 4: AbilityActorInfoSet() binds OnGameplayEffectAppliedDelegateToSelf.
 *         This MUST come after Step 3, otherwise the ASC broadcasts effect events
 *         before the WidgetController's lambda is registered (0 listeners).
 */
void AAuraCharacter::InitAbilityActorInfo()
{
	// Step 1: Initialize ASC with Owner=PlayerState, Avatar=this Pawn
	AAuraPlayerState* AuraPlayerState = GetPlayerState<AAuraPlayerState>();
	check(AuraPlayerState);
	AuraPlayerState->GetAbilitySystemComponent()->InitAbilityActorInfo(AuraPlayerState, this);

	// Step 2: Cache GAS references on the base class
	AbilitySystemComponent = AuraPlayerState->GetAbilitySystemComponent();
	AttributeSet = AuraPlayerState->GetAttributeSet();
	
	OnAscRegisteredDelegate.Broadcast(AbilitySystemComponent);
	AbilitySystemComponent->RegisterGameplayTagEvent(FAuraGameTagManager::Get().Debuff_Stun, EGameplayTagEventType::NewOrRemoved).AddUObject(this, &AAuraCharacter::OnStunTagChanged); 

	// Step 3: HUD initialization - only on the owning client (HUD is local-only)
	if (AAuraPlayerController* AuraPlayerController = GetController<AAuraPlayerController>())
	{
		if (AAuraHUD* AuraHUD = AuraPlayerController->GetHUD<AAuraHUD>())
		{
			AuraHUD->InitOverlayWidget(AuraPlayerController, AuraPlayerState, AbilitySystemComponent, AttributeSet);
		}
	}

	// Step 4: Bind ASC effect-applied delegate AFTER UI listeners are registered
	Cast<UAuraAbilitySystemComponent>(AuraPlayerState->GetAbilitySystemComponent())->AbilityActorInfoSet();
}

void AAuraCharacter::OnRep_Stunned()
{
	const FAuraGameTagManager& TagManager = FAuraGameTagManager::Get();
	if (UAuraAbilitySystemComponent* AuraASC =  Cast<UAuraAbilitySystemComponent>(AbilitySystemComponent))
	{
		FGameplayTagContainer BlockedTag; 
		BlockedTag.AddTag(TagManager.Player_BlockInputHeld);
		BlockedTag.AddTag(TagManager.PLayer_BlockCursorTrace);
		BlockedTag.AddTag(TagManager.PLayer_BlockInputPressed);
		BlockedTag.AddTag(TagManager.PLayer_BlockInputReleased);
		
		if (bIsStunned)
		{
			AuraASC->AddLooseGameplayTags(BlockedTag) ;
		}
		else
		{
			AuraASC->RemoveLooseGameplayTags(BlockedTag);
		}
	}
	
}

void AAuraCharacter::OnRep_Burned()
{
}
