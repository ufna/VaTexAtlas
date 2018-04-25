// Copyright 2016 Vladimir Alyamkin. All Rights Reserved.

#include "VtaTextureAtlasAssetImportFactory.h"
#include "VtaEditorPlugin.h"
#include "VtaEditorPluginSettings.h"
#include "VtaTextureAtlasAsset.h"
#include "VtaAsset.h"
#include "VtaSlateTexture.h"

#include "Json.h"
#include "Engine/Texture2D.h"
#include "ObjectTools.h"
#include "PackageTools.h"
#include "Misc/Paths.h"
#include "Misc/FileHelper.h"
#include "HAL/FileManager.h"

#define LOCTEXT_NAMESPACE "VtaEditorPlugin"

class FVtaTextureAtlasAssetImportUI : public TSharedFromThis<FVtaTextureAtlasAssetImportUI>
{
public:
	explicit FVtaTextureAtlasAssetImportUI(bool bInIsValidMultipackName)
		: bIsValidMultipackName(bInIsValidMultipackName)
		, bImportAsMultipack(bInIsValidMultipackName)
		, TextureCompressionSetting(TextureCompressionSettings::TC_EditorIcon)
		, bUserCanceled(false)
	{}

	bool GetImportAsMultipack() const { return bImportAsMultipack; }
	TextureCompressionSettings GetTextureCompressionSetting() const { return TextureCompressionSetting; }

	TSharedRef<SWidget> MakeCompressionSettingSelectorItemWidget(TSharedPtr<TextureCompressionSettings> TextureSetting) const
	{
		return SNew(STextBlock).Text(UEnum::GetDisplayValueAsText<TextureCompressionSettings>(TEXT("/Script/Engine.TextureCompressionSettings"), TextureSetting.IsValid() ? *TextureSetting : TC_Default));
	}

	FText GetSelectedCompressionSettingText() const
	{
		if (!CompressionSettingsSelector.IsValid())
		{
			return FText::GetEmpty();
		}

		return UEnum::GetDisplayValueAsText<TextureCompressionSettings>(TEXT("/Script/Engine.TextureCompressionSettings"), CompressionSettingsSelector->GetSelectedItem().IsValid()? *CompressionSettingsSelector->GetSelectedItem() : TC_Default);
	}

	FReply OnOk()
	{
		bUserCanceled = false;
		bImportAsMultipack = MultipackOptionCheckbox.IsValid() ? MultipackOptionCheckbox->IsChecked() : false;
		TextureCompressionSetting = CompressionSettingsSelector.IsValid() && CompressionSettingsSelector->GetSelectedItem().IsValid() ? *CompressionSettingsSelector->GetSelectedItem() : TC_Default;
		if (DialogWindow.IsValid())
		{
			DialogWindow->RequestDestroyWindow();
		}
		return FReply::Handled();
	}

	FReply OnCancel()
	{
		bUserCanceled = true;
		if (DialogWindow.IsValid())
		{
			DialogWindow->RequestDestroyWindow();
		}
		return FReply::Handled();
	}

