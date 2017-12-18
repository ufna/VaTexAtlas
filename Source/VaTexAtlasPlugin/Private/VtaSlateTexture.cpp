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

FSlateAtlasData UVtaSlateTexture::GetSlateAtlasData() const
{
	return FSlateAtlasData(AtlasTexture, StartUV, SizeUV);
}
