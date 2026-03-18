// @Copyright HaolunYuan

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "GameplayTagContainer.h"
#include "AttributeSet.h"
#include "AttributeDataAsset.generated.h"

/**
 * One UI row worth of metadata for an attribute entry.
 *
 * Authoring side (editor):
 *   - AttributeTag / display text fields are designer-facing metadata.
 * Runtime side:
 *   - AttributeRelated is the GAS attribute handle used to read current values.
 */
USTRUCT(BlueprintType)
struct FAuraAttributeTagMetadatas
{
	GENERATED_BODY()

public:
	/** GameplayTag used as stable ID for this row (ex: Attributes.Primary.Strength). */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	FGameplayTag AttributeTag = FGameplayTag();

	/** UI display name shown to players. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	FText TagDisplayName = FText();

	/** UI tooltip / description shown to players. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	FText TagDisplayDescription = FText();

	/** GAS attribute handle used to read/write numeric value from AttributeSet. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	FGameplayAttribute AttributeRelated;
};

/**
 * DataAsset that maps gameplay attribute tags to UI metadata + GAS handles.
 * This is the source of truth for Attribute Menu row generation.
 */
UCLASS(BlueprintType)
class AURA_API UAttributeDataAsset : public UDataAsset
{
	GENERATED_BODY()

public:
	/** Lookup helper by exact gameplay tag. Returns nullptr if not found. */
	const FAuraAttributeTagMetadatas* GetAttributeDataEntryByTag(const FGameplayTag& InTag) const;

	/** Read-only list used at runtime by widget controllers. */
	const TArray<FAuraAttributeTagMetadatas>& GetAllAttributeDataEntries() const { return AttributeTagsDataEntries; }

	/**
	 * Mutable list accessor (editor/pipeline use only).
	 * Bug-prone if used at runtime because external code can mutate ordering/content unexpectedly.
	 */
	TArray<FAuraAttributeTagMetadatas>& GetAllAttributeDataEntries() { return AttributeTagsDataEntries; }

#if WITH_EDITOR
	/**
	 * Editor utility: sync rows from registered native tags under "Attributes".
	 * Existing rows are preserved; missing rows are appended.
	 */
	UFUNCTION(CallInEditor)
	void SyncFromNativeTags();
#endif // WITH_EDITOR

private:
	/** Returns index of existing row with exact tag, or -1 if no row exists. */
	int32 FindExistingEntryIndexIfExists(const FGameplayTag& InTag) const;

	/** Authored rows used by Attribute Menu and related UI systems. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
	TArray<FAuraAttributeTagMetadatas> AttributeTagsDataEntries;
};
