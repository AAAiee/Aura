// @Copyright HaolunYuan

#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/BTService.h"
#include "FindNearestActor.generated.h"

class UBlackboardComponent;
class UBehaviorTreeComponent;
class APawn;

/**
 * Behavior-tree service that keeps the nearest valid opposing actor cached on the blackboard.
 */
UCLASS()
class AURA_API UFindNearestActor : public UBTService
{
	GENERATED_BODY()

public:
	UFindNearestActor();

protected:
	/* UBTService */
	virtual void TickNode(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds) override;

	/* Target Resolution */
	void FindNearestTarget(const APawn* ControllerPawn, UBlackboardComponent& Blackboard);

	/* Blackboard Keys */
	UPROPERTY(EditAnywhere, Category = "Blackboard")
	FBlackboardKeySelector TargetToFollow;

	UPROPERTY(EditAnywhere, Category = "Blackboard")
	FBlackboardKeySelector DistanceToTarget;

	// Exposed for debugging and Blueprint inspection when the service is specialized further.
	UPROPERTY(BlueprintReadOnly, Category = "Target")
	FName TargetTagName;
};
