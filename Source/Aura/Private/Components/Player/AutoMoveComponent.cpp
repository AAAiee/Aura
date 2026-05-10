// @Copyright HaolunYuan

#include "Components/Player/AutoMoveComponent.h"

#include "Components/SplineComponent.h"
#include "GameFramework/PlayerController.h"
#include "NavigationPath.h"
#include "NavigationSystem.h"

UAutoMoveComponent::UAutoMoveComponent()
{
	// Ticking is enabled only while a spline path is actively being followed.
	PrimaryComponentTick.bCanEverTick = true;
	PrimaryComponentTick.bStartWithTickEnabled = false;

	SetIsReplicatedByDefault(true);

	SplineComponent = CreateDefaultSubobject<USplineComponent>(TEXT("AutoMoveSpline"));
}

void UAutoMoveComponent::RequestToMoveToLocation(const FVector& InTargetPosition)
{
	const AActor* Owner = GetOwner();
	if (!Owner)
	{
		return;
	}

	// Incrementing the move request ID lets both sides discard stale network responses.
	const int32 MoveRequestId = ++LatestMoveRequestId;
	bHasPendingPathRequest = true;

	// Clients request the path from the server so navigation remains authoritative.
	if (!Owner->HasAuthority())
	{
		Server_RequestMoveToLocation(FVector_NetQuantize(InTargetPosition), MoveRequestId);
		return;
	}

	BuildAndSendPathToClient(InTargetPosition, MoveRequestId);
}

void UAutoMoveComponent::MoveDirectlyToLocation(const FVector& InTargetPosition)
{
	APawn* OwnerPawn = GetOwnerPawn();
	if (!OwnerPawn)
	{
		return;
	}

	// Direct movement is player intent, so it cancels any queued or active click-to-move path.
	if (bIsAutoMoving || bHasPendingPathRequest)
	{
		RequestCancelAutoMove();
	}

	const FVector Direction = (InTargetPosition - OwnerPawn->GetActorLocation()).GetSafeNormal2D();
	if (!Direction.IsNearlyZero())
	{
		OwnerPawn->AddMovementInput(Direction, 1.f);
	}
}

void UAutoMoveComponent::RequestCancelAutoMove()
{
	const AActor* Owner = GetOwner();
	if (!Owner)
	{
		return;
	}

	const int32 MoveRequestId = ++LatestMoveRequestId;
	bHasPendingPathRequest = false;

	StopAutoMove();

	if (Owner->HasAuthority())
	{
		Client_CancelAutoMove(MoveRequestId);
	}
	else
	{
		Server_CancelAutoMove(MoveRequestId);
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
		FollowSpline();
	}
}

APawn* UAutoMoveComponent::GetOwnerPawn()
{
	// This component can live on either the pawn or controller depending on the Blueprint setup.
	if (APawn* PawnOwner = Cast<APawn>(GetOwner()))
	{
		return PawnOwner;
	}

	if (APlayerController* PlayerController = Cast<APlayerController>(GetOwner()))
	{
		return PlayerController->GetPawn();
	}

	return nullptr;
}

void UAutoMoveComponent::BuildAndSendPathToClient(const FVector& InTargetPosition, const int32 MoveRequestId)
{
	APawn* OwnerPawn = GetOwnerPawn();
	if (!OwnerPawn)
	{
		bHasPendingPathRequest = false;
		return;
	}

	// The server asks the navigation system for a path synchronously.
	UNavigationSystemV1* NavSys = FNavigationSystem::GetCurrent<UNavigationSystemV1>(GetWorld());
	if (!NavSys)
	{
		bHasPendingPathRequest = false;
		return;
	}

	UNavigationPath* NavPath = NavSys->FindPathToLocationSynchronously(GetWorld(), OwnerPawn->GetActorLocation(), InTargetPosition);
	if (!NavPath || NavPath->PathPoints.Num() == 0)
	{
		bHasPendingPathRequest = false;
		return;
	}

	// NetQuantize trims precision slightly to reduce path replication bandwidth.
	TArray<FVector_NetQuantize> NetPoints;
	NetPoints.Reserve(NavPath->PathPoints.Num());

	for (const FVector& Point : NavPath->PathPoints)
	{
		NetPoints.Add(FVector_NetQuantize(Point));
	}

	Client_StartAutoMove(NetPoints, MoveRequestId);
}

