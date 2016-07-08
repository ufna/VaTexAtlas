// Copyright 2016 Vladimir Alyamkin. All Rights Reserved.

#include "VtaEditorPlugin.h"
#include "VtaTextureAtlasDataModel.h"

#define LOCTEXT_NAMESPACE "VtaEditorPlugin"

#define VTA_IMPORT_ERROR(FormatString, ...) \
	if (!bSilent) { UE_LOG(LogVaTexAtlasEditor, Error, FormatString, __VA_ARGS__); }
#define VTA_IMPORT_WARNING(FormatString, ...) \
	if (!bSilent) { UE_LOG(LogVaTexAtlasEditor, Warning, FormatString, __VA_ARGS__); }


//////////////////////////////////////////////////////////////////////////
// FVtaSize

FVtaSize::FVtaSize()
	: W(0)
	, H(0)
{

}

bool FVtaSize::ParseFromJSON(TSharedPtr<FJsonObject> Tree, const FString& NameForErrors, bool bSilent)
{
	bool bSuccessfullyParsed = true;

	// @TODO

	return bSuccessfullyParsed;
}


//////////////////////////////////////////////////////////////////////////
// FVtaRegion

FVtaRegion::FVtaRegion()
	: X(0)
	, Y(0)
	, W(0)
	, H(0)
{

}

bool FVtaRegion::ParseFromJSON(TSharedPtr<FJsonObject> Tree, const FString& NameForErrors, bool bSilent)
{
	bool bSuccessfullyParsed = true;

	// @TODO

	return bSuccessfullyParsed;
}


//////////////////////////////////////////////////////////////////////////
// FVtaFrame

bool FVtaFrame::ParseFromJSON(TSharedPtr<FJsonObject> Tree, const FString& NameForErrors, bool bSilent)
{
	bool bSuccessfullyParsed = true;

	// @TODO

	return bSuccessfullyParsed;
}


//////////////////////////////////////////////////////////////////////////
// FVtaMeta

bool FVtaMeta::ParseFromJSON(TSharedPtr<FJsonObject> Tree, const FString& NameForErrors, bool bSilent)
{
	bool bSuccessfullyParsed = true;

	// Try parsing the generator app
	if (!Tree->TryGetStringField(TEXT("app"), App))
	{
		App = TEXT("(missing app)");
		VTA_IMPORT_WARNING(TEXT("Expected a 'app' field in the meta object of '%s'."), *NameForErrors);
	}

	// Try parsing the version
	if (!Tree->TryGetStringField(TEXT("version"), Version))
	{
		Version = TEXT("(missing version)");
		VTA_IMPORT_WARNING(TEXT("Expected a 'version' field in the meta object of '%s'."), *NameForErrors);
	}

	// Validate the TexturePacker version
	const FString ExpectedVersion(TEXT("1.0"));
	if (Version != ExpectedVersion)
	{
		// Not 100% we can handle it but we'll try
		VTA_IMPORT_WARNING(TEXT("Unknown 'version' '%s' (expected '%s') VTA file '%s'. Parsing will continue but the format may not be fully supported"), *Version, *ExpectedVersion, *NameForErrors);
	}

	// Validate the generator app
	const FString TexturePackerApp(TEXT("http://www.codeandweb.com/texturepacker"));
	if (App == TexturePackerApp)
	{
		// Cool, we (mostly) know how to handle these sorts of files!
		if (!bSilent)
		{
			UE_LOG(LogVaTexAtlasEditor, Log, TEXT("Parsing texture atlas VTA v%s exported from '%s'"), *Version, *App);
		}
	}
	else if (!App.IsEmpty())
	{
		// Not 100% we can handle it but we'll try
		VTA_IMPORT_WARNING(TEXT("Unexpected 'app' named '%s' while parsing VTA v%s file '%s'. Parsing will continue but the format may not be fully supported"), *App, *Version, *NameForErrors);
	}

	// Image filename
	if (!Tree->TryGetStringField(TEXT("image"), Image))
	{
		Image = TEXT("(missing image)");
		VTA_IMPORT_ERROR(TEXT("Expected a 'image' field in the meta object of '%s'."), *NameForErrors);
		bSuccessfullyParsed = false;
	}

	// Image format
	if (!Tree->TryGetStringField(TEXT("format"), Format))
	{
		Image = TEXT("(missing format)");
		VTA_IMPORT_ERROR(TEXT("Expected a 'image' field in the meta object of '%s'."), *NameForErrors);
		bSuccessfullyParsed = false;
	}

	// Image size
	const TSharedPtr<FJsonObject>* SizeDescriptor;
	if (Tree->TryGetObjectField(TEXT("size"), SizeDescriptor))
	{
		const bool bParsedEntityOK = Size.ParseFromJSON(*SizeDescriptor, NameForErrors, bSilent);
		bSuccessfullyParsed = bSuccessfullyParsed && bParsedEntityOK;
	}
	else
	{
		VTA_IMPORT_ERROR(TEXT("Expected a 'size' field in the meta object of '%s'."), *NameForErrors);
		bSuccessfullyParsed = false;
	}

	return bSuccessfullyParsed;
}


//////////////////////////////////////////////////////////////////////////
// FVtaDataFile

FVtaDataFile::FVtaDataFile()
	: bSuccessfullyParsed(false)
{
}

void FVtaDataFile::ParseFromJSON(TSharedPtr<FJsonObject> Tree, const FString& NameForErrors, bool bSilent, bool bPreparseOnly)
{
	bSuccessfullyParsed = true;

	// Parse the meta object
	const TSharedPtr<FJsonObject>* MetaDescriptor;
	if(Tree->TryGetObjectField(TEXT("meta"), MetaDescriptor))
	{
		const bool bParsedEntityOK = Meta.ParseFromJSON(*MetaDescriptor, NameForErrors, bSilent);
		bSuccessfullyParsed = bSuccessfullyParsed && bParsedEntityOK;
	}
	else
	{
		VTA_IMPORT_ERROR(TEXT("JSON exported from TexturePacker in file '%s' has no meta."), *NameForErrors);
		bSuccessfullyParsed = false;
	}

	// Load the rest of the data if we're doing a full parse
	if (!bPreparseOnly)
	{
		// Parse the entities array
		const TArray<TSharedPtr<FJsonValue>>* FramesDescriptors;
		if (Tree->TryGetArrayField(TEXT("frames"), FramesDescriptors))
		{
			for (TSharedPtr<FJsonValue> FrameDescriptor : *FramesDescriptors)
			{
				FVtaFrame& Frame = *new (Frames) FVtaFrame();
				const bool bParsedEntityOK = Frame.ParseFromJSON(FrameDescriptor->AsObject(), NameForErrors, bSilent);
				bSuccessfullyParsed = bSuccessfullyParsed && bParsedEntityOK;
			}
		}
		else
		{
			VTA_IMPORT_ERROR(TEXT("JSON exported from TexturePacker in file '%s' has no frames."), *NameForErrors);
			bSuccessfullyParsed = false;
		}
	}
}

bool FVtaDataFile::IsValid() const
{
	return bSuccessfullyParsed;
}


//////////////////////////////////////////////////////////////////////////

#undef LOCTEXT_NAMESPACE
