// @Copyright HaolunYuan

#include "Components/AbilitySystem/Debuff/DebuffNiagaraComponent.h"

#include "AbilitySystemBlueprintLibrary.h"
#include "AbilitySystemComponent.h"
#include "Interaction/CombatInterface.h"

UDebuffNiagaraComponent::UDebuffNiagaraComponent()
{
	UActorComponent::SetAutoActivate(false);
	SetVisibility(false, true);
}

/* Debuff Visual State : BeginPlay() OnDebuffTagChangedCallback() OnOwnerDeath() *****************************/
void UDebuffNiagaraComponent::BeginPlay()
{
	// Pipeline:
	// 1. Resolve the owner's ASC and combat interface.
	// 2. Bind debuff tag-count changes immediately, or defer binding until ASC registration.
	// 3. Bind owner death so visual effects are cleared even if the debuff tag remains.
	Super::BeginPlay();

	UAbilitySystemComponent* ASC = UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(GetOwner());
	ICombatInterface* CombatInterface = Cast<ICombatInterface>(GetOwner());

	if (ASC)
	{
		ASC->RegisterGameplayTagEvent(DebuffTag, EGameplayTagEventType::NewOrRemoved).AddUObject(this, &UDebuffNiagaraComponent::OnDebuffTagChangedCallback);
	}
	else if (CombatInterface)
	{
		CombatInterface->GetOnAscRegisteredDelegate().AddWeakLambda(this, [this](UAbilitySystemComponent* ASC)
		{
			ASC->RegisterGameplayTagEvent(DebuffTag, EGameplayTagEventType::NewOrRemoved).AddUObject(this, &UDebuffNiagaraComponent::OnDebuffTagChangedCallback);
		});
	}

	if (CombatInterface)
	{
		CombatInterface->GetOnCharacterDieDelegate().AddDynamic(this, &UDebuffNiagaraComponent::OnOwnerDeath);
	}
}

void UDebuffNiagaraComponent::OnDebuffTagChangedCallback(const FGameplayTag CallbackTag, int32 NewCount)
{
	// Pipeline:
	// 1. Confirm the owner is still valid, alive, and combat-capable.
	// 2. Activate and reveal the Niagara effect while the debuff tag count is positive.
	// 3. Immediately deactivate and hide the effect when the tag is removed or the owner cannot show it.
	AActor* Owner = GetOwner();
	if (IsValid(Owner)
		&& Owner->Implements<UCombatInterface>()
		&& !ICombatInterface::Execute_IsDead(Owner)
		&& NewCount > 0)
	{
		SetVisibility(true, true);
		Activate(true);
	}
	else
	{
		DeactivateImmediate();
		SetVisibility(false, true);
	}
}

void UDebuffNiagaraComponent::OnOwnerDeath(AActor* DeadActor)
{
	// Pipeline:
	// 1. Stop the Niagara system immediately.
	// 2. Hide the component so no debuff visual lingers on the dead actor.
	DeactivateImmediate();
	SetVisibility(false, true);
}
