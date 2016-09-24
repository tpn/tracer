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
// PythonAllocators
//

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
PYTHON_EX_API REGISTER_FUNCTION RegisterFunction;

////////////////////////////////////////////////////////////////////////////////
// Inline functions.
////////////////////////////////////////////////////////////////////////////////

FORCEINLINE
BOOL
IsQualifiedPath(_In_ PSTRING Path)
{
    BOOL Qualified;
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
