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

#ifdef __cplusplus
extern "C" {
#endif

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

#ifdef _DEBUG_ENGINE_NO_API_EXPORT_IMPORT

//
// We're being included by someone who doesn't want dllexport or dllimport.
// This is useful for creating new .exe-based projects for things like unit
// testing or performance testing/profiling.
//

#define DEBUG_ENGINE_API
#define DEBUG_ENGINE_DATA extern

#else

//
// We're being included by an external component.
//

#define DEBUG_ENGINE_API __declspec(dllimport)
#define DEBUG_ENGINE_DATA extern __declspec(dllimport)

#endif

#include <Windows.h>
#include "../Rtl/Rtl.h"
#include "../StringTable/StringTable.h"

//
// Disable source browsing for DbgEng.h to avoid this:
//
// dbgeng.h(18410): warning BK4504: file contains too many references;
//                  ignoring further references from this source
//

#pragma component(browser, off)
#include <DbgEng.h>
#pragma component(browser, on)

#include "DebugEngineInterfaces.h"

#endif

//
// DEBUG_ENGINE-related function typedefs and structures.
//

typedef union _DEBUG_ENGINE_FLAGS {
    struct _Struct_size_bytes_(sizeof(ULONG)) {
        ULONG Unused:1;
    };
    LONG AsLong;
    ULONG AsULong;
} DEBUG_ENGINE_FLAGS;
C_ASSERT(sizeof(DEBUG_ENGINE_FLAGS) == sizeof(ULONG));

//
// This captures state information about the current debug engine session.
// It is currently used to coordinate behavior in event callbacks.
//

typedef union _DEBUG_ENGINE_STATE {
    struct _Struct_size_bytes_(sizeof(ULONG)) {

        //
        // Transient state flags.
        //

        ULONG RegisteringEventCallbacks:1;
        ULONG CreatingProcess:1;
        ULONG InCreateProcessCallback:1;

        //
        // When set, indicates that ExitDispatch() should be called against the
        // exit dispatch client when able.  This is currently done in the event
        // callback for DebugEngineChangeEngineState() when the execution status
        // changes and it is not from a wait being satisfied.
        //

        ULONG ExitDispatchWhenAble:1;

        //
        // Final state flags.
        //

        ULONG EventCallbacksRegistered:1;
        ULONG ProcessCreated:1;

    };
    LONG AsLong;
    ULONG AsULong;
} DEBUG_ENGINE_STATE;
C_ASSERT(sizeof(DEBUG_ENGINE_STATE) == sizeof(ULONG));
typedef DEBUG_ENGINE_STATE
      *PDEBUG_ENGINE_STATE;

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
    // State.
    //

    DEBUG_ENGINE_STATE State;

    //
    // Pointer to an initialized RTL structure.
    //

    struct _RTL *Rtl;

    //
    // Pointer to our parent DEBUG_ENGINE_SESSION.
    //

    struct _DEBUG_ENGINE_SESSION *Session;

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

    PCGUID IID_Registers;
    PIDEBUGREGISTERS IRegisters;
    PDEBUGREGISTERS Registers;

    PCGUID IID_SystemObjects;
    PIDEBUGSYSTEMOBJECTS ISystemObjects;
    PDEBUGSYSTEMOBJECTS SystemObjects;

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
        PDEBUG_ENGINE_OUTPUT_CALLBACK DefaultOutputCallback;
        PDEBUG_ENGINE_OUTPUT_CALLBACK2 OutputCallback2;

        struct _DEBUG_ENGINE_OUTPUT *CurrentOutput;
        struct _DEBUG_ENGINE_OUTPUT *DefaultOutput;

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

} DEBUG_ENGINE;
typedef DEBUG_ENGINE *PDEBUG_ENGINE;
typedef DEBUG_ENGINE **PPDEBUG_ENGINE;

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
_Requires_lock_not_held_(*EnginePointer->Lock)
VOID
(DESTROY_DEBUG_ENGINE)(
    _Pre_notnull_ _Post_satisfies_(*EnginePointer == 0)
        struct _DEBUG_ENGINE **EnginePointer,
    _In_opt_ PBOOL IsProcessTerminating
    );
typedef DESTROY_DEBUG_ENGINE *PDESTROY_DEBUG_ENGINE;

//
// Event callbacks.
//

#define DEBUG_CALLBACK_PROLOGUE(Name)                             \
    PDEBUG_ENGINE Engine;                                         \
    Engine = CONTAINING_RECORD(This->lpVtbl, DEBUG_ENGINE, Name);

#define DEBUG_SESSION_CALLBACK_PROLOGUE(Name)                     \
    PDEBUG_ENGINE Engine;                                         \
    PDEBUG_ENGINE_SESSION Session;                                \
    Engine = CONTAINING_RECORD(This->lpVtbl, DEBUG_ENGINE, Name); \
    Session = Engine->Session;

