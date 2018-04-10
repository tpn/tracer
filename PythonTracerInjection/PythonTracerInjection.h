/*++

Copyright (c) 2017 Trent Nelson <trent@trent.me>

Module Name:

    PythonTracerInjection.h

Abstract:

    This is the main header file for the PythonTracerInjection component.
    This component is responsible for provided the necessary injection
    facilities for Python tracing.

--*/

#pragma once

#ifdef __cplusplus
extern "C" {
#endif

#ifdef _PYTHON_TRACER_INJECTION_INTERNAL_BUILD

//
// This is an internal build of the PythonTracerInjection component.
//

#ifdef _PYTHON_TRACER_INJECTION_DLL_BUILD

//
// This is the DLL build.
//

#define PYTHON_TRACER_INJECTION_API __declspec(dllexport)
#define PYTHON_TRACER_INJECTION_DATA extern __declspec(dllexport)

#else

//
// This is the static library build.
//

#define PYTHON_TRACER_INJECTION_API
#define PYTHON_TRACER_INJECTION_DATA extern

#endif

#include "stdafx.h"

#else

//
// We're being included by an external component.
//

#define PYTHON_TRACER_INJECTION_API __declspec(dllimport)
#define PYTHON_TRACER_INJECTION_DATA extern __declspec(dllimport)

#include <Windows.h>
#include "../Rtl/Rtl.h"
#include "../Python/Python.h"
#include "../DebugEngine/DebugEngine.h"
#include "../DebugEngine/DebugEngineInterfaces.h"
#include "../TraceStore/TraceStore.h"
#include "../TracerCore/TracerCore.h"
#include "../StringTable.h"
#include "../PythonTracer/PythonTracer.h"
#include "../TracerConfig/TracerConfig.h"
#include "../TracedPythonSession/TracedPythonSession.h"

#endif

INITIALIZE_TRACER_INJECTION InitializePythonTracerInjection;

#ifdef __cplusplus
}; // extern "C"
#endif

// vim:set ts=8 sw=4 sts=4 tw=80 expandtab                                     :
