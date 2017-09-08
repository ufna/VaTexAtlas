// Copyright 2015-2017 Mail.Ru Group. All Rights Reserved.

#include "VtaPlugin.h"
#include "VtaPluginFunctionLibrary.h"

void UVtaPluginFunctionLibrary::DrawSlateTexture(AHUD* HUD, UVtaSlateTexture* Texture, float ScreenX, float ScreenY, float ScreenW, float ScreenH, float TextureU, float TextureV, float TextureUWidth, float TextureVHeight, FLinearColor TintColor, EBlendMode BlendMode, float Scale, bool bScalePosition, float Rotation, FVector2D RotPivot)
{
	if (HUD)
	{
		TextureU = Texture->StartUV.X + TextureU * Texture->SizeUV.X;
		TextureV = Texture->StartUV.Y + TextureV * Texture->SizeUV.Y;
		TextureUWidth = TextureUWidth * Texture->SizeUV.X;
		TextureVHeight = TextureVHeight * Texture->SizeUV.Y;
		
		HUD->DrawTexture(Texture->AtlasTexture, ScreenX, ScreenY, ScreenW, ScreenH, TextureU, TextureV, TextureUWidth, TextureVHeight, TintColor, BlendMode, Scale, bScalePosition, Rotation, RotPivot);
	}
}