#define CHILD_DEBUG_SESSION_CALLBACK_PROLOGUE(Type, Name)         \
    PDEBUG_ENGINE Engine;                                         \
    PDEBUG_ENGINE_SESSION Session;                                \
    Type Context;                                                 \
    Engine = CONTAINING_RECORD(This->lpVtbl, DEBUG_ENGINE, Name); \
    Session = Engine->Session;                                    \
    Context = (Type)Session->ChildContext;

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
    _In_ PCDEBUGEVENTCALLBACKS EventCallbacks,
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
(CALLBACK DEBUG_ENGINE_PARSE_EXAMINE_SYMBOLS_LINE_OUTPUT_CALLBACK)(
    _In_ struct _DEBUG_ENGINE_OUTPUT *Output
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
            (PDWORD64)Source,
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
            (PDWORD64)Source,
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
// DEBUG_ENGINE_OUTPUT-related functions and structures.
//


typedef
BOOL
(CALLBACK DEBUG_ENGINE_LINE_OUTPUT_CALLBACK)(
    _In_ struct _DEBUG_ENGINE_OUTPUT *Output
    );
typedef DEBUG_ENGINE_LINE_OUTPUT_CALLBACK
      *PDEBUG_ENGINE_LINE_OUTPUT_CALLBACK;

typedef
BOOL
(CALLBACK DEBUG_ENGINE_PARTIAL_OUTPUT_CALLBACK)(
    _In_ struct _DEBUG_ENGINE_OUTPUT *Output
    );
typedef DEBUG_ENGINE_PARTIAL_OUTPUT_CALLBACK
      *PDEBUG_ENGINE_PARTIAL_OUTPUT_CALLBACK;

typedef
BOOL
(CALLBACK DEBUG_ENGINE_OUTPUT_COMPLETE_CALLBACK)(
    _In_ struct _DEBUG_ENGINE_OUTPUT *Output
    );
typedef DEBUG_ENGINE_OUTPUT_COMPLETE_CALLBACK
      *PDEBUG_ENGINE_OUTPUT_COMPLETE_CALLBACK;

typedef
_Check_return_
_Success_(return != 0)
BOOL
(DEBUG_ENGINE_SAVE_OUTPUT_LINE)(
    _In_ struct _DEBUG_ENGINE_OUTPUT *Output
    );
typedef DEBUG_ENGINE_SAVE_OUTPUT_LINE *PDEBUG_ENGINE_SAVE_OUTPUT_LINE;

typedef
BOOL
(CALLBACK DEBUG_ENGINE_PARSE_LINES_INTO_CUSTOM_STRUCTURE_CALLBACK)(
    _In_ struct _DEBUG_ENGINE_OUTPUT *Output
    );
typedef DEBUG_ENGINE_PARSE_LINES_INTO_CUSTOM_STRUCTURE_CALLBACK
      *PDEBUG_ENGINE_PARSE_LINES_INTO_CUSTOM_STRUCTURE_CALLBACK;

typedef union _DEBUG_ENGINE_OUTPUT_STATE {
    struct _Struct_size_bytes_(sizeof(ULONG)) {

        //
        // Transient state flags.
        //

        ULONG Initialized:1;
        ULONG CommandBuilt:1;
        ULONG CommandExecuting:1;
        ULONG InPartialOutputCallback:1;
        ULONG DispatchingLineCallbacks:1;
        ULONG SavingOutputLine:1;
        ULONG InLineOutputCallback:1;
        ULONG DispatchingOutputCompleteCallbacks:1;
        ULONG ParsingLinesIntoCustomStructure:1;
        ULONG InOutputCompleteCallback:1;
        ULONG CommandComplete:1;

        //
        // Final state flags.
        //

        ULONG Failed:1;
        ULONG Succeeded:1;
        ULONG StatusInPageError:1;
        ULONG SavingOutputLineFailed:1;
        ULONG SavingOutputLineSucceeded:1;
        ULONG ExecuteCommandFailed:1;
        ULONG ExecuteCommandSucceeded:1;
        ULONG LineOutputCallbackFailed:1;
        ULONG LineOutputCallbackSucceeded:1;
        ULONG DispatchingLineCallbacksFailed:1;
        ULONG DispatchingLineCallbacksSucceeded:1;
        ULONG OutputCompleteCallbackFailed:1;
        ULONG OutputCompleteCallbackSucceeded:1;
        ULONG DispatchingOutputCompleteCallbackFailed:1;
        ULONG DispatchingOutputCompleteCallbackSucceeded:1;
        ULONG ParsingLinesIntoCustomStructureFailed:1;
        ULONG ParsingLinesIntoCustomStructureSucceeded:1;
    };
    LONG AsLong;
    ULONG AsULong;
} DEBUG_ENGINE_OUTPUT_STATE;
C_ASSERT(sizeof(DEBUG_ENGINE_OUTPUT_STATE) == sizeof(ULONG));
typedef DEBUG_ENGINE_OUTPUT_STATE
      *PDEBUG_ENGINE_OUTPUT_STATE;

typedef struct _LINKED_PARTIAL_LINE {
    LIST_ENTRY ListEntry;
    union {
        STRING PartialLine;
        UNICODE_STRING PartialLineWide;
    };
} LINKED_PARTIAL_LINE;
typedef LINKED_PARTIAL_LINE *PLINKED_PARTIAL_LINE;

//
// These flags are used to customize the type of output wanted by the caller.
//

typedef union _DEBUG_ENGINE_OUTPUT_FLAGS {
    struct _Struct_size_bytes_(sizeof(ULONG)) {

        //
        // When set, indicates the caller wants to receive raw partial output
        // callbacks for each chunk of text that the debugger engine generates.
        //

        ULONG EnablePartialOutputCallbacks:1;

        //
        // When set, indicates the caller wants to receive line-oriented
        // callbacks.  If this flag is set, the caller must provide non-NULL
        // callbacks for the LineOutputCallback, SavePartialLineCallback and
        // RetrievePartialLineCallback callbacks.
        //
        // N.B. Callers are free to specify both EnablePartialOutputCallbacks
        //      and EnableLineOrientedCallbacks (or none; in which case, no
        //      callbacks will be sent to the client, but the command will
        //      be executed anyway).
        //

        ULONG EnableLineOutputCallbacks:1;

        //
        // When set, indicates the caller has provided line, text and custom
        // structure allocators, which will be automatically called to persist
        // lines (LINKED_LINE structs pointing to individual lines), text and
        // custom structures.
        //

        ULONG EnableLineTextAndCustomStructureAllocators:1;

        //
        // When set, invokes a parser routine against the command's output, if
        // applicable.
        //

        ULONG EnableOutputParsingToCustomStructure:1;

        //
        // This flag will be automatically set when a condition is met to
        // warrant dispatching line output callbacks.
        //

        ULONG DispatchOutputLineCallbacks:1;

        //
        // When set, __debugbreak() will be issued if an internal error is
        // encountered during line parsing.
        //

        ULONG DebugBreakOnLineParsingError:1;

        //
        // When set, indicates the caller has overridden the pSavedLinesListHead
        // pointer and thus, it should not be initialized to point to the
        // SavedLinesListHead LIST_ENTRY.
        //
        // This is used by commands like unassemble function and display type,
        // where a single function or type will generate multiple lines of
        // output from the debugger engine.  (As opposed to examining symbols,
        // where each line represents an individual symbol.)
        //

        ULONG OverrideSavedLinesListHead:1;

        //
        // When set, indicates the caller wants wide character (WCHAR) output.
        // (Not currently supported.)
        //

        ULONG WideCharacterOutput:1;
    };
    LONG AsLong;
    ULONG AsULong;
} DEBUG_ENGINE_OUTPUT_FLAGS;
C_ASSERT(sizeof(DEBUG_ENGINE_OUTPUT_FLAGS) == sizeof(ULONG));
typedef DEBUG_ENGINE_OUTPUT_FLAGS *PDEBUG_ENGINE_OUTPUT_FLAGS;

typedef struct _Struct_size_bytes_(SizeOfStruct) _DEBUG_ENGINE_OUTPUT {

    //
    // Size of structure, in bytes.
    //

    _Field_range_(== , sizeof(struct _DEBUG_ENGINE_OUTPUT)) ULONG SizeOfStruct;

    //
    // Flags.
    //

    DEBUG_ENGINE_OUTPUT_FLAGS Flags;

    //
    // State.
    //

    DEBUG_ENGINE_OUTPUT_STATE State;

    //
    // Counters that track internal state.
    //

    ULONG NumberOfCallbacks;
    ULONG NumberOfLines;
    ULONG NumberOfPartialCallbacks;
    ULONG TotalBufferLengthInChars;
    ULONG TotalBufferSizeInBytes;
    ULONG LargestChunkSizeInBytes;
    ULONG NumberOfParsedLines;
    ULONG Unused1;
    USHORT ShortestLineInBytes;
    USHORT LongestLineInBytes;
    USHORT CurrentNumberOfConsecutivePartialCallbacks;
    USHORT LongestNumberOfConsecutivePartialCallbacks;

    //
    // Captures the last HRESULT of an operation.
    //

    HRESULT LastResult;

    //
    // Pointer to the debug engine session this output was bound to via the
    // InitializeDebugEngineOutput() call.
    //

    struct _DEBUG_ENGINE_SESSION *Session;

    //
    // User's context opaque pointer.
    //

    PVOID Context;

    //
    // Pointer to an ALLOCATOR structure that can be used by the debug engine
    // for any temporary allocations (such as string buffer allocation when
    // creating commands).
    //

    PALLOCATOR Allocator;

    //
    // Pointer to an RTL_PATH structure representing the target module that
    // this output is related to, if applicable.
    //

    PRTL_PATH ModulePath;

    //
    // Pointer to the string representation of the command that was dispatched
    // to the debugger's ExecuteWide() function.
    //

    PCUNICODE_STRING Command;

    //
    // Opaque pointer to the command template used to assemble this command.
    //

    struct _DEBUG_ENGINE_COMMAND_TEMPLATE *CommandTemplate;

    //
    // Options used for this command.
    //

    ULONG CommandOptions;

    //
    // Timestamps for command start and end times.
    //

    struct {
        LARGE_INTEGER CommandStart;
        LARGE_INTEGER CommandEnd;
    } Timestamp;

    //
    // Callbacks that will be invoked by the engine.  PartialOutputCallback
    // will be called one or more times as the debug engine produces output.
    // OutputCompleteCallback will be called once, after execution of the
    // command has completed.
    //

    PDEBUG_ENGINE_LINE_OUTPUT_CALLBACK LineOutputCallback;
    PDEBUG_ENGINE_PARTIAL_OUTPUT_CALLBACK PartialOutputCallback;
    PDEBUG_ENGINE_OUTPUT_COMPLETE_CALLBACK OutputCompleteCallback;

    //
    // Internal function pointer invoked when saving lines.
    //

    PDEBUG_ENGINE_SAVE_OUTPUT_LINE SaveOutputLine;

    //
    // Allocators that will be used to save line and text information
    // automatically as part of output callback processing.
    //

    PALLOCATOR LineAllocator;
    PALLOCATOR TextAllocator;

    //
    // A custom allocator that will be used for allocating instances of a
    // command's specialized output, e.g. DEBUG_ENGINE_EXAMINED_SYMBOL for
    // the ExamineSymbols() command.
    //

    PALLOCATOR CustomStructureAllocator;

    //
    // A custom allocator that can be used for secondary allocations related to
    // persisting the main custom structure.  This is used, for example, to
    // allocate individual STRING structures that describe an examined symbol's
    // function arguments.
    //

    PALLOCATOR CustomStructureSecondaryAllocator;

    //
    // If custom allocation is being done, the following string will refer to
    // the name of the structure that was allocated.  This will be backed by
    // readonly memory within the DebugEngine DLL and must not be modified.
    //

    PCSTRING CustomStructureName;

    //
    // If applicable, this will be set to the name of the secondary element
    // allocated in support of the custom structure.
    //

    PCSTRING CustomStructureSecondaryName;

    //
    // Pointer to the custom structure.  Examined symbols will be an array based
    // at the indicated address; displayed types and unassembled functions will
    // be single records.
    //

    ULONG NumberOfCustomStructures;
    union {
        PVOID CustomStructure;
        PVOID FirstCustomStructure;
        struct _DEBUG_ENGINE_DISPLAYED_TYPE *DisplayedType;
        struct _DEBUG_ENGINE_EXAMINED_SYMBOL *ExaminedSymbol;
        struct _DEBUG_ENGINE_UNASSEMBLED_FUNCTION *UnassembledFunction;
    };
    LIST_ENTRY CustomStructureListHead;

    //
    // Internal pointer to the relevant command's line parsing function.
    //

    PDEBUG_ENGINE_PARSE_LINES_INTO_CUSTOM_STRUCTURE_CALLBACK
        ParseLinesIntoCustomStructureCallback;

    //
    // If line-oriented callback has been requested, the following structure
    // will be filled out with the line details on each callback invocation.
    //

    union {
        STRING Line;
        UNICODE_STRING LineWide;
    };

    //
    // Saved lines.  NumberOfSavedLines should match NumberOfLines when the
    // completing the output callback if the caller has requested line and
    // text persistence.  NumberOfSavedBytes should match TotalBufferSizeInBytes
    // at the same point.
    //

    ULONG NumberOfSavedLines;
    ULONG NumberOfSavedBytes;
    LIST_ENTRY SavedLinesListHead;
    PLIST_ENTRY pSavedLinesListHead;

    //
    // If any lines fail parsing, they'll be captured here.
    //

    ULONG NumberOfFailedLines;
    ULONG Unused2;
    LIST_ENTRY FailedLinesListHead;

    //
    // Track the contiguous nature of the underlying line text allocations.
    //

    struct {

        //
        // When set, indicates all buffers for the line allocations were
        // contiguous; i.e. first line's buffer can be used to access the
        // entire text output of the command.
        //

        ULONG Contiguous:1;

        //
        // When set, indicates the opposite of the above; there was at least
        // one buffer that broke the contiguous memory layout.  The field
        // FirstDiscontiguousBufferIndex will capture for which line this
        // happened.
        //

        ULONG Discontiguous:1;

    } SavedLineBufferAllocationState;

    //
    // 0-based index of the first discontiguous buffer, if any.
    //

    ULONG FirstDiscontiguousBufferIndex;
    PCSTR FirstSavedLineBuffer;

    //
    // Track the contiguous nature of the linked line structure allocations.
    //

    struct {

        //
        // When set, indicates all LINKED_LINEs allocated were contiguous.
        // (This implies the lines can be enumerated as an array instead of
        //  having to walk the linked list.)
        //

        ULONG Contiguous:1;

        //
        // When set, indicates the opposite of the above; there was at least
        // one buffer that broke the contiguous memory layout.  The field
        // FirstDiscontiguousBufferIndex will capture which line this happened
        // for.
        //

        ULONG Discontiguous:1;

    } SavedLineAllocationState;

    //
    // 0-based index of the first discontiguous linked line, if any.
    //

    ULONG FirstDiscontiguousLineIndex;
    PLINKED_LINE FirstSavedLine;

    //
    // Partial lines.
    //

    ULONG NumberOfPartialLines;
    LIST_ENTRY PartialLinesListHead;

    //
    // Raw text returned by DbgEng for this callback invocation.
    //

    union {
        STRING Chunk;
        UNICODE_STRING ChunkWide;
    };

} DEBUG_ENGINE_OUTPUT;
typedef DEBUG_ENGINE_OUTPUT *PDEBUG_ENGINE_OUTPUT;

typedef
_Check_return_
_Success_(return != 0)
BOOL
(INITIALIZE_DEBUG_ENGINE_OUTPUT)(
    _Inout_ PDEBUG_ENGINE_OUTPUT Output,
    _In_ struct _DEBUG_ENGINE_SESSION *DebugEngineSession,
    _In_ PALLOCATOR Allocator,
    _In_opt_ PALLOCATOR LineAllocator,
    _In_opt_ PALLOCATOR TextAllocator,
    _In_opt_ PALLOCATOR CustomStructureAllocator,
    _In_opt_ PALLOCATOR CustomStructureSecondaryAllocator,
    _In_opt_ PDEBUG_ENGINE_LINE_OUTPUT_CALLBACK LineOutputCallback,
    _In_opt_ PDEBUG_ENGINE_PARTIAL_OUTPUT_CALLBACK PartialOutputCallback,
    _In_opt_ PDEBUG_ENGINE_OUTPUT_COMPLETE_CALLBACK OutputCompleteCallback,
    _In_opt_ PVOID Context,
    _In_opt_ PRTL_PATH ModulePath
    );
typedef INITIALIZE_DEBUG_ENGINE_OUTPUT *PINITIALIZE_DEBUG_ENGINE_OUTPUT;

typedef
_Check_return_
_Success_(return != 0)
BOOL
(INITIALIZE_DEBUG_ENGINE_OUTPUT_SIMPLE)(
    _In_ PDEBUG_ENGINE_OUTPUT Output,
    _In_ struct _DEBUG_ENGINE_SESSION *DebugEngineSession
    );
typedef INITIALIZE_DEBUG_ENGINE_OUTPUT_SIMPLE
      *PINITIALIZE_DEBUG_ENGINE_OUTPUT_SIMPLE;

//
// DEBUG_ENGINE_EXAMINE_SYMBOLS-related functions and structures.
//

typedef enum _DEBUG_ENGINE_EXAMINE_SYMBOLS_SCOPE {
    UnknownScope = -1,
    PrivateGlobalScope = 0,
    PrivateInlineScope,
    PrivateFunctionScope,
    PublicGlobalScope,
    PublicFunctionScope,

    //
    // Make sure InvalidScope comes last.
    //

    InvalidScope

} DEBUG_ENGINE_EXAMINE_SYMBOLS_SCOPE;

//
// The order of these enumeration symbols must match the exact order of the
// corresponding string in the relevant ExamineSymbolsBasicTypes[1..n] STRING
// structure (see DebugEngineConstants.c).  This is because string tables are
// created from the delimited strings and the match index is cast directly to
// an enum of this type.
//

typedef enum _DEBUG_ENGINE_EXAMINE_SYMBOLS_TYPE {
    UnknownType = -1,

    //
    // First 16 types captured by BasicTypeStringTable1.
    //

    NoType = 0,
    FunctionType,

    CharType,
    WideCharType,
    ShortType,
    LongType,
    Integer64Type,
    IntegerType,

    UnsignedCharType,
    UnsignedWideCharType,
    UnsignedShortType,
    UnsignedLongType,
    UnsignedInteger64Type,
    UnsignedIntegerType,

    UnionType,
    StructType,

    //
    // Next 16 types captured by BasicTypeStringTable2.
    //

    CLRType = 16,
    BoolType,
    VoidType,
    ClassType,
    FloatType,
    DoubleType,
    SALExecutionContextType,
    ENativeStartupStateType,

    //
    // Any types that don't map directly to literal type names extracted from
    // the output string are listed here.  The first one starts at 48 in order
    // to differentiate it from the string tables.
    //

    //
    // Call site of an inline function.
    //

    InlineCallerType = 48,

    //
    // Enum is special in that it doesn't map to a string in the string table;
    // if a type can't be inferred from the list above, it defaults to Enum.
    //

    EnumType,

    //
    // Any enumeration value >= InvalidType is invalid.  Make sure this always
    // comes last in the enum layout.
    //

    InvalidType

} DEBUG_ENGINE_EXAMINE_SYMBOLS_TYPE;

typedef union _DEBUG_ENGINE_EXAMINE_SYMBOLS_COMMAND_OPTIONS {
    struct _Struct_size_bytes_(sizeof(ULONG)) {

        //
        // Verbose output.  Corresponds to /v.
        //

        ULONG Verbose:1;

        //
        // Include type information, if known.  Corresponds to /t.
        //

        ULONG TypeInformation:1;

    };
    LONG AsLong;
    ULONG AsULong;
} DEBUG_ENGINE_EXAMINE_SYMBOLS_COMMAND_OPTIONS;
C_ASSERT(sizeof(DEBUG_ENGINE_EXAMINE_SYMBOLS_COMMAND_OPTIONS) == sizeof(ULONG));

typedef
_Check_return_
_Success_(return != 0)
BOOL
(DEBUG_ENGINE_EXAMINE_SYMBOLS)(
    _In_ PDEBUG_ENGINE_OUTPUT Output,
    _In_ DEBUG_ENGINE_OUTPUT_FLAGS OutputFlags,
    _In_ DEBUG_ENGINE_EXAMINE_SYMBOLS_COMMAND_OPTIONS CommandOptions
    );
typedef DEBUG_ENGINE_EXAMINE_SYMBOLS *PDEBUG_ENGINE_EXAMINE_SYMBOLS;

//
// DEBUG_ENGINE_EXAMINED_SYMBOL-related structures and functions.
//
// This structure represents a parsed representation of a line of output from
// the debugger engine's execution of the examine symbols command.
//

typedef union _DEBUG_ENGINE_EXAMINED_SYMBOL_FLAGS {
    struct _Struct_size_bytes_(sizeof(ULONG)) {

        //
        // When set, indicates that a pointer (*) was detected in the type.
        // The PointerDepth field will be set to a non-zero value representing
        // the number of literal asterisks detected.
        //

        ULONG IsPointer:1;

        //
        // When set, indicates that at least one array descriptor ([]) was
        // detected in the type.  The NumberOfDimensions field will be set to
        // a non-zero value representing the number of paired square brackets
        // detected, e.g. `struct _foo [3][5]` would have a value of 2 for the
        // NumberOfDimensions field..
        //

        ULONG IsArray:1;

        //
        // If a colon is detected in the symbol name, the name is assumed to
        // be a (demangled) C++ symbol name.  The following flag will be set.
        //
        // N.B. Only one colon is tested for, despite two being technically the
        //      valid indicator of a C++ name.  However, nothing else should use
        //      a colon in their name, so we should be fine.
        //

        ULONG IsCpp:1;

        //
        // This bit will be set if the basic type of the symbol could not be
        // recognized.
        //

        ULONG UnknownType:1;

    };
    LONG AsLong;
    ULONG AsULong;
} DEBUG_ENGINE_EXAMINED_SYMBOL_FLAGS;
typedef DEBUG_ENGINE_EXAMINED_SYMBOL_FLAGS *PDEBUG_ENGINE_EXAMINED_SYMBOL_FLAGS;
C_ASSERT(sizeof(DEBUG_ENGINE_EXAMINED_SYMBOL_FLAGS) == sizeof(ULONG));

//
// An enum for representing x64 registers.
//

typedef enum _DEBUG_ENGINE_FUNCTION_ARGUMENT_REGISTER_X64 {
    RCX,
    RDX,
    R8,
    R9,
    XMM0,
    XMM1,
    XMM2,
    XMM3,
    YMM0,
    YMM1,
    YMM2,
    YMM3,
    ZMM0,
    ZMM1,
    ZMM2,
    ZMM3,
} DEBUG_ENGINE_FUNCTION_ARGUMENT_REGISTER_X64;

typedef enum _DEBUG_ENGINE_FUNCTION_ARGUMENT_TYPE {
    UnknownArgumentType = -1,

    //
    // First 16 types captured by FunctionArgumentTypes1.
    //

    CharArgumentType = 0,
    WideCharArgumentType,
    ShortArgumentType,
    LongArgumentType,
    Integer64ArgumentType,
    IntegerArgumentType,

    UnsignedCharArgumentType,
    UnsignedWideCharArgumentType,
    UnsignedShortArgumentType,
    UnsignedLongArgumentType,
    UnsignedInteger64ArgumentType,
    UnsignedIntegerArgumentType,

    UnionArgumentType,
    StructArgumentType,

    FloatArgumentType,
    DoubleArgumentType,

    //
    // Next non-vector 16 types captured by FunctionArgumentTypes2.
    //

    BoolArgumentType = 16,
    VoidArgumentType,
    ClassArgumentType,
    FunctionArgumentType,

    //
    // Vector types start at 32.
    //

    VectorArgument64Type = 32,
    VectorArgument128Type,
    VectorArgument256Type,
    VectorArgument512Type,

    //
    // Enum is special in that it doesn't map to a string in the string table;
    // if a type can't be inferred from the list above, it defaults to Enum.
    //

    EnumArgumentType = 48,

    //
    // Any enumeration value >= InvalidArgumentType is invalid.  Make sure this
    // always comes last in the enum layout.
    //

    InvalidArgumentType

} DEBUG_ENGINE_FUNCTION_ARGUMENT_TYPE;

//
// This structure is used to capture information about a function argument.
//

typedef union _DEBUG_ENGINE_FUNCTION_ARGUMENT_FLAGS {
    struct _Struct_size_bytes_(sizeof(ULONG)) {

        //
        // When set, indicates this argument is being passed in one of the
        // standard four x64 calling convention registers: RCX, RDX, R8 and R9.
        //

        ULONG InRegister:1;

        //
        // When set, indicates this argument is being passed on the stack.
        // This applies to fifth arguments onward.
        //

        ULONG OnStack:1;

        //
        // When set, indicates that this argument is a pointer.
        //

        ULONG IsPointer:1;

        //
        // When set, indicates this is a user-defined type (UDT), e.g. structs,
        // unions, enums and classes.
        //

        ULONG IsUdt:1;

        //
        // When set, indicates that the argument is a structure.  This will
        // typically always be set in conjunction with IsPointer.
        //

        ULONG IsStruct:1;

        //
        // When set, indicates the argument is a union.
        //

        ULONG IsUnion:1;

        //
        // When set, indicates the argument is an enum.
        //

        ULONG IsEnum:1;

        //
        // When set, indicates the argument is a C++ class.  This will typically
        // always be set in conjunction with IsPointer.
        //

        ULONG IsClass:1;

        //
        // If a colon is detected in the symbol name, the name is assumed to
        // be a (demangled) C++ symbol name.  The following flag will be set.
        //
        // N.B. Only one colon is tested for, despite two being technically the
        //      valid indicator of a C++ name.  However, nothing else should use
        //      a colon in their name, so we should be fine.
        //

        ULONG IsCpp:1;

        //
        // When set, indicates the argument is a vector register.  This covers
        // any register over the native pointer size, e.g. typically 128-bits
        // or larger.
        //

        ULONG IsVector:1;

        //
        // The following flags are only set if IsVector is set.
        //

        //
        // When set, indicates that this is a 64-bit MMX (__m64) register.
        //

        ULONG IsMmx:1;

        //
        // When set, indicates that this is a 128-bit XMM (__m128) register.
        //

        ULONG IsXmm:1;

        //
        // When set, indicates that this is a 256-bit YMM (__m256) register.
        //

        ULONG IsYmm:1;

        //
        // When set, indicates that this is a 512-bit ZMM (__m512) register.
        //

        ULONG IsZmm:1;

        //
        // When set, indicates the vector type is floating point (e.g. of
        // type float or double).
        //

        ULONG IsFloatingPointVectorType:1;

        //
        // When set, indicates the vector type is a SIMD type, e.g. __m128
        // (XMM) or __m256 (YMM).
        //

        ULONG IsSimdVectorType:1;

        //
        // When set, indicates the vector type is a homogenous float aggregate
        // (HFA) value.  This is any composite type that has up to four members
        // of identical floating-point vector types, e.g.:
        //
        //      typedef struct _HFA3 {
        //          double X;
        //          double Y;
        //          double Z;
        //      } HFA3;
        //

        ULONG IsHfaVectorType:1;

        //
        // When set, indicates the vector type is a homogenous vector aggregate
        // (HVA) value.  This is any composite type that has up to four members
        // of identical vector type, e.g.:
        //
        //      typedef struct _HVA3 {
        //          __m256 X;
        //          __m256 Y;
        //          __m256 Z;
        //      } HVA3;
        //

        ULONG IsHvaVectorType:1;
    };
    LONG AsLong;
    ULONG AsULong;
} DEBUG_ENGINE_FUNCTION_ARGUMENT_FLAGS;
C_ASSERT(sizeof(DEBUG_ENGINE_FUNCTION_ARGUMENT_FLAGS) == sizeof(ULONG));

typedef struct _DEBUG_ENGINE_FUNCTION_ARGUMENT {

    //
    // Size of the structure, in bytes.
    //

    _Field_range_(==, sizeof(struct _DEBUG_ENGINE_FUNCTION_ARGUMENT))
        ULONG SizeOfStruct;

    //
    // Flags.
    //

    DEBUG_ENGINE_FUNCTION_ARGUMENT_FLAGS Flags;

    //
    // Size of the argument, in bytes.
    //

    SHORT SizeInBytes;

    //
    // Position of the argument in the function signature.
    //

    USHORT ArgumentNumber;

    //
    // If passed on the stack, the offset from the stack pointer upon entry
    // into the function.
    //

    USHORT StackPointerOffset;

    //
    // If this is a pointer, the following field reflects the depth of the
    // pointer.  That is, literally how many asterisks were detected in the
    // argument name.
    //

    USHORT PointerDepth;

    struct {

        //
        // Atom (hash) value of the argument type's character representation.
        //

        USHORT ArgumentType;

        //
        // Atom (hash) value of the argument type's name, if applicable.
        //

        USHORT ArgumentTypeName;

    } Atom;

    //
    // The basic type of the argument.
    //

    DEBUG_ENGINE_FUNCTION_ARGUMENT_TYPE ArgumentType;

    //
    // String structures that capture the character representation of the
    // argument type and argument type name.  The latter only applies to
    // UDTs (e.g. structs, unions etc).
    //

    struct {

        //
        // The basic type name of this argument.
        //

        STRING ArgumentType;

        //
        // For composite types like structs/unions etc, the name of the type.
        //

        STRING ArgumentTypeName;

    } String;

    //
    // If passed via register, the register identifier.
    //

    union {
        DEBUG_ENGINE_FUNCTION_ARGUMENT_REGISTER_X64 x64;
    } Register;

    //
    // Link to the parent DEBUG_ENGINE_EXAMINED_SYMBOL via ArgumentsListHead.
    //

    LIST_ENTRY ListEntry;

    //
    // A pointer to the displayed type for this argument, if applicable.
    //

    struct _DEBUG_ENGINE_DISPLAYED_TYPE *Type;

} DEBUG_ENGINE_FUNCTION_ARGUMENT;
typedef DEBUG_ENGINE_FUNCTION_ARGUMENT *PDEBUG_ENGINE_FUNCTION_ARGUMENT;

//
// This structure is wired up such that each STRING structure points to the
// relevant part of the line output.
//

typedef struct _DEBUG_ENGINE_EXAMINED_SYMBOL_LINE_STRINGS {

    //
    // Points to the raw line.
    //

    STRING Line;

    //
    // Points to the initial scope part of the line, e.g. "prv global".
    //

    STRING Scope;

    //
    // Points to the hex address of the symbol.
    //

    STRING Address;

    //
    // Points to the size element of the symbol.
    //

    STRING Size;

    //
    // Points to the basic type name of the symbol.
    //

    STRING BasicType;

    //
    // For user defined types (UDT) -- classes, structs and unions, this
    // will point to the name of the type.
    //

    STRING TypeName;

    //
    // Points to any array information present after the type name but
    // before the module name.  For example, given:
    //
    //      double [5] python27!bigtens = ...
    //      struct _UNICODE_STRING *[95] Python!ApiSetFilesW ...
    //
    // Array will capture the "[5]" and "[95]" respectively.  (The presence
    // of one or more preceeding asterisks will be captured in the flag
    // 'IsPointer'.)
    //

    STRING Array;

    //
    // Points to the module name, if applicable.
    //

    STRING ModuleName;

    //
    // Points to the symbol name, if applicable.
    //

    union {
        STRING SymbolName;
        STRING Function;
        STRING Structure;
        STRING Union;
    };

    //
    // Remaining string after the space that trails the symbol name will
    // be specific to the type being examined.
    //

    STRING Remaining;

    //
    // Specific symbol types can refine the range of bytes that the
    // "Remaining" string above points to via this Value field.
    //

    union {
        STRING Value;
        STRING FunctionArguments;
    };

} DEBUG_ENGINE_EXAMINED_SYMBOL_LINE_STRINGS;
typedef DEBUG_ENGINE_EXAMINED_SYMBOL_LINE_STRINGS *PDEBUG_ENGINE_EXAMINED_SYMBOL_LINE_STRINGS;

typedef struct _DEBUG_ENGINE_EXAMINED_SYMBOL_FUNCTION {

    //
    // Number of arguments to the function.
    //

    USHORT NumberOfArguments;

    //
    // Pad out to a pointer boundary.
    //

    USHORT Reserved[3];

    //
    // Linked-list head of all function arguments.
    //

    LIST_ENTRY ArgumentsListHead;

} DEBUG_ENGINE_EXAMINED_SYMBOL_FUNCTION;
typedef DEBUG_ENGINE_EXAMINED_SYMBOL_FUNCTION *PDEBUG_ENGINE_EXAMINED_SYMBOL_FUNCTION;

typedef struct _Struct_size_bytes_(SizeOfStruct) _DEBUG_ENGINE_EXAMINED_SYMBOL {

    //
    // Size of structure, in bytes.
    //

    _Field_range_(== , sizeof(struct _DEBUG_ENGINE_EXAMINED_SYMBOL))
        ULONG SizeOfStruct;

    //
    // Flags.
    //

    DEBUG_ENGINE_EXAMINED_SYMBOL_FLAGS Flags;

    //
    // The type of the symbol.
    //

    DEBUG_ENGINE_EXAMINE_SYMBOLS_TYPE Type;

    //
    // The scope of the symbol.
    //

    DEBUG_ENGINE_EXAMINE_SYMBOLS_SCOPE Scope;

    //
    // String structures representing various parts of the line.
    //

    DEBUG_ENGINE_EXAMINED_SYMBOL_LINE_STRINGS Strings;

    //
    // Pointer to the owning Output structure; this allows navigation back to
    // the debug engine and calling context.
    //

    PDEBUG_ENGINE_OUTPUT Output;

    ULONG Size;
    ULONG Padding1;

    //
    // The hex address in the text is converted to a 64-bit integer address and
    // stored in the following field.
    //

    LARGE_INTEGER Address;


    //
    // Linked list entry that links to the parent output.
    //

    LIST_ENTRY ListEntry;

    //
    // Variable parts of the structure depending on the flags/type.
    //

    union {

        DEBUG_ENGINE_EXAMINED_SYMBOL_FUNCTION Function;

    };

} DEBUG_ENGINE_EXAMINED_SYMBOL;
typedef DEBUG_ENGINE_EXAMINED_SYMBOL *PDEBUG_ENGINE_EXAMINED_SYMBOL;
typedef DEBUG_ENGINE_EXAMINED_SYMBOL **PPDEBUG_ENGINE_EXAMINED_SYMBOL;

//
// UnassembleFunction-related structures and functions.
//

typedef union _DEBUG_ENGINE_UNASSEMBLED_FUNCTION_FLAGS {
    struct _Struct_size_bytes_(sizeof(ULONG)) {

        //
        // The function modifies no non-volatile registers.  (This includes the
        // stack pointer, thus, leaf entry functions cannot call other functions
        // as that implicitly manipulates the stack.)
        //

        ULONG IsLeafEntry:1;

        //
        // When set, indicates that this function manipulates non-volatile
        // registers.  This is the most common.
        //

        ULONG IsNestedEntry:1;

        //
        // When set, indicates that one or more arguments are vector arguments
        // passed in vector registers.
        //

        ULONG HasVectorArguments:1;

        //
        // When set, indicates that this function has arguments passed via the
        // stack.  This will usually be for the fifth argument onward.
        //

        ULONG HasStackArguments:1;

        //
        // When set, indicates the return type is an integer type and will be
        // accessible in the AL/AX/EAX/RAX register.
        //

        ULONG IntegerReturnType:1;

        //
        // When set, indicates the return type is a vector type (floating point
        // or SIMD) and will be returned via the XMM0/YMM0 register.
        //

        ULONG VectorReturnType:1;

        //
        // Indicates the vector return type is a floating-point type.  This will
        // only be set if VectorReturnType is set.
        //

        ULONG FloatingPointVectorReturnType:1;

        //
        // Indicates the vector return type is a SIMD type.  This will only be
        // set if VectorReturnType is set.
        //

        ULONG SimdVectorReturnType:1;

        //
        // The following flags relate to the function's calling convention.
        //

        //
        // Function uses the __stdcall calling convention.
        //

        ULONG IsStdcall:1;

        //
        // Function uses the __cdecl calling convention.
        //

        ULONG IsCdecl:1;

        //
        // Function uses the __fastcall calling convention.  This will be set
        // for all x64 functions except for __clrcall or __vectorcall.
        //

        ULONG IsFastcall:1;

        //
        // Function uses the __thiscall calling convention.  Only applicable to
        // C++ programs.
        //

        ULONG IsThiscall:1;

        //
        // Function uses the __clrcall calling convention.
        //

        ULONG IsClrcall:1;

        //
        // Function uses the __vectorcall calling convention.
        //

        ULONG IsVectorcall:1;

    };

    LONG AsLong;
    ULONG AsULong;

} DEBUG_ENGINE_UNASSEMBLED_FUNCTION_FLAGS;
C_ASSERT(sizeof(DEBUG_ENGINE_UNASSEMBLED_FUNCTION_FLAGS) == sizeof(ULONG));

typedef struct _Struct_size_bytes_(SizeOfStruct) _DEBUG_ENGINE_UNASSEMBLED_FUNCTION {

    //
    // Size of structure, in bytes.
    //

    _Field_range_(== , sizeof(struct _DEBUG_ENGINE_UNASSEMBLED_FUNCTION))
        ULONG SizeOfStruct;

    //
    // Flags.
    //

    DEBUG_ENGINE_UNASSEMBLED_FUNCTION_FLAGS Flags;

    //
    // Pointer to the owning Output structure; this allows navigation back to
    // the debug engine and calling context.
    //

    PDEBUG_ENGINE_OUTPUT Output;

    //
    // Pointer to the DEBUG_SESSION_EXAMINED_SYMBOL structure that triggered
    // this function unassembly.  From the Symbol we can derive the function
    // signature, name, number of arguments, argument list, etc.
    //

    PDEBUG_ENGINE_EXAMINED_SYMBOL Symbol;

    //
    // List head of saved lines.  This overrides the DEBUG_ENGINE_OUTPUT's
    // SavedLineListHead structure.
    //

    LIST_ENTRY SavedLinesListHead;

    //
    // Number of instructions.
    //

    ULONG NumberOfInstructions;

    //
    // Number of basic blocks.
    //

    ULONG NumberOfBasicBlocks;

} DEBUG_ENGINE_UNASSEMBLED_FUNCTION;
typedef DEBUG_ENGINE_UNASSEMBLED_FUNCTION
      *PDEBUG_ENGINE_UNASSEMBLED_FUNCTION;
typedef DEBUG_ENGINE_UNASSEMBLED_FUNCTION
    **PPDEBUG_ENGINE_UNASSEMBLED_FUNCTION;

typedef union {
    struct _Struct_size_bytes_(sizeof(ULONG)) {

        //
        // Display number of instructions.  Corresponds to /i.
        //

        ULONG DisplayNumberOfInstructions:1;

        //
        // Relax blocking requirements to allow multiple exits.  Corresponds
        // to /m.
        //

        ULONG RelaxBlockingRequirements:1;
    };
    LONG AsLong;
    ULONG AsULong;
} DEBUG_ENGINE_UNASSEMBLE_FUNCTION_FLAGS;
C_ASSERT(sizeof(DEBUG_ENGINE_UNASSEMBLE_FUNCTION_FLAGS) == sizeof(ULONG));

typedef union _DEBUG_ENGINE_UNASSEMBLE_FUNCTION_COMMAND_OPTIONS {
    struct _Struct_size_bytes_(sizeof(ULONG)) {

        //
        // Only display call instructions in a routine.
        //
        // Corresponds to /c.
        //

        ULONG CallInstructionsOnly:1;

        //
        // Create linked callee names.
        //
        // Corresponds to /D.
        //

        ULONG CreateLinkedCalleeNames:1;

        //
        // Relax blocking requirements to permit multiple exits.
        //
        // Corresponds to /m.
        //

        ULONG RelaxBlockingRequirements:1;

        //
        // Sort display by address instead of function offset.  This option
        // presents a memory-layout view of a full function.
        //
        // Corresponds to /o.
        //

        ULONG SortByAddress:1;

        //
        // Creates linked call lines for accessing call information and
        // creating breakpoints.
        //
        // Corresponds to /O.
        //

        ULONG CreateLinkedCallLines:1;

        //
        // Display the number of instructions in the routine.
        //
        // Corresponds to /i.
        //

        ULONG DisplayInstructionCount:1;

        //
        // Unused.
        //

        ULONG Unused:26;

    };

    LONG AsLong;
    ULONG AsULong;

} DEBUG_ENGINE_UNASSEMBLE_FUNCTION_COMMAND_OPTIONS;
C_ASSERT(
    sizeof(DEBUG_ENGINE_UNASSEMBLE_FUNCTION_COMMAND_OPTIONS) ==
    sizeof(ULONG)
);

typedef
_Check_return_
_Success_(return != 0)
BOOL
(DEBUG_ENGINE_UNASSEMBLE_FUNCTION)(
    _In_ PDEBUG_ENGINE_OUTPUT Output,
    _In_ DEBUG_ENGINE_OUTPUT_FLAGS OutputFlags,
    _In_ DEBUG_ENGINE_UNASSEMBLE_FUNCTION_COMMAND_OPTIONS CommandOptions,
    _In_ PCUNICODE_STRING FunctionName
    );
typedef DEBUG_ENGINE_UNASSEMBLE_FUNCTION *PDEBUG_ENGINE_UNASSEMBLE_FUNCTION;

//
// DisplayType-related structures and functions.
//

typedef union _DEBUG_ENGINE_DISPLAY_TYPE_COMMAND_OPTIONS {
    struct _Struct_size_bytes_(sizeof(ULONG)) {

        //
        // Show each array element on a new line.
        //
        // Corresponds to -a.
        //

        ULONG ShowArrayElements:1;

        //
        // Display blocks recursively.
        //
        // Corresponds to -b.
        //

        ULONG DisplayBlocksRecursively:1;

        //
        // Compact output; all fields are displayed on one line, if possible.
        //

        ULONG CompactOutput:1;

        //
        // Force type enumeration.
        //
        // Corresponds to -e.
        //

        ULONG ForceTypeEnumeration:1;

        //
        // Recursively dump subtype fields.
        //
        // Corresponds to -r9.
        //

        ULONG RecursivelyDumpSubtypes:1;

        //
        // Verbose output.  Includes total size of a structure and the number
        // of elements, if applicable.
        //
        // Corresponds to -v.
        //

        ULONG Verbose:1;

        //
        // Unused.
        //

        ULONG Unused:26;

    };

    LONG AsLong;
    ULONG AsULong;

} DEBUG_ENGINE_DISPLAY_TYPE_COMMAND_OPTIONS;
C_ASSERT(sizeof(DEBUG_ENGINE_DISPLAY_TYPE_COMMAND_OPTIONS) == sizeof(ULONG));

typedef enum _DEBUG_ENGINE_DISPLAY_TYPES {
    UnknownDisplayTypeType = -1,

    //
    // First 11 types captured by DisplayTypes.
    //

    CharDisplayType = 0,
    UnsignedCharDisplayType,
    WideCharDisplayType,

    ShortIntegerDisplayType,
    LongIntegerDisplayType,
    LongLongIntegerDisplayType,

    UnsignedShortIntegerDisplayType,
    UnsignedLongIntegerDisplayType,
    UnsignedLongLongIntegerDisplayType,

    VoidDisplayType,
    EnumDisplayType,

    //
    // Next set of types start at 16.
    //

    PointerToDisplayType = 16,
    StructDisplayType,
    UnionDisplayType,

    BitfieldPositionDisplayType,
    ArrayDisplayType,

    //
    // Any enumeration value >= InvalidDisplayType is invalid.  Make sure this
    // always comes last in the enum layout.
    //

    InvalidDisplayType

} DEBUG_ENGINE_DISPLAY_TYPES;
typedef DEBUG_ENGINE_DISPLAY_TYPES *PDEBUG_ENGINE_DISPLAY_TYPES;

typedef
_Check_return_
_Success_(return != 0)
BOOL
(DEBUG_ENGINE_DISPLAY_TYPE)(
    _In_ PDEBUG_ENGINE_OUTPUT Output,
    _In_ DEBUG_ENGINE_OUTPUT_FLAGS OutputFlags,
    _In_ DEBUG_ENGINE_DISPLAY_TYPE_COMMAND_OPTIONS CommandOptions,
    _In_ PCUNICODE_STRING SymbolName
    );
typedef DEBUG_ENGINE_DISPLAY_TYPE *PDEBUG_ENGINE_DISPLAY_TYPE;

typedef union _DEBUG_ENGINE_DISPLAYED_TYPE_FLAGS {
    LONG AsLong;
    ULONG AsULong;
    struct _Struct_size_bytes_(sizeof(ULONG)) {
        ULONG Unused:1;
    };
} DEBUG_ENGINE_DISPLAYED_TYPE_FLAGS;
C_ASSERT(sizeof(DEBUG_ENGINE_DISPLAYED_TYPE_FLAGS) == sizeof(ULONG));

typedef struct _Struct_size_bytes_(SizeOfStruct) _DEBUG_ENGINE_DISPLAYED_TYPE {

    //
    // Size of structure, in bytes.
    //

    _Field_range_(== , sizeof(struct _DEBUG_ENGINE_DISPLAYED_TYPE))
        ULONG SizeOfStruct;

    //
    // Flags.
    //

    DEBUG_ENGINE_DISPLAYED_TYPE_FLAGS Flags;

    //
    // Pointer to the owning Output structure; this allows navigation back to
    // the debug engine and calling context.
    //

    PDEBUG_ENGINE_OUTPUT Output;

    //
    // Pointer to the DEBUG_SESSION_EXAMINED_SYMBOL structure that triggered
    // this function unassembly.  From the Symbol we can derive the function
    // signature, name, number of arguments, argument list, etc.
    //

    PDEBUG_ENGINE_EXAMINED_SYMBOL Symbol;

    //
    // List head of saved lines.  This overrides the DEBUG_ENGINE_OUTPUT's
    // SavedLineListHead structure.
    //

    LIST_ENTRY SavedLinesListHead;

    //
    // Total size of the type, in bytes.
    //

    ULONG TotalSizeInBytes;

    union {

        struct {

            //
            // Number of unique field offsets.
            //

            ULONG NumberOfUniqueFieldOffsets;

            //
            // Number of fields.
            //

            ULONG NumberOfFields;

        } Structure;

    };

} DEBUG_ENGINE_DISPLAYED_TYPE;
typedef DEBUG_ENGINE_DISPLAYED_TYPE   *PDEBUG_ENGINE_DISPLAYED_TYPE;
typedef DEBUG_ENGINE_DISPLAYED_TYPE **PPDEBUG_ENGINE_DISPLAYED_TYPE;

//
// SettingsMeta-related structures and functions.
//

typedef union _DEBUG_ENGINE_SETTINGS_META_COMMAND_OPTIONS {
    struct _Struct_size_bytes_(sizeof(ULONG)) {

        //
        // List current settings.
        //
        // Corresponds to "list".
        //

        ULONG List:1;

        //
        // Load settings from xml.
        //
        // Corresponds to "load <xmlpath>".
        //

        ULONG Load:1;

        //
        // Unused.
        //

        ULONG Unused:30;

    };

    LONG AsLong;
    ULONG AsULong;

} DEBUG_ENGINE_SETTINGS_META_COMMAND_OPTIONS;
C_ASSERT(sizeof(DEBUG_ENGINE_SETTINGS_META_COMMAND_OPTIONS) == sizeof(ULONG));

typedef
_Check_return_
_Success_(return != 0)
BOOL
(DEBUG_ENGINE_SETTINGS_META)(
    _In_ PDEBUG_ENGINE_OUTPUT Output,
    _In_ DEBUG_ENGINE_OUTPUT_FLAGS OutputFlags,
    _In_ DEBUG_ENGINE_SETTINGS_META_COMMAND_OPTIONS CommandOptions,
    _In_opt_ PCUNICODE_STRING Arguments
    );
typedef DEBUG_ENGINE_SETTINGS_META *PDEBUG_ENGINE_SETTINGS_META;

typedef
_Check_return_
_Success_(return != 0)
BOOL
(DEBUG_ENGINE_LIST_SETTINGS)(
    _In_ struct _DEBUG_ENGINE_SESSION *Session
    );
typedef DEBUG_ENGINE_LIST_SETTINGS *PDEBUG_ENGINE_LIST_SETTINGS;

//
// Generic static and dynamic command execution structures and typedefs.
//

typedef
_Check_return_
_Success_(return != 0)
_Requires_lock_not_held_(Session->Engine->Lock)
BOOL
(DEBUG_ENGINE_SESSION_EXECUTE_STATIC_COMMAND)(
    _In_ struct _DEBUG_ENGINE_SESSION *Session,
    _In_ PCUNICODE_STRING Command,
    _In_opt_ PDEBUG_ENGINE_LINE_OUTPUT_CALLBACK LineOutputCallback
    );
typedef DEBUG_ENGINE_SESSION_EXECUTE_STATIC_COMMAND
      *PDEBUG_ENGINE_SESSION_EXECUTE_STATIC_COMMAND;

typedef union _DEBUG_ENGINE_DYNAMIC_COMMAND_FLAGS {
    LONG AsLong;
    ULONG AsULong;
    struct _Struct_size_bytes_(sizeof(ULONG)) {
        ULONG Unused:1;
    };
} DEBUG_ENGINE_DYNAMIC_COMMAND_FLAGS;
C_ASSERT(sizeof(DEBUG_ENGINE_DYNAMIC_COMMAND_FLAGS) == sizeof(ULONG));
typedef DEBUG_ENGINE_DYNAMIC_COMMAND_FLAGS
      *PDEBUG_ENGINE_DYNAMIC_COMMAND_FLAGS;

typedef struct _Struct_size_bytes_(SizeOfStruct) _DEBUG_ENGINE_DYNAMIC_COMMAND {

    //
    // Size of structure, in bytes.
    //

    _Field_range_(== , sizeof(struct _DEBUG_ENGINE_OUTPUT)) ULONG SizeOfStruct;

    //
    // Flags.
    //

    DEBUG_ENGINE_DYNAMIC_COMMAND_FLAGS Flags;

    //
    // Number of arguments in the array below.
    //

    _Field_range_(<=, 64) BYTE NumberOfArguments;

    USHORT Unused[3];

    //
    // Optional bitmap that can be used to isolate arguments in the array below.
    //

    ULONGLONG Bitmap;

    PCUNICODE_STRING Arguments;

} DEBUG_ENGINE_DYNAMIC_COMMAND;
typedef DEBUG_ENGINE_DYNAMIC_COMMAND *PDEBUG_ENGINE_DYNAMIC_COMMAND;

typedef
_Check_return_
_Success_(return != 0)
_Requires_lock_not_held_(Session->Engine->Lock)
BOOL
(DEBUG_ENGINE_SESSION_EXECUTE_DYNAMIC_COMMAND)(
    _In_ struct _DEBUG_ENGINE_SESSION *Session,
    _In_ PDEBUG_ENGINE_DYNAMIC_COMMAND DynamicCommand,
    _In_opt_ PDEBUG_ENGINE_LINE_OUTPUT_CALLBACK LineOutputCallback
    );
typedef DEBUG_ENGINE_SESSION_EXECUTE_DYNAMIC_COMMAND
      *PDEBUG_ENGINE_SESSION_EXECUTE_DYNAMIC_COMMAND;

//
// DEBUG_ENGINE_SESSION-related function typedefs, unions and structures.
//
// This structure is the main way to interface with the DebugEngine
// functionality exposed by this component.
//

typedef
VOID
(DESTROY_DEBUG_ENGINE_SESSION)(
    _Pre_notnull_ _Post_satisfies_(*Session == 0)
        struct _DEBUG_ENGINE_SESSION **Session,
    _In_opt_ PBOOL IsProcessTerminating
    );
typedef DESTROY_DEBUG_ENGINE_SESSION   *PDESTROY_DEBUG_ENGINE_SESSION;
typedef DESTROY_DEBUG_ENGINE_SESSION **PPDESTROY_DEBUG_ENGINE_SESSION;
DEBUG_ENGINE_API DESTROY_DEBUG_ENGINE_SESSION DestroyDebugEngineSession;

typedef
_Check_return_
_Success_(return != 0)
BOOL
(INITIALIZE_CHILD_DEBUG_ENGINE_SESSION)(
    _In_ struct _DEBUG_ENGINE_SESSION *Parent,
    _In_ PCDEBUGEVENTCALLBACKS EventCallbacks,
    _In_ PCGUID InterfaceId,
    _In_ DEBUG_EVENT_CALLBACKS_INTEREST_MASK InterestMask,
    _Outptr_result_nullonfailure_ struct _DEBUG_ENGINE_SESSION **SessionPointer
    );
typedef INITIALIZE_CHILD_DEBUG_ENGINE_SESSION
      *PINITIALIZE_CHILD_DEBUG_ENGINE_SESSION;

typedef
_Check_return_
_Success_(return != 0)
BOOL
(INITIALIZE_CHILD_DEBUG_ENGINE)(
    _In_ PRTL Rtl,
    _Inout_ PDEBUG_ENGINE Engine,
    _In_ PDEBUG_ENGINE Parent
    );
typedef INITIALIZE_CHILD_DEBUG_ENGINE *PINITIALIZE_CHILD_DEBUG_ENGINE;

typedef
HRESULT
(EVENT_DISPATCH)(
    _In_ struct _DEBUG_ENGINE_SESSION *Session,
    _In_ ULONG Flags,
    _In_opt_ ULONG TimeoutInMilliseconds
    );
typedef EVENT_DISPATCH *PEVENT_DISPATCH;

typedef union _DEBUG_ENGINE_SESSION_FLAGS {
    struct {

        //
        // When set, indicates the debug engine has attached to the current
        // process.
        //

        ULONG InProc:1;

        //
        // When set, indicates the debug engine has attached to another process.
        //

        ULONG OutOfProc:1;

        //
        // When set, indicates that an existing process was attached to.
        //

        ULONG AttachedToExistingProcess:1;

        //
        // When set, indicates a new process was created.
        //

        ULONG CreatedNewProcess:1;

        //
        // When set, indicates injection intent.
        //

        ULONG InjectionIntent:1;

        //
        // When set, indicates a server was started for this debug engine
        // session (e.g. via Control->StartServer()).
        //

        ULONG StartedServer:1;

    };

    LONG AsLong;
    ULONG AsULong;

} DEBUG_ENGINE_SESSION_FLAGS;
C_ASSERT(sizeof(DEBUG_ENGINE_SESSION_FLAGS) == sizeof(ULONG));
typedef DEBUG_ENGINE_SESSION_FLAGS *PDEBUG_ENGINE_SESSION_FLAGS;

//
// Command line options enumeration.
//

typedef enum _DEBUG_ENGINE_COMMAND_LINE_OPTION {

    UnknownCommandLineOption = -1,

    //
    // Corresponds to -p/--pid.
    //

    PidCommandLineOption = 0,

    //
    // Corresponds to -c/--commandline.
    //

    CommandLineCommandLineOption,

} DEBUG_ENGINE_COMMAND_LINE_OPTION;
typedef DEBUG_ENGINE_COMMAND_LINE_OPTION *PDEBUG_ENGINE_COMMAND_LINE_OPTION;

//
// Define event loop function pointer typedefs.
//

typedef
_Success_(return != 0)
BOOL
(DEBUG_ENGINE_SESSION_EVENT_LOOP)(
    _In_ struct _DEBUG_ENGINE_SESSION *Session
    );
typedef DEBUG_ENGINE_SESSION_EVENT_LOOP *PDEBUG_ENGINE_SESSION_EVENT_LOOP;

typedef
_Success_(return != 0)
BOOL
(DEBUG_ENGINE_SESSION_EVENT_LOOP_RUN_ONCE)(
    _In_ struct _DEBUG_ENGINE_SESSION *Session,
    _Out_ PBOOL TerminateLoop
    );
typedef DEBUG_ENGINE_SESSION_EVENT_LOOP_RUN_ONCE
       *PDEBUG_ENGINE_SESSION_EVENT_LOOP_RUN_ONCE;

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
    // Debug engine (Internal).
    //

    struct _DEBUG_ENGINE *Engine;

    //
    // Secondary debug engine used for dispatching Control->ExitDispatch()
    // calls.  This results in Session->WaitForEvent() returning and entering
    // a predictable APC-dequeuing pattern before attempt to wait for debug
    // engine events again.
    //

    struct _DEBUG_ENGINE *ExitDispatchEngine;

    //
    // Tracer injection modules, if applicable.
    //

    struct _TRACER_INJECTION_MODULES *InjectionModules;

    //
    // Parent/child fields.
    //

    struct _DEBUG_ENGINE_SESSION *Parent;

    //
    // If we're a parent session, children are linked to the Children guarded
    // list via the ListEntry field.
    //

    GUARDED_LIST Children;
    LIST_ENTRY ListEntry;

    //
    // Initializer for children.
    //

    PINITIALIZE_CHILD_DEBUG_ENGINE_SESSION InitializeChildDebugEngineSession;

    //
    // Opaque context for children.
    //

    PVOID ChildContext;

    //
    // When this counter hits zero, the AllInjectionInitializersReady event will
    // be set.
    //

    volatile ULONGLONG OutstandingInjectionInitializers;
    HANDLE AllInjectionInitializersReady;

    //
    // When this counter is above zero, indicates there are pending APCs.
    //

    volatile ULONGLONG NumberOfPendingApcs;

    HANDLE ReadyForApcEvent;
    HANDLE WaitForApcEvent;

    APC Apc;

    //
    // Shutdown and shutdown complete events.
    //

    HANDLE ShutdownEvent;
    HANDLE ShutdownCompleteEvent;

    //
    // Destructor.
    //

    PDESTROY_DEBUG_ENGINE_SESSION Destroy;

    //
    // Initialize a DEBUG_ENGINE_OUTPUT structure prior to executing a command.
    //

    PINITIALIZE_DEBUG_ENGINE_OUTPUT InitializeDebugEngineOutput;

    //
    // Event dispatching functions.  Parent sessions typically use WaitForEvent
    // and children use DispatchCallbacks.  EventDispatch will be set to the
    // most appropriate event for the given instance.
    //

    PEVENT_DISPATCH WaitForEvent;
    PEVENT_DISPATCH DispatchCallbacks;
    PEVENT_DISPATCH EventDispatch;

    //
    // Event loop functions.
    //

    PDEBUG_ENGINE_SESSION_EVENT_LOOP EventLoop;
    PDEBUG_ENGINE_SESSION_EVENT_LOOP_RUN_ONCE EventLoopRunOnce;

    ////////////////////////////////////////////////////////////////////////////
    // Commands.
    ////////////////////////////////////////////////////////////////////////////

    //
    // Examine symbols.
    //

    PDEBUG_ENGINE_EXAMINE_SYMBOLS ExamineSymbols;

    //
    // Unassemble function.
    //

    PDEBUG_ENGINE_UNASSEMBLE_FUNCTION UnassembleFunction;

    //
    // Display type.
    //

    PDEBUG_ENGINE_DISPLAY_TYPE DisplayType;

    ////////////////////////////////////////////////////////////////////////////
    // Meta Commands.
    ////////////////////////////////////////////////////////////////////////////

    //
    // Settings
    //

    PDEBUG_ENGINE_SETTINGS_META SettingsMeta;
    PDEBUG_ENGINE_LIST_SETTINGS ListSettings;

    ////////////////////////////////////////////////////////////////////////////
    // Generic Static/Dynamic Commands.
    ////////////////////////////////////////////////////////////////////////////

    PDEBUG_ENGINE_SESSION_EXECUTE_STATIC_COMMAND ExecuteStaticCommand;
    PDEBUG_ENGINE_SESSION_EXECUTE_DYNAMIC_COMMAND ExecuteDynamicCommand;

    ////////////////////////////////////////////////////////////////////////////
    // End of function pointers.
    ////////////////////////////////////////////////////////////////////////////

    //
    // Rtl structure.
    //

    PRTL Rtl;

    //
    // Allocator structure.
    //

    PALLOCATOR Allocator;

    //
    // TracerConfig structure.
    //

    struct _TRACER_CONFIG *TracerConfig;

    //
    // Commandline-related fields.
    //

    PSTR CommandLineA;
    PWSTR CommandLineW;

    DEBUG_ENGINE_COMMAND_LINE_OPTION CommandLineOption;

    LONG NumberOfArguments;
    PPSTR ArgvA;
    PPWSTR ArgvW;

    //
    // Target information extracted from command line.
    //

    union {

        //
        // This will be set if CommandLineOption == PidCommandLineOption
        // (corresponding to -p/--pid).
        //

        ULONG TargetProcessId;

        //
        // This will be set if CommandLineOption == CommandLineCommandLineOption
        // (corresponding to -c/--commandline).
        //

        PCSTR TargetCommandLineA;
    };

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

    ULONG InitialProcessId;
    ULONG InitialThreadId;

    UNICODE_STRING InitialModuleNameW;

    HKEY RunHistoryRegistryKey;

    //
    // StringTable-specific functions.
    //

    PCREATE_STRING_TABLE_FROM_DELIMITED_STRING
        CreateStringTableFromDelimitedString;

    PALLOCATOR StringTableAllocator;
    PALLOCATOR StringArrayAllocator;

    PSTRING_TABLE ExamineSymbolsPrefixStringTable;

    USHORT NumberOfBasicTypeStringTables;
    USHORT NumberOfFunctionArgumentTypeStringTables;
    USHORT NumberOfDisplayTypesStringTables;
    USHORT Padding;

    PSTRING_TABLE ExamineSymbolsBasicTypeStringTable1;
    PSTRING_TABLE ExamineSymbolsBasicTypeStringTable2;

    PSTRING_TABLE FunctionArgumentTypeStringTable1;
    PSTRING_TABLE FunctionArgumentTypeStringTable2;
    PSTRING_TABLE FunctionArgumentVectorTypeStringTable1;

    PSTRING_TABLE DisplayTypesStringTable1;
    PSTRING_TABLE DisplayTypesStringTable2;

    PSTRING_TABLE CommandLineOptionsStringTable;

} DEBUG_ENGINE_SESSION, *PDEBUG_ENGINE_SESSION, **PPDEBUG_ENGINE_SESSION;


