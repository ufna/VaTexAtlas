// Copyright 2016-2019 Vladimir Alyamkin. All Rights Reserved.

#include "VtaEditorPlugin.h"

#include "IVaTexAtlasEditorPlugin.h"
#include "VtaEditorPluginSettings.h"
#include "VtaSlateTexture.h"
#include "VtaSlateTextureThumbnailRenderer.h"
#include "VtaTextureAtlasAssetActions.h"
#include "VtaTextureAtlasAssetTypeActions.h"

#include "ISettingsModule.h"
#include "ThumbnailRendering/ThumbnailManager.h"

#define LOCTEXT_NAMESPACE "VaTexAtlasEditorPlugin"

class FVaTexAtlasEditorPlugin : public IVaTexAtlasEditorPlugin
{
	/** IModuleInterface implementation */
	virtual void StartupModule() override
	{
		// Register asset types
		IAssetTools& AssetTools = FModuleManager::LoadModuleChecked<FAssetToolsModule>("AssetTools").Get();

		TextureAtlasAssetTypeActions = MakeShareable(new FVtaTextureAtlasAssetTypeActions);
		AssetTools.RegisterAssetTypeActions(TextureAtlasAssetTypeActions.ToSharedRef());

		/** New texture atlas asset actions */
		TextureAtlasAssetActions = MakeShareable(new FVtaTextureAtlasAssetActions);
		AssetTools.RegisterAssetTypeActions(TextureAtlasAssetActions.ToSharedRef());

		// Registration thumbnail renderer for slate texture
		UThumbnailManager::Get().RegisterCustomRenderer(UVtaSlateTexture::StaticClass(), UVtaSlateTextureThumbnailRenderer::StaticClass());

		// Registration plugin settings
		if (ISettingsModule* SettingsModule = FModuleManager::GetModulePtr<ISettingsModule>("Settings"))
		{
			SettingsModule->RegisterSettings("Project", "Plugins", "VaTexAtlasEditor",
				LOCTEXT("RuntimeSettingsName", "VaTexAtlasEditor"),
				LOCTEXT("RuntimeSettingsDescription", "Configure atlas generation settings"),
				GetMutableDefault<UVtaEditorPluginSettings>());
		}
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

		if (UObjectInitialized())
		{
			// Unregister thumbnail renderer for slate texture
			UThumbnailManager::Get().UnregisterCustomRenderer(UVtaSlateTexture::StaticClass());
		}

		// Unregister plugin settings
		if (ISettingsModule* SettingsModule = FModuleManager::GetModulePtr<ISettingsModule>("Settings"))
		{
			SettingsModule->UnregisterSettings("Project", "Plugins", "VaTexAtlasEditor");
		}
	}

private:
	/** Asset type action */
	TSharedPtr<IAssetTypeActions> TextureAtlasAssetTypeActions;
	TSharedPtr<IAssetTypeActions> TextureAtlasAssetActions;
};

IMPLEMENT_MODULE(FVaTexAtlasEditorPlugin, VaTexAtlasEditorPlugin)

DEFINE_LOG_CATEGORY(LogVaTexAtlasEditor);

#undef LOCTEXT_NAMESPACE
