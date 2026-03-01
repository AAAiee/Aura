// @Copyright HaolunYuan


#include "Components/Player/AutoMoveComponent.h"
#include "NavigationSystem.h"
#include "NavigationPath.h"
#include "GameFramework/Character.h"


UAutoMoveComponent::UAutoMoveComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
	PrimaryComponentTick.bStartWithTickEnabled = false; // Only tick while actively following a path
	SetIsReplicatedByDefault(true); // Required for Server/Client RPCs to work
}

/**
 * Main entry point ！ handles both client and server cases:
 *
 * Client path:
 *   1. Client calls RequestToMoveToLocation()
 *   2. !HasAuthority ★ sends Server_RequestMoveToLocation RPC
 *   3. Server computes nav path via NavigationSystem
 *   4. Server sends Client_StartAutoMove RPC with the path points
 *   5. Client starts following the path locally
 *
 * Server/Listen path:
 *   1. Already has authority ★ computes nav path directly
 *   2. Sends Client_StartAutoMove RPC to the owning client
 */
void UAutoMoveComponent::RequestToMoveToLocation(const FVector& InTargetPosition)
{
	if (const AActor* Owner = GetOwner())
	{
		if (!Owner->HasAuthority())
		{
			Server_RequestMoveToLocation(FVector_NetQuantize(InTargetPosition));
			return;
		}

		// Server-side: compute the navigation path
		APawn* OwnerPawn = GetOwnerPawn();
		if (!OwnerPawn)
		{
			return;
		}

		UNavigationSystemV1* NavSys = FNavigationSystem::GetCurrent<UNavigationSystemV1>(GetWorld());
		if (!NavSys)
		{
			return;
		}

		UNavigationPath* NavPath = NavSys->FindPathToLocationSynchronously(GetWorld(), OwnerPawn->GetActorLocation(), InTargetPosition);
		if (NavPath && NavPath->PathPoints.Num() > 0)
		{
			// Convert to FVector_NetQuantize for network efficiency
			TArray<FVector_NetQuantize> NetPoints;
			NetPoints.Reserve(NavPath->PathPoints.Num());
			for (const FVector& Point : NavPath->PathPoints)
			{
				NetPoints.Add(FVector_NetQuantize(Point));
			}
			Client_StartAutoMove(NetPoints);
		}
	}
}

/**
 * Cancel auto-move:
 * - Server: sends Client_CancelAutoMove RPC so the client stops.
 * - Client: also stops locally for immediate responsiveness (don't wait for the RPC round-trip).
 */
void UAutoMoveComponent::RequestCancelAutoMove()
{
	if (const AActor* Owner = GetOwner())
	{
		if (Owner->HasAuthority())
		{
			Client_CancelAutoMove();
		}
		else
		{
			// Client-side immediate cancel for responsive WASD override
			bIsAutoMoving = false;
			PathPoints.Empty();
			SetComponentTickEnabled(false);
		}
	}
}


void UAutoMoveComponent::BeginPlay()
{
	Super::BeginPlay();
}


void UAutoMoveComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	if (bIsAutoMoving)
	{
		FollowPath();
	}
}

/** Resolves the pawn ！ works whether the owner is a Pawn directly or a PlayerController. */
APawn* UAutoMoveComponent::GetOwnerPawn()
{
	if (APawn* PawnOwner = Cast<APawn>(GetOwner()))
	{
		return PawnOwner;
	}

	if (APlayerController* PC = Cast<APlayerController>(GetOwner()))
	{
		return PC->GetPawn();
	}

	return nullptr;
}

/** Server RPC implementation ！ just delegates to the main function (which runs the server path). */
void UAutoMoveComponent::Server_RequestMoveToLocation_Implementation(const FVector_NetQuantize InTargetPosition)
{
	RequestToMoveToLocation(FVector(InTargetPosition));
}

/** Client RPC ！ receives the server-computed path and starts following it. Enables tick. */
void UAutoMoveComponent::Client_StartAutoMove_Implementation(const TArray<FVector_NetQuantize>& InPathPoints)
{
	PathPoints.Empty(InPathPoints.Num());
	for (const FVector_NetQuantize& Point : InPathPoints)
	{
		PathPoints.Add(FVector(Point));
	}

	CurrentPathIndex = 0;
	bIsAutoMoving = true;
	SetComponentTickEnabled(true); // Start ticking to follow the path
}

/** Client RPC ！ stops auto-move and disables tick. */
void UAutoMoveComponent::Client_CancelAutoMove_Implementation()
{
	bIsAutoMoving = false;
	PathPoints.Empty();
	SetComponentTickEnabled(false);
}

/**
 * Path following logic ！ runs every tick while bIsAutoMoving.
 *
 * Algorithm:
 *   1. Skip waypoints that are within AcceptanceRadius (may skip multiple in one tick if pawn is fast).
 *   2. If all waypoints are reached, stop auto-move and disable tick.
 *   3. Otherwise, compute direction to the current waypoint and call AddMovementInput.
 *      Using AddMovementInput (instead of SetActorLocation) lets the CharacterMovementComponent
 *      handle prediction, collision, and smooth replication automatically.
 */
void UAutoMoveComponent::FollowPath()
{
	APawn* OwnerPawn = GetOwnerPawn();
	if (!OwnerPawn || PathPoints.Num() == 0)
	{
		bIsAutoMoving = false;
		SetComponentTickEnabled(false);
		return;
	}

	// Advance past any waypoints we're already close enough to
	while (CurrentPathIndex < PathPoints.Num())
	{
		const float DistXY = FVector::Dist2D(OwnerPawn->GetActorLocation(), PathPoints[CurrentPathIndex]);
		if (DistXY <= AcceptanceRadius)
		{
			CurrentPathIndex++;
		}
		else
		{
			break;
		}
	}

	// Reached the final destination ！ stop
	if (CurrentPathIndex >= PathPoints.Num())
	{
		bIsAutoMoving = false;
		PathPoints.Empty();
		SetComponentTickEnabled(false);
		return;
	}

	// Move toward the current waypoint
	const FVector Direction = (PathPoints[CurrentPathIndex] - OwnerPawn->GetActorLocation()).GetSafeNormal2D();
	OwnerPawn->AddMovementInput(Direction, 1.f);
}

