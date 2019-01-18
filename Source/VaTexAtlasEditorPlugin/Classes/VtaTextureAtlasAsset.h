// Copyright 2016-2019 Vladimir Alyamkin. All Rights Reserved.

#pragma once

#include "VtaAsset.h"
#include "VtaTextureAtlasDataModel.h"

#include "VtaTextureAtlasAsset.generated.h"

UCLASS(meta = (DisplayThumbnail = "true"))
class VATEXATLASEDITORPLUGIN_API UVtaTextureAtlasAsset : public UObject
{
	GENERATED_BODY()

public:
	UVtaTextureAtlasAsset();

	UPROPERTY(VisibleAnywhere, Category = "Texture Atlas Asset")
	FString AtlasName;

	UPROPERTY(VisibleAnywhere, Category = "Texture Atlas Asset")
	FString ImportPath;

	UPROPERTY(VisibleAnywhere, Category = "Texture Atlas Asset")
	uint32 bIsMultipack : 1;

	UPROPERTY(VisibleAnywhere, Category = "Texture Atlas Asset")
	TEnumAsByte<TextureCompressionSettings> TextureCompressionSetting;

	UPROPERTY(VisibleAnywhere, Category = "Texture Atlas Asset")
	TMap<FString, FVtaDataFile> ImportedData;

	UPROPERTY(VisibleAnywhere, Category = "Texture Atlas Asset")
	TMap<FString, class UTexture2D*> Textures;

	UPROPERTY(VisibleAnywhere, Category = "Texture Atlas Asset")
	TMap<FString, FVtaAsset> Frames;

	UPROPERTY(VisibleAnywhere, Category = "Texture Atlas Asset")
	class UDataTable* DataTable;

	class UTexture2D* GetTexture(const FString& Name);
	FVtaAsset* GetFrame(const FString& Name);

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
};