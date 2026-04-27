// @Copyright HaolunYuan

#pragma once

#include "CoreMinimal.h"
#include "AIController.h"
#include "AuraAIController.generated.h"

class UBlackboardComponent;
class UBehaviorTreeComponent;

/**
 * AI controller that owns the runtime blackboard and behavior-tree components for Aura enemies.
 */
UCLASS()
class AURA_API AAuraAIController : public AAIController
{
	GENERATED_BODY()

public:
	AAuraAIController();

protected:
	/* AI Runtime Components */
	// Dedicated behavior tree component so AI-controlled combatants can run authored BT assets.
	UPROPERTY()
	TObjectPtr<UBehaviorTreeComponent> BehaviorTreeComponent;
};
