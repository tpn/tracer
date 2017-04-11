/*++

Copyright (c) 2017 Trent Nelson <trent@trent.me>

Module Name:

    PythonTracerInjectionPrivate.h

Abstract:

    This is the private header file for the PythonTracerInjection component.
    It defines function typedefs and function declarations for all major
    (i.e. not local to the module) functions available for use by individual
    modules within this component.

--*/

#pragma once

#ifdef __cplusplus
extern "C" {
#endif

#include "stdafx.h"

extern CONST DEBUGEVENTCALLBACKS PythonTracerInjectionDebugEventCallbacks;

typedef struct _PYTHON_TRACER_INJECTION_CONTEXT {
    TRACER_INJECTION_CONTEXT InjectionContext;

    struct {
        GUARDED_LIST ListHead;
    } TracedPythonSessions;

} PYTHON_TRACER_INJECTION_CONTEXT;
typedef PYTHON_TRACER_INJECTION_CONTEXT *PPYTHON_TRACER_INJECTION_CONTEXT;

typedef
BOOL
(CALLBACK COMPLETE_PYTHON_TRACER_INJECTION)(
    _Inout_ PPYTHON_TRACER_INJECTION_CONTEXT Context
    );
typedef COMPLETE_PYTHON_TRACER_INJECTION *PCOMPLETE_PYTHON_TRACER_INJECTION;
COMPLETE_PYTHON_TRACER_INJECTION CompletePythonTracerInjection;

INITIALIZE_TRACER_INJECTION_THREAD_ENTRY PythonTracerInjectionThreadEntry;

#ifdef __cplusplus
}; // extern "C"
#endif

// vim:set ts=8 sw=4 sts=4 tw=80 expandtab                                     :
