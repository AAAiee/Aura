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
 * Player Controller for the Aura project.
 *
 * Responsibilities:
 *   - Enhanced Input setup (mapping context + action bindings)
 *   - Keyboard WASD movement (camera-relative direction)
 *   - Click-to-move via AutoMoveComponent (server-authoritative nav path)
 *   - Cursor trace for actor highlighting (IHighlightable interface)
 *
 * Input flow:
 *   BeginPlay ¡ú add AuraContext mapping ¡ú SetupInputComponent ¡ú bind Move + OnClickMove
 *
 * Cursor trace flow (every tick):
 *   PlayerTick ¡ú CursorTrace ¡ú line trace under cursor ¡ú highlight/unhighlight actors
 *
 * Replication: bReplicates = true so the controller exists on the server for server RPCs.
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
	/*Movement Handlers Begins*/
	/** WASD movement ¡ª computes camera-relative direction and applies AddMovementInput. */
	void Move(const FInputActionValue& ActionValues);

	/** Left-click movement ¡ª delegates to AutoMoveComponent for server-authoritative pathfinding. */
	void OnClickMove(const FInputActionValue& ActionValues);
	/*Movement Handlers Ends*/

	/**
	 * Runs every tick on the local client. Performs a line trace under the cursor to detect
	 * IHighlightable actors, then manages highlight state transitions (see state table in .cpp).
	 */
	void CursorTrace();

private:
	/*Enhanced Input Assets Begins ¡ª set in the editor on the BP_AuraPlayerController*/
	UPROPERTY(EditAnywhere, Category=Input)
	TObjectPtr<UInputMappingContext> AuraContext;

	UPROPERTY(EditAnywhere, Category=Input)
	TObjectPtr<UInputAction> KeyboardMovementAction;

	UPROPERTY(EditAnywhere, Category =Input)
	TObjectPtr<UInputAction> MouseClickAction;
	/*Enhanced Input Assets Ends*/

	/** Component that handles click-to-move pathfinding via server RPC. */
	UPROPERTY(VisibleAnywhere, Category="Movement")
	TObjectPtr<class UAutoMoveComponent> AutoMoveComponent;

	/*Click-to-Move State Begins*/
	FVector CachedMoveTargetLocation;
	bool bHasCachedMoveTargetLocation = false;
	/*Click-to-Move State Ends*/

	/*Highlight Tracking Begins ¡ª tracks previous and current frame's highlighted actor*/
	UPROPERTY()
	TScriptInterface<IHighlightable>  LastHighlightable;
	UPROPERTY()
	TScriptInterface<IHighlightable>  CurrentHighlightable;
	/*Highlight Tracking Ends*/
};
