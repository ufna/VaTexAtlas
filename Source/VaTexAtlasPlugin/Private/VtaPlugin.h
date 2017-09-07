// Copyright 2015-2017 Mail.Ru Group. All Rights Reserved.

#pragma once

#include "Runtime/Launch/Resources/Version.h"

#if ENGINE_MINOR_VERSION >= 15
#include "CoreMinimal.h"
#else
#include "CoreUObject.h"
#include "Engine.h"
#endif

// You should place include statements to your module's private header files here.  You only need to
// add includes for headers that are used in most of your module's source files though.
#include "ModuleManager.h"

DECLARE_LOG_CATEGORY_EXTERN(LogVaTexAtlas, Log, All);

#include "IVaTexAtlasPlugin.h"

#include "VaTexAtlasPluginClasses.h"

