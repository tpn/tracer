/*++

Copyright (c) 2016 Trent Nelson <trent@trent.me>

Module Name:

    Python.c

Abstract:

    This is the main module for the Python component.

--*/

#pragma once

#ifdef __cplusplus
extern "C" {
#endif

#include "stdafx.h"

#pragma intrinsic(strlen)

_Check_return_
BOOL
LoadPythonData(
    _In_    HMODULE     PythonModule,
    _Out_   PPYTHONDATA PythonData
    )
{
    PPY_HASH_SECRET PyHashSecret;

    if (!PythonModule) {
        return FALSE;
    }

    if (!PythonData) {
        return FALSE;
    }

#define TRY_RESOLVE_TYPE(Name) (               \
    PythonData->##Name.Type = (PPYTYPEOBJECT)( \
        GetProcAddress(                        \
            PythonModule,                      \
            #Name "_Type"                      \
        )                                      \
    )                                          \
)

#define RESOLVE_TYPE(Name)                                          \
    if (!TRY_RESOLVE_TYPE(Name)) {                                  \
        OutputDebugStringA("Failed to resolve Python!" #Name "\n"); \
        return FALSE;                                               \
    }

    if (!TRY_RESOLVE_TYPE(PyString)) {
        RESOLVE_TYPE(PyBytes);
    }

    RESOLVE_TYPE(PyCode);
    RESOLVE_TYPE(PyDict);
    RESOLVE_TYPE(PyTuple);
    RESOLVE_TYPE(PyType);
    RESOLVE_TYPE(PyFunction);
    RESOLVE_TYPE(PyUnicode);
    RESOLVE_TYPE(PyCFunction);
    RESOLVE_TYPE(PyModule);
    TRY_RESOLVE_TYPE(PyInstance);

#define TRY_RESOLVE_STRUCT(Name) (                   \
    PythonData->##Name##Struct.Object = (PPYOBJECT)( \
        GetProcAddress(                              \
            PythonModule,                            \
            #Name "Struct"                           \
        )                                            \
    )                                                \
)

#define RESOLVE_STRUCT(Name)                                        \
    if (!TRY_RESOLVE_STRUCT(Name)) {                                \
        OutputDebugStringA("Failed to resolve Python!" #Name "\n"); \
        return FALSE;                                               \
    }

    RESOLVE_STRUCT(_Py_None);
    RESOLVE_STRUCT(_Py_True);
    RESOLVE_STRUCT(_Py_Zero);
    TRY_RESOLVE_STRUCT(_Py_False);

    //
    // The hash secret is a little fiddly.
    //

    PyHashSecret = (PPY_HASH_SECRET)(
        GetProcAddress(
            PythonModule,
            "_Py_HashSecret"
        )
    );
    if (PyHashSecret) {
        PythonData->_Py_HashSecret.Prefix = PyHashSecret->Prefix;
        PythonData->_Py_HashSecret.Suffix = PyHashSecret->Suffix;
    } else {
        PythonData->_Py_HashSecret.Prefix = 0;
        PythonData->_Py_HashSecret.Suffix = 0;
    }

    return TRUE;
}

_Check_return_
BOOL
LoadPythonFunctions(
    _In_    HMODULE             PythonModule,
    _Inout_ PPYTHONFUNCTIONS    PythonFunctions,
    _In_    BOOL                IsPython2
    )
{
    BOOL Resolved;

    if (!ARGUMENT_PRESENT(PythonModule)) {
        return FALSE;
    }

    if (!ARGUMENT_PRESENT(PythonFunctions)) {
        return FALSE;
    }

#define TRY_RESOLVE_FUNCTION(Type, Name) ( \
    PythonFunctions->Name = (Type)(        \
        GetProcAddress(                    \
            PythonModule,                  \
            #Name                          \
        )                                  \
    )                                      \
)

