// @Copyright HaolunYuan

#pragma once

#include "CoreMinimal.h"
#include "EnhancedInputComponent.h"
#include "Input/AuraInputConfig.h"
#include "AuraInputComponent.generated.h"

/*
 * Enhanced input binding targets must be UObject-derived because Unreal stores
 * receiver objects in its reflection-aware delegate system.
 */
template<typename T>
concept UObjectDerived = TIsDerivedFrom<T, UObject>::Value;

/**
 * Enhanced input component with GameplayTag-aware ability binding.
 *
 * PlayerController code supplies callbacks once, and this component binds every tagged input action
 * from UAuraInputConfig while forwarding the action's GameplayTag as callback payload.
 *
 * Template definitions remain in the header because Unreal's input delegates bind caller-specific
 * UObject types at compile time.
 */
UCLASS()
class AURA_API UAuraInputComponent : public UEnhancedInputComponent
{
	GENERATED_BODY()

public:
	/*
	 * Ability input callbacks receive the gameplay tag bound to the input action,
	 * letting the controller forward input without knowing ability classes.
	 */
	template<typename UserClassType>
	using FAbilityInputFunc = void (UserClassType::*)(FGameplayTag);

	/*
	 * Binds all tag-driven ability inputs from a config asset.
	 * TriggeredFunc / CompletedFunc / CanceledFunc are optional and skipped when nullptr.
	 */
	template<typename UserClassType>
	requires UObjectDerived<UserClassType>
	void BindAbilityActions(
		const UAuraInputConfig* InputConfig,
		UserClassType* Object,
		FAbilityInputFunc<UserClassType> TriggeredFunc,
		FAbilityInputFunc<UserClassType> CompletedFunc,
		FAbilityInputFunc<UserClassType> CanceledFunc);

private:
	// Shared binder for the Triggered / Completed / Canceled variants of an input action.
	template<typename UserClassType>
	requires UObjectDerived<UserClassType>
	void BindAbilityAction(
		const FAuraInputAction& Action,
		const ETriggerEvent TriggerEvent,
		UserClassType* Object,
		FAbilityInputFunc<UserClassType> Func);
};

template<typename UserClassType>
requires UObjectDerived<UserClassType>
void UAuraInputComponent::BindAbilityAction(
	const FAuraInputAction& Action,
	const ETriggerEvent TriggerEvent,
	UserClassType* Object,
	FAbilityInputFunc<UserClassType> Func)
{
	if (!Func)
	{
		return;
	}

	// The tag becomes payload data passed into the bound controller callback.
	BindAction(Action.InputAction, TriggerEvent, Object, Func, Action.InputActionTag);
}

template<typename UserClassType>
requires UObjectDerived<UserClassType>
void UAuraInputComponent::BindAbilityActions(
	const UAuraInputConfig* InputConfig,
	UserClassType* Object,
	FAbilityInputFunc<UserClassType> TriggeredFunc,
	FAbilityInputFunc<UserClassType> CompletedFunc,
	FAbilityInputFunc<UserClassType> CanceledFunc)
{
	checkf(InputConfig, TEXT("BindAbilityActions requires a valid input config."));
	checkf(Object, TEXT("BindAbilityActions requires a valid receiver object."));

	for (const FAuraInputAction& Action : InputConfig->InputActionEntries)
	{
		// Runtime guard mirrors editor validation so broken data assets do not bind empty inputs.
		if (!ensureMsgf(Action.InputAction,
			TEXT("Input config [%s] contains an ability input entry with no InputAction."),
			*GetNameSafe(InputConfig)))
		{
			continue;
		}

		if (!ensureMsgf(Action.InputActionTag.IsValid(),
			TEXT("Input action [%s] in config [%s] has no valid InputActionTag."),
			*GetNameSafe(Action.InputAction),
			*GetNameSafe(InputConfig)))
		{
			continue;
		}

		BindAbilityAction(Action, ETriggerEvent::Triggered, Object, TriggeredFunc);
		BindAbilityAction(Action, ETriggerEvent::Completed, Object, CompletedFunc);
		BindAbilityAction(Action, ETriggerEvent::Canceled, Object, CanceledFunc);
	}
}
