// @Copyright HaolunYuan
#include "Components/Player/AutoMoveComponent.h"
#include "Components/SplineComponent.h"
#include "GameFramework/PlayerController.h"
#include "NavigationPath.h"
#include "NavigationSystem.h"

UAutoMoveComponent::UAutoMoveComponent()
{
	// Enable ticking but default it to disabled until auto-move starts
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

	// Incrementing the move request ID helps us discard outdated responses/commands
	const int32 MoveRequestId = ++LatestMoveRequestId;
	bHasPendingPathRequest = true;

	// If calculating locally, we are either autonomous or server, so if we aren't the server, ask the server
	if (!Owner->HasAuthority())
	{
		Server_RequestMoveToLocation(FVector_NetQuantize(InTargetPosition), MoveRequestId);
		return;
	}

	// If we are the server, calculate it now
	BuildAndSendPathToClient(InTargetPosition, MoveRequestId);
}

void UAutoMoveComponent::MoveDirectlyToLocation(const FVector& InTargetPosition)
{
	APawn* OwnerPawn = GetOwnerPawn();
	if (!OwnerPawn)
	{
		return;
	}

	// Any manual input cancels an existing auto-move
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
	// Handle cases where the component may live on either the pawn or the controller
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

	// The server asks the navigation system for a path synchronously
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

	// NetQuantize truncates precision slightly to save network bandwidth
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
	// Discard out-of-order responses from the network using standard monotonically increasing IDs
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

	// Convert navigation points to spline segments
	for (const FVector_NetQuantize& Point : InPathPoints)
	{
		SplineComponent->AddSplinePoint(FVector(Point), ESplineCoordinateSpace::World, false);
	}

	SplineComponent->UpdateSpline();

	CachedDestination = FVector(InPathPoints.Last());
	bIsAutoMoving = true;

	// Turn ticking on so the FollowSpline logic runs frame-by-frame
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

	// Disable ticking entirely when idle as an optimization
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

	// Finding the mathematically closest point and direction ensures robust recovery if Pawn gets bumped offtrack
	const FVector LocationOnSpline = SplineComponent->FindLocationClosestToWorldLocation(PawnLocation, ESplineCoordinateSpace::World);
	const FVector Direction = SplineComponent->FindDirectionClosestToWorldLocation(PawnLocation, ESplineCoordinateSpace::World).GetSafeNormal2D();

	// Use standard movement input framework allowing the Pawn's character component to do the math (e.g., collisions/acceleration)
	if (!Direction.IsNearlyZero())
	{
		OwnerPawn->AddMovementInput(Direction, 1.f);
	}

	// Compute proximity ignoring Z height
	const float DistanceToDestination = FVector::Dist2D(LocationOnSpline, CachedDestination);
	if (DistanceToDestination <= AcceptanceRadius)
	{
		StopAutoMove();
	}
}