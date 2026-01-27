// @Copyright HaolunYuan

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "AuraPlayerController.generated.h"

class UInputMappingContext;
class UInputAction;
struct FInputActionValue;

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
	void BeginPlay() override;
	/*Input Component Setup*/
	void SetupInputComponent() override;

private:
	/*Movement*/
	void Move(const FInputActionValue& ActionValues);

private:
	/*Input Actions Begin*/ 
	UPROPERTY(EditAnywhere, Category=Input)
	TObjectPtr<UInputMappingContext> AuraContext;

	UPROPERTY(EditAnywhere, Category=Input)
	TObjectPtr<UInputAction> Movement;
	/*Input Actions End*/
	
};
