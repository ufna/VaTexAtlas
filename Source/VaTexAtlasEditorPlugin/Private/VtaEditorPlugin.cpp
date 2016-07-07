// Copyright 2016 Vladimir Alyamkin. All Rights Reserved.

#include "VtaEditorPlugin.h"
#include "VtaTextureAtlasAssetTypeActions.h"

class FVaTexAtlasEditorPlugin : public IVaTexAtlasEditorPlugin
{
	/** IModuleInterface implementation */
	virtual void StartupModule() override
	{
		// Register asset types
		IAssetTools& AssetTools = FModuleManager::LoadModuleChecked<FAssetToolsModule>("AssetTools").Get();

		TextureAtlasAssetTypeActions = MakeShareable(new FVtaTextureAtlasAssetTypeActions);
		AssetTools.RegisterAssetTypeActions(TextureAtlasAssetTypeActions.ToSharedRef());
	}

	virtual void ShutdownModule() override
	{
		if (FModuleManager::Get().IsModuleLoaded("AssetTools"))
		{
			IAssetTools& AssetTools = FModuleManager::GetModuleChecked<FAssetToolsModule>("AssetTools").Get();
			if (TextureAtlasAssetTypeActions.IsValid())
			{
				AssetTools.UnregisterAssetTypeActions(TextureAtlasAssetTypeActions.ToSharedRef());
			}
		}
	}

private:
	TSharedPtr<IAssetTypeActions> TextureAtlasAssetTypeActions;

};

IMPLEMENT_MODULE( FVaTexAtlasEditorPlugin, VaTexAtlasEditorPlugin )

DEFINE_LOG_CATEGORY(LogVaTexAtlasEditor);
