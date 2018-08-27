// Copyright 2016 Vladimir Alyamkin. All Rights Reserved.

#pragma once

#include "Json.h"

#include "VtaTextureAtlasDataModel.generated.h"

//////////////////////////////////////////////////////////////////////////
// FVtaJsonImportable

USTRUCT()
struct FVtaJsonImportable
{
	GENERATED_USTRUCT_BODY()

public:
	virtual bool ParseFromJSON(TSharedPtr<FJsonObject> Tree, const FString& NameForErrors, bool bSilent) PURE_VIRTUAL(FVtaJsonImportable::ParseFromJSON, return false;);
	virtual ~FVtaJsonImportable() {};

};


//////////////////////////////////////////////////////////////////////////
// FVtaSize

USTRUCT()
struct FVtaSize : public FVtaJsonImportable
{
	GENERATED_USTRUCT_BODY()

	UPROPERTY(VisibleAnywhere, Category = "VaTexAtlas")
	int32 W;

	UPROPERTY(VisibleAnywhere, Category = "VaTexAtlas")
	int32 H;

public:
	FVtaSize();

	bool ParseFromJSON(TSharedPtr<FJsonObject> Tree, const FString& NameForErrors, bool bSilent) override;

};


//////////////////////////////////////////////////////////////////////////
// FVtaRegion

USTRUCT()
struct FVtaRegion : public FVtaJsonImportable
{
	GENERATED_USTRUCT_BODY()

	UPROPERTY(VisibleAnywhere, Category = "VaTexAtlas")
	int32 X;

	UPROPERTY(VisibleAnywhere, Category = "VaTexAtlas")
	int32 Y;

	UPROPERTY(VisibleAnywhere, Category = "VaTexAtlas")
	int32 W;

	UPROPERTY(VisibleAnywhere, Category = "VaTexAtlas")
	int32 H;

public:
	FVtaRegion();

	bool ParseFromJSON(TSharedPtr<FJsonObject> Tree, const FString& NameForErrors, bool bSilent) override;

};


//////////////////////////////////////////////////////////////////////////
// FVtaFrame

USTRUCT()
struct FVtaFrame : public FVtaJsonImportable
{
public:
	GENERATED_USTRUCT_BODY()
	
	// "filename": "myimage", (should be trimmed!)
	UPROPERTY(VisibleAnywhere, Category = "VaTexAtlas")
	FString Filename;
	
	// "frame": {"x":387,"y":467,"w":300,"h":347},
	UPROPERTY(VisibleAnywhere, Category = "VaTexAtlas")
	FVtaRegion Frame;

public:
	bool ParseFromJSON(TSharedPtr<FJsonObject> Tree, const FString& NameForErrors, bool bSilent) override;

};


//////////////////////////////////////////////////////////////////////////
// FVtaMeta

USTRUCT()
struct FVtaMeta : public FVtaJsonImportable
{
public:
	GENERATED_USTRUCT_BODY()
		
	// "app": "http://www.codeandweb.com/texturepacker",
	UPROPERTY(VisibleAnywhere, Category = "VaTexAtlas")
	FString App;

	// "version": "1.0",
	UPROPERTY(VisibleAnywhere, Category = "VaTexAtlas")
	FString Version;

	// "image": "testatlas.png",
	UPROPERTY(VisibleAnywhere, Category = "VaTexAtlas")
	FString Image;

	// "format": "RGBA8888",
	UPROPERTY(VisibleAnywhere, Category = "VaTexAtlas")
	FString Format;

	// "size": {"w":1024,"h":1024},
	UPROPERTY(VisibleAnywhere, Category = "VaTexAtlas")
	FVtaSize Size;

public:
	bool ParseFromJSON(TSharedPtr<FJsonObject> Tree, const FString& NameForErrors, bool bSilent) override;

};


//////////////////////////////////////////////////////////////////////////
// FVtaDataFile

USTRUCT()
struct FVtaDataFile
{
public:
	GENERATED_USTRUCT_BODY()
		
	UPROPERTY(VisibleAnywhere, Category = "VaTexAtlas")
	FVtaMeta Meta;

	UPROPERTY(VisibleAnywhere, Category = "VaTexAtlas")
	TArray<FVtaFrame> Frames;

public:
	FVtaDataFile();

	void ParseFromJSON(TSharedPtr<FJsonObject> Tree, const FString& NameForErrors, bool bSilent, bool bPreparseOnly);
	bool IsValid() const;

protected:
	bool bSuccessfullyParsed;

};
