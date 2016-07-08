// Copyright 2016 Vladimir Alyamkin. All Rights Reserved.

#include "VtaEditorPlugin.h"

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
	return EAssetTypeCategories::MaterialsAndTextures;
}

bool FVtaTextureAtlasAssetTypeActions::IsImportedAsset() const
{
	return false;// true;
}

void FVtaTextureAtlasAssetTypeActions::GetResolvedSourceFilePaths(const TArray<UObject*>& TypeAssets, TArray<FString>& OutSourceFilePaths) const
{
	FAssetTypeActions_Base::GetResolvedSourceFilePaths(TypeAssets, OutSourceFilePaths);

	for (auto& Asset : TypeAssets)
	{
		// @TODO 
	}
}

void FVtaTextureAtlasAssetTypeActions::GetActions(const TArray<UObject*>& InObjects, FMenuBuilder& MenuBuilder)
{
	
}

//////////////////////////////////////////////////////////////////////////

#undef LOCTEXT_NAMESPACE
