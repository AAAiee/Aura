// @Copyright HaolunYuan

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "GameplayTagContainer.h"
#include "InventoryManagement/Interface/InvSS_InventoryPlayerControllerInterface.h"
#include "AuraPlayerController.generated.h"

class UInvSS_InventoryComponent;
class AMagicCircle;
class AAuraHUD;
class ACharacter;
class IHighlightable;
class UAuraAbilitySystemComponent;
class UAuraInputComponent;
class UAuraInputConfig;
class UDamageWidgetComponent;
class UInputAction;
class UInputMappingContext;
class UMaterialInterface;
class UNiagaraSystem;
struct FInputActionValue;
struct FHitResult;

/**
 * AAuraPlayerController
 *
 * Coordinates local player input, cursor-driven targeting, click-to-move movement,
 * world-space combat UI, and Gameplay Ability input for the Aura player.
 *
 * This controller exists as the bridge between Enhanced Input, the Aura HUD,
 * AutoMoveComponent, highlightable actors, targeting decals, and the Aura Ability
 * System Component.
 *
 * Important functions:
 *   - ToggleAttributeMenuRequested() - Opens or closes the Attribute Menu.
 *   - ShowMagicCircle() / HideMagicCircle() - Controls ability targeting presentation.
 *   - Client_ShowDamageNumber() - Displays server-resolved combat text locally.
 *   - AbilityInputTagTriggered() - Forwards Enhanced Input ability tags into the ASC.
 *
 * Networking:
 *   - bReplicates is enabled so controller-owned server RPCs can execute.
 *   - Client_ShowDamageNumber() is an owning-client RPC for transient presentation.
 */
UCLASS()
class AURA_API AAuraPlayerController : public APlayerController, public IInvSS_InventoryPlayerControllerInterface
{
	GENERATED_BODY()

public:
	/**
	 * @brief Creates the Aura player controller and its owned movement helper component.
	 */
	AAuraPlayerController();

	// Important functions:
	//   - ToggleAttributeMenuRequested() - Opens or closes the Attribute Menu through the owning Aura HUD.
	//   - ShowMagicCircle() / HideMagicCircle() - Controls the ability targeting decal.
	//   - Client_ShowDamageNumber() - Spawns local combat text for server-resolved hits.
	// ---------------------------------------------------------------------------------------------------------------------
	/* Player-Facing Presentation begins */

	/**
	 * @brief Opens or closes the Attribute Menu through the owning Aura HUD.
	 *
	 * Cancels active click-to-move before changing the menu so cursor-driven movement
	 * and UI state do not compete for player intent.
	 */
	UFUNCTION(BlueprintCallable, Category = "UI")
	void ToggleAttributeMenuRequested();

	/**
	 * @brief Shows the magic circle used for ability targeting feedback.
	 *
	 * @param InMaterial Optional decal material override applied to the spawned circle.
	 *
	 * @note The circle is presentation state owned by this controller.
	 */
	UFUNCTION(BlueprintCallable)
	void ShowMagicCircle(UMaterialInterface* InMaterial);

	/**
	 * @brief Hides and destroys the active magic circle targeting indicator.
	 */
	UFUNCTION(BlueprintCallable)
	void HideMagicCircle();

	UFUNCTION(BlueprintCallable)
	virtual void ToggleInventoryMenu() override;
	
	UFUNCTION(BlueprintCallable)
	virtual UInvSS_InventoryComponent* GetInventoryComponent() override;
	
	/**
	 * @brief Spawns transient world-space combat text near the damaged target.
	 *
	 * @param DamageAmount Amount of damage to display.
	 * @param TargetCharacter Character that should visually anchor the damage text.
	 * @param bIsBlockedHit Whether the server resolved the hit as blocked.
	 * @param bIsCriticalHit Whether the server resolved the hit as critical.
	 *
	 * @note Owning-client RPC. The hit flags are server-resolved and only drive local presentation.
	 */
	UFUNCTION(Client, Reliable)
	void Client_ShowDamageNumber(float DamageAmount, ACharacter* TargetCharacter, bool bIsBlockedHit, bool bIsCriticalHit);

	/* Player-Facing Presentation ends */

	/* APlayerController begins */

	/**
	 * @brief Updates local cursor targeting, highlighting, and targeting-circle placement each frame.
	 *
	 * @param DeltaTime Time elapsed since the previous player tick.
	 */
	virtual void PlayerTick(float DeltaTime) override;

