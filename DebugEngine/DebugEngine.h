/*++

Copyright (c) 2017 Trent Nelson <trent@trent.me>

Module Name:

    DebugEngine.h

Abstract:

    This is the header file for the DebugEngine module, which is a helper
    module that exposes a subset of functionality implemented by the Windows
    Debug Engine component (DbgEng.dll).

--*/

#pragma once

#ifdef _DEBUG_ENGINE_INTERNAL_BUILD

//
// This is an internal build of the DebugEngine component.
//

#ifdef _DEBUG_ENGINE_DLL_BUILD

//
// This is the DLL build.
//

#define DEBUG_ENGINE_API __declspec(dllexport)
#define DEBUG_ENGINE_DATA extern __declspec(dllexport)

#else

//
// This is the static library build.
//

#define DEBUG_ENGINE_API
#define DEBUG_ENGINE_DATA extern

#endif

#include "stdafx.h"

#else

//
// We're being included by an external component.
//

#define DEBUG_ENGINE_API __declspec(dllimport)
#define DEBUG_ENGINE_DATA extern __declspec(dllimport)

#include <Windows.h>
#pragma component(browser, off)
#include <DbgEng.h>
#pragma component(browser, on)
#include "../Rtl/Rtl.h"

#endif

//
// Declare IIDs of the DbgEng COM classes we use.
//

typedef const GUID *PCGUID;

DEBUG_ENGINE_DATA CONST GUID IID_IDEBUG_CLIENT;
DEBUG_ENGINE_DATA CONST GUID IID_IDEBUG_CONTROL;

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

//
// IDebugOutputCallbacks
//

typedef struct IDebugOutputCallbacks2 IDEBUGOUTPUTCALLBACKS;
typedef IDEBUGOUTPUTCALLBACKS *PIDEBUGOUTPUTCALLBACKS;

typedef struct IDebugOutputCallbacks2Vtbl DEBUGOUTPUTCALLBACKS;
typedef DEBUGOUTPUTCALLBACKS *PDEBUGOUTPUTCALLBACKS;

//
// IDebugInputCallbacks
//

typedef struct IDebugInputCallbacks IDEBUGINPUTCALLBACKS;
typedef IDEBUGINPUTCALLBACKS *PIDEBUGINPUTCALLBACKS;

typedef struct IDebugInputCallbacksVtbl DEBUGINPUTCALLBACKS;
typedef DEBUGINPUTCALLBACKS *PDEBUGINPUTCALLBACKS;

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
// IDebugOutputCallbacks
//

typedef
HRESULT
(STDAPICALLTYPE DEBUG_OUTPUT_QUERY_INTERFACE)(
    _In_ PIDEBUGOUTPUTCALLBACKS This,
    _In_ REFIID InterfaceId,
    _Out_ PPVOID Interface
    );
typedef DEBUG_OUTPUT_QUERY_INTERFACE *PDEBUG_OUTPUT_QUERY_INTERFACE;

typedef
ULONG
(STDAPICALLTYPE DEBUG_OUTPUT_ADD_REF)(
    _In_ PIDEBUGOUTPUTCALLBACKS This
    );
typedef DEBUG_OUTPUT_ADD_REF *PDEBUG_OUTPUT_ADD_REF;

typedef
ULONG
(STDAPICALLTYPE DEBUG_OUTPUT_RELEASE)(
    _In_ PIDEBUGOUTPUTCALLBACKS This
    );
typedef DEBUG_OUTPUT_RELEASE *PDEBUG_OUTPUT_RELEASE;

typedef
HRESULT
(STDAPICALLTYPE DEBUG_OUTPUT_OUTPUT_CALLBACK)(
    _In_ PIDEBUGOUTPUTCALLBACKS This,
    _In_ ULONG Mask,
    _In_ PCSTR String
    );
typedef DEBUG_OUTPUT_OUTPUT_CALLBACK *PDEBUG_OUTPUT_OUTPUT_CALLBACK;

typedef
HRESULT
(STDAPICALLTYPE DEBUG_OUTPUT_GET_INTEREST_MASK_CALLBACK)(
    _In_ PIDEBUGOUTPUTCALLBACKS This,
    _Out_ PULONG Mask
    );
typedef DEBUG_OUTPUT_GET_INTEREST_MASK_CALLBACK
      *PDEBUG_OUTPUT_GET_INTEREST_MASK_CALLBACK;

typedef
HRESULT
(STDAPICALLTYPE DEBUG_OUTPUT_OUTPUT2_CALLBACK)(
    _In_ PIDEBUGOUTPUTCALLBACKS This,
    _In_ ULONG Which,
    _In_ ULONG Flags,
    _In_ ULONG64 Arg,
    _In_opt_ PCWSTR Text
    );
