// Copyright 2016 Vladimir Alyamkin. All Rights Reserved.

#include "VtaEditorPlugin.h"
#include "VtaTextureAtlasDataModel.h"

#include "PackageTools.h"

#define LOCTEXT_NAMESPACE "VtaEditorPlugin"

//////////////////////////////////////////////////////////////////////////
// UVtaTextureAtlasFactory

UVtaTextureAtlasFactory::UVtaTextureAtlasFactory(const class FObjectInitializer& PCIP)
	: Super(PCIP)
{
	SupportedClass = UVtaTextureAtlas::StaticClass();
	bCreateNew = false;

	bEditorImport = true;
	bText = true;

	Formats.Add(TEXT("vta;VaTexAtlas data file"));
	Formats.Add(TEXT("json;VaTexAtlas JSON file"));
}

FText UVtaTextureAtlasFactory::GetToolTip() const
{
	return LOCTEXT("VtaTextureAtlasFactoryDescription", "Texture atlas imported from TexturePacker");
}

bool UVtaTextureAtlasFactory::FactoryCanImport(const FString& Filename)
{
	FString FileContent;
	if (FFileHelper::LoadFileToString(FileContent, *Filename))
	{
		TSharedPtr<FJsonObject> DescriptorObject = ParseJSON(FileContent, FString(), true);
		if (DescriptorObject.IsValid())
		{
			FVtaDataFile GlobalInfo;
			GlobalInfo.ParseFromJSON(DescriptorObject, Filename, true, true);

			return GlobalInfo.IsValid();
		}
	}

	return false;
}

UObject* UVtaTextureAtlasFactory::FactoryCreateText(UClass* InClass, UObject* InParent, FName InName, EObjectFlags Flags, UObject* Context, const TCHAR* Type, const TCHAR*& Buffer, const TCHAR* BufferEnd, FFeedbackContext* Warn)
{
	Flags |= RF_Transactional;

	FEditorDelegates::OnAssetPreImport.Broadcast(this, InClass, InParent, InName, Type);

	FAssetToolsModule& AssetToolsModule = FModuleManager::GetModuleChecked<FAssetToolsModule>("AssetTools");

	const FString CurrentFilename = UFactory::GetCurrentFilename();
	FString CurrentSourcePath;
	FString FilenameNoExtension;
	FString UnusedExtension;
	FPaths::Split(CurrentFilename, CurrentSourcePath, FilenameNoExtension, UnusedExtension);

	const FString LongPackagePath = FPackageName::GetLongPackagePath(InParent->GetOutermost()->GetPathName());

	const FString NameForErrors(InName.ToString());
	const FString FileContent(BufferEnd - Buffer, Buffer);
	TSharedPtr<FJsonObject> DescriptorObject = ParseJSON(FileContent, NameForErrors);

	UObject* Result = nullptr;

	// Parse the file 
	FVtaDataFile DataModel;
	if (DescriptorObject.IsValid())
	{
		DataModel.ParseFromJSON(DescriptorObject, NameForErrors, false, false);
	}

	// Create the new 'hub' asset and convert the data model over
	if (DataModel.IsValid())
	{
		const bool bSilent = false;

		//Result = NewObject<UVtaTextureAtlasAssetImportData>(InParent, InName, Flags);
		//Result->Modify();

		// @TODO: Do some things here maybe?
		//Result->ImportedData = DataModel;

		// @TODO

		// Result->PostEditChange();
	}

	if (Result != nullptr)
	{
		//@TODO: Need to do this
		// Store the current file path and timestamp for re-import purposes
		// 		UAssetImportData* ImportData = UVtaTextureAtlasAssetImportData::GetImportDataForTileMap(Result);
		// 		ImportData->SourceFilePath = FReimportManager::SanitizeImportFilename(CurrentFilename, Result);
		// 		ImportData->SourceFileTimestamp = IFileManager::Get().GetTimeStamp(*CurrentFilename).ToString();
	}

	FEditorDelegates::OnAssetPostImport.Broadcast(this, Result);

	return Result;
}

TSharedPtr<FJsonObject> UVtaTextureAtlasFactory::ParseJSON(const FString& FileContents, const FString& NameForErrors, bool bSilent)
{
	// Load the file up (JSON format)
	if (!FileContents.IsEmpty())
	{
		const TSharedRef< TJsonReader<> >& Reader = TJsonReaderFactory<>::Create(FileContents);

		TSharedPtr<FJsonObject> DescriptorObject;
		if (FJsonSerializer::Deserialize(Reader, DescriptorObject) && DescriptorObject.IsValid())
		{
			// File was loaded and deserialized OK!
			return DescriptorObject;
		}
		else
		{
			if (!bSilent)
			{
				UE_LOG(LogVaTexAtlasEditor, Error, TEXT("Failed to parse Texture Atlas '%s'. Error: '%s'"), *NameForErrors, *Reader->GetErrorMessage());
			}

			return nullptr;
		}
	}
	else
	{
		if (!bSilent)
		{
			UE_LOG(LogVaTexAtlasEditor, Error, TEXT("VTA data file '%s' was empty. This texture atlas cannot be imported."), *NameForErrors);
		}

		return nullptr;
	}
}

UObject* UVtaTextureAtlasFactory::CreateNewAsset(UClass* AssetClass, const FString& TargetPath, const FString& DesiredName, EObjectFlags Flags)
{
	FAssetToolsModule& AssetToolsModule = FModuleManager::GetModuleChecked<FAssetToolsModule>("AssetTools");

	// Create a unique package name and asset name for the frame
	const FString TentativePackagePath = PackageTools::SanitizePackageName(TargetPath + TEXT("/") + DesiredName);
	FString DefaultSuffix;
	FString AssetName;
	FString PackageName;
	AssetToolsModule.Get().CreateUniqueAssetName(TentativePackagePath, DefaultSuffix, PackageName, AssetName);

	// Create a package for the asset
	UObject* OuterForAsset = CreatePackage(nullptr, *PackageName);

	// Create a frame in the package
	UObject* NewAsset = NewObject<UObject>(OuterForAsset, AssetClass, *AssetName, Flags);
	FAssetRegistryModule::AssetCreated(NewAsset);

	NewAsset->Modify();
	return NewAsset;
}

UObject* UVtaTextureAtlasFactory::ImportAsset(const FString& SourceFilename, const FString& TargetSubPath)
{
	FAssetToolsModule& AssetToolsModule = FModuleManager::GetModuleChecked<FAssetToolsModule>("AssetTools");

	TArray<FString> FileNames;
	FileNames.Add(SourceFilename);

	TArray<UObject*> ImportedAssets = AssetToolsModule.Get().ImportAssets(FileNames, TargetSubPath);
	return (ImportedAssets.Num() > 0) ? ImportedAssets[0] : nullptr;
}

UTexture2D* UVtaTextureAtlasFactory::ImportTexture(const FString& SourceFilename, const FString& TargetSubPath)
{
	UTexture2D* ImportedTexture = Cast<UTexture2D>(ImportAsset(SourceFilename, TargetSubPath));

	if (ImportedTexture != nullptr)
	{
		// @TODO Optionally change compression and LOD group here
	}

	return ImportedTexture;
}

//////////////////////////////////////////////////////////////////////////

#undef LOCTEXT_NAMESPACE