	/* APlayerController ends */
	
protected:
	UFUNCTION()
	void OnToggleInventoryMenu(const FInputActionValue& ActionValues);
	
	/* APlayerController begins */

	/**
	 * @brief Initializes input mapping, cursor mode, HUD cache, and the player camera orientation.
	 */
	virtual void BeginPlay() override;

	/**
	 * @brief Binds native and Gameplay Ability input actions through Enhanced Input.
	 */
	virtual void SetupInputComponent() override;

	/**
	 * @brief Handles possession changes for the Aura player controller.
	 *
	 * @param InPawn Pawn being possessed by this controller.
	 */
	virtual void OnPossess(APawn* InPawn) override;

	/* APlayerController ends */

	/**
	 * @brief Points the possessed Aura character toward the fixed combat camera direction.
	 *
	 * @return true when the camera setup succeeds or no Aura character needs adjustment; false if a required spring arm is missing.
	 */
	bool MakePlayerFacingCameraLookAtDirection();

private:
	/* Input Setup begins */

	/**
	 * @brief Adds the Aura input mapping context to the local player's Enhanced Input subsystem.
	 */
	void InitializeInputContext() const;

	/**
	 * @brief Configures mouse cursor visibility and game/UI input mode.
	 */
	void InitializeMouseCursorMode();

	/**
	 * @brief Binds native movement, menu, targeting, and helper actions.
	 *
	 * @param AuraEnhancedInputComponent Aura-specific Enhanced Input component used for the bindings.
	 */
	void BindNativeInputActions(UAuraInputComponent* AuraEnhancedInputComponent);

	/* Input Setup ends */
	/* UI Menu Flow begins */

	/**
	 * @brief Handles the Attribute Menu input action and forwards it to the public toggle request.
	 *
	 * @param ActionValues Enhanced Input payload, retained for the bound action signature.
	 */
	void OnToggleAttributeMenu(const FInputActionValue& ActionValues);
	

	/**
	 * @brief Checks whether visible floating menus should block cursor movement commands.
	 *
	 * @return true when a menu that consumes movement intent is visible; otherwise false.
	 */
	bool IsAnyMenuOnScreen() const;

	/* UI Menu Flow ends */
	/* Movement Commands begins */

	/**
	 * @brief Applies camera-relative keyboard movement to the possessed pawn.
	 *
	 * @param ActionValues Enhanced Input value containing a 2D movement vector.
	 */
	void Move(const FInputActionValue& ActionValues);

	/**
	 * @brief Requests click-to-move navigation toward the cached cursor hit location.
	 *
	 * @param ActionValues Enhanced Input payload, retained for the bound action signature.
	 */
	void OnClickMove(const FInputActionValue& ActionValues);

	/**
	 * @brief Moves directly toward the cached cursor hit location while the move-to-cursor action is held.
	 *
	 * @param ActionValues Enhanced Input payload, retained for the bound action signature.
	 */
	void OnMoveToCursor(const FInputActionValue& ActionValues);

	/**
	 * @brief Cancels active or pending auto-move requests.
	 */
	void CancelAutoMoveIfActive() const;

	/**
	 * @brief Resolves the cached cursor impact point into a legal movement destination.
	 *
	 * @param OutMoveTargetLocation Receives the cursor impact point when movement is allowed.
	 *
	 * @return true when a cached target exists and current targeting state allows movement.
	 */
	bool TryGetCachedMoveTargetLocation(FVector& OutMoveTargetLocation) const;

	/* Movement Commands ends */
	/* Cursor Targeting begins */

	/**
	 * @brief Traces under the cursor and updates cached hit, highlight, targeting, and magic-circle state.
	 */
	void CursorTrace();

	/**
	 * @brief Stores the latest cursor trace result for movement and targeting systems.
	 *
	 * @param CursorHitResult Hit result produced by the cursor trace.
	 */
	void UpdateCachedCursorHitResult(const FHitResult& CursorHitResult);

	/**
	 * @brief Updates the actor currently eligible for highlight feedback.
	 *
	 * @param CursorHitResult Hit result used to identify highlightable actors.
	 */
	void UpdateCurrentHighlightable(const FHitResult& CursorHitResult);

	/**
	 * @brief Applies highlight or unhighlight calls when the hovered highlightable actor changes.
	 */
	void ApplyHighlightStateTransition() const;

	/**
	 * @brief Moves the active magic circle to the latest cached cursor impact point.
	 */
	void UpdateMagicCirclePosition() const;

