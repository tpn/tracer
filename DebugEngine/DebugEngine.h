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
#include "../Rtl/Rtl.h"
#include "../StringTable/StringTable.h"

#endif

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

typedef union _DEBUG_ENGINE_OUTPUT_STATE {
    LONG AsLong;
    ULONG AsULong;
    struct _Struct_size_bytes_(sizeof(ULONG)) {
        ULONG Initialized:1;
        ULONG CommandBuilt:1;
        ULONG CommandExecuting:1;
        ULONG InPartialOutputCallback:1;
        ULONG CommandComplete:1;
        ULONG Failed:1;
        ULONG Succeeded:1;
    };
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
    LONG AsLong;
    ULONG AsULong;
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
        // When set, indicates the caller wants wide character (WCHAR) output.
        // (Not currently supported.)
        //

        ULONG WideCharacterOutput:1;
    };
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

    ULONG NumberOfLines;
    ULONG NumberOfPartialCallbacks;
    ULONG TotalBufferLengthInChars;
    ULONG TotalBufferSizeInBytes;
    ULONG LargestChunkSizeInBytes;
    ULONG Unused1;

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
    // Pointers to the command executed by the engine in ANSI and wide formats.
    //

    PUNICODE_STRING Command;

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
    // The following buffer is intended to point to a contiguous text buffer
    // that contains all partial output chunks and whose size is governed by
    // the TotalBufferLengthInChars and TotalBufferSizeInBytes fields above.
    //

    union {
        PSTR Buffer;
        PWSTR BufferWide;
    };

    //
    // If line-oriented callback has been requested, the following structure
    // will be filled out with the line details on each callback invocation.
    //

    union {
        STRING Line;
        UNICODE_STRING LineWide;
    };

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
    _In_ PDEBUG_ENGINE_LINE_OUTPUT_CALLBACK LineOutputCallback,
    _In_ PDEBUG_ENGINE_PARTIAL_OUTPUT_CALLBACK PartialOutputCallback,
    _In_ PDEBUG_ENGINE_OUTPUT_COMPLETE_CALLBACK OutputCompleteCallback,
    _In_opt_ PVOID Context,
    _In_opt_ PRTL_PATH ModulePath
    );
typedef INITIALIZE_DEBUG_ENGINE_OUTPUT *PINITIALIZE_DEBUG_ENGINE_OUTPUT;


//
// DEBUG_ENGINE_EXAMINE_SYMBOLS-related functions and structures.
//

typedef enum _DEBUG_ENGINE_EXAMINE_SYMBOLS_SCOPE {
    PrivateFunctionScope = 0,
    PrivateGlobalScope,
    PrivateInlineScope,
    PublicFunctionScope,
    PublicGlobalScope,
} DEBUG_ENGINE_EXAMINE_SYMBOLS_SCOPE;

typedef enum _DEBUG_ENGINE_EXAMINE_SYMBOLS_TYPE {
    NullType = -1,
    CharType = 0,
    WideCharType,
    ShortType,
    LongType,
    IntegerType,
    Integer64Type,
    FloatType,
    DoubleType,
    UnionType,
    StructType,
    UnsignedType,
    SALExecutionContextType,
    FunctionType,
    CLRType,
    NoType,
    InvalidType = NoType + 1
} DEBUG_ENGINE_EXAMINE_SYMBOLS_TYPE;

