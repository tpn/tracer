/*++

Copyright (c) 2017 Trent Nelson <trent@trent.me>

Module Name:

    stdafx.h

Abstract:

    This is the precompiled header file for the DebugEngine component.

--*/

#pragma once

#ifdef __cplusplus
extern "C" {
#endif

#include "targetver.h"

#include <Windows.h>
#include "../Rtl/Rtl.h"
#include "../StringTable/StringTable.h"
#include "DebugEngine.h"

#ifdef _DEBUG_ENGINE_INTERNAL_BUILD

#pragma component(browser, off)
#include <DbgEng.h>
#pragma component(browser, on)

#include "../TracerConfig/TracerConfig.h"
#include "DebugEngineInterfaces.h"
#include "DebugEnginePrivate.h"
#include "DebugEngineConstants.h"
#include "DebugEngineCommands.h"

#endif // _DEBUG_ENGINE_INTERNAL_BUILD

#ifdef __cplusplus
} // extern "C"
#endif

// vim:set ts=8 sw=4 sts=4 tw=80 expandtab                                     :
