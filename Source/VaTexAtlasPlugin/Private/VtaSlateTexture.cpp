// Copyright 2015-2017 Mail.Ru Group. All Rights Reserved.

#include "VtaSlateTexture.h"
#include "VtaPlugin.h"

UVtaSlateTexture::UVtaSlateTexture(const class FObjectInitializer& PCIP)
: Super(PCIP)
, AtlasTexture(nullptr)
, StartUV(0.f, 0.f)
, SizeUV(1.f, 1.f)
{
	
}

FVector2D UVtaSlateTexture::GetDimensions() const
{
	return GetSlateAtlasData().GetSourceDimensions();
}

UVtaSlateTexture * UVtaSlateTexture::CreateRegion(const FMargin& Margin)
{
	UVtaSlateTexture* NewTexture = DuplicateObject(this, nullptr);
	if (NewTexture) 
	{
		NewTexture->StartUV.X = StartUV.X + SizeUV.X * Margin.Left;
		NewTexture->StartUV.Y = StartUV.Y + SizeUV.Y * Margin.Top;
		FVector2D NewEndUV = StartUV + SizeUV;
		NewEndUV.X = NewEndUV.X - SizeUV.X * Margin.Right;
		NewEndUV.Y = NewEndUV.Y - SizeUV.Y * Margin.Bottom;
		NewTexture->SizeUV = NewEndUV - NewTexture->StartUV;
	}
	return NewTexture;
}

FSlateAtlasData UVtaSlateTexture::GetSlateAtlasData() const
{
	return FSlateAtlasData(AtlasTexture, StartUV, SizeUV);
}
