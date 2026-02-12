// @Copyright HaolunYuan

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "AutoMoveComponent.generated.h"


UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class AURA_API UAutoMoveComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	UAutoMoveComponent();
	/*Returns whether the component is currently driving an auto-move path*/
	FORCEINLINE bool IsAutoMoving() const { return bIsAutoMoving; }

	/*Requests a click-to-move path to the target location via the server.*/
	void RequestToMoveToLocation(const FVector& InTargetPosition);

	/*Cancels any active auto-move path*/
	void RequestCancelAutoMove();

protected:
	virtual void BeginPlay() override;
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

private:
	/*Resolves the pawn that owns this component (directly or via controller).*/
	APawn* GetOwnerPawn();

	/*Server RPC: compute a navigation path for the requested destination*/
	UFUNCTION(Server, Reliable)
	void Server_RequestMoveToLocation(FVector_NetQuantize InTargetPosition);

	/*Client RPC: start auto-move using the path points computed on the server*/
	UFUNCTION(Client, Reliable)
	void Client_StartAutoMove(const TArray<FVector_NetQuantize>& InPathPoints);

	/*Client RPC: stop auto-move and clear any active path state.*/
	UFUNCTION(Client, Reliable)
	void Client_CancelAutoMove();

	/*Advances along the current path by applying movement input each tick.*/
	void FollowPath();

private:
	/*Path points the client follows via AddMovementInput each tick*/
	TArray<FVector> PathPoints;
	int32 CurrentPathIndex = 0;

	bool bIsAutoMoving = false;

	/*How close to a waypoint before advancing to the next one.*/
	UPROPERTY(EditAnywhere, Category = "Movement")
	float AcceptanceRadius = 50.f;
};
