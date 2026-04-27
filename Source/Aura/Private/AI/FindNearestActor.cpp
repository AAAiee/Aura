// @Copyright HaolunYuan

#include "AI/FindNearestActor.h"

#include "AIController.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "Kismet/GameplayStatics.h"

UFindNearestActor::UFindNearestActor()
{
	INIT_SERVICE_NODE_NOTIFY_FLAGS();

	// Restrict the authored keys in the BT editor so this service can only bind compatible types.
	TargetToFollow.AddObjectFilter(
		this,
		GET_MEMBER_NAME_CHECKED(UFindNearestActor, TargetToFollow),
		AActor::StaticClass());

	DistanceToTarget.AddFloatFilter(
		this,
		GET_MEMBER_NAME_CHECKED(UFindNearestActor, DistanceToTarget));
}

void UFindNearestActor::TickNode(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds)
{
	Super::TickNode(OwnerComp, NodeMemory, DeltaSeconds);

	AAIController* AIController = OwnerComp.GetAIOwner();
	if (!IsValid(AIController))
	{
		return;
	}

	UBlackboardComponent* Blackboard = OwnerComp.GetBlackboardComponent();
	if (!IsValid(Blackboard))
	{
		return;
	}

	APawn* ControlledPawn = AIController->GetPawn();
	if (!IsValid(ControlledPawn))
	{
		return;
	}

	FindNearestTarget(ControlledPawn, *Blackboard);
}

void UFindNearestActor::FindNearestTarget(const APawn* ControllerPawn, UBlackboardComponent& Blackboard)
{
	FName LocalTargetTag;
	if (ControllerPawn->ActorHasTag(TEXT("Enemy")))
	{
		LocalTargetTag = TEXT("Player");
	}
	else if (ControllerPawn->ActorHasTag(TEXT("Player")))
	{
		LocalTargetTag = TEXT("Enemy");
	}
	else
	{
		return;
	}

	TargetTagName = LocalTargetTag;

	AActor* ClosestActor = nullptr;
	float ClosestDistance = FLT_MAX;
	TArray<AActor*> TargetActors;
	UGameplayStatics::GetAllActorsWithTag(ControllerPawn, LocalTargetTag, TargetActors);

	for (AActor* TargetActor : TargetActors)
	{
		if (!IsValid(TargetActor) || TargetActor == ControllerPawn)
		{
			continue;
		}

		const float Distance = ControllerPawn->GetDistanceTo(TargetActor);
		if (Distance < ClosestDistance)
		{
			ClosestDistance = Distance;
			ClosestActor = TargetActor;
		}
	}

	if (ClosestActor)
	{
		Blackboard.SetValueAsObject(TargetToFollow.SelectedKeyName, ClosestActor);
		Blackboard.SetValueAsFloat(DistanceToTarget.SelectedKeyName, ClosestDistance);
	}
}
