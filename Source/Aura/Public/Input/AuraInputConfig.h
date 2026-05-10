// @Copyright HaolunYuan

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "GameplayTagContainer.h"
#include "Misc/DataValidation.h"
#include "AuraInputConfig.generated.h"

class UInputAction;

/**
 * One Enhanced Input action paired with the gameplay tag forwarded to ability input.
 */
USTRUCT(BlueprintType)
struct FAuraInputAction
{
	GENERATED_BODY()

	UPROPERTY(EditDefaultsOnly)
	TObjectPtr<UInputAction> InputAction;

	UPROPERTY(EditDefaultsOnly)
	FGameplayTag InputActionTag;

#if WITH_EDITOR
	EDataValidationResult IsDataValid(FDataValidationContext& Context, const int Index) const;
#endif
};

/**
 * Data asset that maps Enhanced Input actions to Aura gameplay input tags.
 */
UCLASS()
class AURA_API UAuraInputConfig : public UDataAsset
{
	GENERATED_BODY()

public:
	UInputAction* GetInputAction(FGameplayTag InInputActionTag, bool bLogNotFound = false) const;

#if WITH_EDITOR
	virtual EDataValidationResult IsDataValid(FDataValidationContext& Context) const override;
#endif

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, meta = (AllowPrivateAccess = true))
	TArray<FAuraInputAction> InputActionEntries;
};
