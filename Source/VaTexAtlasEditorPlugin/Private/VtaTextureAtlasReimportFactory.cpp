// Copyright 2016 Vladimir Alyamkin. All Rights Reserved.

#include "VtaEditorPlugin.h"
#include "VtaTextureAtlasDataModel.h"

#include "PackageTools.h"

#define LOCTEXT_NAMESPACE "VtaEditorPlugin"

//////////////////////////////////////////////////////////////////////////
// UVtaTextureAtlasReimportFactory

UVtaTextureAtlasReimportFactory::UVtaTextureAtlasReimportFactory(const class FObjectInitializer& PCIP)
	: Super(PCIP)
{
	SupportedClass = UVtaTextureAtlas::StaticClass();
	bCreateNew = false;
}

bool UVtaTextureAtlasReimportFactory::CanReimport(UObject* Obj, TArray<FString>& OutFilenames)
{
	UVtaTextureAtlas* TextureAtlas = Cast<UVtaTextureAtlas>(Obj);
	if (TextureAtlas && TextureAtlas->AssetImportData)
	{
		if (OutFilenames.Num() == 0)
		{
			UE_LOG(LogVaTexAtlasEditor, Warning, TEXT("Force asset to store filename from previous import"));
			OutFilenames.Add(TEXT("HORSE FORCE"));
		}

		TextureAtlas->AssetImportData->ExtractFilenames(OutFilenames);
		return true;
	}
	
	return false;
}

void UVtaTextureAtlasReimportFactory::SetReimportPaths(UObject* Obj, const TArray<FString>& NewReimportPaths)
{
	UVtaTextureAtlas* TextureAtlas = Cast<UVtaTextureAtlas>(Obj);
	if (TextureAtlas && ensure(NewReimportPaths.Num() == 1))
	{
		TextureAtlas->AssetImportData->UpdateFilenameOnly(NewReimportPaths[0]);
	}
}

EReimportResult::Type UVtaTextureAtlasReimportFactory::Reimport(UObject* Obj)
{
	UVtaTextureAtlas* TextureAtlas = Cast<UVtaTextureAtlas>(Obj);
	if (!TextureAtlas)
	{
		UE_LOG(LogVaTexAtlasEditor, Error, TEXT("Texture atlas is not valid"));
		return EReimportResult::Failed;
	}

	// Make sure file is valid and exists
	const FString Filename = TextureAtlas->AssetImportData->GetFirstFilename();
	if (!Filename.Len() || IFileManager::Get().FileSize(*Filename) == INDEX_NONE)
	{
		UE_LOG(LogVaTexAtlasEditor, Error, TEXT("Filename is invalid: %s"), *Filename);
		return EReimportResult::Failed;
	}

	// Configure the importer with the reimport settings
	SetReimportData(TextureAtlas);
	ExistingAtlasTextureName = TextureAtlas->TextureName;
	ExistingAtlasTexture = TextureAtlas->Texture;

	// Run the import again
	EReimportResult::Type Result = EReimportResult::Failed;
	bool OutCanceled = false;

	if (ImportObject(TextureAtlas->GetClass(), TextureAtlas->GetOuter(), *TextureAtlas->GetName(), RF_Public | RF_Standalone, Filename, nullptr, OutCanceled) != nullptr)
	{
		UE_LOG(LogVaTexAtlasEditor, Log, TEXT("Imported successfully"));

		TextureAtlas->AssetImportData->Update(Filename);
		
		// Try to find the outer package so we can dirty it up
		if (TextureAtlas->GetOuter())
		{
			TextureAtlas->GetOuter()->MarkPackageDirty();
		}
		else
		{
			TextureAtlas->MarkPackageDirty();
		}

		Result = EReimportResult::Succeeded;
	}
	else
	{
		if (OutCanceled)
		{
			UE_LOG(LogVaTexAtlasEditor, Warning, TEXT("-- import canceled"));
		}
		else
		{
			UE_LOG(LogVaTexAtlasEditor, Warning, TEXT("-- import failed"));
		}

		Result = EReimportResult::Failed;
	}

	return Result;
}

int32 UVtaTextureAtlasReimportFactory::GetPriority() const
{
	return ImportPriority;
}

//////////////////////////////////////////////////////////////////////////

#undef LOCTEXT_NAMESPACE
