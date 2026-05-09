// @Copyright HaolunYuan

#include "UI/WidgetController/AttributeMenuWidgetController.h"
#include "AbilitySystemComponent.h"
#include "Components/AbilitySystem/AuraAttributeSet.h"
#include "Player/AuraPlayerState.h"
#include "Components/AbilitySystem/AuraAbilitySystemComponent.h"

void UAttributeMenuWidgetController::BroadcastInitialValues()
{
	// Guard 1: DataAsset must exist (typically assigned in controller BP defaults).
	check(AttributeTagsDataAsset);

	// Guard 2: Runtime GAS references must be initialized before reading values.
	if (!CachedAttributeSet) return; 

	const TArray<FAuraAttributeTagMetadatas>& AttributeTagsDataEntries = AttributeTagsDataAsset->GetAllAttributeDataEntries();
	check(!AttributeTagsDataEntries.IsEmpty());

	// For each configured row, read runtime value and broadcast to UI.
	for (const FAuraAttributeTagMetadatas& AttributeDataEntry : AttributeTagsDataEntries)
	{
		BroadcastAttributeDataEntry(AttributeDataEntry);
	}

	AAuraPlayerState* AuraPlayerState = CastChecked<AAuraPlayerState>(CachedPlayerState);
	OnAttributePointsChanged.Broadcast(AuraPlayerState->GetAttributePoints()); 
}

void UAttributeMenuWidgetController::BindAllDependencies()
{
	// These references must be valid before binding delegates.
	check(AttributeTagsDataAsset && CachedAbilitySystemComponent && CachedPlayerState);
	AAuraPlayerState* AuraPlayerState = CastChecked<AAuraPlayerState>(CachedPlayerState);


	const TArray<FAuraAttributeTagMetadatas>& AttributeTagsDataEntries = AttributeTagsDataAsset->GetAllAttributeDataEntries();
	for (const FAuraAttributeTagMetadatas& AttributeDataEntry : AttributeTagsDataEntries)
	{
		if (!AttributeDataEntry.AttributeRelated.IsValid())
		{
			UE_LOG(LogTemp, Warning, TEXT("AttributeMenuWidgetController: Invalid AttributeRelated for tag '%s'."), *AttributeDataEntry.AttributeTag.ToString());
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
			this->OnAttributePointsChanged.Broadcast(NewAttributePoints);
		}
	);

}

void UAttributeMenuWidgetController::UpgradeAttribute(const FGameplayTag& AttributeTag)
{
	CastChecked<UAuraAbilitySystemComponent>(CachedAbilitySystemComponent)->UpgradeAttribute(AttributeTag);
}

void UAttributeMenuWidgetController::BroadcastAttributeDataEntry(const FAuraAttributeTagMetadatas& AttributeTagInfo)
{
	if (!CachedAttributeSet)
	{
		return;
	}

	const FGameplayAttribute& AttributeRelated = AttributeTagInfo.AttributeRelated;
	if (!AttributeRelated.IsValid())
	{
		return;
	}

	const float Value = AttributeRelated.GetNumericValue(CachedAttributeSet);
	OnAttributeMenuChange.Broadcast(AttributeTagInfo, Value);
}

