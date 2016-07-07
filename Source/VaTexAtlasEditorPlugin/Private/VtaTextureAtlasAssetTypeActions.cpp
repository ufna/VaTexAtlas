// Copyright 2016 Vladimir Alyamkin. All Rights Reserved.

#include "VtaEditorPlugin.h"

#define LOCTEXT_NAMESPACE "AssetTypeActions"

FText FVtaTextureAtlasAssetTypeActions::GetName() const
{
	return LOCTEXT("FSpriteSheetAssetTypeActionsName", "Sprite Sheet");
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
	return true;
}

void FVtaTextureAtlasAssetTypeActions::GetResolvedSourceFilePaths(const TArray<UObject*>& TypeAssets, TArray<FString>& OutSourceFilePaths) const
{
	for (auto& Asset : TypeAssets)
	{
		// @TODO 
	}
}

void FVtaTextureAtlasAssetTypeActions::GetActions(const TArray<UObject*>& InObjects, FMenuBuilder& MenuBuilder)
{
	
}

#undef LOCTEXT_NAMESPACE
