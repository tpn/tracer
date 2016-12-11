/*++

Copyright (c) 2016 Trent Nelson <trent@trent.me>

Module Name:

    PythonPrivate.h

Abstract:

    This is the private header file for the Python component.  It defines
    function typedefs and function declarations for all major (i.e. not local
    to the module) functions available for use by individual modules within
    this component.

--*/

#pragma once

#ifdef __cplusplus
extern "C" {
#endif

#include "stdafx.h"

////////////////////////////////////////////////////////////////////////////////
// Forward declarations.
////////////////////////////////////////////////////////////////////////////////

//
// Initializers
//

PYTHON_EX_API INITIALIZE_PYTHON InitializePython;
PYTHON_EX_API INITIALIZE_PYTHON_RUNTIME_TABLES InitializePythonRuntimeTables;

//
// PythonAllocators
//

PYTHON_EX_API ALLOCATE_STRING AllocateString;
PYTHON_EX_API ALLOCATE_BUFFER AllocateBuffer;
PYTHON_EX_API ALLOCATE_STRING_BUFFER AllocateStringBuffer;
PYTHON_EX_API ALLOCATE_STRING_AND_BUFFER AllocateStringAndBuffer;
PYTHON_EX_API ALLOCATE_HASHED_STRING AllocateHashedString;
PYTHON_EX_API ALLOCATE_HASHED_STRING_AND_BUFFER AllocateHashedStringAndBuffer;
PYTHON_EX_API FINALIZE_HASHED_STRING FinalizeHashedString;

PYTHON_EX_API ALLOCATE_PYTHON_PATH_TABLE AllocatePythonPathTable;
PYTHON_EX_API ALLOCATE_PYTHON_PATH_TABLE_ENTRY AllocatePythonPathTableEntry;
PYTHON_EX_API ALLOCATE_PYTHON_FUNCTION_TABLE AllocatePythonFunctionTable;
PYTHON_EX_API ALLOCATE_PYTHON_PATH_TABLE_ENTRY_AND_STRING \
    AllocatePythonPathTableEntryAndString;
PYTHON_EX_API ALLOCATE_PYTHON_PATH_TABLE_ENTRY_AND_STRING_WITH_BUFFER \
    AllocatePythonPathTableEntryAndStringWithBuffer;

PYTHON_EX_API ALLOCATE_PYTHON_MODULE_TABLE AllocatePythonModuleTable;
PYTHON_EX_API ALLOCATE_PYTHON_MODULE_TABLE_ENTRY AllocatePythonModuleTableEntry;

PYTHON_EX_API SET_PYTHON_ALLOCATORS SetPythonAllocators;

PYTHON_EX_API FREE_STRING FreeString;
PYTHON_EX_API FREE_BUFFER FreeBuffer;
PYTHON_EX_API FREE_STRING_BUFFER FreeStringBuffer;
PYTHON_EX_API FREE_STRING_AND_BUFFER FreeStringAndBuffer;
PYTHON_EX_API FREE_STRING_BUFFER_DIRECT FreeStringBufferDirect;

PYTHON_EX_API FREE_PYTHON_PATH_TABLE_ENTRY FreePythonPathTableEntry;

//
// PythonPathTableEntry
//

PYTHON_EX_API QUALIFY_PATH QualifyPath;
PYTHON_EX_API REGISTER_FILE RegisterFile;
PYTHON_EX_API REGISTER_DIRECTORY RegisterDirectory;
PYTHON_EX_API GET_PATH_ENTRY_FROM_FRAME GetPathEntryFromFrame;
PYTHON_EX_API GET_PATH_ENTRY_FOR_DIRECTORY GetPathEntryForDirectory;

//
// PythonLineNumbers
//

PYTHON_EX_API RESOLVE_LINE_NUMBERS ResolveLineNumbers;
PYTHON_EX_API RESOLVE_LINE_NUMBERS ResolveLineNumbersForPython2;
PYTHON_EX_API RESOLVE_LINE_NUMBERS ResolveLineNumbersForPython3;

//
// PythonFunction
//

PYTHON_EX_API GET_SELF GetSelf;
PYTHON_EX_API GET_CLASS_NAME_FROM_SELF GetClassNameFromSelf;
PYTHON_EX_API REGISTER_FRAME RegisterFrame;
PYTHON_EX_API REGISTER_FUNCTION RegisterPythonFunction;
PYTHON_EX_API REGISTER_FUNCTION RegisterPyCFunction;

////////////////////////////////////////////////////////////////////////////////
// Inline functions.
////////////////////////////////////////////////////////////////////////////////

FORCEINLINE
BOOL
IsQualifiedPath(_In_ PSTRING Path)
{
    BOOL Qualified = FALSE;
    USHORT Length = Path->Length;
    PSTR Buf = Path->Buffer;

    if (Length >= 3) {

        Qualified = (Buf[1] == ':' && Buf[2] == '\\');

    } else if (Length >= 2) {

        Qualified = (Buf[0] == '\\' && Buf[1] == '\\');

    }

    return Qualified;
}

FORCEINLINE
_Success_(return != 0)
BOOL
IsModuleDirectoryW(
    _In_ PRTL Rtl,
    _In_ PUNICODE_STRING Directory,
    _Out_ PBOOL IsModule
    )
{
    return Rtl->FilesExistW(Rtl,
                            Directory,
                            NumberOfInitPyFiles,
                            (PPUNICODE_STRING)InitPyFilesW,
                            IsModule,
                            NULL,
                            NULL);
}

FORCEINLINE
_Success_(return != 0)
BOOL
IsModuleDirectoryA(
    _In_ PRTL Rtl,
    _In_ PSTRING Directory,
    _Out_ PBOOL IsModule
    )
{
    return Rtl->FilesExistA(Rtl,
                            Directory,
                            NumberOfInitPyFiles,
                            (PPSTRING)InitPyFilesA,
                            IsModule,
                            NULL,
                            NULL);
}

FORCEINLINE
_Success_(return != 0)
BOOL
RegisterNonModuleDirectory(
    _In_ PPYTHON Python,
    _In_ PSTRING Directory,
    _Outptr_result_nullonfailure_ PPPYTHON_PATH_TABLE_ENTRY PathEntryPointer
    )
{
    return RegisterDirectory(Python,
                             Directory,
                             NULL,
                             NULL,
                             PathEntryPointer,
                             TRUE);
}

FORCEINLINE
_Success_(return != 0)
BOOL
RegisterModuleDirectory(
    _In_      PPYTHON Python,
    _In_      PSTRING Directory,
    _In_      PSTRING DirectoryName,
    _In_      PPYTHON_PATH_TABLE_ENTRY AncestorEntry,
    _Outptr_result_nullonfailure_ PPPYTHON_PATH_TABLE_ENTRY PathEntryPointer
    )
{
    return RegisterDirectory(Python,
                             Directory,
                             DirectoryName,
                             AncestorEntry,
                             PathEntryPointer,
                             FALSE);
}

#ifdef __cplusplus
}; // extern "C"
#endif

// vim:set ts=8 sw=4 sts=4 tw=80 expandtab                                     :