//
// Helper macros and inline functions for the debug engine session.
//

#define AcquireDebugEngineLock(Engine) \
    AcquireSRWLockExclusive(&(Engine)->Lock)

#define ReleaseDebugEngineLock(Engine) \
    ReleaseSRWLockExclusive(&(Engine)->Lock)

#define AppendChildSession(Parent, Child) \
    InsertTailGuardedList(&Parent->Children, &Child->ListEntry)

#define AcquireChildrenLockShared(Parent) \
    AcquireGuardedListLockShared(&Parent->Children)

#define ReleaseChildrenLockShared(Parent) \
    ReleaseGuardedListLockShared(&Parent->Children)

#define AcquireChildrenLockExclusive(Parent) \
    AcquireGuardedListLockExclusive(&Parent->Children)

#define ReleaseChildrenLockExclusive(Parent) \
    ReleaseGuardedListLockExclusive(&Parent->Children)

FORCEINLINE
VOID
InjectionInitializationComplete(
    _Inout_ PDEBUG_ENGINE_SESSION Parent,
    _Inout_ PDEBUG_ENGINE_SESSION Child
    )
{
    AppendChildSession(Parent, Child);

    if (!InterlockedDecrement64(&Parent->OutstandingInjectionInitializers)) {
        SetEvent(Parent->AllInjectionInitializersReady);
    }
}