	bool OpenSettingsDialog()
	{
		TArray<TSharedPtr<TextureCompressionSettings>> PossibleCompressionSettings;
		PossibleCompressionSettings.Reserve(TC_MAX);
		for (uint32 Setting = TC_Default; Setting < TC_MAX; ++Setting)
		{
			PossibleCompressionSettings.Add(MakeShareable(new TextureCompressionSettings(static_cast<TextureCompressionSettings>(Setting))));
		}

		CompressionSettingsSelector = SNew(SComboBox<TSharedPtr<TextureCompressionSettings>>)
			.OptionsSource(&PossibleCompressionSettings)
			.OnGenerateWidget(this, &FVtaTextureAtlasAssetImportUI::MakeCompressionSettingSelectorItemWidget)
			.InitiallySelectedItem(PossibleCompressionSettings[TextureCompressionSetting])
			[
				SNew(STextBlock)
				.Text(this, &FVtaTextureAtlasAssetImportUI::GetSelectedCompressionSettingText)
			];

		MultipackOptionCheckbox = SNew(SCheckBox)
			.IsChecked(bIsValidMultipackName ? (bImportAsMultipack ? ECheckBoxState::Checked : ECheckBoxState::Unchecked) : ECheckBoxState::Undetermined)
			[
				SNew(STextBlock)
				.Text(LOCTEXT("MultipackCheckboxCaption", "Import atlas as multipack (will import all files with name {AtlasName}_{Index}.vta)?"))
			];

		DialogWindow = SNew(SWindow)
			.Title(LOCTEXT("TextureAtlasAssetImportOptionsTitle", "Texture Atlas Asset Import Settings"))
			.SizingRule(ESizingRule::Autosized)
			.SupportsMaximize(false).SupportsMinimize(false)
			.HasCloseButton(false)
			[
				SNew(SBorder)
				.BorderImage(FEditorStyle::GetBrush("Menu.Background"))
				.Padding(10)
				[
					SNew(SVerticalBox)
					+ SVerticalBox::Slot()
					.AutoHeight()
					.HAlign(HAlign_Left)
					[
						MultipackOptionCheckbox.ToSharedRef()
					]
					+ SVerticalBox::Slot()
					.AutoHeight()
					.HAlign(HAlign_Left)
					[
						SNew(SHorizontalBox)
						+ SHorizontalBox::Slot()
						.AutoWidth()
						.Padding(10.f, 0.f)
						.VAlign(VAlign_Center)
						[
							SNew(STextBlock).Text(LOCTEXT("TextureAtlasAssetImportTextureCompressionSettings", "Atlas Texture Compression:"))
						]
						+ SHorizontalBox::Slot()
						.AutoWidth()
						[
							CompressionSettingsSelector.ToSharedRef()
						]
					]
					+ SVerticalBox::Slot()
					.FillHeight(1.f)
					[
						SNew(SSpacer)
					]
					+ SVerticalBox::Slot()
					.AutoHeight()
					[
						SNew(SHorizontalBox)
						+ SHorizontalBox::Slot()
						.FillWidth(1.f)
						[
							SNew(SButton)
							.OnClicked(this, &FVtaTextureAtlasAssetImportUI::OnCancel)
							.HAlign(HAlign_Center)
							.Text(LOCTEXT("TextureAtlasAssetImportTextureCancel", "Cancel"))
						]
						+ SHorizontalBox::Slot()
						.FillWidth(1.f)
						[
							SNew(SButton)
							.OnClicked(this, &FVtaTextureAtlasAssetImportUI::OnOk)
							.HAlign(HAlign_Center)
							.Text(LOCTEXT("TextureAtlasAssetImportTextureOk", "Import"))
						]
					]
				]
			];

		GEditor->EditorAddModalWindow(DialogWindow.ToSharedRef());

		return !bUserCanceled;
	}

private:
	bool bIsValidMultipackName;
	bool bImportAsMultipack;
	TextureCompressionSettings TextureCompressionSetting;
	bool bUserCanceled;

	TSharedPtr<SWindow> DialogWindow;
	TSharedPtr<SComboBox< TSharedPtr<TextureCompressionSettings> >> CompressionSettingsSelector;
	TSharedPtr<SCheckBox> MultipackOptionCheckbox;
};

UVtaTextureAtlasAssetImportFactory::UVtaTextureAtlasAssetImportFactory()
{
	SupportedClass = UVtaTextureAtlasAsset::StaticClass();
	bCreateNew = false;
	bEditAfterNew = true;
	bEditorImport = true;

	Formats.Add(TEXT("vta;VaTexAtlas data file"));
}

bool UVtaTextureAtlasAssetImportFactory::FactoryCanImport(const FString& Filename)
{
	auto Settings = GetMutableDefault<UVtaEditorPluginSettings>();
	if (!Settings->bUseNewAtlasAsset)
	{
		return false;
	}

	TSharedPtr<FJsonObject> DescriptorObject = ParseJSON(Filename);
	if (DescriptorObject.IsValid())
	{
		FVtaDataFile GlobalInfo;
		GlobalInfo.ParseFromJSON(DescriptorObject, Filename, true, true);
		return GlobalInfo.IsValid();
	}

	return false;
}

FText UVtaTextureAtlasAssetImportFactory::GetToolTip() const
{
	return LOCTEXT("VtaTextureAtlasAssetImportFactoryDescription", "Texture atlas imported from TexturePacker (new version)");
}

