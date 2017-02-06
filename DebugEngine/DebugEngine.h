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
    LONG AsLong;
    ULONG AsULong;
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
    ULONG NumberOfParsedLines;

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
    // Pointer to the custom structure.  Examined symbols will be an array based
    // at the indicated address; displayed types and unassembled functions will
    // be single records.
    //

    ULONG NumberOfCustomStructures;
    union {
        PVOID CustomStructure;
        PVOID FirstCustomStructure;
        struct _DEBUG_ENGINE_DISPLAYED_TYPE *DisplayedType;
        struct _DEBUG_ENGINE_EXAMINED_SYMBOL *ExaminedSymbols;
        struct _DEBUG_ENGINE_UNASSEMBLED_FUNCTION *UnassembledFunction;
    };

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
    PrivateFunctionScope = 0,
    PrivateGlobalScope,
    PrivateInlineScope,
    PublicFunctionScope,
    PublicGlobalScope,
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
    IntegerType,
    Integer64Type,

    UnsignedCharType,
    UnsignedWideCharType,
    UnsignedShortType,
    UnsignedLongType,
    UnsignedIntegerType,
    UnsignedInteger64Type,

    UnionType,
    StructType,

    //
    // Next 16 types captured by BasicTypeStringTable2.
    //

    CLRType,
    BoolType,
    VoidType,
    ClassType,
    FloatType,
    DoubleType,
    SALExecutionContextType,
    ENativeStartupStateType,

    //
    // Any enumeration value >= InvalidType is invalid.  Make sure this always
    // comes last in the enum layout.
    //

    InvalidType

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
// DEBUG_ENGINE_EXAMINED_SYMBOL-related structures and functions.
//
// This structure represents a parsed representation of a line of output from
// the debugger engine's execution of the examine symbols command.
//

typedef union _DEBUG_ENGINE_EXAMINED_SYMBOL_FLAGS {
    LONG AsLong;
    ULONG AsULong;
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

    };
} DEBUG_ENGINE_EXAMINED_SYMBOL_FLAGS;

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
    // The following strings are wired up to point to the relevant part of the
    // line output.
    //

    struct {

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
        // If this is a function, this will be set to the start of the
        // parameters.
        //

        union {
            STRING Parameters;
        };

    } String;


    //
    // Variable parts of the structure depending on the flags/type.
    //

    union {

        struct {

            USHORT NumberOfArguments;

            LIST_ENTRY FunctionArgumentsListHead;

        } Function;

    };

} DEBUG_ENGINE_EXAMINED_SYMBOL;
typedef DEBUG_ENGINE_EXAMINED_SYMBOL *PDEBUG_ENGINE_EXAMINED_SYMBOL;
typedef DEBUG_ENGINE_EXAMINED_SYMBOL **PPDEBUG_ENGINE_EXAMINED_SYMBOL;

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
// SettingsMeta-related structures and functions.
//

typedef union _DEBUG_ENGINE_SETTINGS_META_COMMAND_OPTIONS {
    LONG AsLong;
    ULONG AsULong;
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

typedef struct _Struct_size_bytes_(SizeOfStruct) DEBUG_ENGINE_DYNAMIC_COMMAND {

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

    USHORT NumberOfBasicTypeStringTables;
    USHORT Padding[3];

    PSTRING_TABLE ExamineSymbolsBasicTypeStringTable1;
    PSTRING_TABLE ExamineSymbolsBasicTypeStringTable2;

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
    _In_ struct _TRACER_CONFIG *TracerConfig,
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
                                           TracerConfig,
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
