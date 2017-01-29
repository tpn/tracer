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

#endif

//
// DEBUG_ENGINE_SYMBOL is used to capture information about a symbol returned
// from the `x` (examine symbols) debugger command.
//

typedef union _DEBUG_ENGINE_SYMBOL_FLAGS {
    LONG AsLong;
    ULONG AsULong;
    struct _Struct_size_bytes_(sizeof(ULONG)) {
        ULONG WideCharacter:1;
    };
} DEBUG_ENGINE_SYMBOL_FLAGS;
C_ASSERT(sizeof(DEBUG_ENGINE_SYMBOL_FLAGS) == sizeof(ULONG));
typedef DEBUG_ENGINE_SYMBOL_FLAGS *PDEBUG_ENGINE_SYMBOL_FLAGS;

typedef struct _Struct_size_bytes_(SizeOfStruct) _DEBUG_ENGINE_SYMBOL {

    //
    // Size of structure, in bytes.
    //

    _Field_range_(== , sizeof(struct _DEBUG_ENGINE_SYMBOL)) ULONG SizeOfStruct;

    //
    // Flags.
    //

    DEBUG_ENGINE_SYMBOL_FLAGS Flags;

    //
    // Raw text returned by DbgEng.
    //

    union {
        STRING RawText;
        UNICODE_STRING RawTextWide;
    };

} DEBUG_ENGINE_SYMBOL;
typedef DEBUG_ENGINE_SYMBOL *PDEBUG_ENGINE_SYMBOL;

typedef
BOOL
(CALLBACK DEBUG_ENGINE_ENUM_SYMBOLS_CALLBACK)(
    _In_ PDEBUG_ENGINE_SYMBOL Symbol,
    _In_opt_ PVOID Context
    );
typedef DEBUG_ENGINE_ENUM_SYMBOLS_CALLBACK *PDEBUG_ENGINE_ENUM_SYMBOLS_CALLBACK;

typedef union _DEBUG_ENGINE_ENUM_SYMBOLS_FLAGS {
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

        //
        // Provide wide character output.
        //

        ULONG WideCharacter:1;

    };

} DEBUG_ENGINE_ENUM_SYMBOLS_FLAGS;
C_ASSERT(sizeof(DEBUG_ENGINE_ENUM_SYMBOLS_FLAGS) == sizeof(ULONG));

typedef
_Success_(return != 0)
BOOL
(DEBUG_ENGINE_ENUM_SYMBOLS)(
    _In_ struct _DEBUG_ENGINE_SESSION *DebugEngineSession,
    _In_ PVOID Context,
    _In_ PALLOCATOR Allocator,
    _In_ PDEBUG_ENGINE_ENUM_SYMBOLS_CALLBACK Callback,
    _In_ DEBUG_ENGINE_ENUM_SYMBOLS_FLAGS Flags,
    _In_ PRTL_PATH ModulePath
    );
typedef DEBUG_ENGINE_ENUM_SYMBOLS *PDEBUG_ENGINE_ENUM_SYMBOLS;

//
// DisassembleFunction-related structures and functions.
//

typedef union _DEBUG_ENGINE_DISASSEMBLED_FUNCTION_FLAGS {
    LONG AsLong;
    ULONG AsULong;
    struct _Struct_size_bytes_(sizeof(ULONG)) {

        //
        // Indicates the text strings are in wide character format.
        //

        ULONG WideCharacter:1;
    };
} DEBUG_ENGINE_DISASSEMBLED_FUNCTION_FLAGS;

typedef
struct _Struct_size_bytes_(SizeOfStruct)
_DEBUG_ENGINE_DISASSEMBLED_FUNCTION {

    //
    // Size of structure, in bytes.
    //

    _Field_range_(== , sizeof(struct _DEBUG_ENGINE_DISASSEMBLED_FUNCTION))
        ULONG SizeOfStruct;

    //
    // Flags.
    //

    DEBUG_ENGINE_DISASSEMBLED_FUNCTION_FLAGS Flags;

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

} DEBUG_ENGINE_DISASSEMBLED_FUNCTION;
typedef DEBUG_ENGINE_DISASSEMBLED_FUNCTION
      *PDEBUG_ENGINE_DISASSEMBLED_FUNCTION;
typedef DEBUG_ENGINE_DISASSEMBLED_FUNCTION
    **PPDEBUG_ENGINE_DISASSEMBLED_FUNCTION;

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
} DEBUG_ENGINE_DISASSEMBLE_FUNCTION_FLAGS;
C_ASSERT(sizeof(DEBUG_ENGINE_DISASSEMBLE_FUNCTION_FLAGS) == sizeof(ULONG));

typedef
_Success_(return != 0)
BOOL
(DEBUG_ENGINE_DISASSEMBLE_FUNCTION)(
    _In_ struct _DEBUG_ENGINE_SESSION *DebugEngineSession,
    _In_ PALLOCATOR Allocator,
    _In_ DEBUG_ENGINE_DISASSEMBLE_FUNCTION_FLAGS Flags,
    _In_ PRTL_PATH ModulePath,
    _In_opt_ PSTRING FunctionName,
    _In_opt_ PUNICODE_STRING FunctionNameWide,
    _Outptr_result_nullonfailure_ PPDEBUG_ENGINE_DISASSEMBLED_FUNCTION
        DisassembledFunctionPointer
    );
typedef DEBUG_ENGINE_DISASSEMBLE_FUNCTION
      *PDEBUG_ENGINE_DISASSEMBLE_FUNCTION;

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
    // Enumerate symbols.
    //

    PDEBUG_ENGINE_ENUM_SYMBOLS EnumSymbols;

    //
    // Disassemble function.
    //

    PDEBUG_ENGINE_DISASSEMBLE_FUNCTION DisassembleFunction;

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
    // Call the initialization function with the same arguments we were passed.
    //

    Success = InitializeDebugEngineSession(Rtl,
                                           Allocator,
                                           InitFlags,
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
