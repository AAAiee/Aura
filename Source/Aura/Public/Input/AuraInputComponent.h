// @Copyright HaolunYuan

#pragma once

#include "CoreMinimal.h"
#include "EnhancedInputComponent.h"
#include "AuraInputConfig.h"
#include "AuraInputComponent.generated.h"

/*
 * Concept:
 * We only want this binding helper to work with UObject-based classes.
 *
 * Why?
 * Because Unreal input binding expects an object instance that participates
 * in Unreal's object system. Constraining this early gives cleaner compile errors.
 */
template<typename T>
concept UObjectDerived = TIsDerivedFrom<T, UObject>::Value;

UCLASS()
class AURA_API UAuraInputComponent : public UEnhancedInputComponent
{
	GENERATED_BODY()

public:

	/*
	 * Educational note:
	 * Instead of having 3 separate callback template parameters
	 * (PressedFuncType, ReleasedFuncType, HeldFuncType),
	 * we define one exact callback shape that this API accepts.
	 *
	 * This makes the API:
	 * - easier to read
	 * - easier to debug
	 * - less error-prone
	 *
	 * This means the bound function must look like:
	 *     void SomeFunction(FGameplayTag InputTag);
	 *
	 * If you want const ref instead, you can change it here in one place.
	 */
	template<typename UserClassType>
	using FAbilityInputFunc = void (UserClassType::*)(FGameplayTag);

	/*
	 * Main binding function.
	 *
	 * PressedFunc / ReleasedFunc / HeldFunc are optional.
	 * Passing nullptr is valid because nullptr can convert to a member-function pointer type.
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

	/*
	 * Small helper to avoid repeating the same binding code 3 times.
	 *
	 * This keeps the main function shorter and easier to understand.
	 */
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
	/*
	 * If no callback was supplied, do nothing.
	 *
	 * Example:
	 * If the caller passes nullptr for ReleasedFunc,
	 * we simply skip binding the Released event.
	 */
	if (!Func)
	{
		return;
	}

	/*
	 * Bind this InputAction to the given trigger event.
	 *
	 * The last argument (Action.InputActionTag) is extra payload data.
	 * Unreal will pass it into the callback when the event fires.
	 */
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
	/*
	 * check() is appropriate here because these are programmer errors:
	 * - InputConfig must exist
	 * - Object must exist
	 *
	 * If either is null, the caller used the API incorrectly.
	 */
	check(InputConfig);
	check(Object);

	/*
	 * InputConfig is a pointer, so we access members with -> not .
	 */
	for (const FAuraInputAction& Action : InputConfig->InputActionEntries)
	{
		/*
		 * Skip invalid entries.
		 *
		 * This is defensive programming:
		 * even if your asset validation is good, it is still smart to guard here.
		 */
		if (!Action.InputAction || !Action.InputActionTag.IsValid())
		{
			continue;
		}

		/*
		 * Started   = initial press
		 * Completed = release
		 * Ongoing   = keep firing while held
		 *
		 * This maps nicely to:
		 * - Pressed
		 * - Released
		 * - Held
		 */
		BindAbilityAction(Action, ETriggerEvent::Started,   Object, PressedFunc );
		BindAbilityAction(Action, ETriggerEvent::Completed, Object, ReleasedFunc);
		BindAbilityAction(Action, ETriggerEvent::Triggered,   Object, HeldFunc);
	}
}