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
	FORCEINLINE bool IsAutoMoving() const { return bIsAutoMoving; }
	void RequestToMoveToLocation(const FVector& InTargetPosition);
	void RequestCancelAutoMove();

protected:
	virtual void BeginPlay() override;
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

private: 
	APawn* GetOwnerPawn();

	UFUNCTION(Server, Reliable)
	void Server_RequestMoveToLocation(FVector_NetQuantize InTargetPosition);

	UFUNCTION(Client, Reliable)
	void Client_StartAutoMove(const TArray<FVector_NetQuantize>& InPathPoints);

	UFUNCTION(Client, Reliable)
	void Client_CancelAutoMove();

	void FollowPath();

private:
	/*Path points the client follows via AddMovementInput each tick*/
	TArray<FVector> PathPoints;
	int32 CurrentPathIndex = 0;

	bool bIsAutoMoving = false;

	/*How close to a waypoint before advancing to the next one*/
	UPROPERTY(EditAnywhere, Category = "Movement")
	float AcceptanceRadius = 50.f;
};