typedef DEBUG_OUTPUT_OUTPUT2_CALLBACK *PDEBUG_OUTPUT_OUTPUT2_CALLBACK;

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

////////////////////////////////////////////////////////////////////////////////
// End COM Function Pointer Type Definitions
////////////////////////////////////////////////////////////////////////////////

//
// Define helper bitflag structs that simplify working with various DEBUG_*
// constants.
//

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
// DebugEngine-related structures.
//

typedef union _DEBUG_ENGINE_FLAGS {
    ULONG AsLong;
    struct {
        ULONG Unused:1;
    };
} DEBUG_ENGINE_FLAGS; *PDEBUG_ENGINE_FLAGS;

typedef struct _Struct_size_bytes_(SizeOfStruct) _DEBUG_ENGINE {

    //
    // Size of the structure, in bytes.
    //

    _Field_range_(==, sizeof(struct _DEBUG_ENGINE)) ULONG SizeOfStruct;

    //
    // Flags.
    //

    DEBUG_ENGINE_FLAGS Flags;

    //
    // IIDs, interfaces and vtables.
    //

    PCGUID IID_Client;
    PIDEBUGCLIENT IClient;
    PDEBUGCLIENT Client;

    PCGUID IID_Control;
    PIDEBUGCONTROL IControl;
    PDEBUGCONTROL Control;

    PCGUID IID_Symbols;
    PIDEBUGSYMBOLS ISymbols;
    PDEBUGSYMBOLS Symbols;

    PCGUID IID_SymbolGroup;
    PIDEBUGSYMBOLGROUP ISymbolGroup;
    PDEBUGSYMBOLGROUP SymbolGroup;

    PCGUID IID_Advanced;
    PIDEBUGADVANCED IAdvanced;
    PDEBUGADVANCED Advanced;

    PCGUID IID_DataSpaces;
    PIDEBUGDATASPACES IDataSpaces;
    PDEBUGDATASPACES DataSpaces;

    //
    // Client/Control state.
    //

    ULONG SessionStatus;

    DEBUG_EXECUTION_STATUS ExecutionStatus;

    ULONG EngineState;
    ULONGLONG EngineStateArg;
    DEBUG_CHANGE_ENGINE_STATE ChangeEngineState;

    ULONG DebuggeeState;
    ULONGLONG DebuggeeStateArg;

    ULONG SymbolState;
    ULONGLONG SymbolStateArg;

    ULONG ActualProcessorType;
    struct {
        ULONG Error;
        ULONG Level;
    } SystemError;

    //
    // Callback-related fields.
    //

    volatile LONG EventCallbackRefCount;
    ULONG EventCallbacksInterestMask;
    IDEBUGEVENTCALLBACKS IEventCallbacks;
    DEBUGEVENTCALLBACKS EventCallbacks;

    volatile LONG OutputCallbackRefCount;
    ULONG OutputCallbacksInterestMask;
    IDEBUGOUTPUTCALLBACKS IOutputCallbacks;
    DEBUGOUTPUTCALLBACKS OutputCallbacks;

    volatile LONG InputCallbackRefCount;
    ULONG Unused;
    IDEBUGINPUTCALLBACKS IInputCallbacks;
    DEBUGINPUTCALLBACKS InputCallbacks;

} DEBUG_ENGINE, *PDEBUG_ENGINE, **PPDEBUG_ENGINE;

//
// Define our DEBUG_ENGINE_CONTEXT structure.
//

typedef union _DEBUG_ENGINE_CONTEXT_FLAGS {
    ULONG AsLong;
    struct {
        ULONG Unused:1;
    };
} DEBUG_ENGINE_CONTEXT_FLAGS; *PDEBUG_ENGINE_CONTEXT_FLAGS;

typedef struct _Struct_size_bytes_(SizeOfStruct) _DEBUG_ENGINE_CONTEXT {

    //
    // Size of the structure, in bytes.
    //

    _Field_range_(==, sizeof(struct _DEBUG_ENGINE_CONTEXT)) ULONG SizeOfStruct;

    //
    // Flags.
    //

    DEBUG_ENGINE_CONTEXT_FLAGS Flags;

    //
    // Debug engine.
    //

    PDEBUG_ENGINE Engine;

} DEBUG_ENGINE_CONTEXT, *PDEBUG_ENGINE_CONTEXT, **PPDEBUG_ENGINE_CONTEXT;

//
// Define DEBUG_ENGINE_SESSION.
//