//
// Flags for CreateAndInitializeDebugEngineSession().
//

typedef union _DEBUG_ENGINE_SESSION_INIT_FLAGS {
    struct {

        //
        // When set, initializes the debug engine session based on the command
        // line.
        //

        ULONG InitializeFromCommandLine:1;

        //
        // When set, the debug engine attaches to the currently running process.
        //

        ULONG InitializeFromCurrentProcess:1;

        //
        // When set, starts a debugging server that can be connected to
        // remotely by other debugger clients (e.g. kd, WinDbg).  This is
        // useful for debugging internal code that has been injected into
        // a remote process that was launched via a debug engine session.
        //

        ULONG StartServer:1;

    };
    LONG AsLong;
    ULONG AsULong;
} DEBUG_ENGINE_SESSION_INIT_FLAGS;


//
// Public function typedefs.
//

typedef
_Check_return_
_Success_(return != 0)
BOOL
(INITIALIZE_DEBUG_ENGINE_SESSION)(
    _In_ PRTL Rtl,
    _In_ PALLOCATOR Allocator,
    _In_ DEBUG_ENGINE_SESSION_INIT_FLAGS Flags,
    _In_ struct _TRACER_CONFIG *TracerConfig,
    _In_opt_ HMODULE StringTableModule,
    _In_opt_ PALLOCATOR StringArrayAllocator,
    _In_opt_ PALLOCATOR StringTableAllocator,
    _Outptr_result_nullonfailure_ PPDEBUG_ENGINE_SESSION SessionPointer
    );
