// Copyright 2015-2019 Mail.Ru Group. All Rights Reserved.

#include "VtaPluginFunctionLibrary.h"

#include "VtaPlugin.h"
#include "VtaSlateTexture.h"

#include "GameFramework/HUD.h"

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

void UVtaPluginFunctionLibrary::DrawSlateTexture9Slice(AHUD* HUD, UVtaSlateTexture* Texture, float ScreenX, float ScreenY, float ScreenW, float ScreenH, float TextureU, float TextureV, float TextureUWidth, float TextureVHeight, float MarginLeft, float MarginTop, float MarginRight, float MarginBottom, float OriginalWidth, float OriginalHeight, FLinearColor TintColor, EBlendMode BlendMode)
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

	// recalculate UV of our texture for the atlas UV coordinates
	const float TextureULeft = Texture->StartUV.X + TextureU * Texture->SizeUV.X;  // the very left U coordinate of our texture
	const float TextureVTop = Texture->StartUV.Y + TextureV * Texture->SizeUV.Y;   // the very top V coordinate of our texture
	const float TextureURight = TextureULeft + TextureUWidth * Texture->SizeUV.X;  // the very right U coordinate of our texture
	const float TextureVBottom = TextureVTop + TextureVHeight * Texture->SizeUV.Y; // the very bottom V coordinate of our texture

	// texture original size
	FVector2D OriginalSize = Texture->GetDimensions();
	if (OriginalWidth > 0.0f)
	{
		OriginalSize.X = OriginalWidth;
	}

	if (OriginalHeight > 0.0f)
	{
		OriginalSize.Y = OriginalHeight;
	}

	// minimal possible final size without cropping (the sum of the margin sizes)
	const FVector2D MinimumSize(OriginalSize.X * (MarginLeft + MarginRight), OriginalSize.Y * (MarginTop + MarginBottom));

	// middle UV coordinates of the quads
	float MiddleU1 = MarginLeft;		  // the right border of the left margin
	float MiddleV1 = MarginTop;			  // the bottom border of the top margin
	float MiddleU2 = 1.0f - MarginRight;  // the left border of the right margin
	float MiddleV2 = 1.0f - MarginBottom; // the top border of the bottom margin

	// should we skip center parts (if they have zero or negative size)
	const bool IsCroppedX = (ScreenW <= MinimumSize.X);
	const bool IsCroppedY = (ScreenH <= MinimumSize.Y);

	// adjust UV for cropped margins (if the size less than minimal)
	if (IsCroppedX && MinimumSize.X != 0.0f)
	{
		float ExtraWidthPart = (MinimumSize.X - ScreenW) / MinimumSize.X;
		MiddleU1 = MarginLeft * (1.0 - ExtraWidthPart);
		MiddleU2 = 1.0f - (MarginRight * (1.0 - ExtraWidthPart));
	}

	if (IsCroppedY && MinimumSize.Y != 0.0f)
	{
		float ExtraHeightPart = (MinimumSize.Y - ScreenH) / MinimumSize.Y;
		MiddleV1 = MarginTop * (1.0 - ExtraHeightPart);
		MiddleV2 = 1.0f - (MarginTop * (1.0 - ExtraHeightPart));
	}

	// recalculate our middle UV coordinates for the atlas UV coordinates
	MiddleU1 = Texture->StartUV.X + MiddleU1 * Texture->SizeUV.X;
	MiddleV1 = Texture->StartUV.Y + MiddleV1 * Texture->SizeUV.Y;
	MiddleU2 = Texture->StartUV.X + MiddleU2 * Texture->SizeUV.X;
	MiddleV2 = Texture->StartUV.Y + MiddleV2 * Texture->SizeUV.Y;

	//  _____________
	// |_lt_|_t_|_rt_|
	// |_l__|_c_|_r__|
	// |_lb_|_b_|_rb_|
	//
	// U coordinates order from left to right (MiddleU1 and MiddleU2 can be equal)
	// TextureULeft => MiddleU1 => MiddleU2 => TextureURight
	//
	// V coordinates order from top to bottom (MiddleV1 and MiddleV2 can be equal)
	// TextureVTop => MiddleV1 => MiddleV2 => TextureVBottom
	//
	// X coordinates order from left to right
	// ScreenX => MiddleX1 => MiddleX2
	//
	// Y coordinates order from top to bottom
	// ScreenY => MiddleY1 => MiddleY2

	// UV heights and weights of the parts
	const float LeftUWidth = MiddleU1 - TextureULeft;
	const float CenterUWidth = MiddleU2 - MiddleU1;
	const float RightUWidth = TextureURight - MiddleU2;
	const float TopVHeight = MiddleV1 - TextureVTop;
	const float CenterVHeight = MiddleV2 - MiddleV1;
	const float BottomVHeight = TextureVBottom - MiddleV2;

	// screen widths and heights
	float LeftScreenWidth = OriginalSize.X * MarginLeft;
	float RightScreenWidth = OriginalSize.X * MarginRight;
	const float CenterScreenWidth = ScreenW - (LeftScreenWidth + RightScreenWidth);
	float TopScreenHeight = OriginalSize.Y * MarginTop;
	float BottomScreenHeight = OriginalSize.Y * MarginBottom;
	const float CenterScreenHeight = ScreenH - (TopScreenHeight + BottomScreenHeight);

	// ajust the sizes for cropped margins (if the size less than minimal)
	if (IsCroppedX && MinimumSize.X != 0.0f)
	{
		float WidthMultiplier = ScreenW / MinimumSize.X;
		LeftScreenWidth *= WidthMultiplier;
		RightScreenWidth *= WidthMultiplier;
	}

	if (IsCroppedY && MinimumSize.Y != 0.0f)
	{
		float HeightMultiplier = ScreenH / MinimumSize.Y;
		TopScreenHeight *= HeightMultiplier;
		BottomScreenHeight *= HeightMultiplier;
	}

	// screen coordinates
	const float MiddleX1 = ScreenX + LeftScreenWidth;
	const float MiddleY1 = ScreenY + TopScreenHeight;
	const float MiddleX2 = ScreenX + ScreenW - RightScreenWidth;
	const float MiddleY2 = ScreenY + ScreenH - BottomScreenHeight;

	// lt
	HUD->DrawTexture(Texture->AtlasTexture, ScreenX, ScreenY, LeftScreenWidth, TopScreenHeight, TextureULeft, TextureVTop, LeftUWidth, TopVHeight, TintColor, BlendMode, 1.0f, false);

	// t
	if (!IsCroppedX)
	{
		HUD->DrawTexture(Texture->AtlasTexture, MiddleX1, ScreenY, CenterScreenWidth, TopScreenHeight, MiddleU1, TextureVTop, CenterUWidth, TopVHeight, TintColor, BlendMode, 1.0f, false);
	}

	// rt
	HUD->DrawTexture(Texture->AtlasTexture, MiddleX2, ScreenY, RightScreenWidth, TopScreenHeight, MiddleU2, TextureVTop, RightUWidth, TopVHeight, TintColor, BlendMode, 1.0f, false);

	if (!IsCroppedY)
	{
		// l
		HUD->DrawTexture(Texture->AtlasTexture, ScreenX, MiddleY1, LeftScreenWidth, CenterScreenHeight, TextureULeft, MiddleV1, LeftUWidth, CenterVHeight, TintColor, BlendMode, 1.0f, false);

		// c
		if (!IsCroppedX)
		{
			HUD->DrawTexture(Texture->AtlasTexture, MiddleX1, MiddleY1, CenterScreenWidth, CenterScreenHeight, MiddleU1, MiddleV1, CenterUWidth, CenterVHeight, TintColor, BlendMode, 1.0f, false);
		}

		// r
		HUD->DrawTexture(Texture->AtlasTexture, MiddleX2, MiddleY1, RightScreenWidth, CenterScreenHeight, MiddleU2, MiddleV1, RightUWidth, CenterVHeight, TintColor, BlendMode, 1.0f, false);
	}

	// lb
	HUD->DrawTexture(Texture->AtlasTexture, ScreenX, MiddleY2, LeftScreenWidth, BottomScreenHeight, TextureULeft, MiddleV2, LeftUWidth, BottomVHeight, TintColor, BlendMode, 1.0f, false);

	// b
	if (!IsCroppedX)
	{
		HUD->DrawTexture(Texture->AtlasTexture, MiddleX1, MiddleY2, CenterScreenWidth, BottomScreenHeight, MiddleU1, MiddleV2, CenterUWidth, BottomVHeight, TintColor, BlendMode, 1.0f, false);
	}

	// rb
	HUD->DrawTexture(Texture->AtlasTexture, MiddleX2, MiddleY2, RightScreenWidth, BottomScreenHeight, MiddleU2, MiddleV2, RightUWidth, BottomVHeight, TintColor, BlendMode, 1.0f, false);
}

FSlateBrush UVtaPluginFunctionLibrary::CopyBrushWithSlateTexture(UVtaSlateTexture* Texture, const FSlateBrush& SourceBrush)
{
	FSlateBrush Result = SourceBrush;
	Result.SetResourceObject(Texture);

	return Result;
}