typedef union _DEBUG_ENGINE_SESSION_FLAGS {
    ULONG AsLong;
    struct {
        ULONG Unused:1;
    };
} DEBUG_ENGINE_SESSION_FLAGS; *PDEBUG_ENGINE_SESSION_FLAGS;

//
// Forward definition of the start function such that it can be referenced in
// the struct.
//

typedef
_Check_return_
_Success_(return != 0)
BOOL
(START_DEBUG_ENGINE_SESSION)(
    _In_ struct _DEBUG_ENGINE_SESSION *Session
    );
typedef START_DEBUG_ENGINE_SESSION *PSTART_DEBUG_ENGINE_SESSION;

typedef
VOID
(DESTROY_DEBUG_ENGINE_SESSION)(
    _Pre_notnull_ _Post_null_ struct _DEBUG_ENGINE_SESSION **Session
    );
typedef DESTROY_DEBUG_ENGINE_SESSION   *PDESTROY_DEBUG_ENGINE_SESSION;
typedef DESTROY_DEBUG_ENGINE_SESSION **PPDESTROY_DEBUG_ENGINE_SESSION;
DEBUG_ENGINE_API DESTROY_DEBUG_ENGINE_SESSION DestroyDebugEngineSession;

typedef struct _Struct_size_bytes_(SizeOfStruct) _DEBUG_ENGINE_SESSION {

    //
    // Size of the structure, in bytes.
    //

    _Field_range_(==, sizeof(struct _DEBUG_ENGINE_SESSION)) ULONG SizeOfStruct;

    //
    // Flags.
    //

    DEBUG_ENGINE_SESSION_FLAGS Flags;

    //
    // Debug engine.
    //

    DEBUG_ENGINE Engine;

    //
    // The start routine for the session.
    //

    PSTART_DEBUG_ENGINE_SESSION Start;

    //
    // Destructor.
    //

    PDESTROY_DEBUG_ENGINE_SESSION Destroy;

    //
    // Rtl structure.
    //

    PRTL Rtl;

    //
    // TracerConfig structure.
    //

    struct _TRACER_CONFIG *TracerConfig;

    //
    // Commandline-related fields.
    //

    PSTR CommandLineA;
    PWSTR CommandLineW;

    STRING CommandLineString;
    UNICODE_STRING CommandLineUnicodeString;

    LONG NumberOfArguments;
    PPSTR ArgvA;
    PPWSTR ArgvW;

    //
    // Target information extracted from command line.
    //

    ULONG TargetProcessId;
    ULONG TargetMainThreadId;

    //
    // Trace Session Directory.
    //

    struct _RTL_PATH TraceSessionDirectory;

    //
    // Run History registry path.
    //

    UNICODE_STRING RunHistoryRegistryPath;

    //
    // Handles to the target process and main thread.
    //

    HANDLE TargetProcessHandle;
    HANDLE TargetMainThreadHandle;

    HKEY RunHistoryRegistryKey;

} DEBUG_ENGINE_SESSION, *PDEBUG_ENGINE_SESSION, **PPDEBUG_ENGINE_SESSION;

//
// Flags for CreateAndInitializeDebugEngineSession().
//

typedef union _DEBUG_ENGINE_SESSION_INIT_FLAGS {
    LONG AsLong;
    ULONG AsULong;
    struct {

        //
        // When set, extracts the target process ID and other relevant session
        // parameters from the command line.
        //

        ULONG InitializeFromCommandLine:1;

        //
        // When set, the debug engine attaches to the currently running process.
        //

        ULONG InitializeFromCurrentProcess:1;
    };
} DEBUG_ENGINE_SESSION_INIT_FLAGS;

//
// Public function typedefs.
//

typedef
_Check_return_
_Success_(return != 0)
BOOL
(INTIALIZE_DEBUG_ENGINE_SESSION)(
    _In_ PRTL Rtl,
    _In_ PALLOCATOR Allocator,
    _In_ DEBUG_ENGINE_SESSION_INIT_FLAGS Flags,
    _Outptr_result_nullonfailure_ PPDEBUG_ENGINE_SESSION SessionPointer
    );
typedef INTIALIZE_DEBUG_ENGINE_SESSION *PINTIALIZE_DEBUG_ENGINE_SESSION;

//
// Inline Functions
//

FORCEINLINE
_Check_return_
_Success_(return != 0)
BOOL
LoadAndInitializeDebugEngineSession(
    _In_ PUNICODE_STRING DebugEngineDllPath,
    _In_ PRTL Rtl,
    _In_ PALLOCATOR Allocator,
    _In_ DEBUG_ENGINE_SESSION_INIT_FLAGS InitFlags,
    _Out_ PPDEBUG_ENGINE_SESSION SessionPointer,
    _Out_ PPDESTROY_DEBUG_ENGINE_SESSION DestroyDebugEngineSessionPointer
    )
