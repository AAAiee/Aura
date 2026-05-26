// @Copyright HaolunYuan

#include "UI/WidgetController/AttributeMenuWidgetController.h"

#include "AbilitySystemComponent.h"
#include "AuraGameTagManager.h"
#include "AuraLogCategory.h"
#include "Components/AbilitySystem/AuraAbilitySystemComponent.h"
#include "Components/AbilitySystem/AuraAttributeSet.h"
#include "Player/AuraPlayerState.h"


void UAttributeMenuWidgetController::BroadcastInitialValues()
{
	check(AttributeTagsDataAsset);

	if (!CachedAttributeSet)
	{
		return;
	}

	const TArray<FAuraAttributeTagMetadatas>& AttributeTagsDataEntries = AttributeTagsDataAsset->GetAllAttributeDataEntries();
	check(!AttributeTagsDataEntries.IsEmpty());

	// For each configured row, read runtime value and broadcast to UI.
	for (const FAuraAttributeTagMetadatas& AttributeDataEntry : AttributeTagsDataEntries)
	{
		BroadcastAttributeDataEntry(AttributeDataEntry);
	}
}

void UAttributeMenuWidgetController::BindAllDependencies()
{
	AAuraPlayerState* AuraPlayerState = GetAuraPlayerState();

	const TArray<FAuraAttributeTagMetadatas>& AttributeTagsDataEntries = AttributeTagsDataAsset->GetAllAttributeDataEntries();
	for (const FAuraAttributeTagMetadatas& AttributeDataEntry : AttributeTagsDataEntries)
	{
		if (!AttributeDataEntry.AttributeRelated.IsValid())
		{
			UE_LOG(LogAura, Warning, TEXT("AttributeMenuWidgetController: Invalid AttributeRelated for tag '%s'."), *AttributeDataEntry.AttributeTag.ToString());
			continue;
		}

		CachedAbilitySystemComponent->GetGameplayAttributeValueChangeDelegate(AttributeDataEntry.AttributeRelated).AddLambda(
			[this, AttributeTagsInfo = AttributeDataEntry](const FOnAttributeChangeData& ChangedData)
			{
				// Reusing helper keeps formatting/logic identical across all update pathways.
				this->BroadcastAttributeDataEntry(AttributeTagsInfo);
			}
		);
	}

	AuraPlayerState->OnAttributePointsChanged.AddLambda(
		[this](int32 NewAttributePoints)
		{
			if (bAssignmentSessionInProgress)
			{
				SessionAttributePointsAvailable = NewAttributePoints;
				OnSessionAttributePointsAvailableChanged.Broadcast(SessionAttributePointsAvailable);
			}
		}
	);
}

void UAttributeMenuWidgetController::UpgradeAttribute(const FGameplayTag& AttributeTag)
{
	check(bAssignmentSessionInProgress);
	check(SessionAttributePointsAvailable > 0);
	UE_LOG(LogAura, Log, TEXT("Pressed the + Button"));

	// Attribute assignment is previewed locally first. We spend from the session pool, update the
	// pending delta for this attribute, then broadcast the preview value without touching GAS yet.
	SessionAttributePointsAvailable -= 1;
	OnSessionAttributePointsAvailableChanged.Broadcast(SessionAttributePointsAvailable);
	UE_LOG(LogAura, Log, TEXT("Attribute points available after decrement: %d"), SessionAttributePointsAvailable);

	int32* const CurrentDelta = SessionPrimaryAttributeDeltas.Find(AttributeTag);
	check(CurrentDelta);
	(*CurrentDelta)++;
	const int32 CurrentDeltaValue = *CurrentDelta;

	OnSessionAttributeDeltaChanged.Broadcast(AttributeTag, CurrentDeltaValue);

	const FAuraAttributeTagMetadatas* Info = AttributeTagsDataAsset->GetAttributeDataEntryByTag(AttributeTag);
	check(Info);

	const float* BaseValue = SessionBasePrimaryAttributeValues.Find(AttributeTag);
	check(BaseValue);
	const float AttributeValueAfterUpdate = static_cast<float>(CurrentDeltaValue) + (*BaseValue);
	OnAttributeMenuChange.Broadcast(*Info, AttributeValueAfterUpdate);

	bWaitingForConfirmation = true;
	UE_LOG(LogAura, Log, TEXT("%s upgraded to %f"), *AttributeTag.ToString(), AttributeValueAfterUpdate);
}