void UAutoMoveComponent::Server_RequestMoveToLocation_Implementation(const FVector_NetQuantize InTargetPosition, const int32 MoveRequestId)
{
	// Discard out-of-order responses from the network using monotonically increasing IDs.
	if (MoveRequestId <= LatestMoveRequestId)
	{
		return;
	}

	LatestMoveRequestId = MoveRequestId;
	bHasPendingPathRequest = true;

	BuildAndSendPathToClient(FVector(InTargetPosition), MoveRequestId);
}

void UAutoMoveComponent::Server_CancelAutoMove_Implementation(const int32 MoveRequestId)
{
	if (MoveRequestId < LatestMoveRequestId)
	{
		return;
	}

	LatestMoveRequestId = MoveRequestId;
	bHasPendingPathRequest = false;

	Client_CancelAutoMove(MoveRequestId);
}

void UAutoMoveComponent::Client_StartAutoMove_Implementation(const TArray<FVector_NetQuantize>& InPathPoints, const int32 MoveRequestId)
{
	if (MoveRequestId < LatestMoveRequestId)
	{
		return;
	}

	LatestMoveRequestId = MoveRequestId;
	bHasPendingPathRequest = false;

	StartFollowingPath(InPathPoints);
}

void UAutoMoveComponent::Client_CancelAutoMove_Implementation(const int32 MoveRequestId)
{
	if (MoveRequestId < LatestMoveRequestId)
	{
		return;
	}

	LatestMoveRequestId = MoveRequestId;
	bHasPendingPathRequest = false;

	StopAutoMove();
}

void UAutoMoveComponent::StartFollowingPath(const TArray<FVector_NetQuantize>& InPathPoints)
{
	if (!SplineComponent || InPathPoints.Num() == 0)
	{
		StopAutoMove();
		return;
	}

	SplineComponent->ClearSplinePoints(false);

	// Convert navigation points to spline segments.
	for (const FVector_NetQuantize& Point : InPathPoints)
	{
		SplineComponent->AddSplinePoint(FVector(Point), ESplineCoordinateSpace::World, false);
	}

	SplineComponent->UpdateSpline();

	CachedDestination = FVector(InPathPoints.Last());
	bIsAutoMoving = true;

	// Turn ticking on so FollowSpline can advance movement frame by frame.
	SetComponentTickEnabled(true);
}

void UAutoMoveComponent::StopAutoMove()
{
	bIsAutoMoving = false;
	CachedDestination = FVector::ZeroVector;

	if (SplineComponent)
	{
		SplineComponent->ClearSplinePoints();
	}

	// Disable ticking entirely when idle as an optimization.
	SetComponentTickEnabled(false);
}

void UAutoMoveComponent::FollowSpline()
{
	APawn* OwnerPawn = GetOwnerPawn();
	if (!OwnerPawn || !SplineComponent || SplineComponent->GetNumberOfSplinePoints() == 0)
	{
		StopAutoMove();
		return;
	}

	const FVector PawnLocation = OwnerPawn->GetActorLocation();

	// Closest-point recovery keeps the pawn moving even if collision nudges it off the spline.
	const FVector LocationOnSpline = SplineComponent->FindLocationClosestToWorldLocation(PawnLocation, ESplineCoordinateSpace::World);
	const FVector Direction = SplineComponent->FindDirectionClosestToWorldLocation(PawnLocation, ESplineCoordinateSpace::World).GetSafeNormal2D();

	// Standard movement input lets the pawn's movement component handle acceleration and collision.
	if (!Direction.IsNearlyZero())
	{
		OwnerPawn->AddMovementInput(Direction, 1.f);
	}

	// Compute proximity ignoring Z height.
	const float DistanceToDestination = FVector::Dist2D(LocationOnSpline, CachedDestination);
	if (DistanceToDestination <= AcceptanceRadius)
	{
		StopAutoMove();
	}
}
