/*++

Copyright (c) 2016 Trent Nelson <trent@trent.me>

Module Name:

    stdafx.h

Abstract:

    This is the precompiled header file for the DebugEngine component.

--*/

#pragma once

#include "targetver.h"

#include <Windows.h>

#undef EXTERN_C
#define EXTERN_C
#pragma component(browser, off)
#include <DbgEng.h>
#pragma component(browser, on)

#include "../Rtl/Rtl.h"

#include "DebugEngine.h"

#ifdef _DEBUG_ENGINE_INTERNAL_BUILD
#include "DebugEnginePrivate.h"
#endif

// vim:set ts=8 sw=4 sts=4 tw=80 expandtab                                     :
