// Copyright 2016 Vladimir Alyamkin. All Rights Reserved.

#pragma once

#include "VtaSlateTexture.h"
#include "VtaAsset.h"
#include "VtaTextureAtlas.h"

#include "VtaTextureAtlasImportFactory.generated.h"

/**
 * Imports a texture atlas (and associated frames and textures) from a JSON file exported from Adobe Flash CS6, Texture Packer, or other compatible tool
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
	
	void ImportOrReimportDataTable(UVtaTextureAtlas* TextureAtlas, const FString& TargetPath, const FString& DesiredName, EObjectFlags Flags);
	void ImportOrReimportMultiAtlasDataTable(const FString& TargetPath, const FString& MultiAtlasName, EObjectFlags Flags);
	
	static FString BuildFrameName(const FString& AtlasName, const FString& FrameName);
	static FString BuildSlateTextureName(const FString& AtlasName, const FString& FrameName);

	//////////////////////////////////////////////////////////////////////////
	// Reimport (used by derived class to provide existing data)

protected:
	void SetReimportData(UVtaTextureAtlas* TextureAtlas);
	void ResetImportData();
	UMaterialInstanceConstant* FindExistingFrame(const FString& Name);
	UVtaSlateTexture* FindExistingSlateTexture(const FString& Name);
	
	UMaterialInstanceConstant* FindMaterialByFrameName(const FString& Name, TArray<TSoftObjectPtr<UMaterialInstanceConstant>> List);
	UVtaSlateTexture* FindSlateTextureByFrameName(const FString& Name, TArray<TSoftObjectPtr<UVtaSlateTexture>> List);
	

protected:
	bool bIsReimporting;

	/** The name of the atlas texture during a previous import */
	FString ExistingAtlasTextureName;

	/** The asset that was created for ExistingAtlasTextureName during a previous import */
	UTexture2D* ExistingAtlasTexture;
	
	/** Table */
	UPROPERTY(Transient)
	UDataTable* ExistingTable;

	/** Map of a frame name (as seen in the importer) -> UMaterialInstanceConstant */
	TMap<FString, UMaterialInstanceConstant*> ExistingFrames;
	
	/** Map of a slate texture name (as seen in the importer) -> UVtaSlateTexture */
	TMap<FString, UVtaSlateTexture*> ExistingSlateTextures;

};
