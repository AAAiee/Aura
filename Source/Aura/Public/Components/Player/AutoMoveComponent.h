// @Copyright HaolunYuan

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "AutoMoveComponent.generated.h"

/**
 * Click-to-move component that handles server-authoritative pathfinding.
 *
 * Network architecture:
 *   - Client clicks ¡ú RequestToMoveToLocation() ¡ú Server RPC computes nav path
 *   - Server sends path points back ¡ú Client RPC starts following the path locally
 *   - Movement uses AddMovementInput (not teleportation) so CharacterMovementComponent
 *     handles prediction, replication, and smoothing automatically.
 *
 * Tick is DISABLED by default and only enabled while actively following a path.
 * This avoids wasting CPU on idle components.
 *
 * Attach this component to the PlayerController (not the Pawn) so it persists
 * across pawn respawns.
 */
UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class AURA_API UAutoMoveComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	UAutoMoveComponent();

	FORCEINLINE bool IsAutoMoving() const { return bIsAutoMoving; }

	/**
	 * Entry point for click-to-move.
	 * If called on the client, sends a Server RPC. If called on the server, computes the path directly.
	 */
	void RequestToMoveToLocation(const FVector& InTargetPosition);

	/** Stops any active auto-move. Cancels on both client (immediate) and server (via RPC). */
	void RequestCancelAutoMove();

protected:
	virtual void BeginPlay() override;
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

private:
	/** Resolves the pawn from the owner (works whether owner is a Pawn or a Controller). */
	APawn* GetOwnerPawn();

	/** Server RPC ¡ª computes a nav path and sends the result to the owning client. */
	UFUNCTION(Server, Reliable)
	void Server_RequestMoveToLocation(FVector_NetQuantize InTargetPosition);

	/** Client RPC ¡ª receives path points from the server and begins following them. */
	UFUNCTION(Client, Reliable)
	void Client_StartAutoMove(const TArray<FVector_NetQuantize>& InPathPoints);

	/** Client RPC ¡ª immediately stops auto-move and clears path state. */
	UFUNCTION(Client, Reliable)
	void Client_CancelAutoMove();

	/**
	 * Called every tick while bIsAutoMoving.
	 * Advances through waypoints using AddMovementInput so CMC handles prediction.
	 * Stops when the final waypoint is reached.
	 */
	void FollowPath();

private:
	TArray<FVector> PathPoints;
	int32 CurrentPathIndex = 0;
	bool bIsAutoMoving = false;

	/** How close (2D distance) the pawn must be to a waypoint before advancing to the next one. */
	UPROPERTY(EditAnywhere, Category = "Movement")
	float AcceptanceRadius = 50.f;
};
