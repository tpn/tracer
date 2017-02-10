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


_Use_decl_annotations_
BOOL
AllocateString(
    PPYTHON  Python,
    PPSTRING StringPointer
    )
{
    PSTRING String;

    if (!ARGUMENT_PRESENT(Python)) {
        return FALSE;
    }

    if (!ARGUMENT_PRESENT(StringPointer)) {
        return FALSE;
    }

    String = (PSTRING)ALLOCATE(StringBuffer, sizeof(STRING));

    if (!String) {
        return FALSE;
    }

    String->Length = 0;
    String->MaximumLength = 0;
    String->Buffer = NULL;

    *StringPointer = String;

    return TRUE;
}

_Use_decl_annotations_
BOOL
AllocateStringBuffer(
    PPYTHON  Python,
    USHORT   StringBufferSizeInBytes,
    PSTRING  String
    )
{
    ULONG AlignedSize;
    PVOID Buffer;

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

_Use_decl_annotations_
BOOL
AllocateStringAndBuffer(
    PPYTHON  Python,
    USHORT   StringBufferSizeInBytes,
    PPSTRING StringPointer
    )
{
    ULONG AllocSize;
    ULONG AlignedSize;
    PSTRING String;

    if (StringBufferSizeInBytes == 0) {
        return FALSE;
    }

    AlignedSize = ALIGN_UP(StringBufferSizeInBytes, sizeof(ULONG_PTR));

    AllocSize = AlignedSize + sizeof(STRING);

    String = ALLOCATE(StringBuffer, AllocSize);
    if (!String) {
        return FALSE;
    }

    String->Length = 0;
    String->MaximumLength = (USHORT)AlignedSize;
    String->Buffer = (PCHAR)RtlOffsetToPointer(String, sizeof(STRING));

    *StringPointer = String;

    return TRUE;
}

_Use_decl_annotations_
BOOL
AllocateHashedString(
    PPYTHON  Python,
    PPPYTHON_HASHED_STRING HashedStringPointer
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
        ALLOCATE(StringBuffer, sizeof(PYTHON_HASHED_STRING))
    );

    if (!HashedString) {
        return FALSE;
    }

    SecureZeroMemory(HashedString, sizeof(*HashedString));

    *HashedStringPointer = HashedString;

    return TRUE;
}

_Use_decl_annotations_
BOOL
AllocateHashedStringAndBuffer(
    PPYTHON                Python,
    USHORT                 StringBufferSizeInBytes,
    PPPYTHON_HASHED_STRING HashedStringPointer
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
        ALLOCATE(StringBuffer, sizeof(*HashedString))
    );

    if (!HashedString) {
        return FALSE;
    }

    SecureZeroMemory(HashedString, sizeof(*HashedString));

    Buffer = ALLOCATE(StringBuffer, StringBufferSizeInBytes);

    if (!Buffer) {
        FREE(StringBuffer, HashedString);
        return FALSE;
    }

    HashedString->MaximumLength = StringBufferSizeInBytes;
    HashedString->Buffer = (PCHAR)Buffer;

    *HashedStringPointer = HashedString;

    return TRUE;
}

