// Copyright 2016 Vladimir Alyamkin. All Rights Reserved.

#include "VtaEditorPlugin.h"

UVtaTextureAtlasFactory::UVtaTextureAtlasFactory(const class FObjectInitializer& PCIP)
	: Super(PCIP)
{

}

UObject* UVtaTextureAtlasFactory::FactoryCreateNew(UClass* Class, UObject* InParent, FName Name, EObjectFlags Flags, UObject* Context, FFeedbackContext* Warn)
{
	UVtaTextureAtlas* NewObjectAsset = NewObject<UVtaTextureAtlas>(InParent, Class, Name, Flags | RF_Transactional);
	
	return NewObjectAsset;
}
