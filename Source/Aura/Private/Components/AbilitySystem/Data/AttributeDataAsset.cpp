// @Copyright HaolunYuan

#include "Components/AbilitySystem/Data/AttributeDataAsset.h"
#include "AuraGameTagManager.h"
#include "GameplayTagsManager.h"

const FAuraAttributeTagMetadatas* UAttributeDataAsset::GetAttributeDataEntryByTag(const FGameplayTag& InTag) const
{
	// Exact-match search keeps lookups deterministic and avoids accidental parent-tag matches.
	return AttributeTagsDataEntries.FindByPredicate([&InTag](const FAuraAttributeTagMetadatas& TagMetaData)
		{
			return InTag.MatchesTagExact(TagMetaData.AttributeTag);
		});
}

const TArray<FAuraAttributeTagMetadatas>& UAttributeDataAsset::GetAllAttributeDataEntries() const
{
	return AttributeTagsDataEntries;
}

TArray<FAuraAttributeTagMetadatas>& UAttributeDataAsset::GetAllAttributeDataEntries()
{
	return AttributeTagsDataEntries;
}

#if WITH_EDITOR
void UAttributeDataAsset::SyncFromNativeTags()
{
	// Required so changes are tracked by transaction/undo system in editor.
	Modify();

	checkf(FAuraGameTagManager::IsNativeTagInfosValid(), TEXT("NativeTagInfos Are not valid yet"));
	const auto& AttributesTagInfos = FAuraGameTagManager::GetNativeGameplayTagInfos();
	const FGameplayTag AttributesRootTag = UGameplayTagsManager::Get().RequestGameplayTag(FName("Attributes"));

	for (const auto& AttributeTagInfo : AttributesTagInfos)
	{
		// Only include tags under the "Attributes" branch.
		if (!AttributeTagInfo.NativeTag.MatchesTag(AttributesRootTag))
		{
			continue;
		}

		// Defensive normalization for malformed entries.
		const FGameplayTag NativeTag = AttributeTagInfo.NativeTag.IsValid() ? AttributeTagInfo.NativeTag : FGameplayTag();
		if (!NativeTag.IsValid())
		{
			continue;
		}

		// Skip if already present to avoid duplicated rows.
		if (FindExistingEntryIndexIfExists(NativeTag) != -1)
		{
			continue;
		}

		// Derive friendly display name from last token in tag path.
		const FString NativeTagName = AttributeTagInfo.TagName.ToString();
		FString DisplayName = NativeTagName;
		if (!NativeTagName.Split(TEXT("."), nullptr, &DisplayName, ESearchCase::IgnoreCase, ESearchDir::FromEnd))
		{
			DisplayName = NativeTagName;
		}

		const FString NativeDescription = AttributeTagInfo.Description;
		FAuraAttributeTagMetadatas& NewEntry = AttributeTagsDataEntries.AddZeroed_GetRef();
		NewEntry.AttributeTag = NativeTag;
		NewEntry.TagDisplayDescription = FText::FromString(NativeDescription);
		NewEntry.TagDisplayName = FText::FromString(DisplayName);
		// NOTE: AttributeRelated is intentionally left unset here and should be assigned in asset authoring.
	}

	// Marks package dirty so editor knows asset has unsaved changes.
	MarkPackageDirty();
}
#endif

int32 UAttributeDataAsset::FindExistingEntryIndexIfExists(const FGameplayTag& InTag) const
{
	// -1 means not found.
	if (!InTag.IsValid())
	{
		return -1;
	}

	for (int32 Index = 0; Index < AttributeTagsDataEntries.Num(); Index++)
	{
		if (AttributeTagsDataEntries[Index].AttributeTag.MatchesTagExact(InTag))
		{
			return Index;
		}
	}

	return -1;
}

