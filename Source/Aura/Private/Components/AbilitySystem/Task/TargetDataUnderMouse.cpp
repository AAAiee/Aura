// @Copyright HaolunYuan


#include "Components/AbilitySystem/Task/TargetDataUnderMouse.h"
#include "AbilitySystemComponent.h"

UTargetDataUnderMouse* UTargetDataUnderMouse::GetTargetDataUnderMouse(UGameplayAbility* OwningAbility)
{
	UTargetDataUnderMouse* MyObj = NewAbilityTask<UTargetDataUnderMouse>(OwningAbility); 
	return MyObj;

}

void UTargetDataUnderMouse::Activate()
{
	if (!Ability || !AbilitySystemComponent.IsValid())
	{
		EndTask();
		return;
	}

	/*
	 * Target-data replication flow:
	 *   - Local owner path:
	 *       sample the cursor hit immediately, send it to the server, and broadcast the same data
	 *       locally so prediction-driven ability logic can continue without waiting a round-trip.
	 *   - Remote/server path:
	 *       register a delegate keyed by the current spec handle + prediction key, then wait for
	 *       the owning client to submit its cursor hit through ServerSetReplicatedTargetData.
	 */
	const bool bIsLocallyControlled = Ability->GetCurrentActorInfo()->IsLocallyControlled();
	if (bIsLocallyControlled)
	{
		// Local prediction path: gather cursor data immediately and mirror it to the server.
		SendMouseCursorData();
	}
	else
	{
		// Remote path: the server waits for the owning client to send its predicted cursor hit.
		const FGameplayAbilitySpecHandle SpecHandle = GetAbilitySpecHandle() ;
		const FPredictionKey PredictionKey = GetActivationPredictionKey();
		AbilitySystemComponent.Get()->AbilityTargetDataSetDelegate(SpecHandle, PredictionKey).AddUObject(this, &UTargetDataUnderMouse::OnTargetDataReplicatedCallback);

		const bool bCalledReplicated = AbilitySystemComponent.Get()->CallReplicatedTargetDataDelegatesIfSet(SpecHandle, PredictionKey); 

		if (!bCalledReplicated)
		{
			SetWaitingOnRemotePlayerData();
		}
	}

}

void UTargetDataUnderMouse::SendMouseCursorData()
{
	FScopedPredictionWindow ScopedPrediction(AbilitySystemComponent.Get());

	APlayerController* PC = Ability->GetCurrentActorInfo()->PlayerController.Get();
	if (!ensureMsgf(PC, TEXT("TargetDataUnderMouse::SendMouseCursorData requires a valid PlayerController.")))
	{
		EndTask();
		return;
	}

	FHitResult CursorHitResult;
	PC->GetHitResultUnderCursor(ECollisionChannel::ECC_Visibility, false, CursorHitResult);

	FGameplayAbilityTargetData_SingleTargetHit* Data =  new FGameplayAbilityTargetData_SingleTargetHit();
	Data->HitResult = CursorHitResult;
	FGameplayAbilityTargetDataHandle DataHandle;
	DataHandle.Add(Data); 

	// The spec handle + prediction key pair is what lets the server match this cursor hit to the
	// exact ability activation that requested it.
	AbilitySystemComponent->ServerSetReplicatedTargetData(GetAbilitySpecHandle(),GetActivationPredictionKey(), DataHandle,FGameplayTag(), AbilitySystemComponent->ScopedPredictionKey);

	if (ShouldBroadcastAbilityTaskDelegates())
	{
		OnValidateData.Broadcast(DataHandle);
	}

	// This task only produces a single target-data payload per activation, so we can clean it up
	// immediately after local broadcast instead of keeping dormant tasks alive.
	EndTask();
}

void UTargetDataUnderMouse::OnTargetDataReplicatedCallback(const FGameplayAbilityTargetDataHandle& DataHandle, FGameplayTag GameplayTag)
{
	// Once the server-side task receives the payload, unhook the delegate immediately so repeated
	// activations do not accumulate duplicate listeners for the same ability instance.
	AbilitySystemComponent->AbilityTargetDataSetDelegate(GetAbilitySpecHandle(), GetActivationPredictionKey()).RemoveAll(this);
	AbilitySystemComponent->ConsumeClientReplicatedTargetData(GetAbilitySpecHandle(), GetActivationPredictionKey());
	if (ShouldBroadcastAbilityTaskDelegates())
	{
		OnValidateData.Broadcast(DataHandle);
	}

	EndTask();
}
