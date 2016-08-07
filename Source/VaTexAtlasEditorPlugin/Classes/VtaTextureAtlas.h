// Copyright 2016 Vladimir Alyamkin. All Rights Reserved.

#pragma once

#include "VtaTextureAtlasDataModel.h"
#include "VtaTextureAtlas.generated.h"

UCLASS(BlueprintType, meta = (DisplayThumbnail = "true"))
class VATEXATLASEDITORPLUGIN_API UVtaTextureAtlas : public UObject
{
	GENERATED_UCLASS_BODY()

public:
	// @TODO: For debug only
	UPROPERTY(VisibleAnywhere, Category = "VaTexAtlas")
	FVtaDataFile ImportedData;

	/** The names of frames during import */
	UPROPERTY(VisibleAnywhere, Category = "VaTexAtlas")
	TArray<FString> FrameNames;

	/** */
	UPROPERTY(VisibleAnywhere, Category = "VaTexAtlas")
	TArray< TAssetPtr<class UMaterialInstanceConstant> > Frames;

	/** The name of the atlas texture during import */
	UPROPERTY(VisibleAnywhere, Category = "VaTexAtlas")
	FString TextureName;

	/** The asset that was created for TextureName */
	UPROPERTY(VisibleAnywhere, Category = "VaTexAtlas")
	UTexture2D* Texture;

#if WITH_EDITORONLY_DATA
	// Import data for this 
	UPROPERTY(VisibleAnywhere, Instanced, Category = ImportSettings)
	class UAssetImportData* AssetImportData;

	// UObject interface
	virtual void PostInitProperties() override;
	virtual void GetAssetRegistryTags(TArray<FAssetRegistryTag>& OutTags) const override;
	virtual void Serialize(FArchive& Ar) override;
	// End of UObject interface
#endif

	/** Empty existing data */
	void EmptyData();

};
