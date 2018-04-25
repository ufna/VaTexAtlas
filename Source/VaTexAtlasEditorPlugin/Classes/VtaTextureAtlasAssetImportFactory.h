// Copyright 2016 Vladimir Alyamkin. All Rights Reserved.

#pragma once
#include "Factories/Factory.h"
#include "EditorReimportHandler.h"
#include "VtaAsset.h"
#include "VtaTextureAtlasAssetImportFactory.generated.h"

UCLASS()
class VATEXATLASEDITORPLUGIN_API UVtaTextureAtlasAssetImportFactory final: public UFactory, public FReimportHandler
{
	GENERATED_BODY()

public:
	UVtaTextureAtlasAssetImportFactory();

	// Begin UFactory interface
	virtual bool FactoryCanImport(const FString& Filename) override;
	virtual FText GetToolTip() const override;
	virtual UObject* FactoryCreateFile(UClass* InClass, UObject* InParent, FName InName, EObjectFlags Flags, const FString& Filename, const TCHAR* Parms, FFeedbackContext* Warn, bool& bOutOperationCanceled) override;
	virtual void Cleanup() { CurrentAtlas = nullptr; }
	// End UFactory interface

	// Begin ReimportHandler interface
	virtual bool CanReimport(UObject* Obj, TArray<FString>& OutFilenames) override;
	virtual void SetReimportPaths(UObject* Obj, const TArray<FString>& NewReimportPaths) override;
	virtual EReimportResult::Type Reimport(UObject* Obj) override;
	virtual const UObject* GetFactoryObject() const override { return this; }
	// End ReimportHandler interface
	
private:
	UPROPERTY(Transient)
	class UVtaTextureAtlasAsset* CurrentAtlas;

	static bool SplitMultipackAtlasName(const FString& Filename, FString& OutAtlasName, int32& OutIndex);

	static TSharedPtr<class FJsonObject> ParseJSON(const FString& FileName);
	static bool ImportAtlas(class UVtaTextureAtlasAsset* Atlas, FFeedbackContext* Warn);
	
	static UObject* CreateAsset(UClass* Class, const FString& TargetPath, const FString& Name, EObjectFlags Flags);
	static UObject* ImportAsset(const FString& SourceFilename, const FString& TargetPath);
	static UObject* FindAsset(UClass* AssetClass, const FString& Path, const FString& Name);

	static class UTexture2D* ProcessTexture(class UVtaTextureAtlasAsset* Atlas, const FString& TextureName);
	static FVtaAsset ProcessFrame(class UVtaTextureAtlasAsset* Atlas, class UTexture2D* AtlasTexture, const struct FVtaMeta& Meta, const struct FVtaFrame& Frame);

	static FString GetNormalizedFrameName(const FString& Name);
};