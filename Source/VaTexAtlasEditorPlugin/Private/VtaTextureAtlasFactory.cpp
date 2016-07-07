// Copyright 2016 Vladimir Alyamkin. All Rights Reserved.

#include "VtaEditorPlugin.h"

UVtaTextureAtlasFactory::UVtaTextureAtlasFactory(const class FObjectInitializer& PCIP)
	: Super(PCIP)
{
	SupportedClass = UVtaTextureAtlas::StaticClass();
	bCreateNew = true;
	bEditAfterNew = true;
}

UObject* UVtaTextureAtlasFactory::FactoryCreateNew(UClass* Class, UObject* InParent, FName Name, EObjectFlags Flags, UObject* Context, FFeedbackContext* Warn)
{
	check(Class->IsChildOf(UVtaTextureAtlas::StaticClass()));
	return NewObject<UVtaTextureAtlas>(InParent, Class, Name, Flags);
}

bool UVtaTextureAtlasFactory::CanCreateNew() const
{
	return true;
}
