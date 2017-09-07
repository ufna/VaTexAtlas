// Copyright 2015-2017 Mail.Ru Group. All Rights Reserved.

#include "VtaPlugin.h"
#include "VtaSlateTexture.h"

UVtaSlateTexture::UVtaSlateTexture(const class FObjectInitializer& PCIP)
: Super(PCIP)
, AtlasTexture(nullptr)
, StartUV(0.f, 0.f)
, SizeUV(1.f, 1.f)
{
	
}

FSlateAtlasData UVtaSlateTexture::GetSlateAtlasData() const
{
	return FSlateAtlasData(AtlasTexture, StartUV, SizeUV);
}
