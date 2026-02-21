// @Copyright HaolunYuan

#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "AuraWidgetController.generated.h"

class UAbilitySystemComponent;
class UAttributeSet;

/**
 * Struct that bundles the core references needed by any Widget Controller.
 * Passed during initialization to supply the controller with access to the Player Controller,
 * Player State, Ability System Component, and Attribute Set.
 */
USTRUCT(BlueprintType)
struct FWidgetControllerParameters
{
	GENERATED_BODY()

	/*Constructors*/
	FWidgetControllerParameters() = default;

	FWidgetControllerParameters(APlayerController* PC, APlayerState* PS, UAbilitySystemComponent* ASC, UAttributeSet* AS)
		: PlayerController(PC), PlayerState(PS), AbilitySystemComponent(ASC), AttributeSet(AS) { }

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
 * Base class for all Widget Controllers in the Aura project.
 * Widget Controllers act as intermediaries between the UI (Widgets) and the game data layer
 * (PlayerState, AbilitySystemComponent, AttributeSet). Derived classes override BroadcastInitialValues
 * and BindAlldDependencies to push data to widgets and react to attribute changes.
 */
UCLASS()
class AURA_API UAuraWidgetController : public UObject
{
	GENERATED_BODY()

public:
	/** Caches the provided parameters so derived controllers can access game data. */
	void SetWidgetControllerParams(const FWidgetControllerParameters& Parameters);

	/** Broadcasts the current attribute values to the UI. Override in derived classes. */
	virtual void BroadcastInitialValues() {};

	/** Binds delegates to attribute change events so the UI updates in real time. Override in derived classes. */
	virtual void BindAlldDependencies() {};

protected:
	/*Data Ref Begins ¡ª cached references for retrieving game data*/
	UPROPERTY(BlueprintReadOnly, Category =WidgetCongtroller)
	TObjectPtr<APlayerController> CachedPlayerController;

	UPROPERTY(BlueprintReadOnly, Category =WidgetController)
	TObjectPtr<APlayerState> CachedPlayerState;

	UPROPERTY(BlueprintReadOnly, Category =WidgetController)
	TObjectPtr<UAbilitySystemComponent> CachedAbilitySystemComponent;

	UPROPERTY(BlueprintReadOnly, Category =WidgetController)
	TObjectPtr<UAttributeSet> CachedAttributeSet;
	/*Data Ref Ends*/

};
