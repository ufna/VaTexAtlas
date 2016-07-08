// Copyright 2016 Vladimir Alyamkin. All Rights Reserved.

#pragma once

#include "VtaTextureAtlasDataModel.h" // @TODO: For debug only
#include "VtaTextureAtlasImportData.generated.h"

/**
 * This is the 'hub' asset that tracks other imported assets for a texture atlas exported from TexturePacker
 */
UCLASS()
class UVtaTextureAtlasImportData : public UObject
{
	GENERATED_UCLASS_BODY()

public:
	// @TODO: For debug only
	UPROPERTY(VisibleAnywhere, Category = "VaTexAtlas")
	FVtaDataFile ImportedData;

	/** Import data for this */
	UPROPERTY(VisibleAnywhere, Instanced, Category = ImportSettings)
	class UAssetImportData* AssetImportData;

	/** Override to ensure we write out the asset import data */
	virtual void GetAssetRegistryTags(TArray<FAssetRegistryTag>& OutTags) const override;
};
