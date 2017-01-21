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

#include "stdafx.h"

#ifdef __cplusplus
extern "C" {
#endif

#pragma component(browser, off)
#include <DbgEng.h>
#pragma component(browser, on)

#define DEBUG_ENGINE_API RTL_API

//
// Declare IIDs of the DbgEng COM classes we use.
//

typedef const GUID *PCGUID;

RTL_DATA CONST GUID IID_IDEBUG_CLIENT;
RTL_DATA CONST GUID IID_IDEBUG_CONTROL;

//
// Define typedefs for the COM interfaces we want to use.
//

typedef struct IDebugClient7 IDEBUGCLIENT7;
typedef IDEBUGCLIENT7 *PIDEBUGCLIENT7;

typedef struct IDebugClient7Vtbl IDEBUGCLIENT7VTBL;
typedef IDEBUGCLIENT7VTBL *PIDEBUGCLIENT7VTBL;

typedef struct IDebugControl7 IDEBUGCONTROL7;
typedef IDEBUGCONTROL7 *PIDEBUGCONTROL7;

typedef struct IDebugControl7Vtbl IDEBUGCONTROL7VTBL;
typedef IDEBUGCONTROL7VTBL *PIDEBUGCONTROL7VTBL;

typedef IDEBUGCLIENT7     IDEBUGCLIENT;
typedef IDEBUGCLIENT   *PIDEBUGCLIENT;
typedef IDEBUGCLIENT **PPIDEBUGCLIENT;

typedef IDEBUGCLIENT7VTBL   DEBUGCLIENT;
typedef DEBUGCLIENT      *PDEBUGCLIENT;
typedef DEBUGCLIENT    **PPDEBUGCLIENT;

typedef IDEBUGCONTROL7     IDEBUGCONTROL;
typedef IDEBUGCONTROL   *PIDEBUGCONTROL;
typedef IDEBUGCONTROL **PPIDEBUGCONTROL;

typedef IDEBUGCONTROL7VTBL   DEBUGCONTROL;
typedef DEBUGCONTROL      *PDEBUGCONTROL;
typedef DEBUGCONTROL    **PPDEBUGCONTROL;

typedef struct IDebugEventCallbacks IDEBUGEVENTCALLBACKS;
typedef IDEBUGEVENTCALLBACKS *PIDEBUGEVENTCALLBACKS;

typedef struct IDebugEventCallbacksVtbl DEBUGEVENTCALLBACKSVTBL;
typedef DEBUGEVENTCALLBACKSVTBL *PDEBUGEVENTCALLBACKSVTBL;

//
//

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
HRESULT
(STDAPICALLTYPE IUNKNOWN_ADD_REF)(
    _In_ PIUNKNOWN This
    );
typedef IUNKNOWN_ADD_REF *PIUNKNOWN_ADD_REF;

typedef
HRESULT
(STDAPICALLTYPE IUNKNOWN_RELEASE)(
    _In_ PIUNKNOWN This
    );
typedef IUNKNOWN_RELEASE *PIUNKNOWN_RELEASE;

//
// IDebugEventCallbacks
//

typedef
HRESULT
(STDAPICALLTYPE DEBUG_EVENT_GET_INTEREST_MASK)(
    _In_ PIDEBUGEVENTCALLBACKS This,
    _Out_ PULONG Mask
    );

typedef
HRESULT
(STDAPICALLTYPE DEBUG_EVENT_BREAKPOINT_CALLBACK)(
    _In_ PIDEBUGEVENTCALLBACKS This,
    _In_ PDEBUG_BREAKPOINT Breakpoint
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
    _In_opt_ PCSTR ModuleName,
    _In_opt_ PCSTR ImageName,
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
    _In_opt_ PCSTR ModuleName,
    _In_opt_ PCSTR ImageName,
    _In_ ULONG CheckSum,
    _In_ ULONG TimeDateStamp
    );
typedef DEBUG_EVENT_LOAD_MODULE_CALLBACK *PDEBUG_EVENT_LOAD_MODULE_CALLBACK;

typedef
HRESULT
(STDAPICALLTYPE DEBUG_EVENT_UNLOAD_MODULE_CALLBACK)(
    _In_ PIDEBUGEVENTCALLBACKS This,
    _In_opt_ PCSTR ImageBaseName,
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
(STDAPICALLTYPE DEBUG_OUTPUT_CALLBACK)(
    _In_ PDEBUG_OUTPUT_CALLBACKS This,
    _In_ ULONG Mask,
    _In_ PCSTR String
    );

////////////////////////////////////////////////////////////////////////////////
// End COM Function Pointer Type Definitions
////////////////////////////////////////////////////////////////////////////////

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
// Forward definition of the start fuction such that it can be referenced in
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
    ULONG AsLong;
    struct {

        //
        // When set, extracts the target process ID and other relevant session
        // parameters from the command line.
        //

        ULONG InitializeFromCommandLine:1;
    };
} DEBUG_ENGINE_SESSION_INIT_FLAGS;

//
// Public function typedefs.
//

typedef
_Check_return_
_Success_(return != 0)
BOOL
(CREATE_AND_INTIALIZE_DEBUG_ENGINE_SESSION)(
    _In_ PRTL Rtl,
    _In_ PALLOCATOR Allocator,
    _In_ DEBUG_ENGINE_SESSION_INIT_FLAGS Flags,
    _Outptr_result_nullonfailure_ PPDEBUG_ENGINE_SESSION SessionPointer
    );
typedef CREATE_AND_INTIALIZE_DEBUG_ENGINE_SESSION
      *PCREATE_AND_INTIALIZE_DEBUG_ENGINE_SESSION;

//
// Public function declarations.
//

#pragma component(browser, off)
DEBUG_ENGINE_API CREATE_AND_INTIALIZE_DEBUG_ENGINE_SESSION
                 CreateAndInitializeDebugEngineSession;
#pragma component(browser, on)

#ifdef __cplusplus
} // extern "C"
#endif

// vim:set ts=8 sw=4 sts=4 tw=80 expandtab                                     :
