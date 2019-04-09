// Copyright 2016-2019 Vladimir Alyamkin. All Rights Reserved.

#include "VtaTextureAtlasAssetActions.h"

#include "VtaTextureAtlasAsset.h"

#include "EditorFramework/AssetImportData.h"

#define LOCTEXT_NAMESPACE "VtaEditorPlugin"

//////////////////////////////////////////////////////////////////////////
// FVtaTextureAtlasAssetActions

FText FVtaTextureAtlasAssetActions::GetName() const
{
	return LOCTEXT("FVtaTextureAtlasAssetActions", "Texture Atlas Asset");
}

FColor FVtaTextureAtlasAssetActions::GetTypeColor() const
{
	return FColor::Red;
}

UClass* FVtaTextureAtlasAssetActions::GetSupportedClass() const
{
	return UVtaTextureAtlasAsset::StaticClass();
}

uint32 FVtaTextureAtlasAssetActions::GetCategories()
{
	return EAssetTypeCategories::Misc;
}

bool FVtaTextureAtlasAssetActions::IsImportedAsset() const
{
	return true;
}

void FVtaTextureAtlasAssetActions::GetResolvedSourceFilePaths(const TArray<UObject*>& TypeAssets, TArray<FString>& OutSourceFilePaths) const
{
	FAssetTypeActions_Base::GetResolvedSourceFilePaths(TypeAssets, OutSourceFilePaths);

	for (auto& Asset : TypeAssets)
	{
		const auto TexttureAtlas = CastChecked<UVtaTextureAtlasAsset>(Asset);
		if (TexttureAtlas->AssetImportData)
		{
			TexttureAtlas->AssetImportData->ExtractFilenames(OutSourceFilePaths);
		}
	}
}

void FVtaTextureAtlasAssetActions::GetActions(const TArray<UObject*>& InObjects, FMenuBuilder& MenuBuilder)
{
}

//////////////////////////////////////////////////////////////////////////

#undef LOCTEXT_NAMESPACE
