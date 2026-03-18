// @Copyright HaolunYuan

#include "Components/AbilitySystem/AuraAbilitySystemLibrary.h"
#include "UI/HUD/AuraHUD.h"
#include "Player/AuraPlayerState.h"
#include "Components/AbilitySystem/AuraAbilitySystemComponent.h"
#include "Components/AbilitySystem/AuraAttributeSet.h"
#include "UI/WidgetController/AuraWidgetController.h"

UAuraOverlayWidgetController* UAuraAbilitySystemLibrary::GetOverlayWidgetController(const UObject* WorldContextObject)
{
	if (!WorldContextObject)
	{
		UE_LOG(LogTemp, Error, TEXT("AuraAbilitySystemLibrary::GetOverlayWidgetController - WorldContextObject is null."));
		return nullptr;
	}

	// UI controllers are local-player objects, so we resolve through local player controller.
	APlayerController* LocalPlayerController = GEngine ? GEngine->GetFirstLocalPlayerController(WorldContextObject->GetWorld()) : nullptr;
	if (!LocalPlayerController)
	{
		return nullptr;
	}

	AAuraHUD* AuraHUD = Cast<AAuraHUD>(LocalPlayerController->GetHUD());
	AAuraPlayerState* PS = LocalPlayerController->GetPlayerState<AAuraPlayerState>();
	if (!AuraHUD || !PS)
	{
		return nullptr;
	}

	UAuraAbilitySystemComponent* ASC = Cast<UAuraAbilitySystemComponent>(PS->GetAbilitySystemComponent());
	UAuraAttributeSet* AttributeSet = Cast<UAuraAttributeSet>(PS->GetAttributeSet());
	if (!ASC || !AttributeSet)
	{
		return nullptr;
	}

	const FWidgetControllerParameters Params(LocalPlayerController, PS, ASC, AttributeSet);
	return AuraHUD->GetOverlayWidgetController(Params);
}

UAttributeMenuWidgetController* UAuraAbilitySystemLibrary::GetAttributeMenuWidgetController(const UObject* WorldContextObject)
{
	if (!WorldContextObject)
	{
		UE_LOG(LogTemp, Error, TEXT("AuraAbilitySystemLibrary::GetAttributeMenuWidgetController - WorldContextObject is null."));
		return nullptr;
	}

	APlayerController* LocalPlayerController = GEngine ? GEngine->GetFirstLocalPlayerController(WorldContextObject->GetWorld()) : nullptr;
	if (!LocalPlayerController)
	{
		return nullptr;
	}

	AAuraHUD* AuraHUD = Cast<AAuraHUD>(LocalPlayerController->GetHUD());
	AAuraPlayerState* PS = LocalPlayerController->GetPlayerState<AAuraPlayerState>();
	if (!AuraHUD || !PS)
	{
		return nullptr;
	}

	UAuraAbilitySystemComponent* ASC = Cast<UAuraAbilitySystemComponent>(PS->GetAbilitySystemComponent());
	UAuraAttributeSet* AttributeSet = Cast<UAuraAttributeSet>(PS->GetAttributeSet());
	if (!ASC || !AttributeSet)
	{
		return nullptr;
	}

	const FWidgetControllerParameters Params(LocalPlayerController, PS, ASC, AttributeSet);
	return AuraHUD->GetAttributeMenuWidgetController(Params);
}