UObject* UVtaTextureAtlasAssetImportFactory::FactoryCreateFile(UClass * InClass, UObject * InParent, FName InName, EObjectFlags Flags, const FString & Filename, const TCHAR * Parms, FFeedbackContext * Warn, bool & bOutOperationCanceled)
{
	FString MultipackAtlasName;
	int32 Index = 0;
	bool bCanBeMultipack = SplitMultipackAtlasName(Filename, MultipackAtlasName, Index);

	if (CurrentAtlas)
	{
		if (CurrentAtlas->AtlasName.Equals(MultipackAtlasName) || CurrentAtlas->AtlasName.Equals(FPaths::GetBaseFilename(Filename)))
		{
			UE_LOG(LogVaTexAtlasEditor, Verbose, TEXT("UVtaTextureAtlasAssetImportFactory::FactoryCreateFile: Detected bulk import of one atlas. Skip..."));
			return CurrentAtlas;
		}
	}

	TSharedRef<FVtaTextureAtlasAssetImportUI> SettingsUI = MakeShareable(new FVtaTextureAtlasAssetImportUI(bCanBeMultipack));
	if (!SettingsUI->OpenSettingsDialog())
	{
		UE_LOG(LogVaTexAtlasEditor, Verbose, TEXT("UVtaTextureAtlasAssetImportFactory::FactoryCreateFile: User canceled import"));
		bOutOperationCanceled = true;
		return nullptr;
	}

	Flags |= RF_Transactional;

	InName = SettingsUI->GetImportAsMultipack()? *MultipackAtlasName : *FPaths::GetBaseFilename(Filename);
	FEditorDelegates::OnAssetPreImport.Broadcast(this, InClass, InParent, InName, *FPaths::GetExtension(Filename));

	CurrentAtlas = NewObject<UVtaTextureAtlasAsset>(InParent, InName, Flags);
	CurrentAtlas->Modify();
	CurrentAtlas->AtlasName = InName.ToString();
	CurrentAtlas->ImportPath = FPaths::GetPath(Filename);
	CurrentAtlas->bIsMultipack = SettingsUI->GetImportAsMultipack();
	CurrentAtlas->TextureCompressionSetting = SettingsUI->GetTextureCompressionSetting();

	bool bSuccesfullyImported = ImportAtlas(CurrentAtlas, Warn);
	if (!bSuccesfullyImported)
	{
		if (Warn)
		{
			Warn->Logf(ELogVerbosity::Error, TEXT("Failed to import atlas %s"), *CurrentAtlas->AtlasName);
			bOutOperationCanceled = Warn->ReceivedUserCancel();
		}

		UE_LOG(LogVaTexAtlasEditor, Error, TEXT("Failed to import atlas %s"), *CurrentAtlas->AtlasName);
		ObjectTools::DeleteSingleObject(CurrentAtlas, false);
		CurrentAtlas = nullptr;
		return nullptr;
	}

	CurrentAtlas->PostEditChange();

	FEditorDelegates::OnAssetPostImport.Broadcast(this, CurrentAtlas);
	return CurrentAtlas;
}

bool UVtaTextureAtlasAssetImportFactory::CanReimport(UObject* Obj, TArray<FString>& OutFilenames)
{
	if (!Obj || Obj->GetClass() != UVtaTextureAtlasAsset::StaticClass())
	{
		return false;
	}
	
	UVtaTextureAtlasAsset* Atlas = Cast<UVtaTextureAtlasAsset>(Obj);
	Atlas->AssetImportData->ExtractFilenames(OutFilenames);
	return true;
}

void UVtaTextureAtlasAssetImportFactory::SetReimportPaths(UObject* Obj, const TArray<FString>& NewReimportPaths)
{
	UVtaTextureAtlasAsset* Atlas = Cast<UVtaTextureAtlasAsset>(Obj);
	if (!Atlas || NewReimportPaths.Num() == 0)
	{
		return;
	}

	Atlas->Modify();
	Atlas->ImportPath = FPaths::GetPath(NewReimportPaths[0]);
	int32 UnusedIndex = 0;
	if (!Atlas->bIsMultipack || !SplitMultipackAtlasName(NewReimportPaths[0], Atlas->AtlasName, UnusedIndex))
	{
		Atlas->bIsMultipack = false;
		Atlas->AtlasName = FPaths::GetBaseFilename(NewReimportPaths[0]);
	}

	Atlas->PostEditChange();
}

