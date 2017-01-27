/*++

Copyright (c) 2017 Trent Nelson <trent@trent.me>

Module Name:

    DebugEnginePrivate.h

Abstract:

    This is the private header file for the DebugEngine module.

--*/

#pragma once

#include "stdafx.h"

#ifdef __cplusplus
extern "C" {
#endif

//
// Private function typedefs.
//

typedef
_Check_return_
_Success_(return != 0)
BOOL
(INITIALIZE_DEBUG_ENGINE)(
    _In_ PRTL Rtl,
    _In_ PDEBUG_ENGINE DebugEngine
    );
typedef INITIALIZE_DEBUG_ENGINE *PINITIALIZE_DEBUG_ENGINE;

typedef
_Check_return_
_Success_(return != 0)
BOOL
(CREATE_DEBUG_INTERFACES)(
    _In_ PDEBUG_ENGINE_SESSION Session
    );
typedef CREATE_DEBUG_INTERFACES *PCREATE_DEBUG_INTERFACES;

typedef
_Check_return_
_Success_(return != 0)
BOOL
(DEBUG_ENGINE_DISASSEMBLE_ADDRESS)(
    _In_ PDEBUG_ENGINE_SESSION Session,
    _In_ ULONG64 Offset,
    _In_ PSTRING Buffer
    );
typedef DEBUG_ENGINE_DISASSEMBLE_ADDRESS *PDEBUG_ENGINE_DISASSEMBLE_ADDRESS;

typedef
_Check_return_
_Success_(return != 0)
BOOL
(INITIALIZE_CALLBACKS)(
    _In_ PDEBUG_ENGINE_SESSION Session
    );
typedef INITIALIZE_CALLBACKS *PINITIALIZE_CALLBACKS;

//
// Event callbacks.
//

CONST DEBUGEVENTCALLBACKS DebugEventCallbacks;
CONST DEBUGOUTPUTCALLBACKS DebugOutputCallbacks;
CONST DEBUGINPUTCALLBACKS DebugInputCallbacks;

FORCEINLINE
VOID
CopyIDebugEventCallbacks(
    _Out_writes_bytes_(sizeof(DEBUGEVENTCALLBACKS))
        PDEBUGEVENTCALLBACKS Target
    )
{
    __movsq((PDWORD64)Target,
            (PDWORD64)&DebugEventCallbacks,
            QUADWORD_SIZEOF(DEBUGEVENTCALLBACKS));
}

FORCEINLINE
VOID
CopyIDebugOutputCallbacks(
    _Out_writes_bytes_(sizeof(DEBUGOUTPUTCALLBACKS))
        PDEBUGOUTPUTCALLBACKS Target
    )
{
    __movsq((PDWORD64)Target,
            (PDWORD64)&DebugOutputCallbacks,
            QUADWORD_SIZEOF(DEBUGOUTPUTCALLBACKS));
}

FORCEINLINE
VOID
CopyIDebugInputCallbacks(
    _Out_writes_bytes_(sizeof(DEBUGINPUTCALLBACKS))
        PDEBUGINPUTCALLBACKS Target
    )
{
    __movsq((PDWORD64)Target,
            (PDWORD64)&DebugInputCallbacks,
            QUADWORD_SIZEOF(DEBUGINPUTCALLBACKS));
}


//
// Private function declarations.
//

#pragma component(browser, off)

INITIALIZE_CALLBACKS InitializeCallbacks;
CREATE_DEBUG_INTERFACES CreateDebugInterfaces;
INITIALIZE_DEBUG_ENGINE InitializeDebugEngine;
START_DEBUG_ENGINE_SESSION StartDebugEngineSession;

DEBUG_ENGINE_ENUM_SYMBOLS DebugEngineEnumSymbols;
DEBUG_ENGINE_DISASSEMBLE_FUNCTION DebugEngineDisassembleFunction;

//
// IDebugEventCallbacks
//

DEBUG_EVENT_QUERY_INTERFACE DebugEventQueryInterface;
DEBUG_EVENT_ADD_REF DebugEventAddRef;
DEBUG_EVENT_RELEASE DebugEventRelease;
DEBUG_EVENT_GET_INTEREST_MASK_CALLBACK DebugEventGetInterestMaskCallback;
DEBUG_EVENT_BREAKPOINT_CALLBACK DebugEventBreakpointCallback;
DEBUG_EVENT_EXCEPTION_CALLBACK DebugEventExceptionCallback;
DEBUG_EVENT_CREATE_THREAD_CALLBACK DebugEventCreateThreadCallback;
DEBUG_EVENT_EXIT_THREAD_CALLBACK DebugEventExitThreadCallback;
DEBUG_EVENT_CREATE_PROCESS_CALLBACK DebugEventCreateProcessCallback;
DEBUG_EVENT_EXIT_PROCESS_CALLBACK DebugEventExitProcessCallback;
DEBUG_EVENT_LOAD_MODULE_CALLBACK DebugEventLoadModuleCallback;
DEBUG_EVENT_UNLOAD_MODULE_CALLBACK DebugEventUnloadModuleCallback;
DEBUG_EVENT_SYSTEM_ERROR_CALLBACK DebugEventSystemErrorCallback;
DEBUG_EVENT_SESSION_STATUS_CALLBACK DebugEventSessionStatusCallback;
DEBUG_EVENT_CHANGE_DEBUGGEE_STATE_CALLBACK DebugEventChangeDebuggeeStateCallback;
DEBUG_EVENT_CHANGE_ENGINE_STATE_CALLBACK DebugEventChangeEngineStateCallback;
DEBUG_EVENT_CHANGE_SYMBOL_STATE_CALLBACK DebugEventChangeSymbolStateCallback;

//
// IDebugOutputCallbacks
//

DEBUG_OUTPUT_QUERY_INTERFACE DebugOutputQueryInterface;
DEBUG_OUTPUT_ADD_REF DebugOutputAddRef;
DEBUG_OUTPUT_RELEASE DebugOutputRelease;
DEBUG_OUTPUT_OUTPUT_CALLBACK DebugOutputOutputCallback;
DEBUG_OUTPUT_GET_INTEREST_MASK_CALLBACK DebugOutputGetInterestMaskCallback;
DEBUG_OUTPUT_OUTPUT2_CALLBACK DebugOutputOutput2Callback;

//
// IDebugInputCallbacks
//

DEBUG_INPUT_QUERY_INTERFACE DebugInputQueryInterface;
DEBUG_INPUT_ADD_REF DebugInputAddRef;
DEBUG_INPUT_RELEASE DebugInputRelease;
DEBUG_INPUT_START_INPUT_CALLBACK DebugInputStartInputCallback;
DEBUG_INPUT_END_INPUT_CALLBACK DebugInputEndInputCallback;

#pragma component(browser, on)

#ifdef __cplusplus
} // extern "C"
#endif

// vim:set ts=8 sw=4 sts=4 tw=80 expandtab                                     :