typedef INITIALIZE_DEBUG_ENGINE_SESSION *PINITIALIZE_DEBUG_ENGINE_SESSION;

typedef
_Check_return_
_Success_(return != 0)
BOOL
(INITIALIZE_DEBUG_ENGINE_SESSION_WITH_INJECTION_INTENT)(
    _In_ PRTL Rtl,
    _In_ PALLOCATOR Allocator,
    _In_ DEBUG_ENGINE_SESSION_INIT_FLAGS Flags,
    _In_ struct _TRACER_CONFIG *TracerConfig,
    _In_opt_ HMODULE StringTableModule,
    _In_opt_ PALLOCATOR StringArrayAllocator,
    _In_opt_ PALLOCATOR StringTableAllocator,
    _In_ struct _TRACER_INJECTION_MODULES *InjectionModules,
    _Outptr_result_nullonfailure_ PPDEBUG_ENGINE_SESSION SessionPointer
    );
typedef INITIALIZE_DEBUG_ENGINE_SESSION_WITH_INJECTION_INTENT
      *PINITIALIZE_DEBUG_ENGINE_SESSION_WITH_INJECTION_INTENT;

//
// Inline Functions
//

FORCEINLINE
_Check_return_
_Success_(return != 0)
BOOL
LoadAndInitializeDebugEngineSessionWithInjectionIntent(
    _In_ PUNICODE_STRING DebugEngineDllPath,
    _In_ PRTL Rtl,
    _In_ PALLOCATOR Allocator,
    _In_ DEBUG_ENGINE_SESSION_INIT_FLAGS InitFlags,
    _In_ struct _TRACER_CONFIG *TracerConfig,
    _In_ PUNICODE_STRING StringTableDllPath,
    _In_ PALLOCATOR StringArrayAllocator,
    _In_ PALLOCATOR StringTableAllocator,
    _In_ struct _TRACER_INJECTION_MODULES *InjectionModules,
    _Out_ PPDEBUG_ENGINE_SESSION SessionPointer,
    _Out_ PPDESTROY_DEBUG_ENGINE_SESSION DestroyDebugEngineSessionPointer
    )