#define RESOLVE_FUNCTION(Type, Name)                                \
    if (!TRY_RESOLVE_FUNCTION(Type, Name)) {                        \
        OutputDebugStringA("Failed to resolve Python!" #Name "\n"); \
        return FALSE;                                               \
    }

    RESOLVE_FUNCTION(PPY_INITIALIZE, Py_Initialize);
    RESOLVE_FUNCTION(PPY_INITIALIZE_EX, Py_InitializeEx);
    RESOLVE_FUNCTION(PPY_IS_INITIALIZED, Py_IsInitialized);
    RESOLVE_FUNCTION(PPY_FINALIZE, Py_Finalize);
    RESOLVE_FUNCTION(PPY_GETVERSION, Py_GetVersion);
    RESOLVE_FUNCTION(PPYDICT_GETITEMSTRING, PyDict_GetItemString);
    RESOLVE_FUNCTION(PPYFRAME_GETLINENUMBER, PyFrame_GetLineNumber);
    RESOLVE_FUNCTION(PPYCODE_ADDR2LINE, PyCode_Addr2Line);
    RESOLVE_FUNCTION(PPYEVAL_SETTRACE, PyEval_SetProfile);
    RESOLVE_FUNCTION(PPYEVAL_SETTRACE, PyEval_SetTrace);
    RESOLVE_FUNCTION(PPY_INCREF, Py_IncRef);
    RESOLVE_FUNCTION(PPY_DECREF, Py_DecRef);
    RESOLVE_FUNCTION(PPYGILSTATE_ENSURE, PyGILState_Ensure);
    RESOLVE_FUNCTION(PPYGILSTATE_RELEASE, PyGILState_Release);
    RESOLVE_FUNCTION(PPYOBJECT_HASH, PyObject_Hash);
    RESOLVE_FUNCTION(PPYMEM_MALLOC, PyMem_Malloc);
    RESOLVE_FUNCTION(PPYMEM_REALLOC, PyMem_Realloc);
    RESOLVE_FUNCTION(PPYMEM_FREE, PyMem_Free);
    RESOLVE_FUNCTION(PPYOBJECT_MALLOC, PyObject_Malloc);
    RESOLVE_FUNCTION(PPYOBJECT_REALLOC, PyObject_Realloc);
    RESOLVE_FUNCTION(PPYOBJECT_FREE, PyObject_Free);
    RESOLVE_FUNCTION(PPYGC_COLLECT, PyGC_Collect);
    RESOLVE_FUNCTION(PPYOBJECT_GC_MALLOC, _PyObject_GC_Malloc);
    RESOLVE_FUNCTION(PPYOBJECT_GC_NEW, _PyObject_GC_New);
    RESOLVE_FUNCTION(PPYOBJECT_GC_NEWVAR, _PyObject_GC_NewVar);
    RESOLVE_FUNCTION(PPYOBJECT_GC_RESIZE, _PyObject_GC_Resize);
    RESOLVE_FUNCTION(PPYOBJECT_GC_TRACK, PyObject_GC_Track);
    RESOLVE_FUNCTION(PPYOBJECT_GC_UNTRACK, PyObject_GC_UnTrack);
    RESOLVE_FUNCTION(PPYOBJECT_GC_DEL, PyObject_GC_Del);
    RESOLVE_FUNCTION(PPYOBJECT_INIT, PyObject_Init);
    RESOLVE_FUNCTION(PPYOBJECT_INITVAR, PyObject_InitVar);
    RESOLVE_FUNCTION(PPYOBJECT_NEW, _PyObject_New);
    RESOLVE_FUNCTION(PPYOBJECT_NEWVAR, _PyObject_NewVar);

    TRY_RESOLVE_FUNCTION(PPYOBJECT_COMPARE, PyObject_Compare);
    TRY_RESOLVE_FUNCTION(PPYUNICODE_ASUNICODE, PyUnicode_AsUnicode);
    TRY_RESOLVE_FUNCTION(PPYUNICODE_GETLENGTH, PyUnicode_GetLength);

    //
    // The function signatures for these methods changed from taking
    // 'char' to 'wchar_t' from 2.x to 3.x.  The _AW suffix in the
    // following macro represents ascii/wide.  If we're version 2, we'll
    // changed, so we just assign the resolved function address to both
    // the A and W variants, casting the function pointer type where
    // necessary.  It's up to the caller to call the correct one with
    // the right character type.
    //

#define _TRY_RESOLVE_FUNCTION(Type, Name, Field) ( \
    PythonFunctions->Field = (Type)(               \
        GetProcAddress(                            \
            PythonModule,                          \
            #Name                                  \
        )                                          \
    )                                              \
)

