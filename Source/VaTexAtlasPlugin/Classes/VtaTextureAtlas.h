// Copyright 2016 Vladimir Alyamkin. All Rights Reserved.

#pragma once

#include "VtaTextureAtlas.generated.h"

/**
 * 
 */
UCLASS()
class VATEXATLASPLUGIN_API UVtaTextureAtlas : public UObject
{
	GENERATED_UCLASS_BODY()


public:
	/** */
	UPROPERTY(EditAnywhere, Category = "Atlas Properties")
	FString Name;


};
