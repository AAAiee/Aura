// @Copyright HaolunYuan

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Manifest/InvSS_ItemManifest.h"
#include "InvSS_ItemComponent.generated.h"

/**
 * UInvSS_ItemComponent
 *
 * Holds pickup-side item manifest data before the item is added to an inventory.
 *
 * Pickup actors use this component to expose display data, stack data, and fragments to
 * UInvSS_InventoryComponent. The manifest replicates so clients can inspect pickup data,
 * while authority mutates stack remainder when only part of a stack fits in the bag.
 */
UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class INVENTORYSYSTEM_API UInvSS_ItemComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UInvSS_ItemComponent();

	/**
	 * @brief Returns the pickup display name from the item manifest.
	 */
	FORCEINLINE FText GetItemDisplayName() const { return ItemManifest.GetItemDisplayName(); }

	/**
	 * @brief Returns the item manifest stored on this pickup component.
	 */
	FInvSS_ItemManifest GetItemManifest() const { return ItemManifest; }

	/**
	 * @brief Attempts to set the stack count on the manifest's stackable fragment.
	 *
	 * @return True when this item has a stackable fragment and the count was updated.
	 */
	bool TrySetStackCount(int32 InStackCount);

	/**
	 * @brief Runs pickup presentation and destroys the owning actor.
	 */
	void PickUp() const;

	/* UActorComponent begins */
	virtual void GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const override;
	/* UActorComponent ends */

protected:
	/**
	 * @brief Blueprint hook for pickup effects before the actor is destroyed.
	 */
	UFUNCTION(BlueprintImplementableEvent)
	void OnPickUp() const;

private:
	UPROPERTY(EditAnywhere, Category = "Inventory", Replicated)
	FInvSS_ItemManifest ItemManifest;
};
