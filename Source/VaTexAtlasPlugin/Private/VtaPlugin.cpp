// Copyright 2016 Vladimir Alyamkin. All Rights Reserved.

#include "VtaPlugin.h"

class FVaTexAtlasPlugin : public IVaTexAtlasPlugin
{
	/** IModuleInterface implementation */
	virtual void StartupModule() override
	{

	}

	virtual void ShutdownModule() override
	{

	}
};

IMPLEMENT_MODULE( FVaTexAtlasPlugin, VaTexAtlasPlugin )

DEFINE_LOG_CATEGORY(LogVaTexAtlas);
