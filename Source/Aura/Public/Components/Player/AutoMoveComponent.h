// @Copyright HaolunYuan
#pragma once
#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "AutoMoveComponent.generated.h"
class USplineComponent;
/**
 * Click-to-move component that handles server-authoritative pathfinding.
 *
 * Network architecture:
 *   - Client short-clicks -> RequestToMoveToLocation() -> Server RPC computes nav path
 *   - Server sends path points back -> Client RPC starts following the spline locally
 *   - Held cursor movement skips pathfinding and pushes raw movement input directly
 *
 * Tick is disabled by default and only enabled while actively following a spline.
 * Attach this component to the PlayerController so it persists across pawn respawns.
 */
UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class AURA_API UAutoMoveComponent : public UActorComponent
{
	GENERATED_BODY()
public:
	UAutoMoveComponent();
	FORCEINLINE bool IsAutoMoving() const { return bIsAutoMoving; }
	FORCEINLINE bool HasPendingPathRequest() const { return bHasPendingPathRequest; }
	/** Entry point for short-click move requests. Computes a nav path on the server. */
	void RequestToMoveToLocation(const FVector& InTargetPosition);
	/** Moves toward the target immediately without pathfinding. Used by the held cursor action. */
	void MoveDirectlyToLocation(const FVector& InTargetPosition);
	/** Stops any active or pending auto-move. */
	void RequestCancelAutoMove();
protected:
	virtual void BeginPlay() override;
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;
private:
	APawn* GetOwnerPawn();
	void BuildAndSendPathToClient(const FVector& InTargetPosition, int32 MoveRequestId);
	void StartFollowingPath(const TArray<FVector_NetQuantize>& InPathPoints);
	void FollowSpline();
	void StopAutoMove();
	UFUNCTION(Server, Reliable)
	void Server_RequestMoveToLocation(FVector_NetQuantize InTargetPosition, int32 MoveRequestId);
	UFUNCTION(Server, Reliable)
	void Server_CancelAutoMove(int32 MoveRequestId);
	UFUNCTION(Client, Reliable)
	void Client_StartAutoMove(const TArray<FVector_NetQuantize>& InPathPoints, int32 MoveRequestId);
	UFUNCTION(Client, Reliable)
	void Client_CancelAutoMove(int32 MoveRequestId);
private:
	FVector CachedDestination = FVector::ZeroVector;
	UPROPERTY(VisibleAnywhere)
	TObjectPtr<USplineComponent> SplineComponent;
	int32 LatestMoveRequestId = 0;
	bool bHasPendingPathRequest = false;
	bool bIsAutoMoving = false;
	UPROPERTY(EditAnywhere, Category = "Movement")
	float AcceptanceRadius = 50.f;
};
