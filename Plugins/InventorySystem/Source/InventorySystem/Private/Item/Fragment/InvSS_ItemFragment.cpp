// @Copyright HaolunYuan

#include "Item/Fragment/InvSS_ItemFragment.h"

#include "AbilitySystemBlueprintLibrary.h"
#include "AbilitySystemComponent.h"
#include "GameFramework/Pawn.h"
#include "GameFramework/PlayerController.h"

bool FInvSS_ConsumableFragment::OnConsume(APlayerController* PlayerController) const
{
	if (!IsValid(PlayerController)) return false;
	if (!GameplayEffectClass) return false;

	APawn* Pawn = PlayerController->GetPawn();
	if (!IsValid(Pawn)) return false;

	UAbilitySystemComponent* AbilitySystemComponent =
		UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(Pawn);
	if (!IsValid(AbilitySystemComponent)) return false;

	FGameplayEffectContextHandle ContextHandle = AbilitySystemComponent->MakeEffectContext();
	ContextHandle.AddSourceObject(Pawn);

	const FGameplayEffectSpecHandle SpecHandle =
		AbilitySystemComponent->MakeOutgoingSpec(GameplayEffectClass, EffectLevel, ContextHandle);
	if (!SpecHandle.IsValid()) return false;

	AbilitySystemComponent->ApplyGameplayEffectSpecToSelf(*SpecHandle.Data.Get());
	return true;
}
