// Copyright 2015-2017 Mail.Ru Group. All Rights Reserved.

#include "VtaEditorPlugin.h"

#include "VtaSlateTextureThumbnailRenderer.h"
#include "VtaSlateTexture.h"

#include "CanvasItem.h"
#include "Engine/Engine.h"
#include "Engine/Texture.h"
#include "Engine/Texture2D.h"
#include "ThumbnailRendering/ThumbnailManager.h"
#include "EngineGlobals.h"

#include "CanvasTypes.h"

UVtaSlateTextureThumbnailRenderer::UVtaSlateTextureThumbnailRenderer(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	
}

void UVtaSlateTextureThumbnailRenderer::GetThumbnailSize(UObject* Object, float Zoom, uint32& OutWidth, uint32& OutHeight) const
{
	UVtaSlateTexture* SlateTexture = Cast<UVtaSlateTexture>(Object);

	if (SlateTexture != nullptr && SlateTexture->AtlasTexture)
	{
		OutWidth = FMath::TruncToInt(Zoom * (float)SlateTexture->AtlasTexture->GetSurfaceWidth() * SlateTexture->SizeUV.X);
		OutHeight = FMath::TruncToInt(Zoom * (float)SlateTexture->AtlasTexture->GetSurfaceHeight() * SlateTexture->SizeUV.Y);
	}
	else
	{
		OutWidth = OutHeight = 0;
	}
}

void UVtaSlateTextureThumbnailRenderer::Draw(UObject* Object, int32 X, int32 Y, uint32 Width, uint32 Height, FRenderTarget*, FCanvas* Canvas)
{
	UVtaSlateTexture* SlateTexture = Cast<UVtaSlateTexture>(Object);

	if (SlateTexture != nullptr && SlateTexture->AtlasTexture != nullptr && SlateTexture->AtlasTexture->Resource != nullptr)
	{
		UTexture2D* Texture2D = Cast<UTexture2D>(SlateTexture->AtlasTexture);
		if (Texture2D != nullptr)
		{
			const bool bUseTranslucentBlend = Texture2D->HasAlphaChannel();
			
			if (bUseTranslucentBlend)
			{
				// If using alpha, draw a checkerboard underneath first.
				const int32 CheckerDensity = 8;
				auto* Checker = UThumbnailManager::Get().CheckerboardTexture;
				Canvas->DrawTile(0.0f, 0.0f, Width, Height, 0.0f, 0.0f, CheckerDensity, CheckerDensity, FLinearColor::White, Checker->Resource);
			}
			
			FCanvasTileItem CanvasTile(FVector2D(X, Y), Texture2D->Resource, FVector2D(Width, Height), SlateTexture->StartUV, SlateTexture->StartUV + SlateTexture->SizeUV, FLinearColor::White);
			CanvasTile.BlendMode = bUseTranslucentBlend ? SE_BLEND_Translucent : SE_BLEND_Opaque;
			CanvasTile.Draw(Canvas);
		}
	}
}
