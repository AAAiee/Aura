// @Copyright HaolunYuan

#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "Type/InvSS_GridTypes.h"
#include "Widgets/Utilis/InvSS_WidgetUtils.h"
#include "InvSS_InventoryStatics.generated.h"

class UInvSS_InventoryComponent;
class UInvSS_ItemComponent;

/**
 * UInvSS_InventoryStatics
 *
 * Shared inventory helper functions for controller lookup, item categorization, and 2D grid traversal.
 */
UCLASS()
class INVENTORYSYSTEM_API UInvSS_InventoryStatics : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:
	/**
	 * @brief Resolves the inventory component from a player controller that implements the inventory interface.
	 */
	UFUNCTION(BlueprintCallable)
	static UInvSS_InventoryComponent* GetInventoryComponent(APlayerController* InPlayerController);

	/**
	 * @brief Reads an item component's manifest category.
	 */
	UFUNCTION(BlueprintCallable)
	static EInvSS_ItemCategory GetItemCategoryFromItemComp(const UInvSS_ItemComponent* InItemComp);

	/**
	 * @brief Iterates valid array entries covered by a 2D footprint.
	 */
	template<typename T, typename Func>
	requires (std::is_invocable_v<Func&, T&> || std::is_invocable_v<Func&, T&, int32>)
	static void ForEach2D(TArray<T>& Array, int32 StartIndex, int32 TotalColumnsNum, FIntPoint Range2D, Func&& Action);

	/**
	 * @brief Iterates valid const array entries covered by a 2D footprint.
	 */
	template<typename T, typename Func>
	requires (std::is_invocable_v<Func&, const T&> || std::is_invocable_v<Func&, const T&, int32>)
	static void ForEach2D(const TArray<T>& Array, int32 StartIndex, int32 TotalColumnsNum, FIntPoint Range2D, Func&& Action);
	
	
	static bool IsLeftMouseButtonPressed(const FPointerEvent& MouseEvent);
	static bool IsRightMouseButtonPressed(const FPointerEvent& MouseEvent);
};

template <typename T, typename Func>
requires (std::is_invocable_v<Func&, T&> || std::is_invocable_v<Func&, T&, int32>)
void UInvSS_InventoryStatics::ForEach2D(TArray<T>& Array, const int32 StartIndex, const int32 TotalColumnsNum, const FIntPoint Range2D,
	Func&& Action)
{
	for (int32 Y = 0; Y < Range2D.Y; ++Y)
	{
		for (int32 X = 0; X < Range2D.X; ++X)
		{
			const int32 CurrentIndex = StartIndex + UInvSS_WidgetUtils::GetIndexFromPosition({ X, Y }, TotalColumnsNum);
			if (Array.IsValidIndex(CurrentIndex))
			{
				if constexpr (std::is_invocable_v<Func&, T&, int32>)
				{
					Action(Array[CurrentIndex], CurrentIndex);
				}
				else
				{
					Action(Array[CurrentIndex]);
				}
			}
		}
	}
}

template <typename T, typename Func>
requires (std::is_invocable_v<Func&, const T&> || std::is_invocable_v<Func&, const T&, int32>)
void UInvSS_InventoryStatics::ForEach2D(const TArray<T>& Array, const int32 StartIndex, const int32 TotalColumnsNum, const FIntPoint Range2D,
	Func&& Action)
{
	for (int32 Y = 0; Y < Range2D.Y; ++Y)
	{
		for (int32 X = 0; X < Range2D.X; ++X)
		{
			const int32 CurrentIndex = StartIndex + UInvSS_WidgetUtils::GetIndexFromPosition({ X, Y }, TotalColumnsNum);
			if (Array.IsValidIndex(CurrentIndex))
			{
				if constexpr (std::is_invocable_v<Func&, const T&, int32>)
				{
					Action(Array[CurrentIndex], CurrentIndex);
				}
				else
				{
					Action(Array[CurrentIndex]);
				}
			}
		}
	}
}

