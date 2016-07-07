// Copyright 2016 Vladimir Alyamkin. All Rights Reserved.

#include "VtaEditorPlugin.h"

class FVaTexAtlasEditorPlugin : public IVaTexAtlasEditorPlugin
{
	/** IModuleInterface implementation */
	virtual void StartupModule() override
	{

	}

	virtual void ShutdownModule() override
	{

	}
};

IMPLEMENT_MODULE( FVaTexAtlasEditorPlugin, VaTexAtlasEditorPlugin )

DEFINE_LOG_CATEGORY(LogVaTexAtlasEditor);
