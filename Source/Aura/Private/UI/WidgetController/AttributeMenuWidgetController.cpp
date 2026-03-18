// @Copyright HaolunYuan

#include "UI/WidgetController/AttributeMenuWidgetController.h"
#include "AbilitySystemComponent.h"
#include "Components/AbilitySystem/AuraAttributeSet.h"

void UAttributeMenuWidgetController::BroadcastInitialValues()
{
	// Guard 1: DataAsset must exist (typically assigned in controller BP defaults).
	if (!AttributeTagsDataAsset)
	{
		UE_LOG(LogTemp, Error, TEXT("AttributeMenuWidgetController: AttributeTagsDataAsset is null."));
		return;
	}

	// Guard 2: Runtime GAS references must be initialized before reading values.
	if (!CachedAttributeSet)
	{
		UE_LOG(LogTemp, Error, TEXT("AttributeMenuWidgetController: CachedAttributeSet is null."));
		return;
	}

	const TArray<FAuraAttributeTagMetadatas>& AttributeTagsDataEntries = AttributeTagsDataAsset->GetAllAttributeDataEntries();
	if (AttributeTagsDataEntries.IsEmpty())
	{
		// Not fatal, but usually means DataAsset setup is incomplete.
		UE_LOG(LogTemp, Warning, TEXT("AttributeMenuWidgetController: AttributeTagsDataEntries is empty."));
		return;
	}

	// For each configured row, read runtime value and broadcast to UI.
	for (const FAuraAttributeTagMetadatas& AttributeDataEntry : AttributeTagsDataEntries)
	{
		BroadcastAttributeDataEntry(AttributeDataEntry);
	}
}

void UAttributeMenuWidgetController::BindAllDependencies()
{
	// These references must be valid before binding delegates.
	if (!AttributeTagsDataAsset || !CachedAbilitySystemComponent)
	{
		UE_LOG(LogTemp, Warning, TEXT("AttributeMenuWidgetController: Cannot bind dependencies (missing DataAsset or ASC)."));
		return;
	}

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

