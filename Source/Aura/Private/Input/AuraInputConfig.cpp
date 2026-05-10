// @Copyright HaolunYuan


#include "Input/AuraInputConfig.h"
#include "InputAction.h"
#include "Misc/DataValidation.h"



UInputAction* UAuraInputConfig::GetInputAction(FGameplayTag InInputActionTag, bool bLogNotFound /*= false*/) const
{
	for (const FAuraInputAction& Entry : InputActionEntries)
	{
		if (Entry.InputActionTag == InInputActionTag)
		{
			return Entry.InputAction;
		}
	}
	if (bLogNotFound)
	{
		UE_LOG(LogTemp, Warning, TEXT("Can't find input actions related to the input tag, check data asset [%s] to see if the pair [%s] does exist!"), *GetNameSafe(this), *InInputActionTag.GetTagName().ToString());
	}
	return nullptr;
}


#if WITH_EDITOR
EDataValidationResult UAuraInputConfig::IsDataValid(FDataValidationContext& Context) const
{
	EDataValidationResult Result = CombineDataValidationResults(Super::IsDataValid(Context), EDataValidationResult::Valid);

	uint16 Index = 0;

	for (const FAuraInputAction& Entry : InputActionEntries)
	{
		Result = CombineDataValidationResults(Result, Entry.IsDataValid(Context, Index));
		Index++;
	}

	return Result;
}
#endif


#if WITH_EDITOR
EDataValidationResult FAuraInputAction::IsDataValid(FDataValidationContext& Context, const int Index) const
{
	EDataValidationResult Result = EDataValidationResult::Valid;

	if (InputAction == nullptr)
	{
		Result = EDataValidationResult::Invalid;
		const FText ErrorMessage = FText::FromString(FString::Printf(TEXT("InputConfig entry %d: InputAction is missing for tag '%s'."), Index, *InputActionTag.ToString()));
		Context.AddError(ErrorMessage);
	}

	if (!InputActionTag.IsValid())
	{
		Result = EDataValidationResult::Invalid;
		const FString ActionName = InputAction ? InputAction->GetName() : TEXT("None");
		const FText ErrorMessage = FText::FromString(FString::Printf(TEXT("InputConfig entry %d: InputActionTag is invalid. Assigned InputAction: '%s'."), Index, *ActionName));
		Context.AddError(ErrorMessage);
	}

	return Result;
}
#endif