EReimportResult::Type UVtaTextureAtlasAssetImportFactory::Reimport(UObject* Obj)
{
	UVtaTextureAtlasAsset* Atlas = Cast<UVtaTextureAtlasAsset>(Obj);
	if (!IsValid(Atlas))
	{
		UE_LOG(LogVaTexAtlasEditor, Error, TEXT("Object %s is not valid texture atlas asset."), *Obj->GetFullName());
		return EReimportResult::Failed;
	}

	return ImportAtlas(Atlas, GWarn) ? EReimportResult::Succeeded : EReimportResult::Failed;
}

TSharedPtr<class FJsonObject> UVtaTextureAtlasAssetImportFactory::ParseJSON(const FString& FileName)
{
	FString FileContents;
	if (!FFileHelper::LoadFileToString(FileContents, *FileName))
	{
		UE_LOG(LogVaTexAtlasEditor, Error, TEXT("Failed to load Texture Atlas '%s'."), *FileName);
		return nullptr;
	}
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
			UE_LOG(LogVaTexAtlasEditor, Error, TEXT("Failed to parse Texture Atlas '%s'. Error: '%s'"), *FileName, *Reader->GetErrorMessage());
			return nullptr;
		}
	}
	else
	{
		UE_LOG(LogVaTexAtlasEditor, Error, TEXT("VTA data file '%s' was empty. This texture atlas cannot be imported."), *FileName);
		return nullptr;
	}
}

