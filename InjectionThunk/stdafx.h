/*++

Copyright (c) 2017 Trent Nelson <trent@trent.me>

Module Name:

    stdafx.h

Abstract:

    This is the precompiled header file for the InjectionThunk component.

--*/

#pragma once

#ifdef __cplusplus
extern "C" {
#endif

#include "../Rtl/Rtl.h"
#include "../DebugEngine/DebugEngine.h"
#include "../DebugEngine/DebugEngineInterfaces.h"
#include "../TraceStore/TraceStore.h"
#include "../TracerHeap/TracerHeap.h"
#include "../TracerConfig/TracerConfig.h"
#include "InjectionThunk.h"

#ifdef _INJECTION_THUNK_INTERNAL_BUILD
#include "InjectionThunkPrivate.h"
#endif

#ifdef __cplusplus
} // extern "C"
#endif

// vim:set ts=8 sw=4 sts=4 tw=80 expandtab                                     :