/*++

Routine Description:

    This routine loads the DebugEngine DLL from the given path, resolves the
    routine InitializeDebugEngineSession(), then calls it with the same
    arguments as passed in.

Arguments:

    DebugEngineDllPath - Supplies a pointer to a UNICODE_STRING that contains
        the fully-qualified path of the DebugEngine DLL to load.

    Rtl - Supplies a pointer to an RTL structure.

    Allocator - Supplies a pointer to an alternate ALLOCATOR to use.

    InitFlags - Supplies flags that can be used to customize the type of
        debug session created.

    SessionPointer - Supplies a pointer that will receive the address of the
        DEBUG_ENGINE_SESSION structure allocated.  This pointer is immediately
        cleared (that is, '*SessionPointer = NULL;' is performed once it is
        deemed non-NULL), and a value will only be set if initialization
        was successful.

    DestroyDebugEngineSessionPointer - Supplies a pointer to the address of a
        variable that will receive the address of the DLL export by the same
        name.  This should be called in order to destroy a successfully
        initialized session.

Return Value:

    TRUE on Success, FALSE if an error occurred.  *SessionPointer will be
    updated with the value of the newly created DEBUG_ENGINE_SESSION structure.

See Also:

    InitializeDebugEngineSession().

--*/
{
    BOOL Success;
    HMODULE Module;
    PINTIALIZE_DEBUG_ENGINE_SESSION InitializeDebugEngineSession;
    PDESTROY_DEBUG_ENGINE_SESSION DestroyDebugEngineSession;

    //
    // Validate arguments.
    //

    if (!ARGUMENT_PRESENT(DebugEngineDllPath)) {
        return FALSE;
    }

    if (!ARGUMENT_PRESENT(Allocator)) {
        return FALSE;
    }

    if (!ARGUMENT_PRESENT(SessionPointer)) {
        return FALSE;
    }

    if (!IsValidMinimumDirectoryUnicodeString(DebugEngineDllPath)) {
        return FALSE;
    }

    //
    // Attempt to load the library.
    //

    Module = LoadLibraryW(DebugEngineDllPath->Buffer);

    if (!Module) {
        OutputDebugStringA("Failed to load TracedPythonSessionDllPath.\n");
        return FALSE;
    }

    //
    // Resolve the initialize and destroy functions.
    //

    InitializeDebugEngineSession = (PINTIALIZE_DEBUG_ENGINE_SESSION)(
        GetProcAddress(
            Module,
            "InitializeDebugEngineSession"
        )
    );

    if (!InitializeDebugEngineSession) {
        OutputDebugStringA("Failed to resolve InitializeDebugEngineSession\n");
        goto Error;
    }

    DestroyDebugEngineSession = (PDESTROY_DEBUG_ENGINE_SESSION)(
        GetProcAddress(
            Module,
            "DestroyDebugEngineSession"
        )
    );

    if (!DestroyDebugEngineSession) {
        OutputDebugStringA("Failed to resolve DestroyDebugEngineSession\n");
        goto Error;
    }

    //
    // Call the initialization function with the same arguments we were passed.
    //

    Success = InitializeDebugEngineSession(
        Rtl,
        Allocator,
        InitFlags,
        SessionPointer
    );

    if (!Success) {
        goto Error;
    }

    //
    // Update the caller's DestroyDebugEngineSession function pointer and the
    // session pointer to the same function.
    //

    *DestroyDebugEngineSessionPointer = DestroyDebugEngineSession;
    (*SessionPointer)->Destroy = DestroyDebugEngineSession;

    //
    // Return success.
    //

    return TRUE;

Error:

    //
    // Attempt to destroy the session.
    //

    if (SessionPointer && *SessionPointer && DestroyDebugEngineSession) {

        //
        // This will also clear the caller's session pointer.
        //

        DestroyDebugEngineSession(SessionPointer);
    }

    //
    // Attempt to free the module.
    //

    if (Module) {
        FreeLibrary(Module);
    }

    return FALSE;
}

//
// Public function declarations.
//

#pragma component(browser, off)
DEBUG_ENGINE_API INTIALIZE_DEBUG_ENGINE_SESSION InitializeDebugEngineSession;
DEBUG_ENGINE_API DESTROY_DEBUG_ENGINE_SESSION DebugEngineSession;
#pragma component(browser, on)

#ifdef __cplusplus
} // extern "C"
#endif

// vim:set ts=8 sw=4 sts=4 tw=80 expandtab                                     :
