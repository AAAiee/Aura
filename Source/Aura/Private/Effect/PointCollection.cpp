// @Copyright HaolunYuan

#include "Effect/PointCollection.h"

#include "Components/BillboardComponent.h"
#include "Components/AbilitySystem/AuraAbilitySystemLibrary.h"
#include "Kismet/KismetMathLibrary.h"

/* Ground Point Placement : APointCollection() GetGroundPoints() *****************************/
APointCollection::APointCollection()
{
	// Pipeline:
	// 1. Disable ticking because point collections are queried on demand by abilities.
	// 2. Create the root point, then register every reusable point in ImmutablePtrs.
	// 3. Attach child points to the root so designers can author their relative layout in the editor.
	PrimaryActorTick.bCanEverTick = false;

	Pt_0 = CreateDefaultSubobject<USceneComponent>("Pt_0");
	ImmutablePtrs.Add(Pt_0);
	SetRootComponent(Pt_0);

	Pt_1 = CreateDefaultSubobject<USceneComponent>("Pt_1");
	ImmutablePtrs.Add(Pt_1);
	Pt_1->SetupAttachment(GetRootComponent());

	Pt_2 = CreateDefaultSubobject<USceneComponent>("Pt_2");
	ImmutablePtrs.Add(Pt_2);
	Pt_2->SetupAttachment(GetRootComponent());

	Pt_3 = CreateDefaultSubobject<USceneComponent>("Pt_3");
	ImmutablePtrs.Add(Pt_3);
	Pt_3->SetupAttachment(GetRootComponent());

	Pt_4 = CreateDefaultSubobject<USceneComponent>("Pt_4");
	ImmutablePtrs.Add(Pt_4);
	Pt_4->SetupAttachment(GetRootComponent());

	Pt_5 = CreateDefaultSubobject<USceneComponent>("Pt_5");
	ImmutablePtrs.Add(Pt_5);
	Pt_5->SetupAttachment(GetRootComponent());

	Pt_6 = CreateDefaultSubobject<USceneComponent>("Pt_6");
	ImmutablePtrs.Add(Pt_6);
	Pt_6->SetupAttachment(GetRootComponent());

	Pt_7 = CreateDefaultSubobject<USceneComponent>("Pt_7");
	ImmutablePtrs.Add(Pt_7);
	Pt_7->SetupAttachment(GetRootComponent());

	Pt_8 = CreateDefaultSubobject<USceneComponent>("Pt_8");
	ImmutablePtrs.Add(Pt_8);
	Pt_8->SetupAttachment(GetRootComponent());

	Pt_9 = CreateDefaultSubobject<USceneComponent>("Pt_9");
	ImmutablePtrs.Add(Pt_9);
	Pt_9->SetupAttachment(GetRootComponent());

	Pt_10 = CreateDefaultSubobject<USceneComponent>("Pt_10");
	ImmutablePtrs.Add(Pt_10);
	Pt_10->SetupAttachment(GetRootComponent());

	Pt_11 = CreateDefaultSubobject<USceneComponent>("Pt_11");
	ImmutablePtrs.Add(Pt_11);
	Pt_11->SetupAttachment(GetRootComponent());
}

TArray<USceneComponent*> APointCollection::GetGroundPoints(const FVector& GroundLocation, int32 NumPoints, float YawOverride)
{
	// Pipeline:
	// 1. Verify the caller requested no more points than the collection owns.
	// 2. Prepare each reusable point through the current point-adjustment path.
	// 3. Trace vertically near the requested ground location while ignoring nearby live actors.
	// 4. Snap the point to the ground impact and align it to the surface normal before returning it.
	check(ImmutablePtrs.Num() >= NumPoints);

	TArray<USceneComponent*> ArrayCopy;

	for (USceneComponent* Pt : ImmutablePtrs)
	{
		if (!Pt == Pt_0)
		{
			const FVector Pt0Location = Pt->GetComponentLocation();
			FVector Pt0ToPt = Pt->GetComponentLocation() - Pt0Location;
			Pt0ToPt = Pt0ToPt.RotateAngleAxis(YawOverride, FVector::UpVector);
			Pt->SetWorldLocation(Pt0Location + Pt0ToPt);
		}

		const FVector PtLocation = Pt->GetComponentLocation();
		const FVector RaisedLocation = FVector(PtLocation.X, PtLocation.Y, GroundLocation.Z + 500.f);
		const FVector LoweredLocation = FVector(PtLocation.X, PtLocation.Y, GroundLocation.Z - 500.f);

		FHitResult HitResult;

		TArray<AActor*> IgnoreActors;
		UAuraAbilitySystemLibrary::GetLivePlayersWithinRadius(this,
			IgnoreActors,
			{ this },
			1000.f,
			GetActorLocation());

		FCollisionQueryParams QueryParams;
		QueryParams.AddIgnoredActors(IgnoreActors);
		GetWorld()->LineTraceSingleByProfile(HitResult,
			LoweredLocation,
			RaisedLocation,
			FName("BlockALl"),
			QueryParams);

		const FVector AdjustedLocation = FVector(PtLocation.X, PtLocation.Y, HitResult.ImpactPoint.Z);
		Pt->SetWorldLocation(AdjustedLocation);
		Pt->SetWorldRotation(UKismetMathLibrary::MakeRotFromZ(HitResult.ImpactNormal));

		ArrayCopy.Add(Pt);
	}

	return ArrayCopy;
}

void APointCollection::BeginPlay()
{
	Super::BeginPlay();
}
