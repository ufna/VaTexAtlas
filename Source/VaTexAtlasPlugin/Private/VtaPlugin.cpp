// Copyright 2015-2017 Mail.Ru Group. All Rights Reserved.

#include "VtaPlugin.h"
#include "IVaTexAtlasPlugin.h"

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
