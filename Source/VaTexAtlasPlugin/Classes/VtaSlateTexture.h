// Copyright 2015-2019 Mail.Ru Group. All Rights Reserved.

#pragma once

#include "Slate/SlateTextureAtlasInterface.h"

#include "VtaSlateTexture.generated.h"

UCLASS(BlueprintType, Blueprintable, meta = (DisplayThumbnail = "true"))
class VATEXATLASPLUGIN_API UVtaSlateTexture : public UObject, public ISlateTextureAtlasInterface
{
	GENERATED_UCLASS_BODY()

public:
	/** The texture pointer for the Atlas */
	UPROPERTY(BlueprintReadOnly, VisibleAnywhere, Category = "VaTexAtlas")
	UTexture* AtlasTexture;

	/** The region start position in UVs */
	UPROPERTY(BlueprintReadOnly, VisibleAnywhere, Category = "VaTexAtlas")
	FVector2D StartUV;

	/** The region size in UVs */
	UPROPERTY(BlueprintReadOnly, VisibleAnywhere, Category = "VaTexAtlas")
	FVector2D SizeUV;

	UFUNCTION(BlueprintCallable, Category = "VaTexAtlas")
	FVector2D GetDimensions() const;

	/** Returns new slate texture with part of given slate texture*/
	UFUNCTION(BlueprintCallable, Category = "VaTexAtlas")
	UVtaSlateTexture* CreateRegion(const FMargin& Margin);

public:
	/** Return FSlateAtlasData for ISlateTextureAtlasInterface */
	virtual FSlateAtlasData GetSlateAtlasData() const override;
};