bool UVtaTextureAtlasAssetImportFactory::ImportAtlas(UVtaTextureAtlasAsset* Atlas, FFeedbackContext* Warn)
{
	check(Atlas);
	if (!Warn)
	{
		Warn = GWarn;
	}

	if (!FPaths::DirectoryExists(Atlas->ImportPath))
	{
		Warn->Logf(ELogVerbosity::Error, TEXT("Directory %s doesn't exist"), *Atlas->ImportPath);
		return false;
	}

	TArray<FString> SourceDataFiles;
	if (Atlas->bIsMultipack)
	{
		IFileManager::Get().FindFiles(SourceDataFiles, *FPaths::Combine(Atlas->ImportPath, FString::Printf(TEXT("%s_?*.vta"), *Atlas->AtlasName)), true, false);
		for (auto& SourceDataFile : SourceDataFiles)
		{
			SourceDataFile = FPaths::Combine(Atlas->ImportPath, SourceDataFile);
		}
	}
	else
	{
		FString DataFilename = FPaths::Combine(Atlas->ImportPath, FString::Printf(TEXT("%s.vta"), *Atlas->AtlasName));
		if (FPaths::FileExists(DataFilename))
		{
			SourceDataFiles.Add(DataFilename);
		}
	}

	if (SourceDataFiles.Num() == 0)
	{
		Warn->Logf(ELogVerbosity::Error, TEXT("Source data files not found in %s"), *Atlas->ImportPath);
		return false;
	}

	Atlas->Modify();
	const FString AtlasPath = FPackageName::GetLongPackagePath(Atlas->GetPathName());

	TMap<FString, FVtaDataFile> ImportedFiles;
	TMap<FString, UTexture2D*> ImportedTextures;
	TMap<FString, FVtaAsset> ImportedFrames;
	{
		FScopedSlowTask SlowTask(SourceDataFiles.Num(), LOCTEXT("ImportDataFilesTask", "Import data files"), true, *Warn);
		ImportedFiles.Reserve(SourceDataFiles.Num());

		for (const FString& SourceDataFile : SourceDataFiles)
		{
			SlowTask.EnterProgressFrame(1.f, FText::Format(LOCTEXT("ImportDataFileFrame", "Importing {0}"), FText::FromString(SourceDataFile)));
			if (Warn->ReceivedUserCancel())
			{
				return false;
			}

			auto Json = ParseJSON(SourceDataFile);
			if (!Json.IsValid())
			{
				Warn->Logf(ELogVerbosity::Error, TEXT("Failed to import file '%s'"), *SourceDataFile);
				continue;
			}

			FVtaDataFile DataFile;
			DataFile.ParseFromJSON(Json, SourceDataFile, false, false);
			if (!DataFile.IsValid())
			{
				Warn->Logf(ELogVerbosity::Error, TEXT("Failed to import file '%s': Invalid format"), *SourceDataFile);
				continue;
			}

			ImportedFiles.Add(SourceDataFile, DataFile);

			UTexture2D* Texture = ProcessTexture(Atlas, DataFile.Meta.Image);
			
			if (!Texture)
			{
				Warn->Logf(ELogVerbosity::Error, TEXT("Failed to import texture %s"), *DataFile.Meta.Image);
				continue;
			}

			ImportedTextures.Add(DataFile.Meta.Image, Texture);

			for (auto& FrameData : DataFile.Frames)
			{
				const auto& Frame = ProcessFrame(Atlas, Texture, DataFile.Meta, FrameData);

				if (Frame.Material.IsNull() && Frame.SlateTexture.IsNull())
				{
					Warn->Logf(ELogVerbosity::Error, TEXT("Failed to import frame %s"), *FrameData.Filename);
					continue;
				}

				ImportedFrames.Add(FrameData.Filename, Frame);
			}
		}

		Atlas->ImportedData = ImportedFiles;
	}

	TArray<UObject*> PendingDeleteObjects;
	for (const auto& Texture : Atlas->Textures)
	{
		if (!ImportedTextures.Contains(Texture.Key))
		{
			PendingDeleteObjects.Add(Texture.Value);
		}
	}
	Atlas->Textures = ImportedTextures;

	for (const auto& Frame : Atlas->Frames)
	{
		if(!ImportedFrames.Contains(Frame.Key))
		{
			if (!Frame.Value.Material.IsNull())
			{
				PendingDeleteObjects.Add(Frame.Value.Material.Get());
			}

			if (!Frame.Value.SlateTexture.IsNull())
			{
				PendingDeleteObjects.Add(Frame.Value.SlateTexture.Get());
			}
		}
	}
	Atlas->Frames = ImportedFrames;

	{
		FScopedSlowTask SlowTask(0.f, LOCTEXT("CreatingDatatableTask", "Creating data table"), true, *Warn);
		const FString DataTableName = Atlas->AtlasName + (Atlas->bIsMultipack ? TEXT("_MultiDataTable") : TEXT("_DataTable"));
		if (!Atlas->DataTable)
		{
			Atlas->DataTable = Cast<UDataTable>(FindAsset(UDataTable::StaticClass(), AtlasPath, DataTableName));
			if (!Atlas->DataTable)
			{
				Atlas->DataTable = Cast<UDataTable>(CreateAsset(UDataTable::StaticClass(), AtlasPath, *DataTableName, Atlas->GetFlags()));
				Atlas->DataTable->RowStruct = FVtaAsset::StaticStruct();
			}
		}

		Atlas->DataTable->Modify();
		Atlas->DataTable->EmptyTable();
		TArray<FString> SortedFrames;
		ImportedFrames.GetKeys(SortedFrames);
		SortedFrames.Sort();

		TSet<FString> RowNames;
		for (const auto& Frame : SortedFrames)
		{
			FString RowName = GetNormalizedFrameName(Frame);
			FString OriginalRowName = RowName;

			int32 i = 0;
			while (RowNames.Contains(RowName))
			{
				RowName = FString::Printf(TEXT("%s%d"), *OriginalRowName, ++i);
			}
			RowNames.Add(RowName);
			Atlas->DataTable->AddRow(FName(*RowName), *Atlas->GetFrame(Frame));
		}
		Atlas->DataTable->PostEditChange();
	}

	Atlas->PostEditChange();
	Atlas->AssetImportData->Update(SourceDataFiles[0]);
	
	/*
	if (PendingDeleteObjects.Num() > 0)
	{
		ObjectTools::DeleteObjects(PendingDeleteObjects);
	}*/
	return true;
}

UObject* UVtaTextureAtlasAssetImportFactory::CreateAsset(UClass* Class, const FString& TargetPath, const FString& Name, EObjectFlags Flags)
{
	FAssetToolsModule& AssetToolsModule = FModuleManager::GetModuleChecked<FAssetToolsModule>("AssetTools");

	const FString TentativePackagePath = PackageTools::SanitizePackageName(FPaths::Combine(TargetPath, Name));
	FString DefaultSuffix;
	FString AssetName;
	FString PackageName;
	AssetToolsModule.Get().CreateUniqueAssetName(TentativePackagePath, DefaultSuffix, PackageName, AssetName);

	// Create a package for the asset
	UObject* OuterForAsset = CreatePackage(nullptr, *PackageName);

	// Create a frame in the package
	UObject* NewAsset = NewObject<UObject>(OuterForAsset, Class, *AssetName, Flags);
	FAssetRegistryModule::AssetCreated(NewAsset);
	return NewAsset;
}