void UAttributeMenuWidgetController::DeductAttribute(const FGameplayTag& AttributeTag)
{
	check(bAssignmentSessionInProgress);
	UE_LOG(LogAura, Log, TEXT("Pressed the - Button"));

	int32* const CurrentDelta = SessionPrimaryAttributeDeltas.Find(AttributeTag);
	check(CurrentDelta);
	if (*CurrentDelta <= 0)
	{
		return;
	}

	// Refunding is the inverse of UpgradeAttribute: give the point back to the local session pool,
	// lower the preview delta, and broadcast the rolled-back preview value.
	SessionAttributePointsAvailable += 1;
	OnSessionAttributePointsAvailableChanged.Broadcast(SessionAttributePointsAvailable);

	(*CurrentDelta)--;
	OnSessionAttributeDeltaChanged.Broadcast(AttributeTag, *CurrentDelta);
	check(*CurrentDelta >= 0);

	const FAuraAttributeTagMetadatas* Info = AttributeTagsDataAsset->GetAttributeDataEntryByTag(AttributeTag);
	check(Info);

	const float* BaseValue = SessionBasePrimaryAttributeValues.Find(AttributeTag);
	check(BaseValue);
	const float Value = static_cast<float>(*CurrentDelta) + (*BaseValue);
	OnAttributeMenuChange.Broadcast(*Info, Value);
}

void UAttributeMenuWidgetController::BeginAssignmentSession()
{
	check(FAuraGameTagManager::Get().PrimaryAttributeTags.Num() > 0);
	check(AttributeTagsDataAsset);

	// Capture committed AttributeSet values before previewing local changes. If the player closes
	// the menu without confirming, EndAssignmentSession uses these baselines to restore the UI.
	SessionBasePrimaryAttributeValues.Empty();
	SessionPrimaryAttributeDeltas.Empty();
	bAssignmentSessionInProgress = true;
	bWaitingForConfirmation = false;

	for (const FGameplayTag& PrimaryAttributeTag : FAuraGameTagManager::Get().PrimaryAttributeTags)
	{
		const FAuraAttributeTagMetadatas* DataEntry = AttributeTagsDataAsset->GetAttributeDataEntryByTag(PrimaryAttributeTag);
		if (DataEntry && DataEntry->AttributeRelated.IsValid())
		{
			const float AttributeValue = DataEntry->AttributeRelated.GetNumericValue(CachedAttributeSet);

			SessionBasePrimaryAttributeValues.Add(PrimaryAttributeTag, AttributeValue);
			SessionPrimaryAttributeDeltas.Add(PrimaryAttributeTag, 0);

			OnSessionAttributeDeltaChanged.Broadcast(PrimaryAttributeTag, 0);
			UE_LOG(LogAura, Log, TEXT("Attribute %s: %f"), *PrimaryAttributeTag.ToString(), AttributeValue);
		}
	}

	AAuraPlayerState* AuraPlayerState = CastChecked<AAuraPlayerState>(CachedPlayerState);
	SessionAttributePointsAvailable = AuraPlayerState->GetAttributePoints();
	OnSessionAttributePointsAvailableChanged.Broadcast(SessionAttributePointsAvailable);
}

void UAttributeMenuWidgetController::EndAssignmentSession()
{
	if (bWaitingForConfirmation)
	{
		for (const auto& Pair : SessionBasePrimaryAttributeValues)
		{
			const FAuraAttributeTagMetadatas* Info = AttributeTagsDataAsset->GetAttributeDataEntryByTag(Pair.Key);
			const float ResetValue = Pair.Value;
			OnAttributeMenuChange.Broadcast(*Info, ResetValue);
		}

		AAuraPlayerState* AuraPlayerState = CastChecked<AAuraPlayerState>(CachedPlayerState);
		OnSessionAttributePointsAvailableChanged.Broadcast(AuraPlayerState->GetAttributePoints());
	}

	SessionPrimaryAttributeDeltas.Empty();
	SessionBasePrimaryAttributeValues.Empty();
	bWaitingForConfirmation = false;
	bAssignmentSessionInProgress = false;
}

void UAttributeMenuWidgetController::ConfirmAttributeAssignments()
{
	UAuraAbilitySystemComponent* AuraASC = GetAuraAbilitySystemComponent();

	if (!bWaitingForConfirmation)
	{
		return;
	}

	for (auto& Pair : SessionPrimaryAttributeDeltas)
	{
		// Only the confirmed delta crosses into gameplay state. The ASC turns each delta into a
		// server-side gameplay event so passive effects can react through normal GAS pathways.
		AuraASC->UpgradeAttribute(Pair.Key, Pair.Value);
		Pair.Value = 0;
		OnSessionAttributeDeltaChanged.Broadcast(Pair.Key, 0);
	}


	bWaitingForConfirmation = false;
}

void UAttributeMenuWidgetController::BroadcastAttributeDataEntry(const FAuraAttributeTagMetadatas& AttributeTagInfo)
{
	if (!GetAuraAttributeSet())
	{
		return;
	}

	const FGameplayAttribute& AttributeRelated = AttributeTagInfo.AttributeRelated;
	if (!AttributeRelated.IsValid())
	{
		return;
	}

	const float Value = AttributeRelated.GetNumericValue(GetAuraAttributeSet());
	OnAttributeMenuChange.Broadcast(AttributeTagInfo, Value);
}

