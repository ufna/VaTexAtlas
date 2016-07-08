// Copyright 2016 Vladimir Alyamkin. All Rights Reserved.

#pragma once

#include "VtaTextureAtlasImportFactory.generated.h"

/**
 * 
 */
UCLASS()
class VATEXATLASEDITORPLUGIN_API UVtaTextureAtlasImportFactory : public UFactory
{
	GENERATED_UCLASS_BODY()

	// Begin UFactory interface
	virtual FText GetToolTip() const override;
	virtual bool FactoryCanImport(const FString& Filename) override;
	virtual UObject* FactoryCreateText(UClass* InClass, UObject* InParent, FName InName, EObjectFlags Flags, UObject* Context, const TCHAR* Type, const TCHAR*& Buffer, const TCHAR* BufferEnd, FFeedbackContext* Warn) override;
	// End UFactory interface

protected:
	TSharedPtr<FJsonObject> ParseJSON(const FString& FileContents, const FString& NameForErrors, bool bSilent = false);

	static UObject* CreateNewAsset(UClass* AssetClass, const FString& TargetPath, const FString& DesiredName, EObjectFlags Flags);
	static UObject* ImportAsset(const FString& SourceFilename, const FString& TargetSubPath);
	static UTexture2D* ImportTexture(const FString& SourceFilename, const FString& TargetSubPath);
	static UTexture2D* ImportOrReimportTexture(UTexture2D* ExistingTexture, const FString& SourceFilename, const FString& TargetSubPath);


	//////////////////////////////////////////////////////////////////////////
	// Reimport (used by derived class to provide existing data)

protected:
	void SetReimportData(const TArray<FString>& ExistingFrameNames, const TArray< TAssetPtr<class UMaterialInstanceConstant> >& ExistingFrameAssetPtrs);
	void ResetImportData();
	UMaterialInstanceConstant* FindExistingFrame(const FString& Name);

protected:
	bool bIsReimporting;

	/** The name of the atlas texture during a previous import */
	FString ExistingAtlasTextureName;

	/** The asset that was created for ExistingAtlasTextureName during a previous import */
	UTexture2D* ExistingAtlasTexture;

	/** Map of a frame name (as seen in the importer) -> UMaterialInstanceConstant */
	TMap<FString, UMaterialInstanceConstant*> ExistingFrames;

};
