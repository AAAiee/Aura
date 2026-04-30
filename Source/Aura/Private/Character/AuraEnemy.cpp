// @Copyright HaolunYuan

#include "Character/AuraEnemy.h"

#include "AI/AuraAIController.h"
#include "Aura/Aura.h"
#include "AuraGameTagManager.h"
#include "BehaviorTree/BehaviorTree.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "Components/AbilitySystem/AuraAbilitySystemComponent.h"
#include "Components/AbilitySystem/AuraAbilitySystemLibrary.h"
#include "Components/AbilitySystem/AuraAttributeSet.h"
#include "Components/CapsuleComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "UI/WidgetComponent/ActorStatusWidgetComponent.h"
#include "UI/WidgetController/AuraWidgetController.h"

AAuraEnemy::AAuraEnemy()
{
	PrimaryActorTick.bCanEverTick = true;

	// Use the capsule as the primary gameplay collision representation so projectile filtering and
	// overlap checks stay consistent across enemies with different skeletal mesh heights.
	GetCapsuleComponent()->SetCollisionObjectType(ECC_EnemyCollision);
	GetCapsuleComponent()->SetGenerateOverlapEvents(true);
	GetCapsuleComponent()->SetCollisionResponseToChannel(ECC_Projectile, ECR_Overlap);

	bUseControllerRotationPitch = false;
	bUseControllerRotationRoll = false;
	bUseControllerRotationYaw = false;
	GetCharacterMovement()->bUseControllerDesiredRotation = true;

	// Visibility blocking on the mesh allows cursor traces to detect the enemy for highlighting.
	GetMesh()->SetGenerateOverlapEvents(false);
	GetMesh()->SetCollisionResponseToChannel(ECC_Visibility, ECR_Block);

	// Enemies own their ASC directly, so they do not need the PlayerState-driven setup used by the player.
	AbilitySystemComponent = CreateDefaultSubobject<UAuraAbilitySystemComponent>(TEXT("AbilitySystemComponent"));
	AbilitySystemComponent->SetIsReplicated(true);
	AbilitySystemComponent->SetReplicationMode(EGameplayEffectReplicationMode::Minimal);
	AttributeSet = CreateDefaultSubobject<UAuraAttributeSet>(TEXT("AttributeSet"));

	HealthBarComponent = CreateDefaultSubobject<UActorStatusWidgetComponent>(TEXT("EnemyHealthBar"));
	HealthBarComponent->SetupAttachment(GetRootComponent());
}

void AAuraEnemy::HighLightActor()
{
	if (bIsHighlighted)
	{
		return;
	}

	bIsHighlighted = true;

	GetMesh()->SetRenderCustomDepth(true);
	GetMesh()->SetCustomDepthStencilValue(CUSTOM_DEPTH_RED);

	if (Weapon)
	{
		Weapon->SetRenderCustomDepth(true);
		Weapon->SetCustomDepthStencilValue(CUSTOM_DEPTH_RED);
	}
}

void AAuraEnemy::UnhighLightActor()
{
	if (!bIsHighlighted)
	{
		return;
	}

	bIsHighlighted = false;

	GetMesh()->SetRenderCustomDepth(false);

	if (Weapon)
	{
		Weapon->SetRenderCustomDepth(false);
	}
}

void AAuraEnemy::Die()
{
	// Lifespan cleanup is intentionally delayed so remote clients can still see the ragdoll and
	// dissolve sequence kicked off by the shared base-character death flow.
	SetLifeSpan(LifeSpan);

	// Mirror death into the blackboard so active behavior-tree tasks stop treating this pawn as alive.
	AuraAIController->GetBlackboardComponent()->SetValueAsBool(FName("Dead"), true);

	Super::Die();
}

