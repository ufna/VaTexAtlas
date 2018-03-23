// Copyright 2015-2017 Mail.Ru Group. All Rights Reserved.

#pragma once

#include "Materials/MaterialInstanceConstant.h"
#include "Engine/DataTable.h"
#include "VtaSlateTexture.h"
#include "CoreMinimal.h"

#include "VtaAsset.generated.h"

USTRUCT(BlueprintType)
struct VATEXATLASPLUGIN_API FVtaAsset : public FTableRowBase
{
public:
	GENERATED_USTRUCT_BODY()
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "VaTexAtlas")
	TSoftObjectPtr<UMaterialInstanceConstant> Material;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "VaTexAtlas")
	TSoftObjectPtr<UVtaSlateTexture> SlateTexture;
	
	FVtaAsset();
};
