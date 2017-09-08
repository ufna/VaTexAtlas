// Copyright 2015-2017 Mail.Ru Group. All Rights Reserved.

#pragma once

#include "Kismet/BlueprintFunctionLibrary.h"

#include "GameFramework/HUD.h"
#include "VtaSlateTexture.h"

#include "VtaPluginFunctionLibrary.generated.h"

UCLASS()
class VATEXATLASPLUGIN_API UVtaPluginFunctionLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()
	
public:
	
	UFUNCTION(BlueprintCallable, Category="HUD", meta=(DefaultToSelf = "Target", AdvancedDisplay = "6"))
	static void DrawSlateTexture(AHUD* Target, UVtaSlateTexture* Texture, float ScreenX = 0.f, float ScreenY = 0.f, float ScreenW = 100.f, float ScreenH = 100.f, float TextureU = 0.f, float TextureV = 0.f, float TextureUWidth = 1.f, float TextureVHeight = 1.f, FLinearColor Tint = FLinearColor::White, EBlendMode BlendMode = BLEND_Translucent, float Scale = 1.f, bool bScalePosition = false, float Rotation = 0.f, FVector2D RotPivot = FVector2D::ZeroVector);
	
};
