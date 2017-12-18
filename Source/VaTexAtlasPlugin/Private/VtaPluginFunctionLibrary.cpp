// Copyright 2015-2017 Mail.Ru Group. All Rights Reserved.

#include "VtaPluginFunctionLibrary.h"
#include "VtaPlugin.h"
#include "GameFramework/HUD.h"
#include "VtaSlateTexture.h"

void UVtaPluginFunctionLibrary::DrawSlateTexture(AHUD* HUD, UVtaSlateTexture* Texture, float ScreenX, float ScreenY, float ScreenW, float ScreenH, float TextureU, float TextureV, float TextureUWidth, float TextureVHeight, FLinearColor TintColor, EBlendMode BlendMode, float Scale, bool bScalePosition, float Rotation, FVector2D RotPivot, bool bFlipX, bool bFlipY)
{
	if (!HUD /*|| !HUD->IsValidLowLevel()*/)
	{
		UE_LOG(LogVaTexAtlas, Error, TEXT("%s: HUD is invalid"), *VA_FUNC_LINE);
		return;
	}
	
	if (!Texture /*|| !Texture->IsValidLowLevel()*/)
	{
		UE_LOG(LogVaTexAtlas, Error, TEXT("%s: Texture is invalid"), *VA_FUNC_LINE);
		return;
	}

	if (!Texture->AtlasTexture /*|| !Texture->AtlasTexture->IsValidLowLevel()*/)
	{
		UE_LOG(LogVaTexAtlas, Error, TEXT("%s: AtlasTexture is invalid"), *VA_FUNC_LINE);
		return;
	}

	TextureU = Texture->StartUV.X + TextureU * Texture->SizeUV.X;
	TextureV = Texture->StartUV.Y + TextureV * Texture->SizeUV.Y;
	TextureUWidth = TextureUWidth * Texture->SizeUV.X;
	TextureVHeight = TextureVHeight * Texture->SizeUV.Y;
	
	if (bFlipX)
	{
		TextureU += TextureUWidth;
		TextureUWidth *= -1.f;
	}
	
	if (bFlipY)
	{
		TextureV += TextureVHeight;
		TextureVHeight *= -1.f;
	}
		
	HUD->DrawTexture(Texture->AtlasTexture, ScreenX, ScreenY, ScreenW, ScreenH, TextureU, TextureV, TextureUWidth, TextureVHeight, TintColor, BlendMode, Scale, bScalePosition, Rotation, RotPivot);
}

FSlateBrush UVtaPluginFunctionLibrary::CopyBrushWithSlateTexture(UVtaSlateTexture* Texture, const FSlateBrush& SourceBrush)
{
	FSlateBrush Result = SourceBrush;
	Result.SetResourceObject(Texture);	
	return Result;
}
