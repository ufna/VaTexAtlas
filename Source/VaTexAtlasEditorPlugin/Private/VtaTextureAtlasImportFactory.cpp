// Copyright 2016 Vladimir Alyamkin. All Rights Reserved.

#include "VtaEditorPlugin.h"
#include "VtaTextureAtlasDataModel.h"

#include "PackageTools.h"

#define LOCTEXT_NAMESPACE "VtaEditorPlugin"

//////////////////////////////////////////////////////////////////////////
// UVtaTextureAtlasImportFactory

UVtaTextureAtlasImportFactory::UVtaTextureAtlasImportFactory(const class FObjectInitializer& PCIP)
	: Super(PCIP)
	, bIsReimporting(false)
	, ExistingAtlasTexture(nullptr)
{
	SupportedClass = UVtaTextureAtlas::StaticClass();
	bCreateNew = false;
	bEditAfterNew = true;

	bEditorImport = true;
	bText = true;

	Formats.Add(TEXT("vta;VaTexAtlas data file"));
	Formats.Add(TEXT("json;VaTexAtlas JSON file"));
}

FText UVtaTextureAtlasImportFactory::GetToolTip() const
{
	return LOCTEXT("VtaTextureAtlasImportFactoryDescription", "Texture atlas imported from TexturePacker");
}

bool UVtaTextureAtlasImportFactory::FactoryCanImport(const FString& Filename)
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

UObject* UVtaTextureAtlasImportFactory::FactoryCreateText(UClass* InClass, UObject* InParent, FName InName, EObjectFlags Flags, UObject* Context, const TCHAR* Type, const TCHAR*& Buffer, const TCHAR* BufferEnd, FFeedbackContext* Warn)
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

	// Clear existing atlas
	UVtaTextureAtlas* ExistingAtlas = FindObject<UVtaTextureAtlas>(InParent, *InName.ToString());
	if (ExistingAtlas)
	{
		ExistingAtlas->EmptyData();
	}

	UVtaTextureAtlas* Result = nullptr;
	UTexture2D* ImageTexture = nullptr;

	// Parse the file 
	FVtaDataFile DataModel;
	if (DescriptorObject.IsValid())
	{
		DataModel.ParseFromJSON(DescriptorObject, NameForErrors, false, false);
	}

	// Create the new 'hub' asset and convert the data model over
	if (DataModel.IsValid())
	{
		const FString TexturesSubPath = LongPackagePath / TEXT("Textures");
		const FString FramesSubPath = LongPackagePath / TEXT("Frames");

		// Create asset
		Result = NewObject<UVtaTextureAtlas>(InParent, InName, Flags);
		Result->Modify();

		// Cache data for debug
		Result->ImportedData = DataModel;

		// Load the base texture
		const FString SourceSheetImageFilename = FPaths::Combine(*CurrentSourcePath, *DataModel.Meta.Image);
		ImageTexture = ImportOrReimportTexture((bIsReimporting && (ExistingAtlasTextureName == DataModel.Meta.Image)) ? ExistingAtlasTexture : nullptr, SourceSheetImageFilename, TexturesSubPath);
		if (ImageTexture == nullptr)
		{
			UE_LOG(LogVaTexAtlasEditor, Warning, TEXT("Failed to import atlas image '%s'."), *SourceSheetImageFilename);
		}

		// Load parent material for frames
		UMaterial* FrameMaterial = LoadObject<UMaterial>(NULL, TEXT("/VaTexAtlasPlugin/Materials/M_AtlasFrame.M_AtlasFrame"), NULL, LOAD_None, NULL);

		GWarn->BeginSlowTask(LOCTEXT("VtaTextureAtlasImportFactory_ImportingFrames", "Importing Atlas Frames"), true, true);

		// Perform assets import
		for (int32 i = 0; i < DataModel.Frames.Num(); i++)
		{
			auto Frame = DataModel.Frames[i];

			GWarn->StatusUpdate(i, DataModel.Frames.Num(), LOCTEXT("VtaTextureAtlasImportFactory_ImportingFrames", "Importing Atlas Frame"));

			// Check for user canceling the import
			if (GWarn->ReceivedUserCancel())
			{
				break;
			}

			// Calculate UVs in linear space as (U, UMax, V, VMax)
			FLinearColor FrameUVs = FLinearColor::Black;
			FrameUVs.R = (float)Frame.Frame.X / DataModel.Meta.Size.W;
			FrameUVs.G = FrameUVs.R + (float)Frame.Frame.W / DataModel.Meta.Size.W;
			FrameUVs.B = (float)Frame.Frame.Y / DataModel.Meta.Size.H;
			FrameUVs.A = FrameUVs.B + (float)Frame.Frame.H / DataModel.Meta.Size.H;

			// Create a frame in the package
			UMaterialInstanceConstant* TargetFrame = nullptr;

			// Check we have existing frame asset
			if (bIsReimporting)
			{
				TargetFrame = FindExistingFrame(Frame.Filename);
				if (TargetFrame)
				{
					TargetFrame->Modify();
				}
				else
				{
					UE_LOG(LogVaTexAtlasEditor, Error, TEXT("Failed to load existing frame: '%s'"), *Frame.Filename);
				}
			}
			
			// Check we should create new one
			if (TargetFrame == nullptr)
			{
				TargetFrame = CastChecked<UMaterialInstanceConstant>(CreateNewAsset(UMaterialInstanceConstant::StaticClass(), FramesSubPath, Frame.Filename, Flags));
			}

			// Fill parameters for frame
			TargetFrame->SetParentEditorOnly(FrameMaterial);
			TargetFrame->SetTextureParameterValueEditorOnly(TEXT("Atlas"), ImageTexture);
			TargetFrame->SetVectorParameterValueEditorOnly(TEXT("FrameUV"), FrameUVs);

			// Make sure that changes are applied to material instance
			FPropertyChangedEvent FinalRebuildFrameSet(nullptr, EPropertyChangeType::ValueSet);
			TargetFrame->PostEditChangeProperty(FinalRebuildFrameSet);

			// Set frame to Atlas
			Result->FrameNames.Add(Frame.Filename);
			Result->Frames.Add(TargetFrame);
		}

		// Set data to atlas asset
		Result->TextureName = DataModel.Meta.Image;
		Result->Texture = ImageTexture;
		Result->PostEditChange();

		GWarn->EndSlowTask();
	}

	FEditorDelegates::OnAssetPostImport.Broadcast(this, Result);

	// Reset the importer to ensure that no leftover data can contaminate future imports
	ResetImportData();

	return Result;
}

