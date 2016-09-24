/*++

Copyright (c) 2016 Trent Nelson <trent@trent.me>

Module Name:

    PythonAllocators.c

Abstract:

    This module implements allocators for the Python component.  Routines are
    provided to allocate and free various combinations of structure types using
    the appropriate backing allocator from the PYTHON structure.

    This module was designed as a thin wrapper around the TraceStore component.

--*/

#include "stdafx.h"

BOOL
SetPythonAllocators(
    _In_    PPYTHON             Python,
    _In_    PPYTHON_ALLOCATORS  Allocators
    )
{
    PRTL Rtl;
    ULONG SizeOfAllocators = sizeof(*Allocators);
    ULONG NumberOfAllocators = (
        (SizeOfAllocators - (sizeof(ULONG) * 2)) /
        sizeof(PYTHON_ALLOCATOR)
    );

    if (!ARGUMENT_PRESENT(Python)) {
        return FALSE;
    }

    if (!ARGUMENT_PRESENT(Allocators)) {
        return FALSE;
    }

    if (Allocators->SizeInBytes != SizeOfAllocators) {
        return FALSE;
    }

    if (Allocators->NumberOfAllocators != NumberOfAllocators) {
        return FALSE;
    }

    Rtl = Python->Rtl;

    Rtl->RtlCopyMemory(&Python->Allocators,
                       Allocators,
                       SizeOfAllocators);

    return TRUE;
}

BOOL
AllocateString(
    _In_  PPYTHON  Python,
    _Out_ PPSTRING StringPointer
    )
{
    PSTRING String;

    if (!ARGUMENT_PRESENT(Python)) {
        return FALSE;
    }

    if (!ARGUMENT_PRESENT(StringPointer)) {
        return FALSE;
    }

    String = (PSTRING)ALLOCATE(String, sizeof(STRING));

    if (!String) {
        return FALSE;
    }

    String->Length = 0;
    String->MaximumLength = 0;
    String->Buffer = NULL;

    *StringPointer = String;

    return TRUE;
}

BOOL
AllocateStringBuffer(
    _In_  PPYTHON  Python,
    _In_  USHORT   StringBufferSizeInBytes,
    _In_  PSTRING  String
    )
{
    PVOID Buffer;
    ULONG AlignedSize;

    if (!ARGUMENT_PRESENT(Python)) {
        return FALSE;
    }

    if (!ARGUMENT_PRESENT(String)) {
        return FALSE;
    }

    if (StringBufferSizeInBytes == 0) {
        return FALSE;
    }

    AlignedSize = ALIGN_UP(StringBufferSizeInBytes, sizeof(ULONG_PTR));

    Buffer = ALLOCATE(StringBuffer, AlignedSize);
    if (!Buffer) {
        return FALSE;
    }

    String->Length = 0;
    String->MaximumLength = (USHORT)AlignedSize;
    String->Buffer = (PCHAR)Buffer;

    return TRUE;
}

BOOL
AllocateStringAndBuffer(
    _In_  PPYTHON  Python,
    _In_  USHORT   StringBufferSizeInBytes,
    _Out_ PPSTRING StringPointer
    )
{
    PSTRING String;

    if (!AllocateString(Python, &String)) {
        return FALSE;
    }

    if (!AllocateStringBuffer(Python, StringBufferSizeInBytes, String)) {
        FreeString(Python, String);
        return FALSE;
    }

    *StringPointer = String;

    return TRUE;
}


VOID
FreeString(
    _In_        PPYTHON Python,
    _In_opt_    PSTRING String
    )
{


}

VOID
FreeStringAndBuffer(
    _In_        PPYTHON Python,
    _In_opt_    PSTRING String
    )
{


}

VOID
FreeStringBuffer(
    _In_        PPYTHON Python,
    _In_opt_    PSTRING String
    )
{


}

VOID
FreeStringBufferDirect(
    _In_        PPYTHON Python,
    _In_opt_    PVOID   Buffer
    )
{


}


BOOL
AllocateHashedString(
    _In_  PPYTHON  Python,
    _Out_ PPPYTHON_HASHED_STRING HashedStringPointer
    )
{
    PPYTHON_HASHED_STRING HashedString;

    if (!ARGUMENT_PRESENT(Python)) {
        return FALSE;
    }

    if (!ARGUMENT_PRESENT(HashedStringPointer)) {
        return FALSE;
    }

    HashedString = (PPYTHON_HASHED_STRING)(
        ALLOCATE(HashedString, sizeof(PYTHON_HASHED_STRING))
    );

    if (!HashedString) {
        return FALSE;
    }

    SecureZeroMemory(HashedString, sizeof(*HashedString));

    *HashedStringPointer = HashedString;

    return TRUE;
}

BOOL
AllocateHashedStringAndBuffer(
    _In_  PPYTHON                Python,
    _In_  USHORT                 StringBufferSizeInBytes,
    _Out_ PPPYTHON_HASHED_STRING HashedStringPointer
    )
{
    PVOID Buffer;
    PPYTHON_HASHED_STRING HashedString;

    if (!ARGUMENT_PRESENT(Python)) {
        return FALSE;
    }

    if (!ARGUMENT_PRESENT(HashedStringPointer)) {
        return FALSE;
    }

    HashedString = (PPYTHON_HASHED_STRING)(
        ALLOCATE(HashedString, sizeof(*HashedString))
    );

    if (!HashedString) {
        return FALSE;
    }

    SecureZeroMemory(HashedString, sizeof(*HashedString));

    Buffer = ALLOCATE(Buffer, StringBufferSizeInBytes);

    if (!Buffer) {
        FREE(HashedString, HashedString);
        return FALSE;
    }

    HashedString->MaximumLength = StringBufferSizeInBytes;
    HashedString->Buffer = (PCHAR)Buffer;

    *HashedStringPointer = HashedString;

    return TRUE;
}

