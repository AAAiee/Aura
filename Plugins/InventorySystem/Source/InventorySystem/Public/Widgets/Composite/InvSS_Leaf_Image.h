// @Copyright HaolunYuan

#pragma once

#include "CoreMinimal.h"
#include "InvSS_Leaf.h"
#include "InvSS_Leaf_Image.generated.h"

class USizeBox;
class UImage;
/**
 * Leaf widget that renders an item description image fragment.
 */
UCLASS()
class INVENTORYSYSTEM_API UInvSS_Leaf_Image : public UInvSS_Leaf
{
	GENERATED_BODY()

public:
	void SetImage(UTexture2D* Texture) const;
	void SetBoxSize(const FVector2D& Size) const;
	void SetImageSize(const FVector2D& Size) const;
	FVector2D GetImageSize() const;

private:
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UImage> Image_Icon;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<USizeBox> SizeBox_Icon;
};
