// Copyright 2016 Vladimir Alyamkin. All Rights Reserved.

#include "VtaTextureAtlasAssetTypeActions.h"
#include "VtaTextureAtlas.h"

#include "EditorFramework/AssetImportData.h"

#define LOCTEXT_NAMESPACE "VtaEditorPlugin"

//////////////////////////////////////////////////////////////////////////
// FVtaTextureAtlasAssetTypeActions

FText FVtaTextureAtlasAssetTypeActions::GetName() const
{
	return LOCTEXT("FVtaTextureAtlasAssetTypeActions", "Texture Atlas");
}

FColor FVtaTextureAtlasAssetTypeActions::GetTypeColor() const
{
	return FColor::Cyan;
}

UClass* FVtaTextureAtlasAssetTypeActions::GetSupportedClass() const
{
	return UVtaTextureAtlas::StaticClass();
}

uint32 FVtaTextureAtlasAssetTypeActions::GetCategories()
{
	return EAssetTypeCategories::Misc;
}

bool FVtaTextureAtlasAssetTypeActions::IsImportedAsset() const
{
	return true;
}

void FVtaTextureAtlasAssetTypeActions::GetResolvedSourceFilePaths(const TArray<UObject*>& TypeAssets, TArray<FString>& OutSourceFilePaths) const
{
	FAssetTypeActions_Base::GetResolvedSourceFilePaths(TypeAssets, OutSourceFilePaths);

	for (auto& Asset : TypeAssets)
	{
		const auto TexttureAtlas = CastChecked<UVtaTextureAtlas>(Asset);
		if (TexttureAtlas->AssetImportData)
		{
			TexttureAtlas->AssetImportData->ExtractFilenames(OutSourceFilePaths);
		}
	}
}

void FVtaTextureAtlasAssetTypeActions::GetActions(const TArray<UObject*>& InObjects, FMenuBuilder& MenuBuilder)
{
	
}

//////////////////////////////////////////////////////////////////////////

#undef LOCTEXT_NAMESPACE
