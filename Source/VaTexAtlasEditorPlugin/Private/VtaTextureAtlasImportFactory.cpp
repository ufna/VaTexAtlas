// Copyright 2016-2019 Vladimir Alyamkin. All Rights Reserved.

#include "VtaTextureAtlasImportFactory.h"

#include "VtaEditorPlugin.h"
#include "VtaEditorPluginSettings.h"

#include "Editor.h"
#include "EditorFramework/AssetImportData.h"
#include "EditorReimportHandler.h"
#include "IAssetTools.h"
#include "PackageTools.h"
#include "Runtime/Launch/Resources/Version.h"

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

	ExistingTable = nullptr;
}

FText UVtaTextureAtlasImportFactory::GetToolTip() const
{
	return LOCTEXT("VtaTextureAtlasImportFactoryDescription", "Texture atlas imported from TexturePacker");
}

bool UVtaTextureAtlasImportFactory::FactoryCanImport(const FString& Filename)
{
	auto Settings = GetMutableDefault<UVtaEditorPluginSettings>();
	if (Settings->bUseNewAtlasAsset)
	{
		return false;
	}

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
	auto Settings = GetMutableDefault<UVtaEditorPluginSettings>();

	bool bUseMultiAtlas = false;

	FString MultiAtlasName = InName.ToString();
	if (Settings->bUseMultiAtlas && MultiAtlasName.Len() > 0)
	{
		int32 Index = INDEX_NONE;
		MultiAtlasName.FindLastChar('_', Index);
		if (Index != INDEX_NONE)
		{
			if (FCString::IsNumeric(*MultiAtlasName.Right(MultiAtlasName.Len() - 1 - Index)))
			{
				bUseMultiAtlas = true;
				MultiAtlasName = MultiAtlasName.Left(Index);
			}
		}
	}

	Flags |= RF_Transactional;

	FEditorDelegates::OnAssetPreImport.Broadcast(this, InClass, InParent, InName, Type);

	FAssetToolsModule& AssetToolsModule = FModuleManager::GetModuleChecked<FAssetToolsModule>("AssetTools");

#if ENGINE_MINOR_VERSION <= 12
	const FString CurrentFilename = UFactory::GetCurrentFilename();
#endif
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

	if (bUseMultiAtlas)
	{
		SetMultipackFrames(LongPackagePath, MultiAtlasName);
	}

	UVtaTextureAtlas* Result = nullptr;
	UTexture2D* ImageTexture = nullptr;
	TArray<UVtaTextureAtlas*> ModifiedAtlases;

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

		// Save filename paths
		Result->AssetImportData->Update(CurrentFilename);

		// Cache data for debug
		Result->ImportedData = DataModel;

		// Load the base texture
		const FString SourceAtlasTextureFilename = FPaths::Combine(*CurrentSourcePath, *DataModel.Meta.Image);
		ImageTexture = ImportOrReimportTexture((bIsReimporting && (ExistingAtlasTextureName == DataModel.Meta.Image)) ? ExistingAtlasTexture : nullptr, SourceAtlasTextureFilename, TexturesSubPath);
		if (ImageTexture == nullptr)
		{
			UE_LOG(LogVaTexAtlasEditor, Warning, TEXT("Failed to import atlas image '%s'."), *SourceAtlasTextureFilename);
		}

		// Load parent material for frames
		UMaterial* FrameMaterial = LoadObject<UMaterial>(NULL, TEXT("/VaTexAtlasPlugin/Materials/M_AtlasFrame.M_AtlasFrame"), NULL, LOAD_None, NULL);

		GWarn->BeginSlowTask(LOCTEXT("VtaTextureAtlasImportFactory_ImportingFrames", "Importing Atlas Frames"), true, true);

		// Perform assets import
		for (int32 i = 0; i < DataModel.Frames.Num(); i++)
		{
			auto Frame = DataModel.Frames[i];
			const FString FrameAssetName = BuildFrameName(MultiAtlasName, Frame.Filename);
			const FString SlateTextureAssetName = BuildSlateTextureName(MultiAtlasName, Frame.Filename);

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
			UVtaSlateTexture* TargetSlateTexture = nullptr;

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

				TargetSlateTexture = FindExistingSlateTexture(Frame.Filename);
				if (TargetSlateTexture)
				{
					TargetSlateTexture->Modify();
				}
				else
				{
					UE_LOG(LogVaTexAtlasEditor, Error, TEXT("Failed to load existing slate texture: '%s'"), *Frame.Filename);
				}
			}

			UVtaTextureAtlas* OtherAtlas = MultipackFrames.FindRef(Frame.Filename);
			if (bUseMultiAtlas && OtherAtlas)
			{
				OtherAtlas->Modify();
				TargetFrame = FindMaterialByFrameName(Frame.Filename, OtherAtlas->Frames);
				if (TargetFrame)
				{
					TargetFrame->Modify();
					OtherAtlas->Frames.Remove(TSoftObjectPtr<UMaterialInstanceConstant>(TargetFrame));
				}
				else
				{
					UE_LOG(LogVaTexAtlasEditor, Error, TEXT("Failed to load existing frame from other atlas: '%s'"), *Frame.Filename);
				}

				TargetSlateTexture = FindSlateTextureByFrameName(Frame.Filename, OtherAtlas->SlateTextures);
				if (TargetSlateTexture)
				{
					TargetSlateTexture->Modify();
					OtherAtlas->SlateTextures.Remove(TSoftObjectPtr<UVtaSlateTexture>(TargetSlateTexture));
				}
				else
				{
					UE_LOG(LogVaTexAtlasEditor, Error, TEXT("Failed to load existing slate texture from other atlas: '%s'"), *Frame.Filename);
				}

				OtherAtlas->FrameNames.Remove(Frame.Filename);
				ModifiedAtlases.AddUnique(OtherAtlas);
			}

			if (!TargetFrame)
			{
				TargetFrame = FindMaterialOnDisk(FramesSubPath, FrameAssetName);
				if (TargetFrame)
				{
					TargetFrame->Modify();
				}
			}

			if (!TargetSlateTexture)
			{
				TargetSlateTexture = FindSlateTextureOnDisk(FramesSubPath, SlateTextureAssetName);
				if (TargetSlateTexture)
				{
					TargetSlateTexture->Modify();
				}
			}

			// Check we should create new one
			if (TargetFrame == nullptr && Settings->bGenerateMaterialInstances)
			{
				TargetFrame = CastChecked<UMaterialInstanceConstant>(CreateNewAsset(UMaterialInstanceConstant::StaticClass(), FramesSubPath, FrameAssetName, Flags));
			}

			if (TargetSlateTexture == nullptr && Settings->bGenerateSlateTextures)
			{
				TargetSlateTexture = CastChecked<UVtaSlateTexture>(CreateNewAsset(UVtaSlateTexture::StaticClass(), FramesSubPath, SlateTextureAssetName, Flags));
			}

			if (TargetFrame)
			{
				// Fill parameters for frame
				TargetFrame->SetParentEditorOnly(FrameMaterial);
				TargetFrame->SetTextureParameterValueEditorOnly(TEXT("Atlas"), ImageTexture);
				TargetFrame->SetVectorParameterValueEditorOnly(TEXT("FrameUV"), FrameUVs);

				// Make sure that changes are applied to assets
				FPropertyChangedEvent FinalRebuildFrameSet(nullptr, EPropertyChangeType::ValueSet);
				TargetFrame->PostEditChangeProperty(FinalRebuildFrameSet);

				// Set frame to Atlas
				Result->Frames.Add(TargetFrame);
			}

			if (TargetSlateTexture)
			{
				// Fill parameters for slate texture
				TargetSlateTexture->AtlasTexture = ImageTexture;
				TargetSlateTexture->StartUV = FVector2D(FrameUVs.R, FrameUVs.B);
				TargetSlateTexture->SizeUV = FVector2D(FrameUVs.G - FrameUVs.R, FrameUVs.A - FrameUVs.B);

				// Make sure that changes are applied to assets
				FPropertyChangedEvent FinalRebuildSlateTextureSet(nullptr, EPropertyChangeType::ValueSet);
				TargetSlateTexture->PostEditChangeProperty(FinalRebuildSlateTextureSet);

				// Set slate texture to Atlas
				Result->SlateTextures.Add(TargetSlateTexture);
			}
			// Set frame to Atlas
			Result->FrameNames.Add(Frame.Filename);
		}

		// Set data to atlas asset
		Result->TextureName = DataModel.Meta.Image;
		Result->Texture = ImageTexture;
		Result->PostEditChange();

		GWarn->EndSlowTask();
	}

	ImportOrReimportDataTable(Cast<UVtaTextureAtlas>(Result), LongPackagePath, InName.ToString(), Flags);
	for (auto* ModifiedAtlas : ModifiedAtlases)
	{
		ImportOrReimportDataTable(ModifiedAtlas, LongPackagePath, ModifiedAtlas->GetName(), Flags, false);
	}

	if (bUseMultiAtlas)
	{
		ImportOrReimportMultiAtlasDataTable(LongPackagePath, MultiAtlasName, Flags);
	}

	FEditorDelegates::OnAssetPostImport.Broadcast(this, Result);

	// Reset the importer to ensure that no leftover data can contaminate future imports
	ResetImportData();
	MultipackFrames.Empty();

	return Result;
}