UObject* UVtaTextureAtlasAssetImportFactory::ImportAsset(const FString& SourceFilename, const FString& TargetPath)
{
	FAssetToolsModule& AssetToolsModule = FModuleManager::GetModuleChecked<FAssetToolsModule>("AssetTools");

	TArray<FString> FileNames;
	FileNames.Add(SourceFilename);

	TArray<UObject*> ImportedAssets = AssetToolsModule.Get().ImportAssets(FileNames, TargetPath);
	return (ImportedAssets.Num() > 0) ? ImportedAssets[0] : nullptr;
}

UObject* UVtaTextureAtlasAssetImportFactory::FindAsset(UClass* AssetClass, const FString& Path, const FString& Name)
{
	FAssetRegistryModule& AssetRegistryModule = FModuleManager::LoadModuleChecked<FAssetRegistryModule>("AssetRegistry");

	FARFilter Filter;
	Filter.ClassNames.Add(AssetClass->GetFName());
	Filter.PackagePaths.Add(FName(*Path));

	TArray<FAssetData> Assets;
	AssetRegistryModule.Get().GetAssets(Filter, Assets);
	auto* Asset = Assets.FindByPredicate([Name](const FAssetData& AssetData) { return AssetData.AssetName.ToString().Equals(Name); });

	return Asset? Asset->GetAsset() : nullptr;
}

UTexture2D* UVtaTextureAtlasAssetImportFactory::ProcessTexture(UVtaTextureAtlasAsset* Atlas, const FString& TextureName)
{
	if (!IsValid(Atlas))
	{
		return nullptr;
	}

	FString TexturesPath = FPaths::Combine(FPackageName::GetLongPackagePath(Atlas->GetPathName()), TEXT("Textures"));

	UTexture2D* Result = Atlas->GetTexture(TextureName);
	if (!Result)
	{
		Result = Cast<UTexture2D>(FindAsset(UTexture2D::StaticClass(), TexturesPath, FPaths::GetBaseFilename(TextureName)));
	}

	if (Result)
	{
		Result->AssetImportData->Update(FPaths::Combine(Atlas->ImportPath, TextureName));
		FReimportManager::Instance()->Reimport(Result, true);
	}
	else
	{
		Result = Cast<UTexture2D>(ImportAsset(FPaths::Combine(Atlas->ImportPath, TextureName), TexturesPath));
		if (Result)
		{
			Result->Modify();
			Result->LODGroup = TEXTUREGROUP_UI;
			Result->CompressionSettings = Atlas->TextureCompressionSetting;
			Result->PostEditChange();
		}
	}

	return Result;
}

