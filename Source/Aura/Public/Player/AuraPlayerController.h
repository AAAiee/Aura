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
 *   - Attribute Menu toggling via keyboard/UI request
 *
 * Input flow:
 *   BeginPlay -> add AuraContext mapping -> SetupInputComponent -> bind Move + OnClickMove + ToggleAttributeMenu
 *
 * Cursor trace flow (every tick):
 *   PlayerTick -> CursorTrace -> line trace under cursor -> highlight/unhighlight actors
 *
 * Replication: bReplicates = true so the controller exists on the server for server RPCs.
 */
UCLASS()
class AURA_API AAuraPlayerController : public APlayerController
{
	GENERATED_BODY()

public:
	AAuraPlayerController();

	/** Project Extension — shared entry point for the custom Attribute Menu toggle feature. */
	/** Public entry point so BP/UI can reuse the same toggle logic as the input action. */
	UFUNCTION(BlueprintCallable, Category="UI")
	void ToggleAttributeMenuRequested(); 


protected:
	virtual void BeginPlay() override;
	virtual void PlayerTick(float DeltaTime) override;
	virtual void SetupInputComponent() override;

private:
	/*Movement Handlers Begins*/
	/** WASD movement — computes camera-relative direction and applies AddMovementInput. */
	void Move(const FInputActionValue& ActionValues);

	/** Left-click movement — delegates to AutoMoveComponent for server-authoritative pathfinding. */
	void OnClickMove(const FInputActionValue& ActionValues);
	/*Movement Handlers Ends*/

	/** Enhanced Input callback for the "toggle attribute menu" action. */
	void OnToggleAttributeMenu(const FInputActionValue& ActionValues);

	/**
	 * Runs every tick on the local client. Performs a line trace under the cursor to detect
	 * IHighlightable actors, then manages highlight state transitions (see state table in .cpp).
	 */
	void CursorTrace();

private:
	/*Enhanced Input Assets Begins — set in the editor on the BP_AuraPlayerController*/
	UPROPERTY(EditAnywhere, Category=Input)
	TObjectPtr<UInputMappingContext> AuraContext;

	UPROPERTY(EditAnywhere, Category=Input)
	TObjectPtr<UInputAction> KeyboardMovementAction;

	UPROPERTY(EditAnywhere, Category=Input)
	TObjectPtr<UInputAction> MouseClickAction;

	/** Input Action asset bound to the keyboard shortcut that shows/hides the Attribute Menu. */
	UPROPERTY(EditAnywhere, Category="Input")
	TObjectPtr<UInputAction> ToggleAttributeMenuAction;
	/*Enhanced Input Assets Ends*/

	/** Component that handles click-to-move pathfinding via server RPC. */
	UPROPERTY(VisibleAnywhere, Category="Movement")
	TObjectPtr<class UAutoMoveComponent> AutoMoveComponent;

	/*Click-to-Move State Begins*/
	FVector CachedMoveTargetLocation;
	bool bHasCachedMoveTargetLocation = false;
	/*Click-to-Move State Ends*/

	/*Highlight Tracking Begins — tracks previous and current frame's highlighted actor*/
	UPROPERTY()
	TScriptInterface<IHighlightable>  LastHighlightable;
	UPROPERTY()
	TScriptInterface<IHighlightable>  CurrentHighlightable;
	/*Highlight Tracking Ends*/
};