TSharedPtr<FJsonObject> UVtaTextureAtlasImportFactory::ParseJSON(const FString& FileContents, const FString& NameForErrors, bool bSilent)
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

UObject* UVtaTextureAtlasImportFactory::CreateNewAsset(UClass* AssetClass, const FString& TargetPath, const FString& DesiredName, EObjectFlags Flags)
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

UObject* UVtaTextureAtlasImportFactory::ImportAsset(const FString& SourceFilename, const FString& TargetSubPath)
{
	FAssetToolsModule& AssetToolsModule = FModuleManager::GetModuleChecked<FAssetToolsModule>("AssetTools");

	TArray<FString> FileNames;
	FileNames.Add(SourceFilename);

	TArray<UObject*> ImportedAssets = AssetToolsModule.Get().ImportAssets(FileNames, TargetSubPath);
	return (ImportedAssets.Num() > 0) ? ImportedAssets[0] : nullptr;
}

UTexture2D* UVtaTextureAtlasImportFactory::ImportTexture(const FString& SourceFilename, const FString& TargetSubPath)
{
	UTexture2D* ImportedTexture = Cast<UTexture2D>(ImportAsset(SourceFilename, TargetSubPath));

	if (ImportedTexture != nullptr)
	{
		ImportedTexture->Modify();

		// Default valus are used for UI icons
		ImportedTexture->LODGroup = TEXTUREGROUP_UI;
		ImportedTexture->CompressionSettings = TC_EditorIcon;

		ImportedTexture->PostEditChange();
	}

	return ImportedTexture;
}

UTexture2D* UVtaTextureAtlasImportFactory::ImportOrReimportTexture(UTexture2D* ExistingTexture, const FString& SourceFilename, const FString& TargetSubPath)
{
	UTexture2D* ResultTexture = nullptr;

	// Try reimporting if we have an existing texture
	if (ExistingTexture != nullptr)
	{
		if (FReimportManager::Instance()->Reimport(ExistingTexture, true))
		{
			ResultTexture = ExistingTexture;
		}
	}

	// If that fails, import the original textures
	if (ResultTexture == nullptr)
	{
		ResultTexture = ImportTexture(SourceFilename, TargetSubPath);
	}

	return ResultTexture;
}


//////////////////////////////////////////////////////////////////////////
// Reimport (used by derived class to provide existing data)

void UVtaTextureAtlasImportFactory::SetReimportData(const TArray<FString>& ExistingAtlasNames, const TArray< TAssetPtr<class UMaterialInstanceConstant> >& ExistingAtlasAssetPtrs)
{
	check(ExistingAtlasNames.Num() == ExistingAtlasAssetPtrs.Num());

	if (ExistingAtlasNames.Num() == ExistingAtlasAssetPtrs.Num())
	{
		for (int i = 0; i < ExistingAtlasAssetPtrs.Num(); ++i)
		{
			const TAssetPtr<class UMaterialInstanceConstant> FrameAssetPtr = ExistingAtlasAssetPtrs[i];
			FStringAssetReference FrameStringRef = FrameAssetPtr.ToStringReference();

			if (!FrameStringRef.ToString().IsEmpty())
			{
				UMaterialInstanceConstant* LoadedFrame = Cast<UMaterialInstanceConstant>(StaticLoadObject(UMaterialInstanceConstant::StaticClass(), nullptr, *FrameStringRef.ToString(), nullptr, LOAD_None, nullptr));
				if (LoadedFrame != nullptr)
				{
					ExistingFrames.Add(ExistingAtlasNames[i], LoadedFrame);
				}
			}
		}
	}

	bIsReimporting = true;
}

void UVtaTextureAtlasImportFactory::ResetImportData()
{
	bIsReimporting = false;

	ExistingAtlasTextureName = FString();
	ExistingAtlasTexture = nullptr;

	ExistingFrames.Empty();
}

UMaterialInstanceConstant* UVtaTextureAtlasImportFactory::FindExistingFrame(const FString& Name)
{
	return ExistingFrames.FindRef(Name);
}


//////////////////////////////////////////////////////////////////////////

#undef LOCTEXT_NAMESPACE
