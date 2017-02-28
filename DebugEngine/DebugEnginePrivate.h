/*++

Copyright (c) 2017 Trent Nelson <trent@trent.me>

Module Name:

    DebugEnginePrivate.h

Abstract:

    This is the private header file for the DebugEngine module.  It defines
    function typedefs and declares functions for all major (i.e. not local to
    the individual module/file) routines available for use by individual modules
    within this component, as well as defining supporting structures and enums.

--*/

#pragma once

#include "stdafx.h"

#ifdef __cplusplus
extern "C" {
#endif

#define DEFINE_GUID_EX(name, l, w1, w2, b1, b2, b3, b4, b5, b6, b7, b8) \
    const GUID DECLSPEC_SELECTANY name                                  \
        = { l, w1, w2, { b1, b2,  b3,  b4,  b5,  b6,  b7,  b8 } }

//
// Private function typedefs.
//

typedef
_Check_return_
_Success_(return != 0)
_Requires_exclusive_lock_held_(DebugEngine->Lock)
BOOL
(INITIALIZE_DEBUG_ENGINE)(
    _In_ PRTL Rtl,
    _In_ struct _DEBUG_ENGINE *DebugEngine
    );
typedef INITIALIZE_DEBUG_ENGINE *PINITIALIZE_DEBUG_ENGINE;

typedef
_Requires_lock_not_held_(**EnginePointer->Lock)
VOID
(DESTROY_DEBUG_ENGINE)(
    _Pre_notnull_ _Post_satisfies_(*Session == 0)
        struct _DEBUG_ENGINE **EnginePointer,
    _In_opt_ PBOOL IsProcessTerminating
    );
typedef DESTROY_DEBUG_ENGINE *PDESTROY_DEBUG_ENGINE;

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
(DEBUG_ENGINE_UNASSEMBLE_ADDRESS)(
    _In_ PDEBUG_ENGINE_SESSION Session,
    _In_ ULONG64 Offset,
    _In_ PSTRING Buffer
    );
typedef DEBUG_ENGINE_UNASSEMBLE_ADDRESS *PDEBUG_ENGINE_UNASSEMBLE_ADDRESS;

//
// DEBUG_ENGINE_OUTPUT related function typedefs and structures.
//

typedef
HRESULT
(STDAPICALLTYPE DEBUG_ENGINE_OUTPUT_CALLBACK)(
    _In_ struct _DEBUG_ENGINE *DebugEngine,
    _In_ DEBUG_OUTPUT_MASK OutputMask,
    _In_ PCSTR Text
    );
typedef DEBUG_ENGINE_OUTPUT_CALLBACK *PDEBUG_ENGINE_OUTPUT_CALLBACK;

typedef
HRESULT
(STDAPICALLTYPE DEBUG_ENGINE_OUTPUT_CALLBACK2)(
    _In_ struct _DEBUG_ENGINE *DebugEngine,
    _In_ DEBUG_OUTPUT_TYPE OutputType,
    _In_ DEBUG_OUTPUT_CALLBACK_FLAGS OutputFlags,
    _In_ ULONG64 Arg,
    _In_ PCWSTR Text
    );
typedef DEBUG_ENGINE_OUTPUT_CALLBACK2 *PDEBUG_ENGINE_OUTPUT_CALLBACK2;

//
// DebugEngineCommands-related function typedefs and structures.
//

typedef enum _Enum_is_bitflag_ _DEBUG_ENGINE_COMMAND_ID {
    DebugEngineNullCommandId            =       0,
    ExamineSymbolsCommandId             =       1,
    UnassembleFunctionCommandId         = (1 << 1),
    DisplayTypeCommandId                = (1 << 2),
    SettingsMetaCommandId               = (1 << 3),

    //
    // Make sure the expression within parenthesis is identical to the last
    // enumeration above.
    //

    DebugEngineInvalidCommandId         = (1 << 3) + 1
} DEBUG_ENGINE_COMMAND_ID;

FORCEINLINE
BOOL
IsValidDebugEngineCommandId(
    _In_ DEBUG_ENGINE_COMMAND_ID CommandId
    )
{
    return (
        (CommandId == ExamineSymbolsCommandId) || (
            CommandId >= UnassembleFunctionCommandId &&
            CommandId < DebugEngineInvalidCommandId &&
            IsPowerOf2(CommandId)
        )
    );
}

FORCEINLINE
LONG
CommandIdToArrayIndex(
    _In_ DEBUG_ENGINE_COMMAND_ID CommandId
    )
{
    if (!IsValidDebugEngineCommandId(CommandId)) {
        return -1;
    }

    return TrailingZeros(CommandId);
}

typedef union _DEBUG_ENGINE_COMMAND_TYPE {
    LONG AsLong;
    ULONG AsULong;
    struct _Struct_size_bytes_(sizeof(ULONG)) {
        ULONG ExamineSymbols:1;
        ULONG UnassembleFunction:1;
        ULONG DisplayType:1;
        ULONG Unused:29;
    };
} DEBUG_ENGINE_COMMAND_TYPE;
C_ASSERT(sizeof(DEBUG_ENGINE_COMMAND_TYPE) == sizeof(ULONG));

typedef struct _Struct_size_bytes_(sizeof(ULONG)) _DEBUG_ENGINE_COMMAND_FLAGS {
    ULONG HasOptions:1;
    ULONG HasModuleName:1;
    ULONG HasExclamationPoint:1;
    ULONG MandatoryArgument:1;
    ULONG OptionalArgument:1;
    ULONG ArgumentDefaultsToAsterisk:1;
    ULONG Unused:1;
} DEBUG_ENGINE_COMMAND_FLAGS;
C_ASSERT(sizeof(DEBUG_ENGINE_COMMAND_FLAGS) == sizeof(ULONG));
typedef DEBUG_ENGINE_COMMAND_FLAGS *PDEBUG_ENGINE_COMMAND_FLAGS;

typedef struct _DEBUG_ENGINE_COMMAND_OPTIONS {
    PPWSTR OptionStrings;
} DEBUG_ENGINE_COMMAND_OPTIONS;

typedef
_Check_return_
_Success_(return != 0)
USHORT
(CALLBACK DEBUG_ENGINE_COMMAND_GET_OPTIONS_CALLBACK)(
    _In_ PDEBUG_ENGINE_OUTPUT Output,
    _In_ struct _DEBUG_ENGINE_COMMAND_TEMPLATE *CommandTemplate,
    _In_ ULONG CommandFlags,
    _In_ PUNICODE_STRING OptionsBuffer,
    _In_ PBOOL OptionsBufferTooSmall
    );
typedef DEBUG_ENGINE_COMMAND_GET_OPTIONS_CALLBACK
      *PDEBUG_ENGINE_COMMAND_GET_OPTIONS_CALLBACK;

typedef
struct _Struct_size_bytes_(SizeOfStruct) _DEBUG_ENGINE_COMMAND_TEMPLATE {

    //
    // Size of the structure, in bytes.
    //

    _Field_range_(==, sizeof(struct _DEBUG_ENGINE_COMMAND_TEMPLATE))
        ULONG SizeOfStruct;

    //
    // Flags.
    //

    DEBUG_ENGINE_COMMAND_FLAGS Flags;

    //
    // Command Type.
    //

    DEBUG_ENGINE_COMMAND_TYPE Type;

    //
    // Number of options to the command.
    //

    ULONG NumberOfOptions;

    //
    // Array of UNICODE_STRING option strings.
    //

    PUNICODE_STRING Options;

    //
    // Command name.
    //

    PCSTRING CommandName;
    PCUNICODE_STRING CommandNameWide;

    //
    // Optional friendly/display name for the command.
    //

    PSTRING CommandDisplayName;

    //
    // Optional line parsing routine that converts the line output from the
    // command into a custom structure (or array of custom structures).
    //

    PDEBUG_ENGINE_PARSE_LINES_INTO_CUSTOM_STRUCTURE_CALLBACK
        ParseLinesIntoCustomStructureCallback;

} DEBUG_ENGINE_COMMAND_TEMPLATE;
typedef DEBUG_ENGINE_COMMAND_TEMPLATE *PDEBUG_ENGINE_COMMAND_TEMPLATE;

//
// DEBUG_ENGINE-related function typedefs and structures.
//

typedef union _DEBUG_ENGINE_FLAGS {
    LONG AsLong;
    ULONG AsULong;
    struct _Struct_size_bytes_(sizeof(ULONG)) {
        ULONG Unused:1;
    };
} DEBUG_ENGINE_FLAGS;
C_ASSERT(sizeof(DEBUG_ENGINE_FLAGS) == sizeof(ULONG));

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
    // Pointer to an initialized RTL structure.
    //

    PRTL Rtl;

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

    volatile LONG EventCallbacksRefCount;
    volatile LONG InputCallbacksRefCount;
    volatile LONG OutputCallbacksRefCount;
    volatile LONG OutputCallbacks2RefCount;

    SRWLOCK Lock;

    _Guarded_by_(Lock)
    struct {

        DEBUG_OUTPUT_MASK OutputMask;
        PDEBUG_ENGINE_OUTPUT_CALLBACK OutputCallback;
        PDEBUG_ENGINE_OUTPUT_CALLBACK2 OutputCallback2;

        PDEBUG_ENGINE_OUTPUT CurrentOutput;

        DEBUG_EVENT_CALLBACKS_INTEREST_MASK EventCallbacksInterestMask;
        DEBUG_OUTPUT_CALLBACKS2_INTEREST_MASK OutputCallbacks2InterestMask;

        DEBUGEVENTCALLBACKS EventCallbacks;
        IDEBUGEVENTCALLBACKS IEventCallbacks;

        DEBUGINPUTCALLBACKS InputCallbacks;
        IDEBUGINPUTCALLBACKS IInputCallbacks;

        DEBUGOUTPUTCALLBACKS OutputCallbacks;
        IDEBUGOUTPUTCALLBACKS IOutputCallbacks;

        DEBUGOUTPUTCALLBACKS2 OutputCallbacks2;
        IDEBUGOUTPUTCALLBACKS2 IOutputCallbacks2;

        GUID IID_CurrentEventCallbacks;
        GUID IID_CurrentInputCallbacks;
        GUID IID_CurrentOutputCallbacks;
    };

} DEBUG_ENGINE, *PDEBUG_ENGINE, **PPDEBUG_ENGINE;