void AAuraEnemy::PossessedBy(AController* NewController)
{
	Super::PossessedBy(NewController);

	if (!HasAuthority())
	{
		return;
	}

	AuraAIController = Cast<AAuraAIController>(NewController);
	UBlackboardComponent* Blackboard = AuraAIController ? AuraAIController->GetBlackboardComponent() : nullptr;
	if (!Blackboard || !BehaviourTree)
	{
		return;
	}

	// Blackboard startup lives on the server because AI decision state is authoritative.
	Blackboard->InitializeBlackboard(*BehaviourTree->BlackboardAsset);
	AuraAIController->RunBehaviorTree(BehaviourTree);
	Blackboard->SetValueAsBool(FName("HitReacting"), false);
	Blackboard->SetValueAsBool(FName("IsRangeAttacker"), CharacterClass != ECharacterClass::ECC_Warrior);
}

void AAuraEnemy::SetCombatTarget_Implementation(AActor* InCombatTarget)
{
	check(InCombatTarget);
	CombatTarget = InCombatTarget;
}

AActor* AAuraEnemy::GetCombatTarget_Implementation() const
{
	check(CombatTarget);
	return CombatTarget;
}

float AAuraEnemy::GetZOffset() const
{
	return GetCapsuleComponent()->GetScaledCapsuleHalfHeight();
}

void AAuraEnemy::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

void AAuraEnemy::BeginPlay()
{
	Super::BeginPlay();

	GetCharacterMovement()->MaxWalkSpeed = BaseSpeed;

	/*
	 * Enemy startup flow:
	 *   1. Build the ASC ActorInfo so ability and attribute queries know owner and avatar.
	 *   2. Register the hit-react tag listener used to drive movement state.
	 *   3. On the server, grant startup abilities and apply class-driven default attributes.
	 *   4. On every machine, initialize the status widget from replicated gameplay state.
	 */
	InitAbilityActorInfo();

	AbilitySystemComponent->RegisterGameplayTagEvent(FAuraGameTagManager::Get().Combat_HitReact, EGameplayTagEventType::NewOrRemoved)
		.AddUObject(this, &AAuraEnemy::OnHitReactTagChanged);

	if (HasAuthority())
	{
		UAuraAbilitySystemLibrary::InitializeDefaultAbilities(this, CharacterClass, AbilitySystemComponent);
		InitDefaultAttributes();
	}

	InitializeStatusWidget();
}

void AAuraEnemy::InitAbilityActorInfo()
{
	check(AbilitySystemComponent);

	AbilitySystemComponent->InitAbilityActorInfo(this, this);
	Cast<UAuraAbilitySystemComponent>(AbilitySystemComponent)->AbilityActorInfoSet();
}

void AAuraEnemy::InitDefaultAttributes()
{
	// Class-based defaults are centralized in the ability-system library so players and enemies stay
	// on the same data-driven startup pipeline.
	UAuraAbilitySystemLibrary::InitializeDefaultAttributes(this, CharacterClass, AbilitySystemComponent, EnemyLevel);
}

void AAuraEnemy::InitializeStatusWidget()
{
	check(HealthBarComponent);

	// Enemy widgets do not have player-controller or player-state references, so they bind directly
	// to the actor-facing ASC and AttributeSet that already replicate authoritative combat data.
	const FWidgetControllerParameters Parameters(nullptr, nullptr, AbilitySystemComponent, AttributeSet);
	HealthBarComponent->InitializeWidgetController(Parameters);
}

void AAuraEnemy::OnHitReactTagChanged(const FGameplayTag GameplayTag, int32 NewCount)
{
	if (!HasAuthority())
	{
		return;
	}

	// The tag count is the gameplay truth. Movement simply mirrors it so abilities and GE logic
	// remain the single authority for when an enemy should appear staggered.
	bHitReacting = NewCount > 0;
	if (bHitReacting)
	{
		check(GetCharacterMovement());
		GetCharacterMovement()->MaxWalkSpeed = 0.f;
	}
	else
	{
		GetCharacterMovement()->MaxWalkSpeed = BaseSpeed;
	}

	AuraAIController->GetBlackboardComponent()->SetValueAsBool(FName("HitReacting"), bHitReacting);
}
