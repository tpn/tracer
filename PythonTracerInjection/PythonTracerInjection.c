/*++

Copyright (c) 2016 Trent Nelson <trent@trent.me>

Module Name:

    PythonTracerInjection.c

Abstract:

    This module implements the necessary routines to facilitate tracer
    injection of Python modules.

--*/

#include "stdafx.h"

DEFINE_GUID_EX(IID_IDEBUG_EVENT_CALLBACKS, 0x0690e046, 0x9c23, 0x45ac,
               0xa0, 0x4f, 0x98, 0x7a, 0xc2, 0x9a, 0xd0, 0xd3);

typedef struct _PYTHON_TRACER_INJECTION_CONTEXT {
    ULONGLONG Unused;
} PYTHON_TRACER_INJECTION_CONTEXT;
typedef PYTHON_TRACER_INJECTION_CONTEXT *PPYTHON_TRACER_INJECTION_CONTEXT;

_Use_decl_annotations_
BOOL
InitializePythonTracerInjection(
    PRTL Rtl,
    PALLOCATOR Allocator,
    PTRACER_CONFIG TracerConfig,
    PDEBUG_ENGINE_SESSION Parent
    )
/*++

Routine Description:

    This function initializes tracer injection for Python.

Arguments:

    Rtl - Supplies a pointer to an initialized RTL structure.

    Allocator - Supplies a pointer to an initialized ALLOCATOR structure.

    TracerConfig - Supplies a pointer to an initialized TRACER_CONFIG structure.

    DebugEngineSession - Supplies a pointer to an initialized
        DEBUG_ENGINE_SESSION structure.

Return Value:

    TRUE on Success, FALSE if an error occurred.

--*/
{
    BOOL Success;
    PPYTHON_TRACER_INJECTION_CONTEXT Context;
    PDEBUG_ENGINE_SESSION Session;
    DEBUG_EVENT_CALLBACKS_INTEREST_MASK InterestMask;

    //
    // Allocate Context.
    //

    Context = NULL;

    //
    // Initialize interest mask.
    //

    InterestMask.AsULong = (
        DEBUG_EVENT_BREAKPOINT          |
        DEBUG_EVENT_EXCEPTION           |
        DEBUG_EVENT_CREATE_THREAD       |
        DEBUG_EVENT_EXIT_THREAD         |
        DEBUG_EVENT_CREATE_PROCESS      |
        DEBUG_EVENT_EXIT_PROCESS        |
        DEBUG_EVENT_LOAD_MODULE         |
        DEBUG_EVENT_UNLOAD_MODULE       |
        DEBUG_EVENT_SYSTEM_ERROR        |
        DEBUG_EVENT_CHANGE_SYMBOL_STATE
    );

    //
    // The following block can be useful during debugging to quickly
    // toggle interest masks on and off.
    //

    if (0) {
        InterestMask.AsULong = 0;
        InterestMask.Breakpoint = TRUE;
        InterestMask.Exception = TRUE;
        InterestMask.CreateThread = TRUE;
        InterestMask.ExitThread = TRUE;
        InterestMask.CreateProcess = TRUE;
        InterestMask.ExitProcess = TRUE;
        InterestMask.LoadModule = TRUE;
        InterestMask.UnloadModule = TRUE;
        InterestMask.SystemError = TRUE;
        InterestMask.SessionStatus = TRUE;
        InterestMask.ChangeDebuggeeState = TRUE;
        InterestMask.ChangeEngineState = TRUE;
        InterestMask.ChangeSymbolState = TRUE;
    }

    //
    // Initialize our child debug engine session.
    //

    Success = (
        Parent->InitializeChildDebugEngineSession(
            Parent,
            &PythonTracerInjectionDebugEventCallbacks,
            &IID_IDEBUG_EVENT_CALLBACKS,
            InterestMask,
            &Session
        )
    );

    if (!Success) {
        goto Error;
    }

    Success = TRUE;
    goto End;

Error:

    Success = FALSE;

    //
    // Intentional follow-on to End.
    //

End:

    return Success;
}

// vim:set ts=8 sw=4 sts=4 tw=80 expandtab                                     :
