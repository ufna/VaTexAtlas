// Copyright 2016-2019 Vladimir Alyamkin. All Rights Reserved.

#include "VtaTextureAtlasDataModel.h"

#include "VtaEditorPlugin.h"

#define LOCTEXT_NAMESPACE "VtaEditorPlugin"

#define VTA_IMPORT_ERROR(FormatString, ...)                            \
	if (!bSilent)                                                      \
	{                                                                  \
		UE_LOG(LogVaTexAtlasEditor, Error, FormatString, __VA_ARGS__); \
	}
#define VTA_IMPORT_WARNING(FormatString, ...)                            \
	if (!bSilent)                                                        \
	{                                                                    \
		UE_LOG(LogVaTexAtlasEditor, Warning, FormatString, __VA_ARGS__); \
	}

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

	// An example:
	//   "size" : {"w":1024, "h" : 1024},

	// Try parsing the W
	if (!Tree->TryGetNumberField(TEXT("w"), W))
	{
		VTA_IMPORT_WARNING(TEXT("Expected a 'w' field in the Size object of '%s'."), *NameForErrors);
	}

	// Try parsing the H
	if (!Tree->TryGetNumberField(TEXT("h"), H))
	{
		VTA_IMPORT_WARNING(TEXT("Expected a 'h' field in the Size object of '%s'."), *NameForErrors);
	}

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

	// An example:
	//   {"x":693,"y":1,"w":325,"h":300},

	// Try parsing the X
	if (!Tree->TryGetNumberField(TEXT("x"), X))
	{
		VTA_IMPORT_WARNING(TEXT("Expected a 'x' field in the Region object of '%s'."), *NameForErrors);
	}

	// Try parsing the Y
	if (!Tree->TryGetNumberField(TEXT("y"), Y))
	{
		VTA_IMPORT_WARNING(TEXT("Expected a 'y' field in the Region object of '%s'."), *NameForErrors);
	}

	// Try parsing the W
	if (!Tree->TryGetNumberField(TEXT("w"), W))
	{
		VTA_IMPORT_WARNING(TEXT("Expected a 'w' field in the Region object of '%s'."), *NameForErrors);
	}

	// Try parsing the H
	if (!Tree->TryGetNumberField(TEXT("h"), H))
	{
		VTA_IMPORT_WARNING(TEXT("Expected a 'h' field in the Region object of '%s'."), *NameForErrors);
	}

	return bSuccessfullyParsed;
}

//////////////////////////////////////////////////////////////////////////
// FVtaFrame

bool FVtaFrame::ParseFromJSON(TSharedPtr<FJsonObject> Tree, const FString& NameForErrors, bool bSilent)
{
	bool bSuccessfullyParsed = true;

	// An example:
	//   "filename": "test_image",
	//   "frame": {"x":693,"y":1,"w":325,"h":300},
	//   "rotated": false,										[ignored]
	//   "trimmed": false,										[ignored]
	//   "spriteSourceSize": {"x":0,"y":0,"w":325,"h":300},		[ignored]
	//   "sourceSize": {"w":325,"h":300},						[ignored]
	//   "pivot": {"x":0.5,"y":0.5}								[ignored]

	// Try parsing the filename
	if (!Tree->TryGetStringField(TEXT("filename"), Filename))
	{
		Filename = TEXT("(missing filename)");
		VTA_IMPORT_ERROR(TEXT("Expected a 'filename' field in the Frame object of '%s'."), *NameForErrors);
		bSuccessfullyParsed = false;
	}
	else if (Filename.IsEmpty())
	{
		Filename = TEXT("(empty filename)");
		VTA_IMPORT_ERROR(TEXT("Expected a non-empty 'filename' field in the Frame object of '%s'."), *NameForErrors);
		bSuccessfullyParsed = false;
	}

	// Try parsing the frame region
	const TSharedPtr<FJsonObject>* RegionDescriptor;
	if (Tree->TryGetObjectField(TEXT("frame"), RegionDescriptor))
	{
		const bool bParsedEntityOK = Frame.ParseFromJSON(*RegionDescriptor, NameForErrors, bSilent);
		bSuccessfullyParsed = bSuccessfullyParsed && bParsedEntityOK;
	}
	else
	{
		VTA_IMPORT_ERROR(TEXT("Expected a 'frame' field in the Frame object of '%s'."), *NameForErrors);
		bSuccessfullyParsed = false;
	}

	return bSuccessfullyParsed;
}

//////////////////////////////////////////////////////////////////////////
// FVtaMeta

bool FVtaMeta::ParseFromJSON(TSharedPtr<FJsonObject> Tree, const FString& NameForErrors, bool bSilent)
{
	bool bSuccessfullyParsed = true;

	// An example:
	//   "app": "http://www.codeandweb.com/texturepacker",		[optional]
	//   "version" : "1.0",										[optional]
	//   "image" : "testatlas.png",
	//   "format" : "RGBA8888",									[optional, ignored]
	//   "size" : {"w":1024, "h" : 1024},						[optional, ignored]
	//   "scale" : "1",											[ignored]
	//   "smartupdate" : "..."									[ignored]

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
	if (Tree->TryGetObjectField(TEXT("meta"), MetaDescriptor))
	{
		const bool bParsedEntityOK = Meta.ParseFromJSON(*MetaDescriptor, NameForErrors, bSilent);
		bSuccessfullyParsed = bSuccessfullyParsed && bParsedEntityOK;
	}
	else
	{
		VTA_IMPORT_ERROR(TEXT("JSON exported from TexturePacker in file '%s' has no meta."), *NameForErrors);
		bSuccessfullyParsed = false;
	}

	// Just check that array field for frames exists
	if (bPreparseOnly)
	{
		if (!Tree->HasTypedField<EJson::Array>("frames"))
		{
			VTA_IMPORT_ERROR(TEXT("JSON exported from TexturePacker in file '%s' has no frames array."), *NameForErrors);
			bSuccessfullyParsed = false;
		}
	}
	else
	{
		// Load the rest of the data if we're doing a full parse
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
