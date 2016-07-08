// Copyright 2016 Vladimir Alyamkin. All Rights Reserved.

#pragma once

#include "VtaTextureAtlasFactory.generated.h"

/**
 * 
 */
UCLASS()
class VATEXATLASEDITORPLUGIN_API UVtaTextureAtlasFactory : public UFactory
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

};
