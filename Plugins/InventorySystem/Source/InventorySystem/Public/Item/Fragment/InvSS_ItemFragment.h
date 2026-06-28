#pragma once

#include "CoreMinimal.h"
#include "NativeGameplayTags.h"
#include "InvSS_ItemFragment.generated.h"

/**
 * Base manifest fragment used to tag item-specific data blocks.
 */
USTRUCT(BlueprintType)
struct FInvSS_ItemFragment
{
	GENERATED_BODY()

public:
	FInvSS_ItemFragment() = default;
	FInvSS_ItemFragment& operator=(FInvSS_ItemFragment&&) = default;
	FInvSS_ItemFragment(FInvSS_ItemFragment&&) = default;
	FInvSS_ItemFragment& operator=(const FInvSS_ItemFragment&) = default;
	FInvSS_ItemFragment(const FInvSS_ItemFragment&) = default;
	virtual ~FInvSS_ItemFragment() = default;

	FGameplayTag GetFragmentTag() const { return FragmentTag; }

private:
	UPROPERTY(EditAnywhere, Category = "Item Properties")
	FGameplayTag FragmentTag = FGameplayTag::EmptyTag;
};

/**
 * Stores spatial grid size and padding for inventory rendering and placement.
 */
USTRUCT(BlueprintType)
struct FInvSS_GridFragment : public FInvSS_ItemFragment
{
	GENERATED_BODY()

public:
	FIntPoint GetGridSize() const { return GridSize; }
	float GetGridPadding() const { return GridPadding; }

private:
	UPROPERTY(EditAnywhere, Category = "Item Properties")
	FIntPoint GridSize{1, 1};

	UPROPERTY(EditAnywhere, Category = "Item Properties")
	float GridPadding{0.f};
};

/**
 * Stores the icon texture used by slotted inventory item widgets.
 */
USTRUCT(BlueprintType)
struct FInvSS_ImageFragment : public FInvSS_ItemFragment
{
	GENERATED_BODY()

public:
	FORCEINLINE const UTexture2D* GetImage() const { return Image; }
	FORCEINLINE UTexture2D* GetImageResource() const { return Image.Get(); }
	TObjectPtr<UTexture2D> GetMutableImage() { return Image; }

private:
	UPROPERTY(EditAnywhere, Category = "Item Properties")
	TObjectPtr<UTexture2D> Image{nullptr};

	FVector2D IconDimensions{44.4f, 44.4f};
};

/**
 * Stores stack size rules and the pickup stack count.
 */
USTRUCT(BlueprintType)
struct FInvSS_StackableFragment : public FInvSS_ItemFragment
{
	GENERATED_BODY()

public:
	FORCEINLINE int32 GetMaxStackSize() const { return MaxStackSize; }
	FORCEINLINE int32 GetStackCount() const { return StackCount; }
	FORCEINLINE void SetStackCount(const int32 InCount) { StackCount = InCount; }
	FORCEINLINE void SetMaxStackSize(const int32 InCount) { MaxStackSize = InCount; }

private:
	UPROPERTY(EditAnywhere, Category = "Item Properties")
	int32 MaxStackSize{1};

	UPROPERTY(EditAnywhere, Category = "Item Properties")
	int32 StackCount{1};
};
