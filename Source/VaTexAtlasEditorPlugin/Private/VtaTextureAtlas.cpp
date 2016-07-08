// Copyright 2016 Vladimir Alyamkin. All Rights Reserved.

#include "VtaEditorPlugin.h"

UVtaTextureAtlas::UVtaTextureAtlas(const class FObjectInitializer& PCIP)
	: Super(PCIP)
{

}

#if WITH_EDITORONLY_DATA

void UVtaTextureAtlas::PostInitProperties()
{
	if (!HasAnyFlags(RF_ClassDefaultObject))
	{
		AssetImportData = NewObject<UAssetImportData>(this, TEXT("AssetImportData"));
	}

	Super::PostInitProperties();
}

void UVtaTextureAtlas::GetAssetRegistryTags(TArray<FAssetRegistryTag>& OutTags) const
{
	if (AssetImportData)
	{
		OutTags.Add(FAssetRegistryTag(SourceFileTagName(), AssetImportData->GetSourceData().ToJson(), FAssetRegistryTag::TT_Hidden));
	}

	Super::GetAssetRegistryTags(OutTags);
}

void UVtaTextureAtlas::Serialize(FArchive& Ar)
{
	Super::Serialize(Ar);

	if (Ar.IsLoading() && Ar.UE4Ver() < VER_UE4_ASSET_IMPORT_DATA_AS_JSON && !AssetImportData)
	{
		// AssetImportData should always be valid
		AssetImportData = NewObject<UAssetImportData>(this, TEXT("AssetImportData"));
	}
}

#endif // WITH_EDITORONLY_DATA

void UVtaTextureAtlas::EmptyData()
{
	ImportedData = FVtaDataFile();

	FrameNames.Empty();
	Frames.Empty();

	TextureName = FString();

	Texture = nullptr;
}
