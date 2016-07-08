// Copyright 2016 Vladimir Alyamkin. All Rights Reserved.

#include "VtaEditorPlugin.h"
#include "VtaTextureAtlasDataModel.h"

#define LOCTEXT_NAMESPACE "VtaEditorPlugin"

//////////////////////////////////////////////////////////////////////////
// FVtaFrameRegion

FVtaFrameRegion::FVtaFrameRegion()
	: X(0)
	, Y(0)
	, W(0)
	, H(0)
{

}

bool FVtaFrameRegion::ParseFromJSON(TSharedPtr<FJsonObject> Tree, const FString& NameForErrors, bool bSilent)
{
	return false;
}


//////////////////////////////////////////////////////////////////////////
// FVtaFrame

bool FVtaFrame::ParseFromJSON(TSharedPtr<FJsonObject> Tree, const FString& NameForErrors, bool bSilent)
{
	return false;
}


//////////////////////////////////////////////////////////////////////////
// FVtaMeta

FVtaMeta::FVtaMeta()
	: Width(0)
	, Height(0)
{

}

bool FVtaMeta::ParseFromJSON(TSharedPtr<FJsonObject> Tree, const FString& NameForErrors, bool bSilent)
{
	return false;
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

	
}

bool FVtaDataFile::IsValid() const
{
	return bSuccessfullyParsed;
}


//////////////////////////////////////////////////////////////////////////

#undef LOCTEXT_NAMESPACE
