// Copyright 2015-2019 Mail.Ru Group. All Rights Reserved.

#pragma once

#include "VtaEditorPluginSettings.generated.h"

UCLASS(config = Engine, defaultconfig)
class UVtaEditorPluginSettings : public UObject
{
	GENERATED_UCLASS_BODY()

public:
	/** Generate slate textures */
	UPROPERTY(Config, EditAnywhere, Category="VaTexAtlas Editor Settings")
	bool bGenerateSlateTextures;

	/** Generate material instances */
	UPROPERTY(Config, EditAnywhere, Category="VaTexAtlas Editor Settings")
	bool bGenerateMaterialInstances;

	/** Generate material instances */
	UPROPERTY(Config, EditAnywhere, Category="VaTexAtlas Editor Settings")
	bool bUseMultiAtlas;

	UPROPERTY(Config, EditAnywhere, Category="VaTexAtlas Editor Settings")
	bool bUseNewAtlasAsset;
};
