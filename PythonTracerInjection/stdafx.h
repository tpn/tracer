/*++

Copyright (c) 2017 Trent Nelson <trent@trent.me>

Module Name:

    stdafx.h

Abstract:

    This is the precompiled header file for the PythonTracerInjection component.

--*/

#pragma once

#include "targetver.h"

#include <Windows.h>

#include "../Rtl/Rtl.h"
#include "../Python/Python.h"
#include "../DebugEngine/DebugEngine.h"
#include "../DebugEngine/DebugEngineInterfaces.h"
#include "../TracerConfig/TracerConfig.h"
#include "../TracerCore/TracerCore.h"
#include "../TracerHeap/TracerHeap.h"
#include "../TraceStore/TraceStore.h"
#include "../StringTable.h"
#include "../PythonTracer/PythonTracer.h"
#include "../TracedPythonSession/TracedPythonSession.h"
#include "PythonTracerInjection.h"

#ifdef _PYTHON_TRACER_INJECTION_INTERNAL_BUILD
#include "PythonTracerInjectionPrivate.h"
#endif

// vim:set ts=8 sw=4 sts=4 tw=80 expandtab                                     :
