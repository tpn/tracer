/*++

Copyright (c) 2017 Trent Nelson <trent@trent.me>

Module Name:

    TracerCore.h

Abstract:

    This is the main header file for the TracerCore component.

--*/

#pragma once

#ifdef __cplusplus
extern "C" {
#endif

#define TRACER_CORE_CALL_CONV __stdcall

#ifdef _TRACER_CORE_INTERNAL_BUILD

//
// This is an internal build of the TracerCore component.
//

#define TRACER_CORE_API __declspec(dllexport)
#define TRACER_CORE_DATA extern __declspec(dllexport)

#include "stdafx.h"

#elif _TRACER_CORE_NO_API_EXPORT_IMPORT

//
// We're being included by someone who doesn't want dllexport or dllimport.
// This is useful for creating new .exe-based projects for things like unit
// testing or performance testing/profiling.
//

#define TRACER_CORE_API
#define TRACER_CORE_DATA extern

#else

//
// We're being included by an external component.
//

#define TRACER_CORE_API __declspec(dllimport)
#define TRACER_CORE_DATA extern __declspec(dllimport)

#include "../Rtl/Rtl.h"
#include "../DebugEngine/DebugEngine.h"
#include "../TracerCore/TracerCore.h"

#endif

//
// Define TRACER_INJECTION_BREAKPOINT structure.
//

typedef union _TRACER_INJECTION_BREAKPOINT_ERROR {
    struct {
        ULONG InitializationFailed:1;

        //
        // General breakpoint error flags.
        //

        ULONG GetCurrentProcessIdFailed:1;
        ULONG GetCurrentThreadIdFailed:1;

        //
        // Failure flags related to the initial offset expression breakpoint.
        //

        ULONG AddBreakpointFailed:1;
        ULONG GetIdFailed:1;
        ULONG SetOffsetExpressionFailed:1;
        ULONG AddFlagsEnabledFailed:1;
        ULONG RemoveFlagsEnabledFailed:1;
        ULONG GetStackOffsetFailed:1;
        ULONG GetReturnOffsetFailed:1;

        //
        // Failure flags related to the optional return address breakpoint.
        //

        ULONG AddReturnBreakpointFailed:1;
        ULONG GetReturnIdFailed:1;
        ULONG SetOffsetFailed:1;
        ULONG AddReturnFlagsEnabledFailed:1;
        ULONG RemoveReturnFlagsEnabledFailed:1;
    };

    LONG AsLong;
    ULONG AsULong;
} TRACER_INJECTION_BREAKPOINT_ERROR;

typedef union _TRACER_INJECTION_BREAKPOINT_FLAGS {
    struct {
        ULONG Initialized:1;
        ULONG IsOnceOff:1;
        ULONG BreakpointEnabled:1;
        ULONG ReturnBreakpointEnabled:1;
    };
    LONG AsLong;
    ULONG AsULong;
} TRACER_INJECTION_BREAKPOINT_FLAGS;

typedef
HRESULT
(STDAPICALLTYPE TRACER_INJECTION_HANDLE_BREAKPOINT)(
    _In_ struct _TRACER_INJECTION_CONTEXT *InjectionContext,
    _In_ struct _TRACER_INJECTION_BREAKPOINT *InjectionBreakpoint
    );
typedef TRACER_INJECTION_HANDLE_BREAKPOINT *PTRACER_INJECTION_HANDLE_BREAKPOINT;

//
// Breakpoint specification.
//

typedef struct _TRACER_INJECTION_BREAKPOINT_SPEC {
    PCSTR OffsetExpression;
    PTRACER_INJECTION_HANDLE_BREAKPOINT HandleBreakpoint;
    PTRACER_INJECTION_HANDLE_BREAKPOINT HandleReturnBreakpoint;
} TRACER_INJECTION_BREAKPOINT_SPEC;
typedef TRACER_INJECTION_BREAKPOINT_SPEC
      *PTRACER_INJECTION_BREAKPOINT_SPEC;
typedef const TRACER_INJECTION_BREAKPOINT_SPEC
           *PCTRACER_INJECTION_BREAKPOINT_SPEC;

typedef struct _TRACER_INJECTION_BREAKPOINT {
    ULONG SizeOfStruct;
    ULONG BreakpointId;
    ULONG ReturnBreakpointId;
    ULONG CurrentProcessId;
    ULONG CurrentThreadId;
    ULONG Padding;
    TRACER_INJECTION_BREAKPOINT_FLAGS Flags;
    TRACER_INJECTION_BREAKPOINT_ERROR Error;
    PDEBUGBREAKPOINT Breakpoint;
    PIDEBUGBREAKPOINT IBreakpoint;
    PDEBUGBREAKPOINT ReturnBreakpoint;
    PIDEBUGBREAKPOINT IReturnBreakpoint;

    ULONGLONG StackOffset;
    ULONGLONG ReturnOffset;

    HANDLE CurrentProcessHandle;
    HANDLE CurrentThreadHandle;

    //
    // This is the handle of the thread on which the debug engine was created.
    // This is captured in order to facilitate queuing APCs for debug engine
    // actions from other threads (e.g. unfreezing a thread).
    //

    HANDLE DebugEngineThreadHandle;

    //
    // Inline TRACER_INJECTION_BREAKPOINT_SPEC.
    //

    union {
        struct {
            PCSTR OffsetExpression;
            PTRACER_INJECTION_HANDLE_BREAKPOINT HandleBreakpoint;
            PTRACER_INJECTION_HANDLE_BREAKPOINT HandleReturnBreakpoint;
        };
        TRACER_INJECTION_BREAKPOINT_SPEC Spec;
    };
} TRACER_INJECTION_BREAKPOINT;
typedef TRACER_INJECTION_BREAKPOINT *PTRACER_INJECTION_BREAKPOINT;

//
// Define the TRACER_INJECTION_CONTEXT structure.
//

typedef union _TRACER_INJECTION_CONTEXT_ERROR {
    struct {
        ULONG InitializationFailed:1;
        ULONG BreakpointInitializationFailed:1;
    };
    LONG AsLong;
    ULONG AsULong;
} TRACER_INJECTION_CONTEXT_ERROR;

typedef union _TRACER_INJECTION_CONTEXT_FLAGS {
    struct {
        ULONG BreakpointsInitialized:1;
        ULONG Initialized:1;
    };
    LONG AsLong;
    ULONG AsULong;
} TRACER_INJECTION_CONTEXT_FLAGS;

typedef TRACER_INJECTION_CONTEXT_FLAGS *PTRACER_INJECTION_CONTEXT_FLAGS;

typedef struct _Struct_size_bytes_(SizeOfStruct) _TRACER_INJECTION_CONTEXT {

    //
    // Size of the structure, in bytes.
    //

    _Field_range_(==, sizeof(struct _TRACER_INJECTION_CONTEXT))
        ULONG SizeOfStruct;

    //
    // Error.
    //

    TRACER_INJECTION_CONTEXT_ERROR Error;

    //
    // Flags.
    //

    TRACER_INJECTION_CONTEXT_FLAGS Flags;

    ULONG DebugEngineThreadId;

    HANDLE DebugEngineThreadHandle;

    //
    // Standard fields.
    //

    PRTL Rtl;
    PALLOCATOR Allocator;
    struct _TRACER_CONFIG *TracerConfig;
    struct _DEBUG_ENGINE_SESSION *ParentDebugEngineSession;
    struct _DEBUG_ENGINE_SESSION *DebugEngineSession;

    ULONG NumberOfBreakpoints;
    ULONG Padding;
    PTRACER_INJECTION_BREAKPOINT InjectionBreakpoints;

    HANDLE ResumeEvent;
    PTRACER_INJECTION_BREAKPOINT CurrentInjectionBreakpoint;

} TRACER_INJECTION_CONTEXT;
typedef TRACER_INJECTION_CONTEXT *PTRACER_INJECTION_CONTEXT;
typedef TRACER_INJECTION_CONTEXT **PPTRACER_INJECTION_CONTEXT;

typedef union _TRACER_INJECTION_CONTEXT_INIT_FLAGS {
    struct {
        ULONG Unused:1;
    };
    ULONG AsLong;
} TRACER_INJECTION_CONTEXT_INIT_FLAGS;
typedef TRACER_INJECTION_CONTEXT_INIT_FLAGS
      *PTRACER_INJECTION_CONTEXT_INIT_FLAGS;
C_ASSERT(sizeof(TRACER_INJECTION_CONTEXT_INIT_FLAGS) == sizeof(ULONG));

typedef
_Check_return_
_Success_(return != 0)
BOOL
(CALLBACK INITIALIZE_TRACER_INJECTION_CONTEXT)(
    _In_opt_ PTRACER_INJECTION_CONTEXT InjectionContext,
    _Inout_ PULONG SizeInBytes
    );
typedef INITIALIZE_TRACER_INJECTION_CONTEXT
      *PINITIALIZE_TRACER_INJECTION_CONTEXT;

typedef
_Success_(return != 0)
ULONG
(CDECL TRACER_EXE_MAIN)(
    VOID
    );
typedef TRACER_EXE_MAIN *PTRACER_EXE_MAIN;

typedef
ULONG
(__stdcall INITIALIZE_TRACER_INJECTION_THREAD_ENTRY)(
    _Inout_ PTRACER_INJECTION_CONTEXT InjectionContext
    );
typedef INITIALIZE_TRACER_INJECTION_THREAD_ENTRY
      *PINITIALIZE_TRACER_INJECTION_THREAD_ENTRY;

typedef
_Check_return_
_Success_(return != 0)
BOOL
(CALLBACK INITIALIZE_TRACER_INJECTION_BREAKPOINTS)(
    _Inout_ PTRACER_INJECTION_CONTEXT InjectionContext
    );
typedef INITIALIZE_TRACER_INJECTION_BREAKPOINTS
      *PINITIALIZE_TRACER_INJECTION_BREAKPOINTS;


//
// Public function declarations..
//

#pragma component(browser, off)
TRACER_CORE_API INITIALIZE_TRACER_INJECTION_CONTEXT
                InitializeTracerInjectionContext;
TRACER_CORE_API TRACER_EXE_MAIN TracerExeMain;
#pragma component(browser, on)

#ifdef __cplusplus
} // extern "C"
#endif

// vim:set ts=8 sw=4 sts=4 tw=80 expandtab nowrap                              :
