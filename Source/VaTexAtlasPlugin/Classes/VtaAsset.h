// Copyright 2015-2019 Mail.Ru Group. All Rights Reserved.

#pragma once

#include "VtaSlateTexture.h"

#include "CoreMinimal.h"
#include "Engine/DataTable.h"
#include "Materials/MaterialInstanceConstant.h"

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
