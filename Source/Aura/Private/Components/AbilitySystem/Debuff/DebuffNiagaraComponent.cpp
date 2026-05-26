// @Copyright HaolunYuan


#include "Components/AbilitySystem/Debuff/DebuffNiagaraComponent.h"

#include "AbilitySystemBlueprintLibrary.h"
#include "AbilitySystemComponent.h"
#include "Interaction/CombatInterface.h"

UDebuffNiagaraComponent::UDebuffNiagaraComponent()
{
	UNiagaraComponent::Deactivate(); 
}

void UDebuffNiagaraComponent::BeginPlay()
{
	Super::BeginPlay();
	
	UAbilitySystemComponent* ASC = UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(GetOwner());
	ICombatInterface* CombatInterface = Cast<ICombatInterface>(GetOwner()); 
	
	/*If ASC is already valid at this point, bind the delegates, otherwise bind to ASCconstruct delegate and bind delegates then */
	if (ASC)
	{
		ASC->RegisterGameplayTagEvent(DebuffTag, EGameplayTagEventType::NewOrRemoved).AddUObject(this, &UDebuffNiagaraComponent::OnDebuffTagChangedCallback); 
	}else if (CombatInterface)
	{
		
		CombatInterface->GetOnAscRegisteredDelegate().AddWeakLambda(this, [this](UAbilitySystemComponent* ASC)
		{
			ASC->RegisterGameplayTagEvent(DebuffTag, EGameplayTagEventType::NewOrRemoved).AddUObject(this, &UDebuffNiagaraComponent::OnDebuffTagChangedCallback); 
		}); 
	}
	
	/*Bind delegate to get informed upon owner's death*/
	if (CombatInterface)
	{
		CombatInterface->GetOnCharacterDieDelegate().AddDynamic(this, &UDebuffNiagaraComponent::OnOwnerDeath);
	}
}

void UDebuffNiagaraComponent::OnDebuffTagChangedCallback(const FGameplayTag CallbackTag, int32 NewCount)
{
	const AActor* Owner = GetOwner();
	if (IsValid(GetOwner()) 
		&& GetOwner()->Implements<UCombatInterface>() 
		&& !ICombatInterface::Execute_IsDead(GetOwner()) 
		&& NewCount > 0)
	{
		Activate(); 
	}
	else
	{
		Deactivate();
	}
}

void UDebuffNiagaraComponent::OnOwnerDeath(AActor* DeadActor)
{
	Deactivate(); 
}