#define TRY_RESOLVE_FUNCTION_AW(Type, Name) do {       \
    if (IsPython2) {                                   \
        _TRY_RESOLVE_FUNCTION(Type##A, Name, Name##A); \
    } else {                                           \
        _TRY_RESOLVE_FUNCTION(Type##W, Name, Name##W); \
    }                                                  \
} while (0)

#define RESOLVE_FUNCTION_AW(Type, Name) do {                        \
    Resolved = FALSE;                                               \
    if (IsPython2) {                                                \
        if (_TRY_RESOLVE_FUNCTION(Type##A, Name, Name##A)) {        \
            Resolved = TRUE;                                        \
        }                                                           \
    } else {                                                        \
        if (_TRY_RESOLVE_FUNCTION(Type##W, Name, Name##W)) {        \
            Resolved = TRUE;                                        \
        }                                                           \
    }                                                               \
    if (!Resolved) {                                                \
        OutputDebugStringA("Failed to resolve Python!" #Name "\n"); \
        return FALSE;                                               \
    }                                                               \
} while (0)

    TRY_RESOLVE_FUNCTION_AW(PPYSYS_SET_ARGV_EX, PySys_SetArgvEx);
    RESOLVE_FUNCTION_AW(PPY_SET_PROGRAM_NAME, Py_SetProgramName);
    RESOLVE_FUNCTION_AW(PPY_SET_PYTHON_HOME, Py_SetPythonHome);
    RESOLVE_FUNCTION_AW(PPY_MAIN, Py_Main);
    RESOLVE_FUNCTION_AW(PPY_GET_PREFIX, Py_GetPrefix);
    RESOLVE_FUNCTION_AW(PPY_GET_EXEC_PREFIX, Py_GetExecPrefix);
    RESOLVE_FUNCTION_AW(PPY_GET_PROGRAM_NAME, Py_GetProgramName);
    RESOLVE_FUNCTION_AW(PPY_GET_PROGRAM_FULL_PATH, Py_GetProgramFullPath);

    return TRUE;
}

_Check_return_
BOOL
LoadPythonSymbols(
    _In_    HMODULE     PythonModule,
    _Out_   PPYTHON     Python
    )
{
    BOOL IsPython2;
    PPYTHONFUNCTIONS PythonFunctions;

    if (!PythonModule) {
        return FALSE;
    }

    if (!Python) {
        return FALSE;
    }

    if (!LoadPythonData(PythonModule, &Python->PythonData)) {
        return FALSE;
    }

    IsPython2 = (Python->MajorVersion == 2);
    PythonFunctions = &Python->PythonFunctions;

    if (!LoadPythonFunctions(PythonModule, PythonFunctions, IsPython2)) {
        return FALSE;
    }

    Python->PythonModule = PythonModule;

    return TRUE;
}

_Check_return_
BOOL
LoadPythonExData(
    _In_    HMODULE         PythonModule,
    _Out_   PPYTHONEXDATA   PythonExData
    )
{
    if (!PythonModule) {
        return FALSE;
    }

    if (!PythonExData) {
        return FALSE;
    }

    return TRUE;
}

_Check_return_
BOOL
LoadPythonExFunctions(
    _In_opt_    HMODULE             PythonExModule,
    _Inout_     PPYTHONEXFUNCTIONS  PythonExFunctions
)
{

    if (!PythonExModule) {
        return FALSE;
    }

    if (!PythonExFunctions) {
        return FALSE;
    }

#define TRY_RESOLVE_FUNCTIONEX(Type, Name) ( \
    PythonExFunctions->##Name = (Type)(      \
        GetProcAddress(                      \
            PythonExModule,                  \
            #Name                            \
        )                                    \
    )                                        \
)

#define RESOLVE_FUNCTIONEX(Type, Name)                                \
    if (!TRY_RESOLVE_FUNCTIONEX(Type, Name)) {                        \
        OutputDebugStringA("Failed to resolve PythonEx!" #Name "\n"); \
        return FALSE;                                                 \
    }

    RESOLVE_FUNCTIONEX(PALLOCATE_STRING, AllocateString);
    RESOLVE_FUNCTIONEX(PALLOCATE_STRING_AND_BUFFER, AllocateStringAndBuffer);
    RESOLVE_FUNCTIONEX(PALLOCATE_STRING_BUFFER, AllocateStringBuffer);
    RESOLVE_FUNCTIONEX(PFREE_STRING_BUFFER, FreeStringBuffer);
    RESOLVE_FUNCTIONEX(PFREE_STRING_BUFFER_DIRECT, FreeStringBufferDirect);
    RESOLVE_FUNCTIONEX(PALLOCATE_BUFFER, AllocateBuffer);
    RESOLVE_FUNCTIONEX(PFREE_BUFFER, FreeBuffer);
    RESOLVE_FUNCTIONEX(PREGISTER_FRAME, RegisterFrame);
    RESOLVE_FUNCTIONEX(PHASH_AND_ATOMIZE_ANSI, HashAndAtomizeAnsi);
    RESOLVE_FUNCTIONEX(PSET_PYTHON_ALLOCATORS, SetPythonAllocators);
    RESOLVE_FUNCTIONEX(
        PINITIALIZE_PYTHON_RUNTIME_TABLES,
        InitializePythonRuntimeTables
    );

    return TRUE;
}

PVOID
HeapAllocationRoutine(
    _In_ HANDLE HeapHandle,
    _In_ ULONG ByteSize
    )
{
    return HeapAlloc(HeapHandle, 0, ByteSize);
}

VOID
HeapFreeRoutine(
    _In_ HANDLE HeapHandle,
    _In_ PVOID Buffer
    )
{
    HeapFree(HeapHandle, 0, Buffer);
}

PVOID
NTAPI
FunctionTableAllocationRoutine(
    _In_ PRTL_GENERIC_TABLE Table,
    _In_ CLONG ByteSize
    )
{
    PPYTHON Python = (PPYTHON)Table->TableContext;
    if (ByteSize != TargetSizeOfPythonFunctionTableEntry) {
        __debugbreak();
    }
    return ALLOCATE(FunctionTableEntry, ByteSize);
}

VOID
NTAPI
FunctionTableFreeRoutine(
    _In_ PRTL_GENERIC_TABLE Table,
    _In_ PVOID Buffer
    )
{
    PPYTHON Python = (PPYTHON)Table->TableContext;
    FREE(FunctionTableEntry, Buffer);
}

FORCEINLINE
RTL_GENERIC_COMPARE_RESULTS
GenericComparePointer(
    _In_ PRTL_GENERIC_TABLE Table,
    _In_ ULONG_PTR First,
    _In_ ULONG_PTR Second
    )
{
    if (First == Second) {
        return GenericEqual;
    } else if (First < Second) {
        return GenericLessThan;
    }
    return GenericGreaterThan;
}

RTL_GENERIC_COMPARE_RESULTS
FunctionTableCompareRoutine(
    _In_ PRTL_GENERIC_TABLE Table,
    _In_ PPYTHON_FUNCTION First,
    _In_ PPYTHON_FUNCTION Second
    )
{
    return GenericComparePointer(Table, First->Key, Second->Key);
}

_Check_return_
BOOL
LoadPythonExRuntime(
    _In_opt_    HMODULE PythonExModule,
    _Inout_     PPYTHONEXRUNTIME PythonExRuntime
    )
{
    DWORD HeapFlags;

    if (!PythonExModule) {
        return FALSE;
    }

    if (!PythonExRuntime) {
        return FALSE;
    }

    HeapFlags = HEAP_NO_SERIALIZE | HEAP_GENERATE_EXCEPTIONS;
    PythonExRuntime->HeapHandle = HeapCreate(HeapFlags, 0, 0);

    if (!PythonExRuntime->HeapHandle) {
        return FALSE;
    }

    return TRUE;
}

_Check_return_
BOOL
InitializePythonRuntimeTables(
    _In_ PPYTHON Python
    )
{
    BOOL Success;
    PRTL Rtl;
    PPREFIX_TABLE PrefixTable;
    PRTL_GENERIC_TABLE GenericTable;
    PPYTHON_PATH_TABLE PathTable;
    PPYTHON_FUNCTION_TABLE FunctionTable;

    Rtl = Python->Rtl;

    Success = AllocatePythonPathTable(Python, &PathTable);

    if (!Success) {
        return FALSE;
    }

    Python->PathTable = PathTable;

    PrefixTable = &Python->PathTable->PrefixTable;
    SecureZeroMemory(PrefixTable, sizeof(*PrefixTable));
    Rtl->PfxInitialize(PrefixTable);

    PrefixTable = &Python->ModuleTable->PrefixTable;
    SecureZeroMemory(PrefixTable, sizeof(*PrefixTable));
    Rtl->PfxInitialize(PrefixTable);

    Python->FunctionTableCompareRoutine = FunctionTableCompareRoutine;
    Python->FunctionTableAllocateRoutine = FunctionTableAllocationRoutine;
    Python->FunctionTableFreeRoutine = FunctionTableFreeRoutine;

    Success = AllocatePythonFunctionTable(Python, &FunctionTable);

    if (!Success) {
        return FALSE;
    }

    Python->FunctionTable = FunctionTable;

    GenericTable = &Python->FunctionTable->GenericTable;

    Rtl->RtlInitializeGenericTable(GenericTable,
                                   Python->FunctionTableCompareRoutine,
                                   Python->FunctionTableAllocateRoutine,
                                   Python->FunctionTableFreeRoutine,
                                   Python);

    return TRUE;
}

_Check_return_
BOOL
LoadPythonExSymbols(
    _In_opt_    HMODULE             PythonExModule,
    _Inout_     PPYTHON             Python
    )
{
    HMODULE Module;

    if (!Python) {
        return FALSE;
    }

    if (PythonExModule) {
        Module = PythonExModule;

    } else {
        DWORD Flags = (
            GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS          |
            GET_MODULE_HANDLE_EX_FLAG_UNCHANGED_REFCOUNT
        );

        if (!GetModuleHandleEx(Flags,
                               (LPCTSTR)&LoadPythonExFunctions,
                               &Module))
        {
            return FALSE;
        }

        if (!Module) {
            return FALSE;
        }
    }

    if (!LoadPythonExData(Module, &Python->PythonExData)) {
        return FALSE;
    }

    if (!LoadPythonExFunctions(Module, &Python->PythonExFunctions)) {
        return FALSE;
    }

    if (!LoadPythonExRuntime(Module, &Python->PythonExRuntime)) {
        return FALSE;
    }

    Python->PythonExModule = Module;

    return TRUE;
}


_Check_return_
BOOL
IsSupportedPythonVersion(_In_ PPYTHON Python)
{
    return (
        (Python->MajorVersion == 2 && (
            Python->MinorVersion >= 4 &&
            Python->MinorVersion <= 7
        )) ||
        (Python->MajorVersion == 3 && (
            Python->MinorVersion >= 0 &&
            Python->MinorVersion <= 5
        ))
    );
}

_Check_return_
BOOL
ResolvePythonExOffsets(_In_ PPYTHON Python)
{
    //
    // Validate some of our trickier offsets here, which makes it slightly
    // nicer to deal with errors during development versus a C_ASSERT()
    // approach, which doesn't give feedback when the expression fails.
    //

    CONST USHORT OffsetA1 = FIELD_OFFSET(PREFIX_TABLE_ENTRY, NextPrefixTree);
    CONST USHORT OffsetA2 = (
        FIELD_OFFSET(PYTHON_PATH_TABLE_ENTRY, NextPrefixTree)
    );

    CONST USHORT OffsetB1 = FIELD_OFFSET(PREFIX_TABLE_ENTRY, NodeTypeCode);
    CONST USHORT OffsetB2 = (
        FIELD_OFFSET(PYTHON_PATH_TABLE_ENTRY, NodeTypeCode)
    );

    CONST USHORT OffsetC1 = FIELD_OFFSET(PREFIX_TABLE_ENTRY, NameLength);
    CONST USHORT OffsetC2 = (
        FIELD_OFFSET(PYTHON_PATH_TABLE_ENTRY, PrefixNameLength)
    );

    CONST USHORT SizeOfPythonFunction = sizeof(PYTHON_FUNCTION);
    CONST USHORT SizeOfPythonFunctionTableEntry = (
        sizeof(PYTHON_FUNCTION_TABLE_ENTRY)
    );
    CONST USHORT SizeOfTableEntryHeaderHeader = (
        sizeof(TABLE_ENTRY_HEADER_HEADER)
    );

    CONST USHORT SizeOfPythonPathTableEntry = sizeof(PYTHON_PATH_TABLE_ENTRY);
    CONST USHORT TargetFunctionEntrySize = (
        TargetSizeOfPythonFunctionTableEntry
    );
    CONST USHORT TargetPathEntrySize = TargetSizeOfPythonPathTableEntry;

#define ASSERT_EQUAL(Left, Right) \
    if (Left != Right) {          \
        __debugbreak();           \
    }

    ASSERT_EQUAL(OffsetA1, OffsetA2);
    ASSERT_EQUAL(OffsetB1, OffsetB2);
    ASSERT_EQUAL(OffsetC1, OffsetC2);

    ASSERT_EQUAL(SizeOfPythonFunctionTableEntry, TargetFunctionEntrySize);
    ASSERT_EQUAL(SizeOfPythonPathTableEntry, TargetPathEntrySize);

    Python->PythonPathTableEntryOffsets = &PythonPathTableEntryOffsets;
    Python->PythonFunctionOffsets = &PythonFunctionOffsets;

    ASSERT_EQUAL(PythonPathTableEntryOffsets.Size, 128);
    ASSERT_EQUAL(SizeOfPythonFunctionTableEntry, 256);
    ASSERT_EQUAL(PythonFunctionOffsets.Size, 256-SizeOfTableEntryHeaderHeader);

    return TRUE;
}

_Check_return_
BOOL
ResolvePythonOffsets(_In_ PPYTHON Python)
{
    if (!Python) {
        return FALSE;
    }

    if (!IsSupportedPythonVersion(Python)) {
        return FALSE;
    }

    //
    // (It's easier doing this with a separate switch statement for each one.)
    //

    switch (Python->MajorVersion) {
        case 2:
            Python->PyCodeObjectOffsets = &PyCodeObjectOffsets25_27;
            break;
        case 3:
            switch (Python->MinorVersion) {
                case 0:
                case 1:
                case 2:
                    Python->PyCodeObjectOffsets = &PyCodeObjectOffsets30_32;
                    break;
                case 3:
                case 4:
                case 5:
                    Python->PyCodeObjectOffsets = &PyCodeObjectOffsets33_35;
                    break;
                default:
                    return FALSE;
            };
            break;
        default:
            return FALSE;
    };

    //
    // Resolve CodeObject offsets.
    //

    switch (Python->MajorVersion) {
        case 2:
            Python->PyFrameObjectOffsets = &PyFrameObjectOffsets25_33;
            break;
        case 3:
            switch (Python->MinorVersion) {
                case 0:
                case 1:
                case 2:
                case 3:
                    Python->PyFrameObjectOffsets = &PyFrameObjectOffsets25_33;
                    break;
                case 4:
                case 5:
                    Python->PyFrameObjectOffsets = &PyFrameObjectOffsets34_35;
                    break;
                default:
                    return FALSE;
            };
            break;
        default:
            return FALSE;
    };

    return ResolvePythonExOffsets(Python);
}

_Check_return_
BOOL
ResolveAndVerifyPythonVersion(
    _In_    HMODULE     PythonModule,
    _Inout_ PPYTHON     Python
    )
{
    PCCH Version;
    ULONG MajorVersion;
    ULONG MinorVersion;
    ULONG PatchLevel = 0;
    CHAR VersionString[2] = { 0 };
    PRTLCHARTOINTEGER RtlCharToInteger;

    RtlCharToInteger = Python->Rtl->RtlCharToInteger;

    Python->Py_GetVersion = (PPY_GETVERSION)(
        GetProcAddress(
            PythonModule,
            "Py_GetVersion"
        )
    );

    if (!Python->Py_GetVersion) {
        goto error;
    }

    Python->VersionString = Python->Py_GetVersion();
    if (!Python->VersionString) {
        goto error;
    }

    VersionString[0] = Python->VersionString[0];
    if (FAILED(RtlCharToInteger(VersionString, 0, &MajorVersion))) {
        goto error;
    }
    Python->MajorVersion = (USHORT)MajorVersion;

    Python->MinorVersion = 0;
    Python->PatchLevel = 0;
    Version = &Python->VersionString[1];
    if (*Version == '.') {
        Version++;
        VersionString[0] = *Version;
        if (FAILED(RtlCharToInteger(VersionString, 0, &MinorVersion))) {
            goto error;
        }
        Python->MinorVersion = (USHORT)MinorVersion;

        Version++;
        if (*Version == '.') {
            Version++;
            VersionString[0] = *Version;
            if (FAILED(RtlCharToInteger(VersionString, 0, &PatchLevel))) {
                goto error;
            }
            Python->PatchLevel = (USHORT)PatchLevel;
            Version++;
            if (*Version && (*Version >= '0' && *Version <= '9')) {
                PatchLevel = 0;
                VersionString[0] = *Version;
                if (FAILED(RtlCharToInteger(VersionString, 0, &PatchLevel))) {
                    goto error;
                }
                Python->PatchLevel = (USHORT)(
                    (Python->PatchLevel * 10) + PatchLevel
                );
            }
        }
    }

    return ResolvePythonOffsets(Python);

error:
    return FALSE;
}

_Check_return_
BOOL
InitializePython(
    _In_                         PRTL                Rtl,
    _In_                         HMODULE             PythonModule,
    _Out_bytecap_(*SizeOfPython) PPYTHON             Python,
    _Inout_                      PULONG              SizeOfPython
    )
{
    if (!Python) {
        if (SizeOfPython) {
            *SizeOfPython = sizeof(*Python);
        }
        return FALSE;
    }

    if (!SizeOfPython) {
        return FALSE;
    }

    if (*SizeOfPython < sizeof(*Python)) {
        return FALSE;
    } else if (*SizeOfPython == 0) {
        *SizeOfPython = sizeof(*Python);
    }

    if (!PythonModule) {
        return FALSE;
    }

    if (!Rtl) {
        return FALSE;
    }

    if (!Rtl->LoadShlwapi(Rtl)) {
        goto Error;
    }

    SecureZeroMemory(Python, sizeof(*Python));

    Python->Rtl = Rtl;

    if (!ResolveAndVerifyPythonVersion(PythonModule, Python)) {
        goto Error;
    }

    if (!LoadPythonSymbols(PythonModule, Python)) {
        goto Error;
    }

    if (!LoadPythonExSymbols(NULL, Python)) {
        goto Error;
    }

    Python->Size = *SizeOfPython;

    Python->NumberOfCacheElements = (
        sizeof(Python->CodeObjectCache) /
        sizeof(Python->CodeObjectCache[0])
    );

    return TRUE;

Error:
    // Clear any partial state.
    SecureZeroMemory(Python, sizeof(*Python));
    return FALSE;
}


PYTHON_API
BOOL
HashAndAtomizeAnsi(
    _In_    PPYTHON Python,
    _In_    PSTR String,
    _Out_   PLONG HashPointer,
    _Out_   PULONG AtomPointer
    )
{

    return HashAndAtomizeAnsiInline(Python,
                                    String,
                                    HashPointer,
                                    AtomPointer);
}


#ifdef __cplusplus
}; // extern "C"
#endif

// vim:set ts=8 sw=4 sts=4 tw=80 expandtab                                     :