FVtaAsset UVtaTextureAtlasAssetImportFactory::ProcessFrame(UVtaTextureAtlasAsset* Atlas, UTexture2D* AtlasTexture, const FVtaMeta& Meta, const FVtaFrame& Frame)
{
	if (!IsValid(Atlas))
	{
		return FVtaAsset();
	}

	FString FramesPath = FPaths::Combine(FPackageName::GetLongPackagePath(Atlas->GetPathName()), TEXT("Frames"));
	FLinearColor FrameUVs = FLinearColor::Black;
	FrameUVs.R = (float)Frame.Frame.X / Meta.Size.W;
	FrameUVs.G = FrameUVs.R + (float)Frame.Frame.W / Meta.Size.W;
	FrameUVs.B = (float)Frame.Frame.Y / Meta.Size.H;
	FrameUVs.A = FrameUVs.B + (float)Frame.Frame.H / Meta.Size.H;

	auto* Settings = GetMutableDefault<UVtaEditorPluginSettings>();
	FVtaAsset* ExistingFrame = Atlas->GetFrame(Frame.Filename);
	FVtaAsset Result;
	UMaterial* FrameMaterial = LoadObject<UMaterial>(NULL, TEXT("/VaTexAtlasPlugin/Materials/M_AtlasFrame.M_AtlasFrame"), NULL, LOAD_None, NULL);
	if (Settings->bGenerateMaterialInstances && FrameMaterial)
	{
		UMaterialInstanceConstant* MaterialInstance = ExistingFrame ? ExistingFrame->Material.Get() : nullptr;
		if (!MaterialInstance)
		{
			const FString& AssetName = FString::Printf(TEXT("MIA_%s_%s"), *Atlas->AtlasName, *Frame.Filename);
			MaterialInstance = Cast<UMaterialInstanceConstant>(FindAsset(UMaterialInstanceConstant::StaticClass(), FramesPath, AssetName));
			if (!MaterialInstance)
			{
				MaterialInstance = Cast<UMaterialInstanceConstant>(CreateAsset(UMaterialInstanceConstant::StaticClass(), FramesPath, AssetName, Atlas->GetFlags()));
			}
		}

		if (MaterialInstance)
		{
			MaterialInstance->Modify();
			MaterialInstance->SetParentEditorOnly(FrameMaterial);
			MaterialInstance->SetTextureParameterValueEditorOnly(TEXT("Atlas"), AtlasTexture);
			MaterialInstance->SetVectorParameterValueEditorOnly(TEXT("FrameUV"), FrameUVs);

			// Make sure that changes are applied to assets
			FPropertyChangedEvent FinalRebuildFrameSet(nullptr, EPropertyChangeType::ValueSet);
			MaterialInstance->PostEditChangeProperty(FinalRebuildFrameSet);
			Result.Material = MaterialInstance;
		}
	}

	if (Settings->bGenerateSlateTextures)
	{
		UVtaSlateTexture* SlateTexture = ExistingFrame? ExistingFrame->SlateTexture.Get() : nullptr;
		if (!SlateTexture)
		{
			const FString& AssetName = FString::Printf(TEXT("ST_%s_%s"), *Atlas->AtlasName, *Frame.Filename);
			SlateTexture = Cast<UVtaSlateTexture>(FindAsset(UVtaSlateTexture::StaticClass(), FramesPath, AssetName));
			if (!SlateTexture)
			{
				SlateTexture = Cast<UVtaSlateTexture>(CreateAsset(UVtaSlateTexture::StaticClass(), FramesPath, AssetName, Atlas->GetFlags()));
			}
		}

		if (SlateTexture)
		{
			SlateTexture->Modify();
			SlateTexture->AtlasTexture = AtlasTexture;
			SlateTexture->StartUV = FVector2D(FrameUVs.R, FrameUVs.B);
			SlateTexture->SizeUV = FVector2D(FrameUVs.G - FrameUVs.R, FrameUVs.A - FrameUVs.B);

			// Make sure that changes are applied to assets
			FPropertyChangedEvent FinalRebuildSlateTextureSet(nullptr, EPropertyChangeType::ValueSet);
			SlateTexture->PostEditChangeProperty(FinalRebuildSlateTextureSet);
			Result.SlateTexture = SlateTexture;
		}
	}

	return Result;
}

FString UVtaTextureAtlasAssetImportFactory::GetNormalizedFrameName(const FString& Name)
{
	int32 Position = INDEX_NONE;
	FString Result = Name.FindLastChar(TEXT('.'), Position) ? Name.Left(Position) : Name;
	Result = TEXT("_") + PackageTools::SanitizePackageName(Result);

	while (Result.FindChar(TEXT('_'), Position))
	{
		Result.RemoveAt(Position);
		if (Position < Result.Len())
		{
			Result.InsertAt(Position, FChar::ToUpper(Result[Position]));
			Result.RemoveAt(Position + 1);
		}
	}
	return Result;
}

bool UVtaTextureAtlasAssetImportFactory::SplitMultipackAtlasName(const FString& Filename, FString& OutAtlasName, int32& OutIndex)
{
	FString BaseFilename = FPaths::GetBaseFilename(Filename);
	if (BaseFilename.Len() == 0)
	{
		UE_LOG(LogVaTexAtlasEditor, Error, TEXT("Failed to split atlas name '%s': Invalid filename"), *Filename);
		return false;
	}

	int32 SplitIndex = INDEX_NONE;
	BaseFilename.FindLastChar('_', SplitIndex);
	if (SplitIndex == INDEX_NONE || !FCString::IsNumeric(*BaseFilename.Right(BaseFilename.Len() - SplitIndex - 1)))
	{
		UE_LOG(LogVaTexAtlasEditor, Error, TEXT("Failed to split atlas name '%s': Is not valid multipack atlas name"), *Filename);
		OutAtlasName = BaseFilename;
		return false;
	}

	OutAtlasName = BaseFilename.Left(SplitIndex);
	OutIndex = FCString::Atoi(*BaseFilename.Right(BaseFilename.Len() - SplitIndex - 1));
	return true;
}

#undef LOCTEXT_NAMESPACE