_Use_decl_annotations_
BOOL
FinalizeHashedString(
    PPYTHON                 Python,
    PPYTHON_HASHED_STRING   HashedString,
    PPPYTHON_HASHED_STRING  ExistingHashedStringPointer
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

_Use_decl_annotations_
BOOL
AllocateBuffer(
    PPYTHON Python,
    ULONG   SizeInBytes,
    PPVOID  BufferPointer
    )
{
    PVOID Buffer;

    if (!ARGUMENT_PRESENT(Python)) {
        return FALSE;
    }

    if (!ARGUMENT_PRESENT(BufferPointer)) {
        return FALSE;
    }

    Buffer = ALLOCATE(StringBuffer, SizeInBytes);

    if (!Buffer) {
        return FALSE;
    }

    *BufferPointer = Buffer;

    return TRUE;
}

_Use_decl_annotations_
BOOL
AllocatePythonFunctionTable(
    PPYTHON                 Python,
    PPPYTHON_FUNCTION_TABLE FunctionTablePointer
    )
{
    PVOID Buffer;

    if (!ARGUMENT_PRESENT(Python)) {
        return FALSE;
    }

    if (!ARGUMENT_PRESENT(FunctionTablePointer)) {
        return FALSE;
    }

    Buffer = ALLOCATE(PythonFunctionTable, sizeof(PYTHON_FUNCTION_TABLE));

    if (!Buffer) {
        return FALSE;
    }

    *FunctionTablePointer = (PPYTHON_FUNCTION_TABLE)Buffer;

    return TRUE;
}

_Use_decl_annotations_
BOOL
AllocatePythonPathTable(
    PPYTHON Python,
    PPPYTHON_PATH_TABLE PathTablePointer
    )
{
    PVOID Buffer;

    if (!ARGUMENT_PRESENT(Python)) {
        return FALSE;
    }

    if (!ARGUMENT_PRESENT(PathTablePointer)) {
        return FALSE;
    }

    Buffer = ALLOCATE(PythonPathTable, sizeof(PYTHON_PATH_TABLE));

    if (!Buffer) {
        return FALSE;
    }

    *PathTablePointer = (PPYTHON_PATH_TABLE)Buffer;

    return TRUE;

}

_Use_decl_annotations_
BOOL
AllocatePythonPathTableEntry(
    PPYTHON Python,
    PPPYTHON_PATH_TABLE_ENTRY PathTableEntryPointer
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

    Buffer = ALLOCATE(PythonPathTableEntry, sizeof(PYTHON_PATH_TABLE_ENTRY));

    if (!Buffer) {
        return FALSE;
    }

    SecureZeroMemory(Buffer, sizeof(PYTHON_PATH_TABLE_ENTRY));

    *PathTableEntryPointer = (PPYTHON_PATH_TABLE_ENTRY)Buffer;

    return TRUE;
}

_Use_decl_annotations_
BOOL
AllocatePythonPathTableEntryAndString(
    PPYTHON                   Python,
    PPPYTHON_PATH_TABLE_ENTRY PathTableEntryPointer,
    PPSTRING                  StringPointer
    )
{
    return FALSE;
}

_Use_decl_annotations_
BOOL
AllocatePythonPathTableEntryAndStringWithBuffer(
    PPYTHON                   Python,
    ULONG                     StringBufferSize,
    PPPYTHON_PATH_TABLE_ENTRY PathTableEntryPointer,
    PPSTRING                  StringPointer
    )
{
    return FALSE;
}

_Use_decl_annotations_
BOOL
AllocatePythonPathTableEntryAndHashedString(
    PPYTHON                   Python,
    PPPYTHON_PATH_TABLE_ENTRY PathTableEntryPointer,
    PPPYTHON_HASHED_STRING    HashedStringPointer
    )
{
    return FALSE;
}

_Use_decl_annotations_
BOOL
AllocatePythonPathTableEntryAndHashedStringWithBuffer(
    PPYTHON                   Python,
    ULONG                     StringBufferSize,
    PPPYTHON_PATH_TABLE_ENTRY PathTableEntryPointer,
    PPPYTHON_HASHED_STRING    HashedStringPointer
    )
{
    return FALSE;
}

_Use_decl_annotations_
BOOL
AllocatePythonModuleTable(
    PPYTHON Python,
    PPPYTHON_MODULE_TABLE ModuleTablePointer
    )
{
    PVOID Buffer;

    if (!ARGUMENT_PRESENT(Python)) {
        return FALSE;
    }

    if (!ARGUMENT_PRESENT(ModuleTablePointer)) {
        return FALSE;
    }

    Buffer = ALLOCATE(PythonModuleTable, sizeof(PYTHON_MODULE_TABLE));

    if (!Buffer) {
        return FALSE;
    }

    *ModuleTablePointer = (PPYTHON_MODULE_TABLE)Buffer;

    return TRUE;

}

_Use_decl_annotations_
BOOL
AllocatePythonModuleTableEntry(
    PPYTHON Python,
    PPPYTHON_MODULE_TABLE_ENTRY ModuleTableEntryPointer
    )
{
    PVOID Buffer;

    if (!ARGUMENT_PRESENT(Python)) {
        return FALSE;
    }

    if (!ARGUMENT_PRESENT(ModuleTableEntryPointer)) {
        return FALSE;
    }

    //
    // Clear the caller's pointer.
    //

    *ModuleTableEntryPointer = NULL;

    Buffer = ALLOCATE(PythonModuleTableEntry,
                      sizeof(PYTHON_MODULE_TABLE_ENTRY));

    if (!Buffer) {
        return FALSE;
    }

    SecureZeroMemory(Buffer, sizeof(PYTHON_MODULE_TABLE_ENTRY));

    *ModuleTableEntryPointer = (PPYTHON_MODULE_TABLE_ENTRY)Buffer;

    return TRUE;
}


_Use_decl_annotations_
VOID
FreePythonPathTableEntry(
    PPYTHON                  Python,
    PPYTHON_PATH_TABLE_ENTRY PathTableEntry
    )
{

}

_Use_decl_annotations_
VOID
FreeBuffer(
    PPYTHON Python,
    PVOID   Buffer
    )
{

}

_Use_decl_annotations_
VOID
FreeString(
    PPYTHON Python,
    PSTRING String
    )
{


}

_Use_decl_annotations_
VOID
FreeStringAndBuffer(
    PPYTHON Python,
    PSTRING String
    )
{


}

_Use_decl_annotations_
VOID
FreeStringBuffer(
    PPYTHON Python,
    PSTRING String
    )
{


}

_Use_decl_annotations_
VOID
FreeStringBufferDirect(
    PPYTHON Python,
    PVOID   Buffer
    )
{


}


_Use_decl_annotations_
SetPythonAllocators(
    PPYTHON             Python,
    PPYTHON_ALLOCATORS  Allocators
    )
{
    PRTL Rtl;
    ULONG SizeOfAllocators = sizeof(*Allocators);
    ULONG NumberOfAllocators = (
        (SizeOfAllocators - (sizeof(ULONG) * 2)) /
        sizeof(ALLOCATOR)
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

#ifdef __cplusplus
}; // extern "C"
#endif

// vim:set ts=8 sw=4 sts=4 tw=80 expandtab                                     :
