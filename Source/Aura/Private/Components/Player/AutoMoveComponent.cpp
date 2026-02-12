// @Copyright HaolunYuan


#include "Components/Player/AutoMoveComponent.h"
#include "NavigationSystem.h"
#include "NavigationPath.h"
#include "GameFramework/Character.h"


UAutoMoveComponent::UAutoMoveComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
	PrimaryComponentTick.bStartWithTickEnabled = false;
	SetIsReplicatedByDefault(true);
}


void UAutoMoveComponent::RequestToMoveToLocation(const FVector& InTargetPosition)
{
	if (const AActor* Owner = GetOwner())
	{
		if (!Owner->HasAuthority())
		{
			/*Client requests the server to compute a nav path.*/
			Server_RequestMoveToLocation(FVector_NetQuantize(InTargetPosition));
			return;
		}

		/*Server computes a nav path and sends it back to the owning client.*/
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


void UAutoMoveComponent::RequestCancelAutoMove()
{
	if (const AActor* Owner = GetOwner())
	{
		if (Owner->HasAuthority())
		{
			/*Server tells the owning client to stop auto-move.*/
			Client_CancelAutoMove();
		}
		else
		{
			/*Client-side immediate cancel for responsiveness.*/
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


void UAutoMoveComponent::Server_RequestMoveToLocation_Implementation(const FVector_NetQuantize InTargetPosition)
{
	RequestToMoveToLocation(FVector(InTargetPosition));
}


void UAutoMoveComponent::Client_StartAutoMove_Implementation(const TArray<FVector_NetQuantize>& InPathPoints)
{
	PathPoints.Empty(InPathPoints.Num());
	for (const FVector_NetQuantize& Point : InPathPoints)
	{
		PathPoints.Add(FVector(Point));
	}

	CurrentPathIndex = 0;
	bIsAutoMoving = true;
	SetComponentTickEnabled(true);
}


void UAutoMoveComponent::Client_CancelAutoMove_Implementation()
{
	bIsAutoMoving = false;
	PathPoints.Empty();
	SetComponentTickEnabled(false);
}


void UAutoMoveComponent::FollowPath()
{
	APawn* OwnerPawn = GetOwnerPawn();
	if (!OwnerPawn || PathPoints.Num() == 0)
	{
		bIsAutoMoving = false;
		SetComponentTickEnabled(false);
		return;
	}

	/*Advance to the next waypoint if close enough*/
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

	/*Stop when the final destination is reached*/
	if (CurrentPathIndex >= PathPoints.Num())
	{
		bIsAutoMoving = false;
		PathPoints.Empty();
		SetComponentTickEnabled(false);
		return;
	}

	/*Drive movement through AddMovementInput to preserve client prediction*/
	const FVector Direction = (PathPoints[CurrentPathIndex] - OwnerPawn->GetActorLocation()).GetSafeNormal2D();
	OwnerPawn->AddMovementInput(Direction, 1.f);
}

