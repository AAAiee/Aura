#pragma once

#include "CoreMinimal.h"
#include "NativeGameplayTags.h"
#include "InvSS_ItemFragment.generated.h"

class UInvSS_CompositeBase;
class APlayerController;
class UGameplayEffect;

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

	FGameplayTag GetFragmentTag() const;

private:
	UPROPERTY(EditAnywhere, Category = "Item Properties", meta = (Categories = "ItemFragmentTag"))
	FGameplayTag FragmentTag = FGameplayTag::EmptyTag;
};

/**
 * Base fragment for item data that can assimilate into an item description composite widget.
 */
USTRUCT(BlueprintType)
struct FInvSS_InventoryItemFragment : public FInvSS_ItemFragment
{
	GENERATED_BODY()

public:
	virtual bool Assimilate(UInvSS_CompositeBase* Composite) const;

protected:
	bool MatchesFragmentTag(const UInvSS_CompositeBase* CompositeBase) const;
};


/**
 * Stores spatial grid size and padding for inventory rendering and placement.
 */
USTRUCT(BlueprintType)
struct FInvSS_GridFragment : public FInvSS_ItemFragment
{
	GENERATED_BODY()

public:
	FIntPoint GetGridSize() const;
	float GetGridPadding() const;

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
struct FInvSS_ImageFragment : public FInvSS_InventoryItemFragment
{
	GENERATED_BODY()

public:
	virtual bool Assimilate(UInvSS_CompositeBase* Composite) const override;
	FORCEINLINE const UTexture2D* GetImage() const { return Icon; }
	FORCEINLINE UTexture2D* GetImageResource() const { return Icon.Get(); }
	TObjectPtr<UTexture2D> GetMutableImage();

private:
	UPROPERTY(EditAnywhere, Category = "Item Properties")
	TObjectPtr<UTexture2D> Icon{nullptr};

	UPROPERTY(EditAnywhere,Category = "Item Properties")
	FVector2D IconDimensions{44.4f, 44.4f};
};


/**
 * Stores text content for item description composite widgets.
 */
USTRUCT(BlueprintType)
struct FInvSS_TextFragment : public FInvSS_InventoryItemFragment
{
	GENERATED_BODY()

	void SetText(const FText& InText);
	FText GetText() const;
	virtual bool Assimilate(UInvSS_CompositeBase* Composite) const override;


private:
	UPROPERTY(EditAnywhere, Category = "Item Properties")
	FText FragmentText;
};


/**
 * Stores a label/value pair for numeric item description rows.
 */
USTRUCT(BlueprintType)
struct FInvSS_LabeledNumberFragment : public FInvSS_InventoryItemFragment
{
	GENERATED_BODY()

	virtual bool Assimilate(UInvSS_CompositeBase* Composite) const override;


private:
	UPROPERTY(EditAnywhere, Category = "Item Properties")
	FText Text_Label{};

	UPROPERTY(EditAnywhere, Category = "Item Properties")
	float  Text_Value{};

	UPROPERTY(EditAnywhere, Category = "Item Properties")
	bool bCollapsedLabel{false};

	UPROPERTY(EditAnywhere, Category = "Item Properties")
	bool bCollapsedValue{false};

	UPROPERTY(EditAnywhere, Category = "Item Properties")
	int32 MinFractionalDigits{1};

	UPROPERTY(EditAnywhere, Category = "Item Properties")
	int32 MaxFractionalDigits{1};
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

/**
 * Stores consume behavior for stackable consumable items.
 */
USTRUCT(BlueprintType)
struct FInvSS_ConsumableFragment : public FInvSS_ItemFragment
{
	GENERATED_BODY()

public:
	bool OnConsume(APlayerController* PlayerController) const;

private:
	UPROPERTY(EditAnywhere, Category = "Item Properties")
	TSubclassOf<UGameplayEffect> GameplayEffectClass = nullptr;

	UPROPERTY(EditAnywhere, Category = "Item Properties", meta = (ClampMin = "1.0"))
	float EffectLevel = 1.f;
};
