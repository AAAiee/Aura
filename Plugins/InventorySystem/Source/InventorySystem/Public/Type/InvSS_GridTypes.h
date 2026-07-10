#pragma once

#include "CoreMinimal.h"
#include "InvSS_GridTypes.generated.h"

class UInvSS_InventoryItem;

/**
 * Inventory category used to select one spatial grid layout.
 */
UENUM(BlueprintType)
enum class EInvSS_ItemCategory : uint8
{
	Equippable,
	Consumable,
	Craftable,
	None
};

/**
 * Replicated item currently held by the cursor during drag/drop interactions.
 */
USTRUCT(BlueprintType)
struct FInvSS_HeldItemState
{
	GENERATED_BODY()

	UPROPERTY()
	FGuid ItemId;

	UPROPERTY()
	EInvSS_ItemCategory SourceCategory = EInvSS_ItemCategory::None;

	UPROPERTY()
	int32 SourceParentIndex = INDEX_NONE;

	UPROPERTY()
	int32 StackCount = 0;

	bool IsValid() const;

	void Reset();
};

/**
 * One candidate parent slot and fill amount returned by grid availability checks.
 */
USTRUCT()
struct FInvSS_InventorySlotAvailability
{
	GENERATED_BODY()

	FInvSS_InventorySlotAvailability() = default;
	FInvSS_InventorySlotAvailability(const int32 InSlotIndex, const int32 InAmountToFill, const bool bInItemAtIndex)
		: SlotIndex(InSlotIndex), AmountToFill(InAmountToFill), bItemAtIndex(bInItemAtIndex)
	{
	}

	UPROPERTY()
	int32 SlotIndex{INDEX_NONE};

	UPROPERTY()
	int32 AmountToFill{0};

	UPROPERTY()
	bool bItemAtIndex{false};
};

/**
 * Full result of attempting to place or stack an item in a category grid.
 */
USTRUCT()
struct FInvSS_BagAvailabilityResult
{
	GENERATED_BODY()

	FInvSS_BagAvailabilityResult() = default;

	UPROPERTY()
	TObjectPtr<UInvSS_InventoryItem> Item = nullptr;

	UPROPERTY()
	int32 TotalRoomToFill{0};

	UPROPERTY()
	int32 Remainder{0};

	UPROPERTY()
	bool bStackable{false};

	UPROPERTY()
	TArray<FInvSS_InventorySlotAvailability> SlotAvailability;
};

/**
 * Cursor quadrant inside one inventory grid tile.
 */
UENUM(BlueprintType)
enum class EInvSS_TileQuadrant : uint8
{
	EInvSS_TopLeft		UMETA(DisplayName = "Top Left"),
	EInvSS_TopRight		UMETA(DisplayName = "Top Right"),
	EInvSS_BottomLeft	UMETA(DisplayName = "Bottom Left"),
	EInvSS_BottomRight	UMETA(DisplayName = "Bottom Right"),
	EInvSS_None			UMETA(DisplayName = "None")
};

/**
 * Derived tile data for the current cursor position over an inventory grid.
 */
USTRUCT(BlueprintType)
struct FInvSS_TileParameters
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Inventory")
	FIntPoint TileCoordinates{};

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Inventory")
	int32 TileIndex{INDEX_NONE};

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Inventory")
	EInvSS_TileQuadrant TileQuadrant{EInvSS_TileQuadrant::EInvSS_None};

	bool operator==(const FInvSS_TileParameters& Other) const;
};

/**
 * Result of querying whether a hovered item can fit at a candidate grid index.
 */
USTRUCT()
struct FInvSS_SpaceQueryResult
{
	GENERATED_BODY()

	// True if the space queried has no items in it.
	bool bHasSpace{false};

	// Valid if there is a single item we can swap with.
	TWeakObjectPtr<UInvSS_InventoryItem> ValidItem = nullptr;

	// Upper-left index of the valid item, if there is one.
	int32 ItemParentIndex{INDEX_NONE} ;
};