typedef union _DEBUG_ENGINE_EXAMINE_SYMBOLS_COMMAND_OPTIONS {
    LONG AsLong;
    ULONG AsULong;
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
// UnassembleFunction-related structures and functions.
//

typedef union _DEBUG_ENGINE_UNASSEMBLED_FUNCTION_FLAGS {
    LONG AsLong;
    ULONG AsULong;
    struct _Struct_size_bytes_(sizeof(ULONG)) {

        //
        // Indicates the text strings are in wide character format.
        //

        ULONG WideCharacter:1;
    };
} DEBUG_ENGINE_UNASSEMBLED_FUNCTION_FLAGS;

typedef
struct _Struct_size_bytes_(SizeOfStruct)
_DEBUG_ENGINE_UNASSEMBLED_FUNCTION {

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
    // Raw text returned from DbgEng.
    //

    union {
        PSTRING RawText;
        PUNICODE_STRING RawTextWide;
    };

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
    LONG AsLong;
    ULONG AsULong;
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
} DEBUG_ENGINE_UNASSEMBLE_FUNCTION_FLAGS;
C_ASSERT(sizeof(DEBUG_ENGINE_UNASSEMBLE_FUNCTION_FLAGS) == sizeof(ULONG));

typedef union _DEBUG_ENGINE_UNASSEMBLE_FUNCTION_COMMAND_OPTIONS {
    LONG AsLong;
    ULONG AsULong;
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
    _In_ PUNICODE_STRING FunctionName
    );
typedef DEBUG_ENGINE_UNASSEMBLE_FUNCTION *PDEBUG_ENGINE_UNASSEMBLE_FUNCTION;

//
// DisplayType-related structures and functions.
//

typedef union _DEBUG_ENGINE_DISPLAY_TYPE_COMMAND_OPTIONS {
    LONG AsLong;
    ULONG AsULong;
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

} DEBUG_ENGINE_DISPLAY_TYPE_COMMAND_OPTIONS;
C_ASSERT(sizeof(DEBUG_ENGINE_DISPLAY_TYPE_COMMAND_OPTIONS) == sizeof(ULONG));

typedef
_Check_return_
_Success_(return != 0)
BOOL
(DEBUG_ENGINE_DISPLAY_TYPE)(
    _In_ PDEBUG_ENGINE_OUTPUT Output,
    _In_ DEBUG_ENGINE_OUTPUT_FLAGS OutputFlags,
    _In_ DEBUG_ENGINE_DISPLAY_TYPE_COMMAND_OPTIONS CommandOptions,
    _In_ PUNICODE_STRING SymbolName
    );
typedef DEBUG_ENGINE_DISPLAY_TYPE *PDEBUG_ENGINE_DISPLAY_TYPE;

//
// DEBUG_ENGINE_SESSION-related function typedefs, unions and structures.
//
// This structure is the main way to interface with the DebugEngine
// functionality exposed by this component.
//

typedef
VOID
(DESTROY_DEBUG_ENGINE_SESSION)(
    _Pre_notnull_ _Post_null_ struct _DEBUG_ENGINE_SESSION **Session
    );
typedef DESTROY_DEBUG_ENGINE_SESSION   *PDESTROY_DEBUG_ENGINE_SESSION;
typedef DESTROY_DEBUG_ENGINE_SESSION **PPDESTROY_DEBUG_ENGINE_SESSION;
DEBUG_ENGINE_API DESTROY_DEBUG_ENGINE_SESSION DestroyDebugEngineSession;

typedef union _DEBUG_ENGINE_SESSION_FLAGS {
    LONG AsLong;
    ULONG AsULong;
    struct {
        ULONG Unused:1;
    };
} DEBUG_ENGINE_SESSION_FLAGS;
C_ASSERT(sizeof(DEBUG_ENGINE_SESSION_FLAGS) == sizeof(ULONG));
typedef DEBUG_ENGINE_SESSION_FLAGS *PDEBUG_ENGINE_SESSION_FLAGS;


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
    // Destructor.
    //

    PDESTROY_DEBUG_ENGINE_SESSION Destroy;

    //
    // Initialize a DEBUG_ENGINE_OUTPUT structure prior to executing a command.
    //

    PINITIALIZE_DEBUG_ENGINE_OUTPUT InitializeDebugEngineOutput;

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

    //
    // StringTable-specific functions.
    //

    PCREATE_STRING_TABLE_FROM_DELIMITED_STRING
        CreateStringTableFromDelimitedString;

    PALLOCATOR StringTableAllocator;
    PALLOCATOR StringArrayAllocator;

    PSTRING_TABLE ExamineSymbolsPrefixStringTable;
    PSTRING_TABLE ExamineSymbolsBasicTypeStringTable;

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
    _In_opt_ HMODULE StringTableModule,
    _In_opt_ PALLOCATOR StringArrayAllocator,
    _In_opt_ PALLOCATOR StringTableAllocator,
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

    StringTableDllPath - Supplies a pointer to a UNICODE_STRING that contains
        the fully-qualified path of the StringTable DLL to load.

    StringArrayAllocator - Supplies a pointer to an allocator to use for string
        array allocations.

    StringTableAllocator - Supplies a pointer to an allocator to use for string
        table allocations.

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
    HMODULE StringTableModule;
    PINTIALIZE_DEBUG_ENGINE_SESSION InitializeDebugEngineSession;

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
        OutputDebugStringA("Failed to load DebugEngine.dll.\n");
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

    //
    // Attempt to load the StringTable module.
    //

    StringTableModule = LoadLibraryW(StringTableDllPath->Buffer);

    //
    // Call the initialization function with the same arguments we were passed.
    //

    Success = InitializeDebugEngineSession(Rtl,
                                           Allocator,
                                           InitFlags,
                                           StringTableModule,
                                           StringArrayAllocator,
                                           StringTableAllocator,
                                           SessionPointer);

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

        //
        // This will also clear the caller's session pointer.
        //

        (*SessionPointer)->Destroy(SessionPointer);
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
#pragma component(browser, on)

#ifdef __cplusplus
} // extern "C"
#endif

// vim:set ts=8 sw=4 sts=4 tw=80 expandtab                                     :
