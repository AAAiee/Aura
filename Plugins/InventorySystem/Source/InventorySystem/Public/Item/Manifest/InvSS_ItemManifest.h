#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "StructUtils/InstancedStruct.h"
#include "Type/InvSS_GridTypes.h"
#include "InvSS_ItemManifest.generated.h"

class AActor;
struct FInvSS_ItemFragment;
class UInvSS_InventoryItem;

/**
 * FInvSS_ItemManifest
 *
 * Describes item identity, category, display data, and fragment-driven behavior.
 *
 * Pickup item components store this manifest. Inventory entries use Manifest() to create
 * runtime UInvSS_InventoryItem objects with the same fragment data.
 */
USTRUCT(BlueprintType)
struct INVENTORYSYSTEM_API FInvSS_ItemManifest
{
	GENERATED_BODY()

	/**
	 * @brief Creates a runtime inventory item object from this manifest.
	 */
	UInvSS_InventoryItem* Manifest(UObject* Outer) const;
	EInvSS_ItemCategory GetItemCategory() const { return ItemCategory; }
	FText GetItemDisplayName() const { return ItemDisplayName; }
	FGameplayTag GetItemTypeTag() const { return ItemTypeTag; }
	TSubclassOf<AActor> GetWorldItemActorClass() const { return WorldItemActorClass; }

	/**
	 * @brief Returns a typed fragment with the exact requested fragment tag.
	 */
	template<typename T>
	requires std::derived_from<T, FInvSS_ItemFragment>
	const T* GetFragmentOfTypeWithTag(const FGameplayTag& InFragmentTag) const;

	/**
	 * @brief Returns a mutable typed fragment with the exact requested fragment tag.
	 */
	template<typename T>
	requires std::derived_from<T, FInvSS_ItemFragment>
	T* GetMutableFragmentOfTypeWithTag(const FGameplayTag& InFragmentTag);

private:
	UPROPERTY(EditAnywhere, Category = "Inventory", meta = (ExcludeBaseStruct))
	TArray<TInstancedStruct<FInvSS_ItemFragment>> FragmentsArray;

	UPROPERTY(EditAnywhere, Category = "Item Info")
	EInvSS_ItemCategory ItemCategory{EInvSS_ItemCategory::None};

	UPROPERTY(EditAnywhere, Category = "Item Info", NotReplicated)
	FText ItemDisplayName = FText::GetEmpty();

	UPROPERTY(EditAnywhere, Category = "Item Info", meta = (Categories = "GameItems"))
	FGameplayTag ItemTypeTag;

	UPROPERTY(EditAnywhere, Category = "World")
	TSubclassOf<AActor> WorldItemActorClass = nullptr;
};

template <typename T>
requires std::derived_from<T, FInvSS_ItemFragment>
const T* FInvSS_ItemManifest::GetFragmentOfTypeWithTag(const FGameplayTag& InFragmentTag) const
{
	for (const TInstancedStruct<FInvSS_ItemFragment>& Fragment : FragmentsArray)
	{
		if (const T* FragmentOfTypeT = Fragment.GetPtr<T>())
		{
			if (!FragmentOfTypeT->GetFragmentTag().MatchesTagExact(InFragmentTag)) continue;

			return FragmentOfTypeT;
		}
	}
	return nullptr;
}

template <typename T>
requires std::derived_from<T, FInvSS_ItemFragment>
T* FInvSS_ItemManifest::GetMutableFragmentOfTypeWithTag(const FGameplayTag& InFragmentTag)
{
	for (TInstancedStruct<FInvSS_ItemFragment>& Fragment : FragmentsArray)
	{
		if (T* FragmentOfTypeT = Fragment.GetMutablePtr<T>())
		{
			if (!FragmentOfTypeT->GetFragmentTag().MatchesTagExact(InFragmentTag)) continue;

			return FragmentOfTypeT;
		}
	}
	return nullptr;
}
