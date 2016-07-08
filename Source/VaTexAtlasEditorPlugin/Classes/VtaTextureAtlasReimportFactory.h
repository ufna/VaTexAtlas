// Copyright 2016 Vladimir Alyamkin. All Rights Reserved.

#pragma once

#include "VtaTextureAtlasReimportFactory.generated.h"

/**
 * Reimports a UVtaTextureAtlas asset
 */
UCLASS()
class VATEXATLASEDITORPLUGIN_API UVtaTextureAtlasReimportFactory : public UVtaTextureAtlasImportFactory, public FReimportHandler
{
	GENERATED_UCLASS_BODY()

	//~ Begin FReimportHandler Interface
	virtual bool CanReimport(UObject* Obj, TArray<FString>& OutFilenames) override;
	virtual void SetReimportPaths(UObject* Obj, const TArray<FString>& NewReimportPaths) override;
	virtual EReimportResult::Type Reimport(UObject* Obj) override;
	virtual int32 GetPriority() const override;
	//~ End FReimportHandler Interface
};