	/* Cursor Targeting ends */
	/* Ability Input begins */

	/**
	 * @brief Records that the attack-helper key is being held for ability targeting intent.
	 *
	 * @param ActionValues Enhanced Input payload, retained for the bound action signature.
	 */
	void OnAttackHelpPressed(const FInputActionValue& ActionValues);

	/**
	 * @brief Clears attack-helper targeting intent when the helper key is released.
	 *
	 * @param ActionValues Enhanced Input payload, retained for the bound action signature.
	 */
	void OnAttackHelpReleased(const FInputActionValue& ActionValues);

	/**
	 * @brief Returns and caches the Aura Ability System Component from the possessed pawn.
	 *
	 * @return Cached Aura Ability System Component, or nullptr if the pawn has none.
	 */
	UAuraAbilitySystemComponent* GetAuraASC();

	/**
	 * @brief Handles an Enhanced Input triggered event for an ability input tag.
	 *
	 * @param InputTag Gameplay tag identifying the ability input action.
	 *
	 * @note Consumes repeated Triggered events for the same hold until the matching end event arrives.
	 */
	void AbilityInputTagTriggered(FGameplayTag InputTag);

	/**
	 * @brief Handles release/end input for an ability input tag.
	 *
	 * @param InputTag Gameplay tag identifying the ability input action.
	 */
	void AbilityInputTagEnded(FGameplayTag InputTag);

	/**
	 * @brief Checks whether current targeting intent allows inactive abilities to launch.
	 *
	 * @return true when the player is targeting an enemy or holding the attack-helper key.
	 */
	bool CouldLaunchGameplayAbility() const;

	/* Ability Input ends */

	/* Enhanced Input Assets - set in BP_AuraPlayerController */
	UPROPERTY(EditAnywhere, Category = Input)
	TObjectPtr<UInputMappingContext> AuraContext;

	UPROPERTY(EditAnywhere, Category = Input)
	TObjectPtr<UInputAction> KeyboardMovementAction;

	UPROPERTY(EditAnywhere, Category = Input)
	TObjectPtr<UInputAction> ClickToMoveAction;

	UPROPERTY(EditAnywhere, Category = Input)
	TObjectPtr<UInputAction> MoveToCursorAction;

	UPROPERTY(EditAnywhere, Category = Input)
	TObjectPtr<UInputAction> AttackHelpAction;

	UPROPERTY(EditAnywhere, Category = Input)
	TObjectPtr<UInputAction> ToggleInventoryMenuAction;

	/** Input Action asset bound to the keyboard shortcut that shows/hides the Attribute Menu. */
	UPROPERTY(EditAnywhere, Category = "Input")
	TObjectPtr<UInputAction> ToggleAttributeMenuAction;

	UPROPERTY(EditDefaultsOnly, Category = "Input | Data")
	TObjectPtr<UAuraInputConfig> InputConfigs;

	/* Runtime Systems */
	UPROPERTY(VisibleAnywhere, Category = "Movement")
	TObjectPtr<class UAutoMoveComponent> AutoMoveComponent;

	UPROPERTY()
	TObjectPtr<UAuraAbilitySystemComponent> CachedASC;

	UPROPERTY(Transient)
	TObjectPtr<AAuraHUD> CachedAuraHUD;

	TWeakObjectPtr<UInvSS_InventoryComponent> InventoryComponent;

	/* Player-Facing Presentation Assets */
	UPROPERTY(EditDefaultsOnly, Category = "UI")
	TSubclassOf<UDamageWidgetComponent> DamageTextComponentClass;

	UPROPERTY(EditDefaultsOnly, Category = "Niagara System")
	TObjectPtr<UNiagaraSystem> OnClickMoveNiagaraSystem;

	UPROPERTY(EditDefaultsOnly)
	TSubclassOf<AMagicCircle> MagicCircleClass;

	UPROPERTY(Transient)
	TObjectPtr<AMagicCircle> MagicCircle;

	/* Ability Gating State */
	bool bIsAttackHelpKeyPressed = false;
	bool bIsTargeting = false;
	TSet<FGameplayTag> ConsumedTriggeredInputTags;

	/* Cursor / Movement Target Cache */
	UPROPERTY()
	FHitResult CachedCursorHitResult;

	/* Highlight Tracking */
	UPROPERTY()
	TScriptInterface<IHighlightable> LastHighlightable;

	UPROPERTY()
	TScriptInterface<IHighlightable> CurrentHighlightable;
};
