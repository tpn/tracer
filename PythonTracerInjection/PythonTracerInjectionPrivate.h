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

typedef struct _PYTHON_TRACER_INJECTION_BREAKPOINT {
    TRACER_INJECTION_BREAKPOINT InjectionBreakpoint;
} PYTHON_TRACER_INJECTION_BREAKPOINT;
typedef PYTHON_TRACER_INJECTION_BREAKPOINT *PPYTHON_TRACER_INJECTION_BREAKPOINT;

typedef union _PYTHON_TRACER_INJECTION_INITIAL_BREAKPOINTS {
    struct {
        TRACER_INJECTION_BREAKPOINT Python27_Py_Main;
        TRACER_INJECTION_BREAKPOINT Python30_Py_Main;
        TRACER_INJECTION_BREAKPOINT Python31_Py_Main;
        TRACER_INJECTION_BREAKPOINT Python32_Py_Main;
        TRACER_INJECTION_BREAKPOINT Python33_Py_Main;
        TRACER_INJECTION_BREAKPOINT Python34_Py_Main;
        TRACER_INJECTION_BREAKPOINT Python35_Py_Main;
        TRACER_INJECTION_BREAKPOINT Python36_Py_Main;

        TRACER_INJECTION_BREAKPOINT Python27_Py_InitializeEx;
        TRACER_INJECTION_BREAKPOINT Python30_Py_InitializeEx;
        TRACER_INJECTION_BREAKPOINT Python31_Py_InitializeEx;
        TRACER_INJECTION_BREAKPOINT Python32_Py_InitializeEx;
        TRACER_INJECTION_BREAKPOINT Python33_Py_InitializeEx;
        TRACER_INJECTION_BREAKPOINT Python34_Py_InitializeEx;
        TRACER_INJECTION_BREAKPOINT Python35_Py_InitializeEx;
        TRACER_INJECTION_BREAKPOINT Python36_Py_InitializeEx;
    };
    TRACER_INJECTION_BREAKPOINT First;
} PYTHON_TRACER_INJECTION_INITIAL_BREAKPOINTS;

#define NUM_INITIAL_BREAKPOINTS() (                       \
    sizeof(PYTHON_TRACER_INJECTION_INITIAL_BREAKPOINTS) / \
    sizeof(TRACER_INJECTION_BREAKPOINT)                   \
)

typedef PYTHON_TRACER_INJECTION_INITIAL_BREAKPOINTS
      *PPYTHON_TRACER_INJECTION_INITIAL_BREAKPOINTS;

typedef struct _PYTHON_TRACER_INJECTION_CONTEXT {
    TRACER_INJECTION_CONTEXT InjectionContext;

    struct {
        GUARDED_LIST ListHead;
    } TracedPythonSessions;

    PYTHON_TRACER_INJECTION_INITIAL_BREAKPOINTS InitialBreakpoints;

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
INITIALIZE_TRACER_INJECTION_BREAKPOINTS
    InitializePythonTracerInjectionBreakpoints;

RTL_INJECTION_COMPLETE_CALLBACK PythonTracerInjectionCompleteCallback;

#ifdef __cplusplus
}; // extern "C"
#endif

// vim:set ts=8 sw=4 sts=4 tw=80 expandtab                                     :