/*++

Routine Description:

    Initializes a debug session with injection intent.

Arguments:

    DebugEngineDllPath - Supplies a pointer to a UNICODE_STRING that contains
        the fully-qualified path of the DebugEngine DLL to load.

    Rtl - Supplies a pointer to an RTL structure.

    Allocator - Supplies a pointer to an alternate ALLOCATOR to use.

    InitFlags - Supplies flags that can be used to customize the type of
        debug session created.

    TracerConfig - Supplies a pointer to a TRACER_CONFIG structure.

    StringTableDllPath - Supplies a pointer to a UNICODE_STRING that contains
        the fully-qualified path of the StringTable DLL to load.

    StringArrayAllocator - Supplies a pointer to an allocator to use for string
        array allocations.  If the allocator has not yet been initialized (as
        indicated by a NULL value in the Context field), the standard string
        table allocator initialization routine will be called against the
        structure first.

    StringTableAllocator - Supplies a pointer to an allocator to use for string
        table allocations.  If the allocator has not yet been initialized (as
        indicated by a NULL value in the Context field), the standard string
        table allocator initialization routine will be called against the
        structure first.

    InjectionModules - Supplies a pointer to an initialized tracer injection
        modules structure.

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
    DWORD LastError;
    HMODULE Module = NULL;
    HMODULE StringTableModule = NULL;
    PINITIALIZE_DEBUG_ENGINE_SESSION_WITH_INJECTION_INTENT
        InitializeDebugEngineSessionWithInjectionIntent;
    PINITIALIZE_STRING_TABLE_ALLOCATOR InitializeStringTableAllocator;

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

    if (!ARGUMENT_PRESENT(StringArrayAllocator)) {
        return FALSE;
    }

    if (!ARGUMENT_PRESENT(StringTableAllocator)) {
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
        OutputDebugStringA("Failed to load DebugEngine.dll.\n");
        return FALSE;
    }

    //
    // Resolve the initialize function.
    //

    InitializeDebugEngineSessionWithInjectionIntent = (
        (PINITIALIZE_DEBUG_ENGINE_SESSION_WITH_INJECTION_INTENT)(
            GetProcAddress(
                Module,
                "InitializeDebugEngineSessionWithInjectionIntent"
            )
        )
    );

    if (!InitializeDebugEngineSessionWithInjectionIntent) {
        OutputDebugStringA("Failed to resolve InitializeDebugEngineSession"
                           "WithInjectionIntent.\n");
        goto Error;
    }

    //
    // Attempt to load the StringTable module.
    //

    StringTableModule = LoadLibraryW(StringTableDllPath->Buffer);

    if (!StringTableModule) {
        LastError = GetLastError();
        OutputDebugStringA("Failed to load StringTable.dll.\n");
        goto Error;
    }

    //
    // Resolve the StringTable's custom allocator initialization function.
    //

    InitializeStringTableAllocator = (PINITIALIZE_STRING_TABLE_ALLOCATOR)(
        GetProcAddress(
            StringTableModule,
            "InitializeStringTableAllocator"
        )
    );

    if (!InitializeStringTableAllocator) {
        OutputDebugStringA("Failed resolve: InitializeStringTableAllocator\n");
        goto Error;
    }

    //
    // If either of the allocators have not yet been initialized (as indicated
    // by a NULL value in the Context field), initialize them now with the
    // string table allocator.
    //

    if (!StringArrayAllocator->Context) {
        CHECKED_MSG(
            InitializeStringTableAllocator(StringArrayAllocator),
            "InitializeStringTableAllocator(StringArrayAllocator)"
        );
    }

    if (!StringTableAllocator->Context) {
        CHECKED_MSG(
            InitializeStringTableAllocator(StringTableAllocator),
            "InitializeStringTableAllocator(StringTableAllocator)"
        );
    }

    //
    // Call the initialization function with the same arguments we were passed.
    //

    Success = InitializeDebugEngineSessionWithInjectionIntent(
        Rtl,
        Allocator,
        InitFlags,
        TracerConfig,
        StringTableModule,
        StringArrayAllocator,
        StringTableAllocator,
        InjectionModules,
        SessionPointer
    );

    if (!Success) {
        goto Error;
    }

    //
    // Update the caller's DestroyDebugEngineSession function pointer and the
    // session pointer to the same function.
    //

    *DestroyDebugEngineSessionPointer = (*SessionPointer)->Destroy;

    //
    // Return success.
    //

    return TRUE;

Error:

    //
    // Attempt to destroy the session.
    //

    if (SessionPointer && *SessionPointer && (*SessionPointer)->Destroy) {

        BOOL IsProcessTerminating = TRUE;

        //
        // This will also clear the caller's session pointer.
        //

        (*SessionPointer)->Destroy(SessionPointer, &IsProcessTerminating);
    }

    //
    // Attempt to free any modules we loaded.
    //

    if (Module) {
        FreeLibrary(Module);
        Module = NULL;
    }

    if (StringTableModule) {
        FreeLibrary(StringTableModule);
        StringTableModule = NULL;
    }

    return FALSE;
}

FORCEINLINE
_Check_return_
_Success_(return != 0)
BOOL
LoadAndInitializeDebugEngineSession(
    _In_ PUNICODE_STRING DebugEngineDllPath,
    _In_ PRTL Rtl,
    _In_ PALLOCATOR Allocator,
    _In_ DEBUG_ENGINE_SESSION_INIT_FLAGS InitFlags,
    _In_ struct _TRACER_CONFIG *TracerConfig,
    _In_ PUNICODE_STRING StringTableDllPath,
    _In_ PALLOCATOR StringArrayAllocator,
    _In_ PALLOCATOR StringTableAllocator,
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

    TracerConfig - Supplies a pointer to a TRACER_CONFIG structure.

    StringTableDllPath - Supplies a pointer to a UNICODE_STRING that contains
        the fully-qualified path of the StringTable DLL to load.

    StringArrayAllocator - Supplies a pointer to an allocator to use for string
        array allocations.  If the allocator has not yet been initialized (as
        indicated by a NULL value in the Context field), the standard string
        table allocator initialization routine will be called against the
        structure first.

    StringTableAllocator - Supplies a pointer to an allocator to use for string
        table allocations.  If the allocator has not yet been initialized (as
        indicated by a NULL value in the Context field), the standard string
        table allocator initialization routine will be called against the
        structure first.

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
    return (
        LoadAndInitializeDebugEngineSessionWithInjectionIntent(
            DebugEngineDllPath,
            Rtl,
            Allocator,
            InitFlags,
            TracerConfig,
            StringTableDllPath,
            StringArrayAllocator,
            StringTableAllocator,
            NULL,
            SessionPointer,
            DestroyDebugEngineSessionPointer
        )
    );
}

//
// Public function declarations.
//

#pragma component(browser, off)
DEBUG_ENGINE_API INITIALIZE_DEBUG_ENGINE_SESSION InitializeDebugEngineSession;
DEBUG_ENGINE_API INITIALIZE_DEBUG_ENGINE_SESSION_WITH_INJECTION_INTENT
                 InitializeDebugEngineSessionWithInjectionIntent;
#pragma component(browser, on)

#ifdef __cplusplus
} // extern "C"
#endif

// vim:set ts=8 sw=4 sts=4 tw=80 expandtab                                     :
