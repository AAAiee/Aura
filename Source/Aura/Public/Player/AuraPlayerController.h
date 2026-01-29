// @Copyright HaolunYuan

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "AuraPlayerController.generated.h"

class UInputMappingContext;
class UInputAction;
struct FInputActionValue;
class IHighlightable;
struct FHitResult;

/**
 * 
 */
UCLASS()
class AURA_API AAuraPlayerController : public APlayerController
{
	GENERATED_BODY()

public:
	AAuraPlayerController();

protected:
	virtual void BeginPlay() override;
	virtual void PlayerTick(float DeltaTime) override;
	virtual void SetupInputComponent() override;

private:
	/*Movement Related Begins*/
	void Move(const FInputActionValue& ActionValues);
	void OnClickMove(const FInputActionValue& ActionValues);
	/*Movement Related Ends*/

	/*Cursor Functionalities Begins*/
	void CursorTrace();
	void CancelAutoMove();
	/*Cursor Functionalities Ends*/

private:
	/*Inputs Actions Begin*/ 
	UPROPERTY(EditAnywhere, Category=Input)
	TObjectPtr<UInputMappingContext> AuraContext;

	UPROPERTY(EditAnywhere, Category=Input)
	TObjectPtr<UInputAction> KeyboardMovementAction;

	UPROPERTY(EditAnywhere, Category =Input)
	TObjectPtr<UInputAction> MouseClickAction;
	/*Inputs Actions End*/

	/*Left Click Auto Moving Related*/
	UPROPERTY()
	TWeakObjectPtr<AActor> CachedMoveTargetActor;
	FVector CachedMoveTargetLocation;
	bool bIsAutoMoving = false;
	/*Cursor Action Related End*/

	/*Highlight Related Begin*/
	UPROPERTY()
	TScriptInterface<IHighlightable>  LastHighlightable;
	UPROPERTY()
	TScriptInterface<IHighlightable>  CurrentHighlightable;
	/*Highlight Related End*/
};
