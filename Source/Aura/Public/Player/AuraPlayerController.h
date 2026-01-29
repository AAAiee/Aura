// @Copyright HaolunYuan

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "AuraPlayerController.generated.h"

class UInputMappingContext;
class UInputAction;
struct FInputActionValue;
class IHighlightable;

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
	/*Movement Related Ends*/

	/*Cursor Functionalities Begins*/
	void CursorTrace();
	/*Cursor Functionalities Ends*/

private:
	/*Input Actions Begin*/ 
	UPROPERTY(EditAnywhere, Category=Input)
	TObjectPtr<UInputMappingContext> AuraContext;

	UPROPERTY(EditAnywhere, Category=Input)
	TObjectPtr<UInputAction> Movement;
	/*Input Actions End*/


	/*Highlight Related Begin*/
	TScriptInterface<IHighlightable>  LastHighlightable;
	TScriptInterface<IHighlightable>  CurrentHighlightable;
	
};
