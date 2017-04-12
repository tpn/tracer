/*++

Copyright (c) 2017 Trent Nelson <trent@trent.me>

Module Name:

    stdafx.h

Abstract:

    This is the precompiled header file for the TracerCore component.

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
#include "TracerCore.h"

#ifdef _TRACER_CORE_INTERNAL_BUILD
#include "TracerCorePrivate.h"
#endif

#ifdef __cplusplus
} // extern "C"
#endif

// vim:set ts=8 sw=4 sts=4 tw=80 expandtab                                     :
