/*++

Copyright (c) 2017 Trent Nelson <trent@trent.me>

Module Name:

    DebugEngineCOM.h

Abstract:

    This is the private header file for COM-related aspects of the DebugEngine.
    It is kept separate from DebugEnginePrivate.h in order to easily distinguish
    our internal DebugEngine functionality versus boilerplate COM glue.

--*/

#pragma once

#include "stdafx.h"

#ifdef __cplusplus
extern "C" {
#endif

//
// GUID typedefs.
//

typedef GUID *PGUID;
typedef const GUID CGUID;
typedef GUID const *PCGUID;

//
// Define helper bitflag structs that simplify working with various DEBUG_*
// constants.
//

typedef union _DEBUG_EVENT_CALLBACKS_INTEREST_MASK {
    LONG AsLong;
    ULONG AsULong;
    struct _Struct_size_bytes_(sizeof(ULONG)) {

        //
        // DEBUG_EVENT_BREAKPOINT              0x00000001
        //

        ULONG Breakpoint:1;

        //
        // DEBUG_EVENT_EXCEPTION               0x00000002
        //
        //

        ULONG Exception:1;

        //
        // DEBUG_EVENT_CREATE_THREAD           0x00000004
        //

        ULONG CreateThread:1;

        //
        // DEBUG_EVENT_EXIT_THREAD             0x00000008
        //

        ULONG ExitThread:1;

        //
        // DEBUG_EVENT_CREATE_PROCESS          0x00000010
        //

        ULONG CreateProcess:1;

        //
        // DEBUG_EVENT_EXIT_PROCESS            0x00000020
        //

        ULONG ExitProcess:1;

        //
        // DEBUG_EVENT_LOAD_MODULE             0x00000040
        //

        ULONG LoadModule:1;

        //
        // DEBUG_EVENT_UNLOAD_MODULE           0x00000080
        //

        ULONG UnloadModule:1;

        //
        // DEBUG_EVENT_SYSTEM_ERROR            0x00000100
        //

        ULONG SystemError:1;

        //
        // DEBUG_EVENT_SESSION_STATUS          0x00000200
        //

        ULONG SessionStatus:1;

        //
        // DEBUG_EVENT_CHANGE_DEBUGGEE_STATE   0x00000400
        //

        ULONG ChangeDebuggeeState:1;

        //
        // DEBUG_EVENT_CHANGE_ENGINE_STATE     0x00000800
        //

        ULONG ChangeEngineState:1;

        //
        // DEBUG_EVENT_CHANGE_SYMBOL_STATE     0x00001000
        //

        ULONG ChangeSymbolState:1;

        //
        // Unused.
        //

        ULONG Unused:19;
    };
} DEBUG_EVENT_CALLBACKS_INTEREST_MASK;
C_ASSERT(sizeof(DEBUG_EVENT_CALLBACKS_INTEREST_MASK) == sizeof(ULONG));

typedef union _DEBUG_CHANGE_ENGINE_STATE {
    LONG AsLong;
    ULONG AsULong;
    struct _Struct_size_bytes_(sizeof(ULONG)) {

        //
        // Current thread changed.  Argument is ID of new thread or
        // DEBUG_ANY_ID if no thread is current.
        //
        //      0x0001
        //

        ULONG CurrentThread:1;

        //
        // Effective processor changed.  Argument is new processor type.
        //
        //      0x0002
        //

        ULONG EffectiveProcessor:1;

        //
        // Breakpoints changed.  If only a single breakpoint changed, argument
        // is the ID of the breakpoint; otherwise it is DEBUG_ANY_ID.
        //
        //      0x0004
        //

        ULONG Breakpoints:1;

        //
        // Code interpretation level changed.  Argument is the new level.
        //
        //      0x0008
        //

        ULONG CodeLevel:1;

        //
        // Execution status changed.  Argument is the new execution status.
        //
        //      0x0010
        //

        ULONG ExecutionStatus:1;

        //
        // Engine options have changed.  Argument is the new options value.
        //
        //      0x0020
        //

        ULONG EngineOptions:1;

        //
        // Log file information has changed.  Argument is TRUE if a log file
        // was opened, FALSE if a log file was closed.
        //
        //      0x0040
        //

        ULONG LogFile:1;

        //
        // Default number radix has changed.  Argument is the new radix.
        //
        //      0x0080
        //

        ULONG Radix:1;

        //
        // Event filters have changed.  If only a single filter has changed,
        // the argument is the filter's index, otherwise it is DEBUG_ANY_ID.
        //
        //      0x0100
        //

        ULONG EventFilters:1;

        //
        // Process options have changed.  Argument is the new options value.
        //
        //      0x0200
        //

        ULONG ProcessOptions:1;

        //
        // Extensions have been added or removed.
        //
        //      0x0400
        //

        ULONG Extensions:1;

        //
        // Systems have been added or removed.  The argument is the system ID.
        //
        //  N.B. Unlike processes and threads, systems may be created at any
        //       time and not just during WaitForEvent.
        //
        //      0x0800
        //

        ULONG Systems:1;

        //
        // Assembly/disassembly options have changed.  Argument is the new
        // options value.
        //
        //      0x1000
        //

        ULONG AssemblyOptions:1;

        //
        // Expression syntax has changed.  Argument is the new syntax value.
        //
        //      0x2000
        //

        ULONG ExpressionSyntax:1;

        //
        // Text replacements have changed.
        //
        //      0x4000
        //

        ULONG TextReplacements:1;

        //
        // Unused bits.
        //

        ULONG Unused:17;

    };
} DEBUG_CHANGE_ENGINE_STATE, *PDEBUG_CHANGE_ENGINE_STATE;
C_ASSERT(sizeof(DEBUG_CHANGE_ENGINE_STATE) == sizeof(ULONG));

//
// This union wraps the DEBUG_OUTCBI_* constants.
//

typedef union _DEBUG_OUTPUT_CALLBACKS2_INTEREST_MASK {
    LONG AsLong;
    ULONG AsULong;
    struct _Struct_size_bytes_(sizeof(ULONG)) {

        //
        // Notification of all explicit flushes.
        //
        //      0x0001
        //

        ULONG ExplicitFlush:1;

        //
        // Notification of text output.
        //
        //      0x0002
        //

        ULONG Text:1;

        //
        // Notification of DML output.
        //
        //      0x0004
        //

        ULONG Markup:1;


    };
} DEBUG_OUTPUT_CALLBACKS2_INTEREST_MASK;
typedef DEBUG_OUTPUT_CALLBACKS2_INTEREST_MASK
      *PDEBUG_OUTPUT_CALLBACKS2_INTEREST_MASK;
C_ASSERT(sizeof(DEBUG_OUTPUT_CALLBACKS2_INTEREST_MASK) == sizeof(ULONG));

typedef DEBUG_OUTPUT_CALLBACKS2_INTEREST_MASK
        DEBUG_OUTPUT_CALLBACKS2_INTEREST_MASK;

//
// This union wraps the DEBUG_OUTCBF_* constants.
//

typedef union _DEBUG_OUTPUT_CALLBACK_FLAGS {
    LONG AsLong;
    ULONG AsULong;
    struct _Struct_size_bytes_(sizeof(ULONG)) {

        //
        // Content string was followed by an explicit flush.
        //
        //      0x0001
        //

        ULONG ExplicitFlush:1;

        //
        // Markup content string has embedded tags.
        //
        //      0x0002
        //

        ULONG MarkupHasEmbeddedTags:1;

        //
        // Markup content has encoded special characters like ", &, < and >.
        //
        //      0x0004
        //

        ULONG MarkupHasEncodedSpecialCharacters:1;
    };
} DEBUG_OUTPUT_CALLBACK_FLAGS, *PDEBUG_OUTPUT_CALLBACK_FLAGS;
C_ASSERT(sizeof(DEBUG_OUTPUT_CALLBACK_FLAGS) == sizeof(ULONG));

//
// This union wraps the DEBUG_OUTPUT_XXX constants.
//

typedef union _DEBUG_OUTPUT_MASK {
    LONG AsLong;
    ULONG AsULong;
    struct _Struct_size_bytes_(sizeof(ULONG)) {

        //
        // Normal.
        //
        //      0x0001
        //

        ULONG Normal:1;

        //
        // Error.
        //
        //      0x0002
        //

        ULONG Error:1;

        //
        // Warning.
        //
        //      0x0004
        //

        ULONG Warning:1;

        //
        // Additional output.
        //
        //      0x0008
        //

        ULONG Verbose:1;

        //
        // Prompt output.
        //
        //      0x0010
        //

        ULONG Prompt:1;

        //
        // Register dump before prompt.
        //
        //      0x0020
        //

        ULONG PromptRegisters:1;

        //
        // Warnings specific to extension operation.
        //
        //      0x0040
        //

        ULONG ExtensionWarning:1;

        //
        // Debuggee output, such as from OutputDebugString().
        //
        //      0x0080
        //

        ULONG OutputDebuggee:1;

        //
        // Debuggee-generated prompt, such as from DbgPrompt.
        //
        //      0x0100
        //

        ULONG OutputDebuggeePrompt:1;

        //
        // Symbol messages, such as for !sym noisy.
        //
        //      0x0200
        //

        ULONG Symbols:1;

        //
        // Output which modifies the status bar.
        //
        //      0x0400
        //

        ULONG Status:1;

    };
} DEBUG_OUTPUT_MASK, *PDEBUG_OUTPUT_MASK;
C_ASSERT(sizeof(DEBUG_OUTPUT_MASK) == sizeof(ULONG));

//
// This union wraps the constants relating to the debug engine's execution
// status.
//

typedef union _DEBUG_EXECUTION_STATUS {
    struct {
        ULONG LowPart;
        ULONG HighPart;
    };
    LONGLONG AsLongLong;
    ULONGLONG AsULongLong;
    struct _Struct_size_bytes_(sizeof(ULONGLONG)) {

        //
        // Pad out the unused bytes.
        //

        ULONGLONG Padding:32;

        //
        // Engine's execution status is changing due to operations performed
        // during a wait, such as making synchronous calls.  If this bit is not
        // set, execution status is changing due to a wait being satisfied.
        //
        //      0x100000000
        //

        ULONGLONG InsideWait:1;

        //
        // Engine's execution status update is due to a wait timing out.
        // Indicates the status change was not due to an actual event occurring.
        //
        //      0x200000000
        //

        ULONGLONG WaitTimeout:1;

        //
        // Mark the remaining bits as unused.
        //

        ULONGLONG Unused:30;
    };

} DEBUG_EXECUTION_STATUS, *PDEBUG_EXECUTION_STATUS;
C_ASSERT(sizeof(DEBUG_EXECUTION_STATUS) == sizeof(ULONGLONG));

//
// This enum wraps the 'Which' parameter in the Output2 callbacks.
//

typedef enum _DEBUG_OUTPUT_TYPE {
    DebugOutputText = 0,
    DebugOutputMarkup = 1,
    DebugOutputExplicitFlush = 2
} DEBUG_OUTPUT_TYPE;

FORCEINLINE
BOOL
IsTextOutput(
    _In_ DEBUG_OUTPUT_TYPE OutputType
    )
{
    return (OutputType == DebugOutputText);
}

FORCEINLINE
BOOL
IsMarkupOutput(
    _In_ DEBUG_OUTPUT_TYPE OutputType
    )
{
    return (OutputType == DebugOutputMarkup);
}

FORCEINLINE
BOOL
IsExplictFlush(
    _In_ DEBUG_OUTPUT_TYPE OutputType
    )
{
    return (OutputType == DebugOutputExplicitFlush);
}

//
// Define typedefs for the COM interfaces we want to use.
//

//
// IDebugAdvanced4
//

typedef struct IDebugAdvanced4 IDEBUGADVANCED4;
typedef IDEBUGADVANCED4 *PIDEBUGADVANCED4;

typedef struct IDebugAdvanced4Vtbl IDEBUGADVANCED4VTBL;
typedef IDEBUGADVANCED4VTBL *PIDEBUGADVANCED4VTBL;

typedef IDEBUGADVANCED4     IDEBUGADVANCED;
typedef IDEBUGADVANCED   *PIDEBUGADVANCED;
typedef IDEBUGADVANCED **PPIDEBUGADVANCED;

typedef IDEBUGADVANCED4VTBL   DEBUGADVANCED;
typedef DEBUGADVANCED      *PDEBUGADVANCED;
typedef DEBUGADVANCED    **PPDEBUGADVANCED;

//
// IDebugClient7
//

typedef struct IDebugClient7 IDEBUGCLIENT7;
typedef IDEBUGCLIENT7 *PIDEBUGCLIENT7;

typedef struct IDebugClient7Vtbl IDEBUGCLIENT7VTBL;
typedef IDEBUGCLIENT7VTBL *PIDEBUGCLIENT7VTBL;

typedef IDEBUGCLIENT7     IDEBUGCLIENT;
typedef IDEBUGCLIENT   *PIDEBUGCLIENT;
typedef IDEBUGCLIENT **PPIDEBUGCLIENT;

typedef IDEBUGCLIENT7VTBL   DEBUGCLIENT;
typedef DEBUGCLIENT      *PDEBUGCLIENT;
typedef DEBUGCLIENT    **PPDEBUGCLIENT;

//
// IDebugControl7
//

typedef struct IDebugControl7 IDEBUGCONTROL7;
typedef IDEBUGCONTROL7 *PIDEBUGCONTROL7;

typedef struct IDebugControl7Vtbl IDEBUGCONTROL7VTBL;
typedef IDEBUGCONTROL7VTBL *PIDEBUGCONTROL7VTBL;

typedef IDEBUGCONTROL7     IDEBUGCONTROL;
typedef IDEBUGCONTROL   *PIDEBUGCONTROL;
typedef IDEBUGCONTROL **PPIDEBUGCONTROL;

typedef IDEBUGCONTROL7VTBL   DEBUGCONTROL;
typedef DEBUGCONTROL      *PDEBUGCONTROL;
typedef DEBUGCONTROL    **PPDEBUGCONTROL;

//
// IDebugSymbols5
//

typedef struct IDebugSymbols5 IDEBUGSYMBOLS5;
typedef IDEBUGSYMBOLS5 *PIDEBUGSYMBOLS5;

typedef struct IDebugSymbols5Vtbl IDEBUGSYMBOLS5VTBL;
typedef IDEBUGSYMBOLS5VTBL *PIDEBUGSYMBOLS5VTBL;

typedef IDEBUGSYMBOLS5     IDEBUGSYMBOLS;
typedef IDEBUGSYMBOLS   *PIDEBUGSYMBOLS;
typedef IDEBUGSYMBOLS **PPIDEBUGSYMBOLS;

typedef IDEBUGSYMBOLS5VTBL   DEBUGSYMBOLS;
typedef DEBUGSYMBOLS      *PDEBUGSYMBOLS;
typedef DEBUGSYMBOLS    **PPDEBUGSYMBOLS;

//
// IDebugSymbolGroup2
//

typedef struct IDebugSymbolGroup2 IDEBUGSYMBOLGROUPGROUP2;
typedef IDEBUGSYMBOLGROUPGROUP2 *PIDEBUGSYMBOLGROUPGROUP2;

typedef struct IDebugSymbolGroup2Vtbl IDEBUGSYMBOLGROUPGROUP2VTBL;
typedef IDEBUGSYMBOLGROUPGROUP2VTBL *PIDEBUGSYMBOLGROUPGROUP2VTBL;

typedef IDEBUGSYMBOLGROUPGROUP2     IDEBUGSYMBOLGROUP;
typedef IDEBUGSYMBOLGROUP         *PIDEBUGSYMBOLGROUP;
typedef IDEBUGSYMBOLGROUP      **PPIDEBUGSYMBOLGROUP;

typedef IDEBUGSYMBOLGROUPGROUP2VTBL   DEBUGSYMBOLGROUP;
typedef DEBUGSYMBOLGROUP            *PDEBUGSYMBOLGROUP;
typedef DEBUGSYMBOLGROUP           **PPDEBUGSYMBOLGROUP;

//
// IDebugDataSpaces4
//

typedef struct IDebugDataSpaces4 IDEBUGDATASPACES4;
typedef IDEBUGDATASPACES4 *PIDEBUGDATASPACES4;

typedef struct IDebugDataSpaces4Vtbl IDEBUGDATASPACES4VTBL;
typedef IDEBUGDATASPACES4VTBL *PIDEBUGDATASPACES4VTBL;

typedef IDEBUGDATASPACES4     IDEBUGDATASPACES;
typedef IDEBUGDATASPACES   *PIDEBUGDATASPACES;
typedef IDEBUGDATASPACES **PPIDEBUGDATASPACES;

typedef IDEBUGDATASPACES4VTBL   DEBUGDATASPACES;
typedef DEBUGDATASPACES      *PDEBUGDATASPACES;
typedef DEBUGDATASPACES    **PPDEBUGDATASPACES;

//
// IDebugEventCallbacks
//

typedef struct IDebugEventCallbacksWide IDEBUGEVENTCALLBACKS;
typedef IDEBUGEVENTCALLBACKS *PIDEBUGEVENTCALLBACKS;

typedef struct IDebugEventCallbacksWideVtbl DEBUGEVENTCALLBACKS;
typedef DEBUGEVENTCALLBACKS *PDEBUGEVENTCALLBACKS;
typedef const DEBUGEVENTCALLBACKS *PCDEBUGEVENTCALLBACKS;

//
// IDebugInputCallbacks
//

typedef struct IDebugInputCallbacks IDEBUGINPUTCALLBACKS;
typedef IDEBUGINPUTCALLBACKS *PIDEBUGINPUTCALLBACKS;

typedef struct IDebugInputCallbacksVtbl DEBUGINPUTCALLBACKS;
typedef DEBUGINPUTCALLBACKS *PDEBUGINPUTCALLBACKS;
typedef const DEBUGINPUTCALLBACKS *PCDEBUGINPUTCALLBACKS;

//
// IDebugOutputCallbacks
//

typedef struct IDebugOutputCallbacks IDEBUGOUTPUTCALLBACKS;
typedef IDEBUGOUTPUTCALLBACKS *PIDEBUGOUTPUTCALLBACKS;

typedef struct IDebugOutputCallbacksVtbl DEBUGOUTPUTCALLBACKS;
typedef DEBUGOUTPUTCALLBACKS *PDEBUGOUTPUTCALLBACKS;
typedef const DEBUGOUTPUTCALLBACKS *PCDEBUGOUTPUTCALLBACKS;

//
// IDebugOutputCallbacks2
//

typedef struct IDebugOutputCallbacks2 IDEBUGOUTPUTCALLBACKS2;
typedef IDEBUGOUTPUTCALLBACKS2 *PIDEBUGOUTPUTCALLBACKS2;

typedef struct IDebugOutputCallbacks2Vtbl DEBUGOUTPUTCALLBACKS2;
typedef DEBUGOUTPUTCALLBACKS2 *PDEBUGOUTPUTCALLBACKS2;
typedef const DEBUGOUTPUTCALLBACKS2 *PCDEBUGOUTPUTCALLBACKS2;

////////////////////////////////////////////////////////////////////////////////
// COM Function Pointer Type Definitions
////////////////////////////////////////////////////////////////////////////////

//
// IUnknown
//

typedef struct IUnknown IUNKNOWN;
typedef IUNKNOWN *PIUNKNOWN;

typedef
HRESULT
(STDAPICALLTYPE IUNKNOWN_QUERY_INTERFACE)(
    _In_ PIUNKNOWN This,
    _In_ REFIID InterfaceId,
    _Out_ PPVOID Interface
    );
typedef IUNKNOWN_QUERY_INTERFACE *PIUNKNOWN_QUERY_INTERFACE;

typedef
ULONG
(STDAPICALLTYPE IUNKNOWN_ADD_REF)(
    _In_ PIUNKNOWN This
    );
typedef IUNKNOWN_ADD_REF *PIUNKNOWN_ADD_REF;

typedef
ULONG
(STDAPICALLTYPE IUNKNOWN_RELEASE)(
    _In_ PIUNKNOWN This
    );
typedef IUNKNOWN_RELEASE *PIUNKNOWN_RELEASE;

//
// IDebugEventCallbacks
//

typedef
HRESULT
(STDAPICALLTYPE DEBUG_EVENT_QUERY_INTERFACE)(
    _In_ PIDEBUGEVENTCALLBACKS This,
    _In_ REFIID InterfaceId,
    _Out_ PPVOID Interface
    );
typedef DEBUG_EVENT_QUERY_INTERFACE *PDEBUG_EVENT_QUERY_INTERFACE;

typedef
ULONG
(STDAPICALLTYPE DEBUG_EVENT_ADD_REF)(
    _In_ PIDEBUGEVENTCALLBACKS This
    );
typedef DEBUG_EVENT_ADD_REF *PDEBUG_EVENT_ADD_REF;

typedef
ULONG
(STDAPICALLTYPE DEBUG_EVENT_RELEASE)(
    _In_ PIDEBUGEVENTCALLBACKS This
    );
typedef DEBUG_EVENT_RELEASE *PDEBUG_EVENT_RELEASE;

typedef
HRESULT
(STDAPICALLTYPE DEBUG_EVENT_GET_INTEREST_MASK_CALLBACK)(
    _In_ PIDEBUGEVENTCALLBACKS This,
    _Out_ PULONG Mask
    );
typedef DEBUG_EVENT_GET_INTEREST_MASK_CALLBACK
      *PDEBUG_EVENT_GET_INTEREST_MASK_CALLBACK;

typedef
HRESULT
(STDAPICALLTYPE DEBUG_EVENT_BREAKPOINT_CALLBACK)(
    _In_ PIDEBUGEVENTCALLBACKS This,
    _In_ PDEBUG_BREAKPOINT2 Breakpoint
    );
typedef DEBUG_EVENT_BREAKPOINT_CALLBACK *PDEBUG_EVENT_BREAKPOINT_CALLBACK;

typedef
HRESULT
(STDAPICALLTYPE DEBUG_EVENT_EXCEPTION_CALLBACK)(
    _In_ PIDEBUGEVENTCALLBACKS This,
    _In_ PEXCEPTION_RECORD64 Exception,
    _In_ ULONG FirstChance
    );
typedef DEBUG_EVENT_EXCEPTION_CALLBACK *PDEBUG_EVENT_EXCEPTION;

typedef
HRESULT
(STDAPICALLTYPE DEBUG_EVENT_CREATE_THREAD_CALLBACK)(
    _In_ PIDEBUGEVENTCALLBACKS This,
    _In_ ULONG64 Handle,
    _In_ ULONG64 DataOffset,
    _In_ ULONG64 StartOffset
    );
typedef DEBUG_EVENT_CREATE_THREAD_CALLBACK *PDEBUG_EVENT_CREATE_THREAD_CALLBACK;

typedef
HRESULT
(STDAPICALLTYPE DEBUG_EVENT_EXIT_THREAD_CALLBACK)(
    _In_ PIDEBUGEVENTCALLBACKS This,
    _In_ ULONG ExitCode
    );
typedef DEBUG_EVENT_EXIT_THREAD_CALLBACK *PDEBUG_EVENT_EXIT_THREAD_CALLBACK;

typedef
HRESULT
(STDAPICALLTYPE DEBUG_EVENT_CREATE_PROCESS_CALLBACK)(
    _In_ PIDEBUGEVENTCALLBACKS This,
    _In_ ULONG64 ImageFileHandle,
    _In_ ULONG64 Handle,
    _In_ ULONG64 BaseOffset,
    _In_ ULONG ModuleSize,
    _In_opt_ PCWSTR ModuleName,
    _In_opt_ PCWSTR ImageName,
    _In_ ULONG CheckSum,
    _In_ ULONG TimeDateStamp,
    _In_ ULONG64 InitialThreadHandle,
    _In_ ULONG64 ThreadDataOffset,
    _In_ ULONG64 StartOffset
    );
typedef DEBUG_EVENT_CREATE_PROCESS_CALLBACK
      *PDEBUG_EVENT_CREATE_PROCESS_CALLBACK;

typedef
_Analysis_noreturn_
HRESULT
(STDAPICALLTYPE DEBUG_EVENT_EXIT_PROCESS_CALLBACK)(
    _In_ PIDEBUGEVENTCALLBACKS This,
    _In_ ULONG ExitCode
    );
typedef DEBUG_EVENT_EXIT_PROCESS_CALLBACK *PDEBUG_EVENT_EXIT_PROCESS_CALLBACK;

typedef
HRESULT
(STDAPICALLTYPE DEBUG_EVENT_LOAD_MODULE_CALLBACK)(
    _In_ PIDEBUGEVENTCALLBACKS This,
    _In_ ULONG64 ImageFileHandle,
    _In_ ULONG64 BaseOffset,
    _In_ ULONG ModuleSize,
    _In_opt_ PCWSTR ModuleName,
    _In_opt_ PCWSTR ImageName,
    _In_ ULONG CheckSum,
    _In_ ULONG TimeDateStamp
    );
typedef DEBUG_EVENT_LOAD_MODULE_CALLBACK *PDEBUG_EVENT_LOAD_MODULE_CALLBACK;

typedef
HRESULT
(STDAPICALLTYPE DEBUG_EVENT_UNLOAD_MODULE_CALLBACK)(
    _In_ PIDEBUGEVENTCALLBACKS This,
    _In_opt_ PCWSTR ImageBaseName,
    _In_ ULONG64 BaseOffset
    );
typedef DEBUG_EVENT_UNLOAD_MODULE_CALLBACK *PDEBUG_EVENT_UNLOAD_MODULE_CALLBACK;

typedef
HRESULT
(STDAPICALLTYPE DEBUG_EVENT_SYSTEM_ERROR_CALLBACK)(
    _In_ PIDEBUGEVENTCALLBACKS This,
    _In_ ULONG Error,
    _In_ ULONG Level
    );
typedef DEBUG_EVENT_SYSTEM_ERROR_CALLBACK *PDEBUG_EVENT_SYSTEM_ERROR_CALLBACK;

typedef
HRESULT
(STDAPICALLTYPE DEBUG_EVENT_SESSION_STATUS_CALLBACK)(
    _In_ PIDEBUGEVENTCALLBACKS This,
    _In_ ULONG Status
    );
typedef DEBUG_EVENT_SESSION_STATUS_CALLBACK
      *PDEBUG_EVENT_SESSION_STATUS_CALLBACK;

typedef
HRESULT
(STDAPICALLTYPE DEBUG_EVENT_CHANGE_DEBUGGEE_STATE_CALLBACK)(
    _In_ PIDEBUGEVENTCALLBACKS This,
    _In_ ULONG Flags,
    _In_ ULONG64 Argument
    );
typedef DEBUG_EVENT_CHANGE_DEBUGGEE_STATE_CALLBACK
      *PDEBUG_EVENT_CHANGE_DEBUGGEE_STATE_CALLBACK;

typedef
HRESULT
(STDAPICALLTYPE DEBUG_EVENT_CHANGE_ENGINE_STATE_CALLBACK)(
    _In_ PIDEBUGEVENTCALLBACKS This,
    _In_ ULONG Flags,
    _In_ ULONG64 Argument
    );
typedef DEBUG_EVENT_CHANGE_ENGINE_STATE_CALLBACK
      *PDEBUG_EVENT_CHANGE_ENGINE_STATE_CALLBACK;

typedef
HRESULT
(STDAPICALLTYPE DEBUG_EVENT_CHANGE_SYMBOL_STATE_CALLBACK)(
    _In_ PIDEBUGEVENTCALLBACKS This,
    _In_ ULONG Flags,
    _In_ ULONG64 Argument
    );
typedef DEBUG_EVENT_CHANGE_SYMBOL_STATE_CALLBACK
      *PDEBUG_EVENT_CHANGE_SYMBOL_STATE_CALLBACK;

//
// IDebugInputCallbacks
//

typedef
HRESULT
(STDAPICALLTYPE DEBUG_INPUT_QUERY_INTERFACE)(
    _In_ PIDEBUGINPUTCALLBACKS This,
    _In_ REFIID InterfaceId,
    _Out_ PPVOID Interface
    );
typedef DEBUG_INPUT_QUERY_INTERFACE *PDEBUG_INPUT_QUERY_INTERFACE;

typedef
ULONG
(STDAPICALLTYPE DEBUG_INPUT_ADD_REF)(
    _In_ PIDEBUGINPUTCALLBACKS This
    );
typedef DEBUG_INPUT_ADD_REF *PDEBUG_INPUT_ADD_REF;

typedef
ULONG
(STDAPICALLTYPE DEBUG_INPUT_RELEASE)(
    _In_ PIDEBUGINPUTCALLBACKS This
    );
typedef DEBUG_INPUT_RELEASE *PDEBUG_INPUT_RELEASE;

typedef
HRESULT
(STDAPICALLTYPE DEBUG_INPUT_END_INPUT_CALLBACK)(
    _In_ PIDEBUGINPUTCALLBACKS This
    );
typedef DEBUG_INPUT_END_INPUT_CALLBACK *PDEBUG_INPUT_END_INPUT_CALLBACK;

typedef
HRESULT
(STDAPICALLTYPE DEBUG_INPUT_START_INPUT_CALLBACK)(
    _In_ PIDEBUGINPUTCALLBACKS This,
    _In_ ULONG BufferSize
    );
typedef DEBUG_INPUT_START_INPUT_CALLBACK *PDEBUG_INPUT_START_INPUT_CALLBACK;

//
// IDebugOutputCallbacks
//

typedef
HRESULT
(STDAPICALLTYPE DEBUG_OUTPUT_CALLBACKS_QUERY_INTERFACE)(
    _In_ PIDEBUGOUTPUTCALLBACKS This,
    _In_ REFIID InterfaceId,
    _Out_ PPVOID Interface
    );
typedef DEBUG_OUTPUT_CALLBACKS_QUERY_INTERFACE
      *PDEBUG_OUTPUT_CALLBACKS_QUERY_INTERFACE;

typedef
ULONG
(STDAPICALLTYPE DEBUG_OUTPUT_CALLBACKS_ADD_REF)(
    _In_ PIDEBUGOUTPUTCALLBACKS This
    );
typedef DEBUG_OUTPUT_CALLBACKS_ADD_REF *PDEBUG_OUTPUT_CALLBACKS_ADD_REF;

typedef
ULONG
(STDAPICALLTYPE DEBUG_OUTPUT_CALLBACKS_RELEASE)(
    _In_ PIDEBUGOUTPUTCALLBACKS This
    );
typedef DEBUG_OUTPUT_CALLBACKS_RELEASE *PDEBUG_OUTPUT_CALLBACKS_RELEASE;

typedef
HRESULT
(STDAPICALLTYPE DEBUG_OUTPUT_CALLBACKS_OUTPUT)(
    _In_ PIDEBUGOUTPUTCALLBACKS This,
    _In_ ULONG Mask,
    _In_ PCSTR String
    );
typedef DEBUG_OUTPUT_CALLBACKS_OUTPUT *PDEBUG_OUTPUT_CALLBACKS_OUTPUT;

//
// IDebugOutputCallbacks2
//

typedef
HRESULT
(STDAPICALLTYPE DEBUG_OUTPUT_CALLBACKS2_QUERY_INTERFACE)(
    _In_ PIDEBUGOUTPUTCALLBACKS2 This,
    _In_ REFIID InterfaceId,
    _Out_ PPVOID Interface
    );
typedef DEBUG_OUTPUT_CALLBACKS2_QUERY_INTERFACE
      *PDEBUG_OUTPUT_CALLBACKS2_QUERY_INTERFACE;

typedef
ULONG
(STDAPICALLTYPE DEBUG_OUTPUT_CALLBACKS2_ADD_REF)(
    _In_ PIDEBUGOUTPUTCALLBACKS2 This
    );
typedef DEBUG_OUTPUT_CALLBACKS2_ADD_REF *PDEBUG_OUTPUT_CALLBACKS2_ADD_REF;

typedef
ULONG
(STDAPICALLTYPE DEBUG_OUTPUT_CALLBACKS2_RELEASE)(
    _In_ PIDEBUGOUTPUTCALLBACKS2 This
    );
typedef DEBUG_OUTPUT_CALLBACKS2_RELEASE *PDEBUG_OUTPUT_CALLBACKS2_RELEASE;

typedef
HRESULT
(STDAPICALLTYPE DEBUG_OUTPUT_CALLBACKS2_OUTPUT)(
    _In_ PIDEBUGOUTPUTCALLBACKS2 This,
    _In_ ULONG Mask,
    _In_ PCSTR String
    );
typedef DEBUG_OUTPUT_CALLBACKS2_OUTPUT *PDEBUG_OUTPUT_CALLBACKS2_OUTPUT;

typedef
HRESULT
(STDAPICALLTYPE DEBUG_OUTPUT_CALLBACKS2_GET_INTEREST_MASK)(
    _In_ PIDEBUGOUTPUTCALLBACKS2 This,
    _Out_ PULONG Mask
    );
typedef DEBUG_OUTPUT_CALLBACKS2_GET_INTEREST_MASK
      *PDEBUG_OUTPUT_CALLBACKS2_GET_INTEREST_MASK;

typedef
HRESULT
(STDAPICALLTYPE DEBUG_OUTPUT_CALLBACKS2_OUTPUT2)(
    _In_ PIDEBUGOUTPUTCALLBACKS2 This,
    _In_ ULONG Which,
    _In_ ULONG Flags,
    _In_ ULONG64 Arg,
    _In_opt_ PCWSTR Text
    );
typedef DEBUG_OUTPUT_CALLBACKS2_OUTPUT2 *PDEBUG_OUTPUT_CALLBACKS2_OUTPUT2;

////////////////////////////////////////////////////////////////////////////////
// End COM Function Pointer Type Definitions
////////////////////////////////////////////////////////////////////////////////

#ifdef __cplusplus
} // extern "C"
#endif

// vim:set ts=8 sw=4 sts=4 tw=80 expandtab                                     :
