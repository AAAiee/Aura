// @Copyright HaolunYuan

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "AuraActor.generated.h"

class USphereComponent;
class UStaticMeshComponent;
class UPrimitiveComponent;
struct FHitResult;

UCLASS()
class AURA_API AAuraActor : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	AAuraActor();

protected:
	virtual void BeginPlay() override;

	UFUNCTION()
	virtual void OnOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtrherAcotr, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult); 

	UFUNCTION()
	virtual void OverlapEnd(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex);

private:
	UPROPERTY(VisibleAnywhere)
	TObjectPtr<USphereComponent> SphereComponent;

	UPROPERTY(VisibleAnywhere)
	TObjectPtr<UStaticMeshComponent> StaticMeshComponent;

};
