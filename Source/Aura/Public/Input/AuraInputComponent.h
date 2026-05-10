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
	 * PressedFunc / ReleasedFunc / HeldFunc are optional and skipped when nullptr.
	 */
	template<typename UserClassType>
	requires UObjectDerived<UserClassType>
	void BindAbilityActions(
		const UAuraInputConfig* InputConfig,
		UserClassType* Object,
		FAbilityInputFunc<UserClassType> PressedFunc,
		FAbilityInputFunc<UserClassType> ReleasedFunc,
		FAbilityInputFunc<UserClassType> HeldFunc);

private:
	// Shared binder for the Started / Completed / Triggered variants of an input action.
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
	FAbilityInputFunc<UserClassType> PressedFunc,
	FAbilityInputFunc<UserClassType> ReleasedFunc,
	FAbilityInputFunc<UserClassType> HeldFunc)
{
	check(InputConfig);
	check(Object);

	for (const FAuraInputAction& Action : InputConfig->InputActionEntries)
	{
		// Runtime guard mirrors editor validation so broken data assets do not bind empty inputs.
		if (!Action.InputAction || !Action.InputActionTag.IsValid())
		{
			continue;
		}

		BindAbilityAction(Action, ETriggerEvent::Started, Object, PressedFunc);
		BindAbilityAction(Action, ETriggerEvent::Completed, Object, ReleasedFunc);
		BindAbilityAction(Action, ETriggerEvent::Triggered, Object, HeldFunc);
	}
}