#define AcquireDebugEngineLock(Engine) \
    AcquireSRWLockExclusive(&(Engine)->Lock)

#define ReleaseDebugEngineLock(Engine) \
    ReleaseSRWLockExclusive(&(Engine)->Lock)

//
// Event callbacks.
//

#define DEBUG_CALLBACK_PROLOGUE(Name)                             \
    PDEBUG_ENGINE Engine;                                         \
    Engine = CONTAINING_RECORD(This->lpVtbl, DEBUG_ENGINE, Name);

#define DEBUG_ADD_REF(Name) InterlockedIncrement(&Engine->##Name##RefCount)

#define DEBUG_RELEASE(Name) InterlockedIncrement(&Engine->##Name##RefCount)

#define DEBUG_QUERY_INTERFACE(Name, Upper)                       \
    if (InlineIsEqualGUID(InterfaceId, &IID_IUNKNOWN) ||         \
        InlineIsEqualGUID(InterfaceId, &IID_IDEBUG_##Upper##)) { \
        InterlockedIncrement(&Engine->##Name##RefCount);         \
        *Interface = &Engine->I##Name##;                         \
        return S_OK;                                             \
    }                                                            \
    *Interface = NULL

typedef
_Check_return_
_Success_(return != 0)
_Requires_exclusive_lock_held_(Engine->Lock)
BOOL
(DEBUG_ENGINE_SET_EVENT_CALLBACKS)(
    _In_ PDEBUG_ENGINE Engine,
    _In_ PDEBUGEVENTCALLBACKS EventCallbacks,
    _In_ PCGUID InterfaceId,
    _In_ DEBUG_EVENT_CALLBACKS_INTEREST_MASK InterestMask
    );
typedef DEBUG_ENGINE_SET_EVENT_CALLBACKS *PDEBUG_ENGINE_SET_EVENT_CALLBACKS;

typedef
_Check_return_
_Success_(return != 0)
_Requires_exclusive_lock_held_(Engine->Lock)
BOOL
(DEBUG_ENGINE_SET_INPUT_CALLBACKS)(
    _In_ PDEBUG_ENGINE Engine,
    _In_ PDEBUGINPUTCALLBACKS InputCallbacks,
    _In_ PCGUID InterfaceId
    );
typedef DEBUG_ENGINE_SET_INPUT_CALLBACKS *PDEBUG_ENGINE_SET_INPUT_CALLBACKS;

typedef
_Check_return_
_Success_(return != 0)
_Requires_exclusive_lock_held_(Engine->Lock)
BOOL
(DEBUG_ENGINE_SET_OUTPUT_CALLBACKS)(
    _In_ PDEBUG_ENGINE Engine
    );
typedef DEBUG_ENGINE_SET_OUTPUT_CALLBACKS *PDEBUG_ENGINE_SET_OUTPUT_CALLBACKS;

typedef
_Check_return_
_Success_(return != 0)
_Requires_exclusive_lock_held_(Engine->Lock)
BOOL
(DEBUG_ENGINE_SET_OUTPUT_CALLBACKS2)(
    _In_ PDEBUG_ENGINE Engine
    );
typedef DEBUG_ENGINE_SET_OUTPUT_CALLBACKS2
      *PDEBUG_ENGINE_SET_OUTPUT_CALLBACKS2;

typedef
_Check_return_
_Success_(return != 0)
BOOL
(DEBUG_ENGINE_BUILD_COMMAND)(
    _In_ PDEBUG_ENGINE_OUTPUT Output,
    _In_ DEBUG_ENGINE_COMMAND_ID CommandId,
    _In_ ULONG CommandOptions,
    _In_opt_ PCUNICODE_STRING Argument,
    _In_ PUNICODE_STRING CommandBuffer
    );
typedef DEBUG_ENGINE_BUILD_COMMAND *PDEBUG_ENGINE_BUILD_COMMAND;

typedef
_Check_return_
_Success_(return != 0)
_Requires_exclusive_lock_held_(Engine->Lock)
BOOL
(DEBUG_ENGINE_EXECUTE_COMMAND)(
    _In_ PDEBUG_ENGINE_OUTPUT Output
    );
typedef DEBUG_ENGINE_EXECUTE_COMMAND *PDEBUG_ENGINE_EXECUTE_COMMAND;

typedef
_Check_return_
_Success_(return != 0)
BOOL
(DEBUG_ENGINE_LOAD_SETTINGS)(
    _In_ struct _DEBUG_ENGINE_SESSION *Session,
    _In_ PCUNICODE_STRING DebugSettingsXmlPath
    );
typedef DEBUG_ENGINE_LOAD_SETTINGS *PDEBUG_ENGINE_LOAD_SETTINGS;

typedef
_Check_return_
_Success_(return != 0)
BOOL
(CALLBACK DEBUG_ENGINE_PARSE_EXAMINE_SYMBOLS_LINE_OUTPUT_CALLBACK)(
    _In_ PDEBUG_ENGINE_OUTPUT Output
    );
typedef DEBUG_ENGINE_PARSE_EXAMINE_SYMBOLS_LINE_OUTPUT_CALLBACK
      *PDEBUG_ENGINE_PARSE_EXAMINE_SYMBOLS_LINE_OUTPUT_CALLBACK;

//
// Inline functions for copying callback vtables to the engine.
//

_Requires_exclusive_lock_held_(Engine->Lock)
FORCEINLINE
VOID
CopyIDebugEventCallbacks(
    _In_ PDEBUG_ENGINE Engine,
    _Out_writes_bytes_(sizeof(DEBUGEVENTCALLBACKS)) PDEBUGEVENTCALLBACKS Dest,
    _In_ PCDEBUGEVENTCALLBACKS Source
    )
{
    UNREFERENCED_PARAMETER(Engine);
    __movsq((PDWORD64)Dest,
            (PDWORD64)&Source,
            QUADWORD_SIZEOF(DEBUGEVENTCALLBACKS));
}

_Requires_exclusive_lock_held_(Engine->Lock)
FORCEINLINE
VOID
CopyIDebugInputCallbacks(
    _In_ PDEBUG_ENGINE Engine,
    _Out_writes_bytes_(sizeof(DEBUGINPUTCALLBACKS)) PDEBUGINPUTCALLBACKS Dest,
    _In_ PCDEBUGINPUTCALLBACKS Source
    )
{
    UNREFERENCED_PARAMETER(Engine);
    __movsq((PDWORD64)Dest,
            (PDWORD64)&Source,
            QUADWORD_SIZEOF(DEBUGINPUTCALLBACKS));
}

_Requires_exclusive_lock_held_(Engine->Lock)
FORCEINLINE
VOID
CopyIDebugOutputCallbacks(
    _In_ PDEBUG_ENGINE Engine,
    _Out_writes_bytes_(sizeof(DEBUGOUTPUTCALLBACKS))
        PDEBUGOUTPUTCALLBACKS Dest,
    _In_ PCDEBUGOUTPUTCALLBACKS Source
    )
{
    UNREFERENCED_PARAMETER(Engine);
    __movsq((PDWORD64)Dest,
            (PDWORD64)Source,
            QUADWORD_SIZEOF(DEBUGOUTPUTCALLBACKS));
}

_Requires_exclusive_lock_held_(Engine->Lock)
FORCEINLINE
VOID
CopyIDebugOutputCallbacks2(
    _In_ PDEBUG_ENGINE Engine,
    _Out_writes_bytes_(sizeof(DEBUGOUTPUTCALLBACKS2))
        PDEBUGOUTPUTCALLBACKS2 Dest,
    _In_ PCDEBUGOUTPUTCALLBACKS2 Source
    )
{
    UNREFERENCED_PARAMETER(Engine);
    __movsq((PDWORD64)Dest,
            (PDWORD64)Source,
            QUADWORD_SIZEOF(DEBUGOUTPUTCALLBACKS2));
}

//
// Inline function for copying GUIDs.
//

FORCEINLINE
VOID
CopyInterfaceId(
    _Out_writes_bytes_(sizeof(GUID)) PGUID Dest,
    _In_ PCGUID Source
    )
{
    __movsq((PDWORD64)Dest,
            (PDWORD64)Source,
            QUADWORD_SIZEOF(GUID));
}

//
// DebugEngineOutput-related inline functions and function typedefs.
//

FORCEINLINE
VOID
DebugPrintUnknownBasicType(
    _In_ PSTRING String
    )
{
    OutputDebugStringA("DebugEngine: Unknown Type: ");
    PrintStringToDebugStream(String);
}

FORCEINLINE
VOID
DebugPrintUnknownFunctionArgumentType(
    _In_ PSTRING String
    )
{
    OutputDebugStringA("DebugEngine: Unknown Argument Type: ");
    PrintStringToDebugStream(String);
}

typedef
VOID
(UPDATE_OUTPUT_LINE_COUNTERS)(
    _In_ PDEBUG_ENGINE_OUTPUT Output
    );
typedef UPDATE_OUTPUT_LINE_COUNTERS *PUPDATE_OUTPUT_LINE_COUNTERS;

typedef
_Check_return_
_Success_(return != 0)
BOOL
(DISPATCH_OUTPUT_LINE_CALLBACKS)(
    _In_ PDEBUG_ENGINE_OUTPUT Output
    );
typedef DISPATCH_OUTPUT_LINE_CALLBACKS *PDISPATCH_OUTPUT_LINE_CALLBACKS;

typedef
_Check_return_
_Success_(return != 0)
BOOL
(DISPATCH_OUTPUT_COMPLETE_CALLBACKS)(
    _In_ PDEBUG_ENGINE_OUTPUT Output
    );
typedef DISPATCH_OUTPUT_COMPLETE_CALLBACKS *PDISPATCH_OUTPUT_COMPLETE_CALLBACKS;

//
// Private function declarations.
//

#pragma component(browser, off)

CREATE_DEBUG_INTERFACES CreateDebugInterfaces;
DESTROY_DEBUG_ENGINE DestroyDebugEngine;
INITIALIZE_DEBUG_ENGINE InitializeDebugEngine;
INITIALIZE_DEBUG_ENGINE_OUTPUT InitializeDebugEngineOutput;
INITIALIZE_DEBUG_ENGINE_OUTPUT_SIMPLE InitializeDebugEngineOutputSimple;

DEBUG_ENGINE_BUILD_COMMAND DebugEngineBuildCommand;
DEBUG_ENGINE_EXECUTE_COMMAND DebugEngineExecuteCommand;
DEBUG_ENGINE_OUTPUT_CALLBACK DebugEngineOutputCallback;
DEBUG_ENGINE_OUTPUT_CALLBACK2 DebugEngineOutputCallback2;

UPDATE_OUTPUT_LINE_COUNTERS UpdateOutputLineCounters;
DISPATCH_OUTPUT_LINE_CALLBACKS DispatchOutputLineCallbacks;
DISPATCH_OUTPUT_COMPLETE_CALLBACKS DispatchOutputCompleteCallbacks;

DEBUG_ENGINE_SAVE_OUTPUT_LINE DebugEngineSaveOutputLine;
DEBUG_ENGINE_LINE_OUTPUT_CALLBACK DebugStreamLineOutputCallback;
DEBUG_ENGINE_OUTPUT_COMPLETE_CALLBACK DummyOutputCompleteCallback;

DEBUG_ENGINE_DISPLAY_TYPE DebugEngineDisplayType;
DEBUG_ENGINE_EXAMINE_SYMBOLS DebugEngineExamineSymbols;
DEBUG_ENGINE_UNASSEMBLE_FUNCTION DebugEngineUnassembleFunction;

DEBUG_ENGINE_PARSE_LINES_INTO_CUSTOM_STRUCTURE_CALLBACK
    DisplayTypeParseLinesIntoCustomStructureCallback;

DEBUG_ENGINE_PARSE_LINES_INTO_CUSTOM_STRUCTURE_CALLBACK
    ExamineSymbolsParseLinesIntoCustomStructureCallback;

DEBUG_ENGINE_PARSE_LINES_INTO_CUSTOM_STRUCTURE_CALLBACK
    UnassembleFunctionParseLinesIntoCustomStructureCallback;

DEBUG_ENGINE_SETTINGS_META DebugEngineSettingsMeta;
DEBUG_ENGINE_LIST_SETTINGS DebugEngineListSettings;
DEBUG_ENGINE_LOAD_SETTINGS DebugEngineLoadSettings;

DEBUG_ENGINE_SET_OUTPUT_CALLBACKS DebugEngineSetOutputCallbacks;
DEBUG_ENGINE_SET_OUTPUT_CALLBACKS2 DebugEngineSetOutputCallbacks2;

DEBUG_ENGINE_COMMAND_GET_OPTIONS_CALLBACK DisplayTypeGetOptionsCallback;
DEBUG_ENGINE_COMMAND_GET_OPTIONS_CALLBACK ExamineSymbolsGetOptionsCallback;
DEBUG_ENGINE_COMMAND_GET_OPTIONS_CALLBACK UnassembleFunctionGetOptionsCallback;

DEBUG_ENGINE_SESSION_EXECUTE_STATIC_COMMAND
    DebugEngineSessionExecuteStaticCommand;

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
// IDebugInputCallbacks
//

DEBUG_INPUT_QUERY_INTERFACE DebugInputQueryInterface;
DEBUG_INPUT_ADD_REF DebugInputAddRef;
DEBUG_INPUT_RELEASE DebugInputRelease;
DEBUG_INPUT_START_INPUT_CALLBACK DebugInputStartInputCallback;
DEBUG_INPUT_END_INPUT_CALLBACK DebugInputEndInputCallback;

//
// IDebugOutputCallbacks
//

DEBUG_OUTPUT_CALLBACKS_QUERY_INTERFACE DebugOutputCallbacksQueryInterface;
DEBUG_OUTPUT_CALLBACKS_ADD_REF DebugOutputCallbacksAddRef;
DEBUG_OUTPUT_CALLBACKS_RELEASE DebugOutputCallbacksRelease;
DEBUG_OUTPUT_CALLBACKS_OUTPUT DebugOutputCallbacksOutput;

//
// IDebugOutputCallbacks2
//

DEBUG_OUTPUT_CALLBACKS2_QUERY_INTERFACE DebugOutputCallbacks2QueryInterface;
DEBUG_OUTPUT_CALLBACKS2_ADD_REF DebugOutputCallbacks2AddRef;
DEBUG_OUTPUT_CALLBACKS2_RELEASE DebugOutputCallbacks2Release;
DEBUG_OUTPUT_CALLBACKS2_OUTPUT DebugOutputCallbacks2Output;
DEBUG_OUTPUT_CALLBACKS2_GET_INTEREST_MASK DebugOutputCallbacks2GetInterestMask;
DEBUG_OUTPUT_CALLBACKS2_OUTPUT2 DebugOutputCallbacks2Output2;

#pragma component(browser, on)

#ifdef __cplusplus
} // extern "C"
#endif

// vim:set ts=8 sw=4 sts=4 tw=80 expandtab                                     :
