// Copyright 2016 Vladimir Alyamkin. All Rights Reserved.

#pragma once

#include "VtaTextureAtlasFactory.generated.h"

/**
 * 
 */
UCLASS()
class VATEXATLASEDITORPLUGIN_API UVtaTextureAtlasFactory : public UFactory
{
	GENERATED_UCLASS_BODY()

	// Begin UFactory interface
	virtual UObject* FactoryCreateNew(UClass* Class, UObject* InParent, FName Name, EObjectFlags Flags, UObject* Context, FFeedbackContext* Warn) override;
	virtual bool CanCreateNew() const override;
	// End UFactory interface

};