TSharedPtr<FJsonObject> UVtaTextureAtlasImportFactory::ParseJSON(const FString& FileContents, const FString& NameForErrors, bool bSilent)
{
	// Load the file up (JSON format)
	if (!FileContents.IsEmpty())
	{
		const TSharedRef<TJsonReader<>>& Reader = TJsonReaderFactory<>::Create(FileContents);

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

void UVtaTextureAtlasImportFactory::ImportOrReimportDataTable(UVtaTextureAtlas* TextureAtlas, const FString& TargetPath, const FString& DesiredName, EObjectFlags Flags, bool bUseExistingTable)
{
	if (TextureAtlas)
	{
		TextureAtlas->Modify();

		if (bUseExistingTable)
		{
			TextureAtlas->Table = ExistingTable;
		}

		UDataTable* DataTable = nullptr;
		if (TextureAtlas->Table != nullptr)
		{
			DataTable = TextureAtlas->Table;
			DataTable->EmptyTable();
		}
		else
		{
			DataTable = CastChecked<UDataTable>(CreateNewAsset(UDataTable::StaticClass(), TargetPath, DesiredName + TEXT("_DataTable"), Flags));
			DataTable->RowStruct = FVtaAsset::StaticStruct();

			TextureAtlas->Table = DataTable;
			TextureAtlas->PostEditChange();
		}

		DataTable->Modify();

		TSet<FString> ExName;

		for (const FString& Name : TextureAtlas->FrameNames)
		{
			FVtaAsset Asset;
			Asset.Material = FindMaterialByFrameName(Name, TextureAtlas->Frames);
			Asset.SlateTexture = FindSlateTextureByFrameName(Name, TextureAtlas->SlateTextures);

			int32 Position = INDEX_NONE;
			Name.FindLastChar(TEXT('.'), Position);

			FString RowName = (Position < 1) ? Name : Name.Left(Position);
			RowName = TEXT("_") + PackageTools::SanitizePackageName(RowName);

			while (RowName.FindChar(TEXT('_'), Position))
			{
				RowName.RemoveAt(Position);
				if (Position < RowName.Len())
				{
					RowName.InsertAt(Position, FChar::ToUpper(RowName[Position]));
					RowName.RemoveAt(Position + 1);
				}
			}

			FString OriginalRowName = RowName;

			int32 i = 0;
			while (ExName.Contains(RowName))
			{
				RowName = FString::Printf(TEXT("%s%d"), *OriginalRowName, ++i);
			}

			ExName.Add(RowName);
			DataTable->AddRow(FName(*RowName), Asset);
		}

		DataTable->PostEditChange();
	}
}

void UVtaTextureAtlasImportFactory::ImportOrReimportMultiAtlasDataTable(const FString& TargetPath, const FString& MultiAtlasName, EObjectFlags Flags)
{
	TArray<UVtaTextureAtlas*> Atlasses;

	FAssetRegistryModule& AssetRegistryModule = FModuleManager::LoadModuleChecked<FAssetRegistryModule>("AssetRegistry");

	FARFilter Filter;
	Filter.ClassNames.Add(UVtaTextureAtlas::StaticClass()->GetFName());
	Filter.PackagePaths.Add(FName(*TargetPath));

	TArray<FAssetData> Assets;
	AssetRegistryModule.Get().GetAssets(Filter, Assets);

	for (FAssetData& AssetData : Assets)
	{
		FString Name = AssetData.AssetName.ToString();
		if (!Name.Contains(MultiAtlasName + "_"))
		{
			continue;
		}

		UVtaTextureAtlas* OtherTextureAtlas = Cast<UVtaTextureAtlas>(AssetData.GetAsset());
		if (OtherTextureAtlas->Table)
		{
			Atlasses.Add(OtherTextureAtlas);
		}
	}

	if (Assets.Num() > 0)
	{
		Atlasses.Sort([](const UVtaTextureAtlas& A, const UVtaTextureAtlas& B) {
			return A.TextureName > B.TextureName;
		});

		FString MultiDataTableName = MultiAtlasName + "_MultiDataTable";
		FString MultiDataTablePath = TargetPath + "/" + MultiDataTableName;

		UDataTable* DataTable = Cast<UDataTable>(StaticLoadObject(UDataTable::StaticClass(), nullptr, *MultiDataTablePath, nullptr, LOAD_None, nullptr));
		if (DataTable == nullptr)
		{
			DataTable = CastChecked<UDataTable>(CreateNewAsset(UDataTable::StaticClass(), TargetPath, MultiDataTableName, Flags));
			DataTable->RowStruct = FVtaAsset::StaticStruct();
		}

		DataTable->EmptyTable();
		DataTable->Modify();

		TSet<FString> ExName;

		for (UVtaTextureAtlas* OtherTextureAtlas : Atlasses)
		{
			for (FName OriginalRowName : OtherTextureAtlas->Table->GetRowNames())
			{
				FString RowName = OriginalRowName.ToString();
				int32 i = 0;
				while (ExName.Contains(RowName))
				{
					RowName = OriginalRowName.ToString() + FString::FromInt(++i);
				}

				DataTable->AddRow(FName(*RowName), *OtherTextureAtlas->Table->FindRow<FVtaAsset>(FName(*RowName), TEXT("")));
			}
		}

		DataTable->PostEditChange();
	}
}

FString UVtaTextureAtlasImportFactory::BuildFrameName(const FString& AtlasName, const FString& FrameName)
{
	return TEXT("MIA_") + AtlasName + TEXT("_") + FrameName;
}

FString UVtaTextureAtlasImportFactory::BuildSlateTextureName(const FString& AtlasName, const FString& FrameName)
{
	return TEXT("ST_") + AtlasName + TEXT("_") + FrameName;
}

//////////////////////////////////////////////////////////////////////////
// Reimport (used by derived class to provide existing data)

void UVtaTextureAtlasImportFactory::SetMultipackFrames(const FString& Path, const FString& MultuipackName)
{
	FAssetRegistryModule& AssetRegistryModule = FModuleManager::LoadModuleChecked<FAssetRegistryModule>("AssetRegistry");

	FARFilter Filter;
	Filter.ClassNames.Add(UVtaTextureAtlas::StaticClass()->GetFName());
	Filter.PackagePaths.Add(FName(*Path));

	TArray<FAssetData> Assets;
	AssetRegistryModule.Get().GetAssets(Filter, Assets);

	for (FAssetData& AssetData : Assets)
	{
		FString Name = AssetData.AssetName.ToString();
		if (!Name.Contains(MultuipackName + "_"))
		{
			continue;
		}

		UVtaTextureAtlas* OtherTextureAtlas = Cast<UVtaTextureAtlas>(AssetData.GetAsset());
		if (OtherTextureAtlas)
		{
			for (const auto& FrameName : OtherTextureAtlas->FrameNames)
			{
				MultipackFrames.Add(FrameName, OtherTextureAtlas);
			}
		}
	}
}

void UVtaTextureAtlasImportFactory::SetReimportData(UVtaTextureAtlas* TextureAtlas)
{
	ExistingTable = TextureAtlas->Table;

	for (const FString& Name : TextureAtlas->FrameNames)
	{
		UMaterialInstanceConstant* LoadedFrame = FindMaterialByFrameName(Name, TextureAtlas->Frames);
		UVtaSlateTexture* LoadedSlateTexture = FindSlateTextureByFrameName(Name, TextureAtlas->SlateTextures);

		if (LoadedFrame != nullptr)
		{
			ExistingFrames.Add(Name, LoadedFrame);
		}

		if (LoadedSlateTexture != nullptr)
		{
			ExistingSlateTextures.Add(Name, LoadedSlateTexture);
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
	ExistingSlateTextures.Empty();

	ExistingTable = nullptr;
}

UMaterialInstanceConstant* UVtaTextureAtlasImportFactory::FindExistingFrame(const FString& Name)
{
	return ExistingFrames.FindRef(Name);
}

UVtaSlateTexture* UVtaTextureAtlasImportFactory::FindExistingSlateTexture(const FString& Name)
{
	return ExistingSlateTextures.FindRef(Name);
}

UMaterialInstanceConstant* UVtaTextureAtlasImportFactory::FindMaterialByFrameName(const FString& Name, TArray<TSoftObjectPtr<UMaterialInstanceConstant>> List)
{
	FString FindName = PackageTools::SanitizePackageName(Name);
	for (auto AssetPtr : List)
	{
		FString AssetName = AssetPtr.ToSoftObjectPath().GetAssetName();
		if (AssetName.EndsWith(TEXT("_") + Name))
		{
			return Cast<UMaterialInstanceConstant>(StaticLoadObject(UMaterialInstanceConstant::StaticClass(), nullptr, *AssetPtr.ToSoftObjectPath().ToString(), nullptr, LOAD_None, nullptr));
		}
	}

	return nullptr;
}

UVtaSlateTexture* UVtaTextureAtlasImportFactory::FindSlateTextureByFrameName(const FString& Name, TArray<TSoftObjectPtr<UVtaSlateTexture>> List)
{
	FString FindName = PackageTools::SanitizePackageName(Name);
	for (auto AssetPtr : List)
	{
		FString AssetName = AssetPtr.ToSoftObjectPath().GetAssetName();
		if (AssetName.EndsWith(TEXT("_") + Name))
		{
			return Cast<UVtaSlateTexture>(StaticLoadObject(UVtaSlateTexture::StaticClass(), nullptr, *AssetPtr.ToSoftObjectPath().ToString(), nullptr, LOAD_None, nullptr));
		}
	}

	return nullptr;
}

UMaterialInstanceConstant* UVtaTextureAtlasImportFactory::FindMaterialOnDisk(const FString& Path, const FString& Name)
{
	FAssetRegistryModule& AssetRegistryModule = FModuleManager::LoadModuleChecked<FAssetRegistryModule>("AssetRegistry");

	FARFilter Filter;
	Filter.ClassNames.Add(UMaterialInstanceConstant::StaticClass()->GetFName());
	Filter.PackagePaths.Add(FName(*Path));

	TArray<FAssetData> Assets;
	AssetRegistryModule.Get().GetAssets(Filter, Assets);

	for (FAssetData& AssetData : Assets)
	{
		FString AssetName = AssetData.AssetName.ToString();
		if (!AssetName.Equals(Name))
		{
			continue;
		}

		UMaterialInstanceConstant* FrameAsset = Cast<UMaterialInstanceConstant>(AssetData.GetAsset());
		if (FrameAsset)
		{
			return FrameAsset;
		}
	}

	return nullptr;
}

UVtaSlateTexture* UVtaTextureAtlasImportFactory::FindSlateTextureOnDisk(const FString& Path, const FString& Name)
{
	FAssetRegistryModule& AssetRegistryModule = FModuleManager::LoadModuleChecked<FAssetRegistryModule>("AssetRegistry");

	FARFilter Filter;
	Filter.ClassNames.Add(UVtaSlateTexture::StaticClass()->GetFName());
	Filter.PackagePaths.Add(FName(*Path));

	TArray<FAssetData> Assets;
	AssetRegistryModule.Get().GetAssets(Filter, Assets);

	for (FAssetData& AssetData : Assets)
	{
		FString AssetName = AssetData.AssetName.ToString();
		if (!AssetName.Equals(Name))
		{
			continue;
		}

		UVtaSlateTexture* FrameAsset = Cast<UVtaSlateTexture>(AssetData.GetAsset());
		if (FrameAsset)
		{
			return FrameAsset;
		}
	}

	return nullptr;
}

//////////////////////////////////////////////////////////////////////////

#undef LOCTEXT_NAMESPACE
