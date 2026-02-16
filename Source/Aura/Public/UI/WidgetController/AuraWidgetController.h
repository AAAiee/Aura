// @Copyright HaolunYuan

#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "AuraWidgetController.generated.h"

class UAbilitySystemComponent;
class UAttributeSet;

USTRUCT(BlueprintType)
struct FWidgetControllerParameters
{
	GENERATED_BODY()

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
 * Widget Controller is responsible for retrieving data from the player state, and then pass the data to the widget. It will act as a middleman
 */
UCLASS()
class AURA_API UAuraWidgetController : public UObject
{
	GENERATED_BODY()

public:
	void SetWidgetControllerParams(const FWidgetControllerParameters& Parameters);
	virtual void BroadcastInitialValues() {};
	virtual void BindAlldDependencies() {};

protected:
	//Data Ref, where we can retrieve data
	UPROPERTY(BlueprintReadOnly, Category =WidgetCongtroller)
	TObjectPtr<APlayerController> CachedPlayerController;

	UPROPERTY(BlueprintReadOnly, Category =WidgetController)
	TObjectPtr<APlayerState> CachedPlayerState;

	UPROPERTY(BlueprintReadOnly, Category =WidgetController)
	TObjectPtr<UAbilitySystemComponent> CachedAbilitySystemComponent;

	UPROPERTY(BlueprintReadOnly, Category =WidgetController)
	TObjectPtr<UAttributeSet> CachedAttributeSet;
	

};
