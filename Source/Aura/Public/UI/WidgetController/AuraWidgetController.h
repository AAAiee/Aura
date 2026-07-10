// @Copyright HaolunYuan

#pragma once

#include "CoreMinimal.h"
#include "Components/AbilitySystem/Data/AbilityInfo.h"
#include "UObject/NoExportTypes.h"
#include "AuraWidgetController.generated.h"

class APlayerController;
class APlayerState;
class UAbilitySystemComponent;
class UAttributeSet;
class AAuraPlayerController;
class AAuraPlayerState;
class UAuraAbilitySystemComponent;
class UAuraAttributeSet;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FAbilityInfoSignature, const FAuraAbilityInfo&, AbilityInfo);

/**
 * Parameter bundle passed to Widget Controllers during initialization.
 * Contains the four core references every controller needs to access game data.
 */
USTRUCT(BlueprintType)
struct FWidgetControllerParameters
{
	GENERATED_BODY()

	FWidgetControllerParameters() = default;

	FWidgetControllerParameters(APlayerController* PC, APlayerState* PS, UAbilitySystemComponent* ASC, UAttributeSet* AS)
		: PlayerController(PC)
		, PlayerState(PS)
		, AbilitySystemComponent(ASC)
		, AttributeSet(AS)
	{
	}

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TObjectPtr<APlayerController> PlayerController = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TObjectPtr<APlayerState> PlayerState = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TObjectPtr<UAbilitySystemComponent> AbilitySystemComponent = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TObjectPtr<UAttributeSet> AttributeSet = nullptr;
};


/**
 * Base class for all Widget Controllers in the Aura UI system.
 *
 * Role in the MVC pattern:
 *   - Widget (View) displays data and fires user input events.
 *   - WidgetController (Controller) fetches data from GAS and pushes it to the widget via delegates.
 *   - GAS / PlayerState / AttributeSet (Model) holds the actual game state.
 *
 * Derived classes override:
 *   - BroadcastInitialValues() - push current values so the UI starts correct.
 *   - BindAllDependencies()    - subscribe to ASC delegates so the UI updates in real time.
 *
 * NOTE: This is a UObject, NOT an AActor. It has no transform or tick. It lives as long as
 * the HUD keeps a UPROPERTY reference to it.
 */
UCLASS()
class AURA_API UAuraWidgetController : public UObject
{
	GENERATED_BODY()

public:
	/** Stores the four core references (PC, PS, ASC, AS) for derived controllers to use. */
	void SetWidgetControllerParams(const FWidgetControllerParameters& Parameters);

	/** Override to broadcast current attribute values to the UI on startup. */
	UFUNCTION(BlueprintCallable)
	virtual void BroadcastInitialValues();

	/** Override to bind delegates to ASC attribute changes, GE events, etc. */
	virtual void BindAllDependencies();

	UFUNCTION(BlueprintPure)
	AActor* GetAvatarActor() const;

	/** Returns the ASC owner actor, which is often the PlayerState for player-owned ASCs. */
	UFUNCTION(BlueprintPure)
	AActor* GetOwningActor() const;

	/* Convenience accessors for cached Aura-specific references. */
	UFUNCTION(BlueprintPure)
	AAuraPlayerController* GetAuraPlayerController();

	UFUNCTION(BlueprintPure)
	AAuraPlayerState* GetAuraPlayerState();

	UFUNCTION(BlueprintPure)
	UAuraAbilitySystemComponent* GetAuraAbilitySystemComponent();

	UFUNCTION(BlueprintPure)
	UAuraAttributeSet* GetAuraAttributeSet();

	/** Iterates granted ASC ability specs and broadcasts the UI data row for each known ability. */
	void BroadcastAbilityInfo();

	/** Blueprint widgets bind to this to receive ability icon/status/input-slot updates. */
	UPROPERTY(BlueprintAssignable, Category = "Delegate", meta = (AllowPrivateAccess = "true"))
	FAbilityInfoSignature AbilityInfoDelegate;

protected:
	/* Cached references: set once via SetWidgetControllerParams, read by derived classes. */
	UPROPERTY(BlueprintReadOnly, Category = DataRef)
	TObjectPtr<APlayerController> CachedPlayerController;

	UPROPERTY(BlueprintReadOnly, Category = DataRef)
	TObjectPtr<APlayerState> CachedPlayerState;

	UPROPERTY(BlueprintReadOnly, Category = DataRef)
	TObjectPtr<UAbilitySystemComponent> CachedAbilitySystemComponent;

	UPROPERTY(BlueprintReadOnly, Category = DataRef)
	TObjectPtr<UAttributeSet> CachedAttributeSet;

	/* Cached Aura references. */
	UPROPERTY(BlueprintReadOnly, Category = DataRef)
	TObjectPtr<AAuraPlayerController> CachedAuraPlayerController;

	UPROPERTY(BlueprintReadOnly, Category = DataRef)
	TObjectPtr<AAuraPlayerState> CachedAuraPlayerState;

	UPROPERTY(BlueprintReadOnly, Category = DataRef)
	TObjectPtr<UAuraAbilitySystemComponent> CachedAuraAbilitySystemComponent;

	UPROPERTY(BlueprintReadOnly, Category = DataRef)
	TObjectPtr<UAuraAttributeSet> CachedAuraAttributeSet;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Data", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UAbilityInfo> AbilityInfoDataAsset;
};
