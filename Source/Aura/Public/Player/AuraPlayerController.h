// @Copyright HaolunYuan

#pragma once
#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "GameplayTagContainer.h"
#include "AuraPlayerController.generated.h"

class UInputMappingContext;
class UInputAction;
struct FInputActionValue;
class ACharacter;
class IHighlightable;
struct FHitResult;
class UAuraInputConfig;
class UAuraInputComponent;
class UAuraAbilitySystemComponent;
class AAuraHUD;
class UDamageWidgetComponent;

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

	UFUNCTION(BlueprintCallable, Category="UI")
	void ToggleAttributeMenuRequested();

	// Owning-client RPC that spawns transient combat text near the damaged target.
	// The hit flags are already server-resolved and only drive local presentation.
	UFUNCTION(Client,Reliable)
	void Client_ShowDamageNumber(float DamageAmount, ACharacter* TargetCharacter, bool bIsBlockedHit, bool bIsCriticalHit);

protected:
	virtual void BeginPlay() override;
	virtual void PlayerTick(float DeltaTime) override;
	virtual void SetupInputComponent() override;

private:
	/* Setup Helpers */
	void InitializeInputContext() const;
	void InitializeMouseCursorMode();
	void BindNativeInputActions(UAuraInputComponent* AuraEnhancedInputComponent);

	/* UI */
	AAuraHUD* GetAuraHUD() const;
	void OnToggleAttributeMenu(const FInputActionValue& ActionValues);

	/* Movement */
	void Move(const FInputActionValue& ActionValues);
	void OnClickMove(const FInputActionValue& ActionValues);
	void OnMoveToCursor(const FInputActionValue& ActionValues);
	void OnAttackHelpPressed(const FInputActionValue& ActionValues);
	void OnAttackHelpReleased(const FInputActionValue& ActionValues);
	void CancelAutoMoveIfActive() const;
	bool TryGetCachedMoveTargetLocation(FVector& OutMoveTargetLocation) const;

	/* Cursor Trace + Highlight */
	void CursorTrace();
	void UpdateCachedCursorHitResult(const FHitResult& CursorHitResult);
	void UpdateCurrentHighlightable(const FHitResult& CursorHitResult);
	void ApplyHighlightStateTransition();

	/* Ability Input */
	UAuraAbilitySystemComponent* GetAuraASC();
	void AbilityInputTagPressed(FGameplayTag InputTag);
	void AbilityInputTagReleased(FGameplayTag InputTag);
	void AbilityInputTagHeld(FGameplayTag InputTag);
	void ForwardAbilityInputTag(FGameplayTag InputTag, void (UAuraAbilitySystemComponent::*InputHandler)(FGameplayTag));
	bool CouldLaunchGameplayAbility() const;

private:
	/* Enhanced Input Assets - set in BP_AuraPlayerController */
	UPROPERTY(EditAnywhere, Category=Input)
	TObjectPtr<UInputMappingContext> AuraContext;

	UPROPERTY(EditAnywhere, Category=Input)
	TObjectPtr<UInputAction> KeyboardMovementAction;

	UPROPERTY(EditAnywhere, Category=Input)
	TObjectPtr<UInputAction> ClickToMoveAction;

	UPROPERTY(EditAnywhere, Category=Input)
	TObjectPtr<UInputAction> MoveToCursorAction;

	UPROPERTY(EditAnywhere, Category=Input)
	TObjectPtr<UInputAction> AttackHelpAction;

	/** Input Action asset bound to the keyboard shortcut that shows/hides the Attribute Menu. */
	UPROPERTY(EditAnywhere, Category="Input")
	TObjectPtr<UInputAction> ToggleAttributeMenuAction;

	UPROPERTY(EditDefaultsOnly, Category = "Input | Data")
	TObjectPtr<UAuraInputConfig> InputConfigs;

	TObjectPtr<UAuraAbilitySystemComponent> CachedASC;

	UPROPERTY(VisibleAnywhere, Category="Movement")
	TObjectPtr<class UAutoMoveComponent> AutoMoveComponent;

	/* Cursor / Movement Target Cache */
	FHitResult CachedCursorHitResult;

	/* Ability Gating State */
	bool bIsAttackHelpKeyPressed = false;
	bool bIsTargeting = false;

	/* Highlight Tracking */
	UPROPERTY()
	TScriptInterface<IHighlightable>  LastHighlightable;
	UPROPERTY()
	TScriptInterface<IHighlightable>  CurrentHighlightable;


	/*UI*/
	// Widget-component class used for floating combat text spawned by Client_ShowDamageNumber().
	UPROPERTY(EditDefaultsOnly, Category="UI")
	TSubclassOf<UDamageWidgetComponent> DamageTextComponentClass;
};
