// Copyright 2015-2019 Mail.Ru Group. All Rights Reserved.

#include "VtaEditorPluginSettings.h"

UVtaEditorPluginSettings::UVtaEditorPluginSettings(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
	, bGenerateSlateTextures(true)
	, bGenerateMaterialInstances(false)
	, bUseMultiAtlas(false)
	, bUseNewAtlasAsset(true)
{
}
