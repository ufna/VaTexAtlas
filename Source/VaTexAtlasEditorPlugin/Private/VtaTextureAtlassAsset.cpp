// Copyright 2016-2019 Vladimir Alyamkin. All Rights Reserved.

#include "VtaTextureAtlasAsset.h"

#include "EditorFramework/AssetImportData.h"

UVtaTextureAtlasAsset::UVtaTextureAtlasAsset()
{
}

#if WITH_EDITORONLY_DATA

UTexture2D* UVtaTextureAtlasAsset::GetTexture(const FString& Name)
{
	auto* Result = Textures.Find(Name);
	return Result ? *Result : nullptr;
}

FVtaAsset* UVtaTextureAtlasAsset::GetFrame(const FString& Name)
{
	return Frames.Find(Name);
}

void UVtaTextureAtlasAsset::PostInitProperties()
{
	if (!HasAnyFlags(RF_ClassDefaultObject))
	{
		AssetImportData = NewObject<UAssetImportData>(this, TEXT("AssetImportData"));
	}

	Super::PostInitProperties();
}

void UVtaTextureAtlasAsset::GetAssetRegistryTags(TArray<FAssetRegistryTag>& OutTags) const
{
	if (AssetImportData)
	{
		OutTags.Add(FAssetRegistryTag(SourceFileTagName(), AssetImportData->GetSourceData().ToJson(), FAssetRegistryTag::TT_Hidden));
	}

	Super::GetAssetRegistryTags(OutTags);
}

void UVtaTextureAtlasAsset::Serialize(FArchive& Ar)
{
	Super::Serialize(Ar);

	if (Ar.IsLoading() && Ar.UE4Ver() < VER_UE4_ASSET_IMPORT_DATA_AS_JSON && !AssetImportData)
	{
		// AssetImportData should always be valid
		AssetImportData = NewObject<UAssetImportData>(this, TEXT("AssetImportData"));
	}
}

#endif // WITH_EDITORONLY_DATA