BOOL
FinalizeHashedString(
    _In_        PPYTHON                 Python,
    _Inout_     PPYTHON_HASHED_STRING   HashedString,
    _Out_opt_   PPPYTHON_HASHED_STRING  ExistingHashedStringPointer
    )
{
    if (!ARGUMENT_PRESENT(Python)) {
        return FALSE;
    }

    if (!ARGUMENT_PRESENT(HashedString)) {
        return FALSE;
    }

    HashedString->Atom = HashAnsiStringToAtom(&HashedString->String);
    HashString(Python, &HashedString->String);

    return TRUE;
}

BOOL
AllocateBuffer(
    _In_  PPYTHON Python,
    _In_  ULONG   SizeInBytes,
    _Out_ PPVOID  BufferPointer
    )
{
    PVOID Buffer;

    if (!ARGUMENT_PRESENT(Python)) {
        return FALSE;
    }

    if (!ARGUMENT_PRESENT(BufferPointer)) {
        return FALSE;
    }

    Buffer = ALLOCATE(Buffer, SizeInBytes);

    if (!Buffer) {
        return FALSE;
    }

    *BufferPointer = Buffer;

    return TRUE;
}

VOID
FreeBuffer(
    _In_        PPYTHON Python,
    _In_opt_    PVOID   Buffer
    )
{

}

BOOL
AllocatePythonFunctionTable(
    _In_    PPYTHON                 Python,
    _Out_   PPPYTHON_FUNCTION_TABLE FunctionTablePointer
    )
{
    PVOID Buffer;

    if (!ARGUMENT_PRESENT(Python)) {
        return FALSE;
    }

    if (!ARGUMENT_PRESENT(FunctionTablePointer)) {
        return FALSE;
    }

    Buffer = ALLOCATE(FunctionTable, sizeof(PYTHON_FUNCTION_TABLE));

    if (!Buffer) {
        return FALSE;
    }

    *FunctionTablePointer = (PPYTHON_FUNCTION_TABLE)Buffer;

    return TRUE;
}

BOOL
AllocatePythonPathTable(
    _In_ PPYTHON Python,
    _Out_ PPPYTHON_PATH_TABLE PathTablePointer
    )
{
    PVOID Buffer;

    if (!ARGUMENT_PRESENT(Python)) {
        return FALSE;
    }

    if (!ARGUMENT_PRESENT(PathTablePointer)) {
        return FALSE;
    }

    Buffer = ALLOCATE(PathTable, sizeof(PYTHON_PATH_TABLE));

    if (!Buffer) {
        return FALSE;
    }

    *PathTablePointer = (PPYTHON_PATH_TABLE)Buffer;

    return TRUE;

}

BOOL
AllocatePythonPathTableEntry(
    _In_  PPYTHON Python,
    _Out_ PPPYTHON_PATH_TABLE_ENTRY PathTableEntryPointer
    )
{
    PVOID Buffer;

    if (!ARGUMENT_PRESENT(Python)) {
        return FALSE;
    }

    if (!ARGUMENT_PRESENT(PathTableEntryPointer)) {
        return FALSE;
    }

    //
    // Clear the caller's pointer.
    //

    *PathTableEntryPointer = NULL;

    Buffer = ALLOCATE(PathTableEntry, sizeof(PYTHON_PATH_TABLE_ENTRY));

    if (!Buffer) {
        return FALSE;
    }

    SecureZeroMemory(Buffer, sizeof(PYTHON_PATH_TABLE_ENTRY));

    *PathTableEntryPointer = (PPYTHON_PATH_TABLE_ENTRY)Buffer;

    return TRUE;
}

VOID
FreePythonPathTableEntry(
    _In_        PPYTHON                  Python,
    _In_opt_    PPYTHON_PATH_TABLE_ENTRY PathTableEntry
    )
{

}

PYTHON_API
BOOL
AllocatePythonPathTableEntryAndString(
    _In_  PPYTHON                   Python,
    _Out_ PPPYTHON_PATH_TABLE_ENTRY PathTableEntryPointer,
    _Out_ PPSTRING                  StringPointer
    )
{
    return FALSE;
}

BOOL
AllocatePythonPathTableEntryAndStringWithBuffer(
    _In_  PPYTHON                   Python,
    _In_  ULONG                     StringBufferSize,
    _Out_ PPPYTHON_PATH_TABLE_ENTRY PathTableEntryPointer,
    _Out_ PPSTRING                  StringPointer
    )
{
    return FALSE;
}

PYTHON_API
BOOL
AllocatePythonPathTableEntryAndHashedString(
    _In_  PPYTHON                   Python,
    _Out_ PPPYTHON_PATH_TABLE_ENTRY PathTableEntryPointer,
    _Out_ PPPYTHON_HASHED_STRING    HashedStringPointer
    )
{
    return FALSE;
}

PYTHON_API
BOOL
AllocatePythonPathTableEntryAndHashedStringWithBuffer(
    _In_  PPYTHON                   Python,
    _In_  ULONG                     StringBufferSize,
    _Out_ PPPYTHON_PATH_TABLE_ENTRY PathTableEntryPointer,
    _Out_ PPPYTHON_HASHED_STRING    HashedStringPointer
    )
{
    return FALSE;
}

#ifdef __cplusplus
}; // extern "C"
#endif

// vim:set ts=8 sw=4 sts=4 tw=80 expandtab                                     :
