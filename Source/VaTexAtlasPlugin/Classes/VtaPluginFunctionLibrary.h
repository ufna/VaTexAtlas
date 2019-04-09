// Copyright 2015-2019 Mail.Ru Group. All Rights Reserved.

#pragma once

#include "Engine/EngineTypes.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "Styling/SlateBrush.h"

#include "VtaPluginFunctionLibrary.generated.h"

UCLASS()
class VATEXATLASPLUGIN_API UVtaPluginFunctionLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable, Category = "HUD", meta = (DefaultToSelf = "Target", AdvancedDisplay = "6"))
	static void DrawSlateTexture(class AHUD* Target, class UVtaSlateTexture* Texture, float ScreenX = 0.f, float ScreenY = 0.f, float ScreenW = 100.f, float ScreenH = 100.f, float TextureU = 0.f, float TextureV = 0.f, float TextureUWidth = 1.f, float TextureVHeight = 1.f, FLinearColor Tint = FLinearColor::White, EBlendMode BlendMode = BLEND_Translucent, float Scale = 1.f, bool bScalePosition = false, float Rotation = 0.f, FVector2D RotPivot = FVector2D::ZeroVector, bool bFlipX = false, bool bFlipY = false);

	/**
	 * Draws texture as 3x3 grid where the sides stretch horizontally and vertically and the middle stretches to fill.
	 * The corners will not be stretched. The size of the sides and corners is determined by the margins as follows:
	 *
	 *                 _____________________
	 *                |  |   MarginTop   |  |
	 *                |__|_______________|__|   MarginRight
	 *                |  |               |  |  /
	 *              +--> |               | <--+
	 *             /  |__|_______________|__|
	 *  MarginLeft    |  | MarginBottom  |  |
	 *                |__|_______________|__|
	 *
	 * If the size of the resulting image is less than percentage of MarginLeft+MarginRight for width or MarginTop+MarginBottom
	 * then the central part of the borders will be cutted off (saving the proportions and the original scale).
	 *
	 * @param MarginLeft - Left margin for 9Slice. should be from 0.0 to 1.0 (sum of MarginLeft and MarginRight sholdn't be more than 1.0)
	 * @param MarginTop - Top margin for 9Slice. should be from 0.0 to 1.0 (sum of MarginTop and MarginBottom sholdn't be more than 1.0)
	 * @param MarginRight - Right margin for 9Slice. should be from 0.0 to 1.0 (sum of MarginLeft and MarginRight sholdn't be more than 1.0)
	 * @param MarginBottom - Bottom margin for 9Slice. should be from 0.0 to 1.0 (sum of MarginTop and MarginBottom sholdn't be more than 1.0)
	 * @param OriginalWidth - Original width of the texture (if zero, will be taken original texture size). Better to set explicitly to avoid scaling of the corners on changing texture resolution.
	 * @param OriginalHeight - Original height of the texture (if zero, will be taken original texture size). Better to set explicitly to avoid scaling of the corners on changing texture resolution.
	 */
	UFUNCTION(BlueprintCallable, Category = "HUD", meta = (DefaultToSelf = "Target", AdvancedDisplay = "6"))
	static void DrawSlateTexture9Slice(class AHUD* Target, class UVtaSlateTexture* Texture, float ScreenX = 0.f, float ScreenY = 0.f, float ScreenW = 100.f, float ScreenH = 100.f, float TextureU = 0.f, float TextureV = 0.f, float TextureUWidth = 1.f, float TextureVHeight = 1.f, float MarginLeft = 0.f, float MarginTop = 0.f, float MarginRight = 0.f, float MarginBottom = 0.f, float OriginalWidth = 0.f, float OriginalHeight = 0.f, FLinearColor Tint = FLinearColor::White, EBlendMode BlendMode = BLEND_Translucent);

	UFUNCTION(BlueprintPure, Category = "UMG")
	static FSlateBrush CopyBrushWithSlateTexture(class UVtaSlateTexture* Texture, const FSlateBrush& SourceBrush);
};
