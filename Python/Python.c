
#include "stdafx.h"
#include <Windows.h>
#include "Python.h"
#include "../Tracer/Tracing.h"

#pragma intrinsic(strlen)

static CONST UNICODE_STRING W__init__py  = RTL_CONSTANT_STRING(L"__init__.py");
static CONST UNICODE_STRING W__init__pyc = RTL_CONSTANT_STRING(L"__init__.pyc");
static CONST UNICODE_STRING W__init__pyo = RTL_CONSTANT_STRING(L"__init__.pyo");

static CONST PUNICODE_STRING InitPyFilesW[] = {
    (PUNICODE_STRING)&W__init__py,
    (PUNICODE_STRING)&W__init__pyc,
    (PUNICODE_STRING)&W__init__pyo
};

static CONST STRING A__init__py  = RTL_CONSTANT_STRING("__init__.py");
static CONST STRING A__init__pyc = RTL_CONSTANT_STRING("__init__.pyc");
static CONST STRING A__init__pyo = RTL_CONSTANT_STRING("__init__.pyo");

static CONST PUNICODE_STRING InitPyFilesA[] = {
    (PUNICODE_STRING)&A__init__py,
    (PUNICODE_STRING)&A__init__pyc,
    (PUNICODE_STRING)&A__init__pyo
};

static CONST USHORT NumberOfInitPyFiles = (
    sizeof(InitPyFilesA) /
    sizeof(InitPyFilesA[0])
);

static const STRING SELFA = RTL_CONSTANT_STRING("self");

static const PYCODEOBJECTOFFSETS PyCodeObjectOffsets25_27 = {
    FIELD_OFFSET(PYCODEOBJECT25_27, ArgumentCount),
    0, // KeywordOnlyArgCount
    FIELD_OFFSET(PYCODEOBJECT25_27, NumberOfLocals),
    FIELD_OFFSET(PYCODEOBJECT25_27, StackSize),
    FIELD_OFFSET(PYCODEOBJECT25_27, Flags),
    FIELD_OFFSET(PYCODEOBJECT25_27, Code),
    FIELD_OFFSET(PYCODEOBJECT25_27, Constants),
    FIELD_OFFSET(PYCODEOBJECT25_27, Names),
    FIELD_OFFSET(PYCODEOBJECT25_27, LocalVariableNames),
    FIELD_OFFSET(PYCODEOBJECT25_27, FreeVariableNames),
    FIELD_OFFSET(PYCODEOBJECT25_27, CellVariableNames),
    FIELD_OFFSET(PYCODEOBJECT25_27, Filename),
    FIELD_OFFSET(PYCODEOBJECT25_27, Name),
    FIELD_OFFSET(PYCODEOBJECT25_27, FirstLineNumber),
    FIELD_OFFSET(PYCODEOBJECT25_27, LineNumberTable),
    0 // ZombieFrame
};

static const PYCODEOBJECTOFFSETS PyCodeObjectOffsets30_32 = {
    FIELD_OFFSET(PYCODEOBJECT30_32, ArgumentCount),
    FIELD_OFFSET(PYCODEOBJECT30_32, KeywordOnlyArgumentCount),
    FIELD_OFFSET(PYCODEOBJECT30_32, NumberOfLocals),
    FIELD_OFFSET(PYCODEOBJECT30_32, StackSize),
    FIELD_OFFSET(PYCODEOBJECT30_32, Flags),
    FIELD_OFFSET(PYCODEOBJECT30_32, Code),
    FIELD_OFFSET(PYCODEOBJECT30_32, Constants),
    FIELD_OFFSET(PYCODEOBJECT30_32, Names),
    FIELD_OFFSET(PYCODEOBJECT30_32, LocalVariableNames),
    FIELD_OFFSET(PYCODEOBJECT30_32, FreeVariableNames),
    FIELD_OFFSET(PYCODEOBJECT30_32, CellVariableNames),
    FIELD_OFFSET(PYCODEOBJECT30_32, Filename),
    FIELD_OFFSET(PYCODEOBJECT30_32, Name),
    FIELD_OFFSET(PYCODEOBJECT30_32, FirstLineNumber),
    FIELD_OFFSET(PYCODEOBJECT30_32, LineNumberTable),
    0 // ZombieFrame
};

static const PYCODEOBJECTOFFSETS PyCodeObjectOffsets33_35 = {
    FIELD_OFFSET(PYCODEOBJECT33_35, ArgumentCount),
    FIELD_OFFSET(PYCODEOBJECT33_35, KeywordOnlyArgumentCount),
    FIELD_OFFSET(PYCODEOBJECT33_35, NumberOfLocals),
    FIELD_OFFSET(PYCODEOBJECT33_35, StackSize),
    FIELD_OFFSET(PYCODEOBJECT33_35, Flags),
    FIELD_OFFSET(PYCODEOBJECT33_35, Code),
    FIELD_OFFSET(PYCODEOBJECT33_35, Constants),
    FIELD_OFFSET(PYCODEOBJECT33_35, Names),
    FIELD_OFFSET(PYCODEOBJECT33_35, LocalVariableNames),
    FIELD_OFFSET(PYCODEOBJECT33_35, FreeVariableNames),
    FIELD_OFFSET(PYCODEOBJECT33_35, CellVariableNames),
    FIELD_OFFSET(PYCODEOBJECT33_35, Filename),
    FIELD_OFFSET(PYCODEOBJECT33_35, Name),
    FIELD_OFFSET(PYCODEOBJECT33_35, FirstLineNumber),
    FIELD_OFFSET(PYCODEOBJECT33_35, LineNumberTable),
    FIELD_OFFSET(PYCODEOBJECT33_35, ZombieFrame),
};

static const PYFRAMEOBJECTOFFSETS PyFrameObjectOffsets25_33 = {
    FIELD_OFFSET(PYFRAMEOBJECT25_33, ThreadState),
    FIELD_OFFSET(PYFRAMEOBJECT25_33, LastInstruction),
    FIELD_OFFSET(PYFRAMEOBJECT25_33, LineNumber),
    FIELD_OFFSET(PYFRAMEOBJECT25_33, BlockIndex),
    FIELD_OFFSET(PYFRAMEOBJECT25_33, BlockStack),
    FIELD_OFFSET(PYFRAMEOBJECT25_33, LocalsPlusStack),
    0, // Generator
    0  // StillExecuting
};

static const PYFRAMEOBJECTOFFSETS PyFrameObjectOffsets34_35 = {
    0, // ThreadState
    FIELD_OFFSET(PYFRAMEOBJECT34_35, LastInstruction),
    FIELD_OFFSET(PYFRAMEOBJECT34_35, LineNumber),
    FIELD_OFFSET(PYFRAMEOBJECT34_35, BlockIndex),
    FIELD_OFFSET(PYFRAMEOBJECT34_35, BlockStack),
    FIELD_OFFSET(PYFRAMEOBJECT34_35, LocalsPlusStack),
    FIELD_OFFSET(PYFRAMEOBJECT34_35, Generator),
    FIELD_OFFSET(PYFRAMEOBJECT34_35, StillExecuting)
};

_Check_return_
BOOL
LoadPythonData(
    _In_    HMODULE     PythonModule,
    _Out_   PPYTHONDATA PythonData
)
{
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
    RESOLVE_TYPE(PyInstance);
    RESOLVE_TYPE(PyModule);

    return TRUE;
}

_Check_return_
BOOL
LoadPythonFunctions(
    _In_    HMODULE             PythonModule,
    _Inout_ PPYTHONFUNCTIONS    PythonFunctions
)
{
    if (!ARGUMENT_PRESENT(PythonModule)) {
        return FALSE;
    }

    if (!ARGUMENT_PRESENT(PythonFunctions)) {
        return FALSE;
    }

#define TRY_RESOLVE_FUNCTION(Type, Name) ( \
    PythonFunctions->##Name = (Type)(      \
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

    RESOLVE_FUNCTION(PPY_GETVERSION, Py_GetVersion);
    RESOLVE_FUNCTION(PPYDICT_GETITEMSTRING, PyDict_GetItemString);
    RESOLVE_FUNCTION(PPYFRAME_GETLINENUMBER, PyFrame_GetLineNumber);
    RESOLVE_FUNCTION(PPYEVAL_SETTRACE, PyEval_SetProfile);
    RESOLVE_FUNCTION(PPYEVAL_SETTRACE, PyEval_SetTrace);
    RESOLVE_FUNCTION(PPY_INCREF, Py_IncRef);
    RESOLVE_FUNCTION(PPY_DECREF, Py_DecRef);
    RESOLVE_FUNCTION(PPYGILSTATE_ENSURE, PyGILState_Ensure);
    RESOLVE_FUNCTION(PPYGILSTATE_RELEASE, PyGILState_Release);
    RESOLVE_FUNCTION(PPYOBJECT_HASH, PyObject_Hash);
    RESOLVE_FUNCTION(PPYOBJECT_COMPARE, PyObject_Compare);
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

    TRY_RESOLVE_FUNCTION(PPYUNICODE_ASUNICODE, PyUnicode_AsUnicode);
    TRY_RESOLVE_FUNCTION(PPYUNICODE_GETLENGTH, PyUnicode_GetLength);

    return TRUE;
}

_Check_return_
BOOL
LoadPythonSymbols(
    _In_    HMODULE     PythonModule,
    _Out_   PPYTHON     Python
)
{
    if (!PythonModule) {
        return FALSE;
    }

    if (!Python) {
        return FALSE;
    }

    if (!LoadPythonData(PythonModule, &Python->PythonData)) {
        return FALSE;
    }

    if (!LoadPythonFunctions(PythonModule, &Python->PythonFunctions)) {
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
        GetProcAddress(                    \
            PythonExModule,                  \
            #Name                          \
        )                                  \
    )                                      \
)

#define RESOLVE_FUNCTIONEX(Type, Name)                                \
    if (!TRY_RESOLVE_FUNCTIONEX(Type, Name)) {                        \
        OutputDebugStringA("Failed to resolve PythonEx!" #Name "\n"); \
        return FALSE;                                               \
    }

    RESOLVE_FUNCTIONEX(PREGISTER_FRAME, RegisterFrame);
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
    return Python->AllocationRoutine(Python->AllocationContext, ByteSize);
}

VOID
NTAPI
FunctionTableFreeRoutine(
    _In_ PRTL_GENERIC_TABLE Table,
    _In_ PVOID Buffer
)
{
    PPYTHON Python = (PPYTHON)Table->TableContext;
    Python->FreeRoutine(Python->FreeContext, Buffer);
}

FORCEINLINE
RTL_GENERIC_COMPARE_RESULTS
GenericComparePointer(
    _In_ PRTL_GENERIC_TABLE Table,
    _In_ ULONG_PTR First,
    _In_ ULONG_PTR Second
    )
{
    if (First < Second) {
        return GenericLessThan;
    }
    else if (First > Second) {
        return GenericGreaterThan;
    }
    else {
        return GenericEqual;
    }
}

RTL_GENERIC_COMPARE_RESULTS
FunctionTableCompareRoutine(
    _In_ PRTL_GENERIC_TABLE Table,
    _In_ PPYTHON_FUNCTION First,
    _In_ PPYTHON_FUNCTION Second
)
{
    PPYTHON Python = (PPYTHON)Table->TableContext;
    return GenericComparePointer(Table,
                                 (ULONG_PTR)First->CodeObject,
                                 (ULONG_PTR)Second->CodeObject);
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
    _In_        PPYTHON Python,
    _In_opt_    PALLOCATION_ROUTINE AllocationRoutine,
    _In_opt_    PVOID AllocationContext,
    _In_opt_    PFREE_ROUTINE FreeRoutine,
    _In_opt_    PVOID FreeContext
    )
{
    PRTL Rtl;
    Rtl = Python->Rtl;
    PPREFIX_TABLE PrefixTable;
    PRTL_GENERIC_TABLE GenericTable;

    PrefixTable = &Python->PathTable.PrefixTable;
    Rtl->PfxInitialize(PrefixTable);

    Rtl->PfxInitialize(&Python->ModuleNameTable);

    //
    // If the allocation routine isn't provided, default to the generic
    // heap allocation routine.
    //

    if (!ARGUMENT_PRESENT(AllocationRoutine)) {

        Python->AllocationRoutine = HeapAllocationRoutine;
        Python->AllocationContext = Python->HeapHandle;

        Python->FreeRoutine = HeapFreeRoutine;
        Python->FreeContext = Python->HeapHandle;

    }
    else {

        Python->AllocationRoutine = AllocationRoutine;
        Python->AllocationContext = AllocationContext;

        Python->FreeRoutine = FreeRoutine;
        Python->FreeContext = FreeContext;

    }

    Python->FunctionTableCompareRoutine = FunctionTableCompareRoutine;
    Python->FunctionTableAllocateRoutine = FunctionTableAllocationRoutine;
    Python->FunctionTableFreeRoutine = FunctionTableFreeRoutine;

    GenericTable = &Python->FunctionTable.GenericTable;
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
        default:
            return FALSE;
    };


    return TRUE;
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
            if (*Version && *Version >= '0' || *Version <= '9') {
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

    SecureZeroMemory(Python, sizeof(*Python));

    Python->Rtl = Rtl;

    if (!ResolveAndVerifyPythonVersion(PythonModule, Python)) {
        goto error;
    }

    if (!LoadPythonSymbols(PythonModule, Python)) {
        goto error;
    }

    if (!LoadPythonExSymbols(NULL, Python)) {
        goto error;
    }

    Python->Size = *SizeOfPython;

    Python->NumberOfCacheElements = (
        sizeof(Python->CodeObjectCache) /
        sizeof(Python->CodeObjectCache[0])
    );

    return TRUE;

error:
    // Clear any partial state.
    SecureZeroMemory(Python, sizeof(*Python));
    return FALSE;
}

FORCEINLINE
BOOL
GetPythonStringInformation(
    _In_     PPYTHON             Python,
    _In_     PPYOBJECT           StringOrUnicodeObject,
    _Out_    PSIZE_T             Length,
    _Out_    PUSHORT             Width,
    _Out_    PPVOID              Buffer
)
{
    PYOBJECTEX Object;
    Object.Object = StringOrUnicodeObject;

    if (StringOrUnicodeObject->Type == Python->PyString.Type) {

        *Length = Object.String->ObjectSize;
        *Buffer = Object.String->Value;

        *Width = sizeof(CHAR);

    } else if (StringOrUnicodeObject->Type == Python->PyUnicode.Type) {

        if (Python->PyUnicode_AsUnicode && Python->PyUnicode_GetLength) {

            *Length = Python->PyUnicode_GetLength(StringOrUnicodeObject);
            *Buffer = Python->PyUnicode_AsUnicode(StringOrUnicodeObject);

        } else {

            *Length = Object.Unicode->Length;
            *Buffer = Object.Unicode->String;

        }

        *Width = sizeof(WCHAR);

    } else {

        return FALSE;

    }

    return TRUE;
}

FORCEINLINE
BOOL
WrapPythonStringAsString(
    _In_     PPYTHON    Python,
    _In_     PPYOBJECT  StringOrUnicodeObject,
    _Out_    PSTRING    String
    )
{
    SIZE_T Length;
    USHORT Width;
    PVOID  Buffer;

    BOOL Success;

    Success = GetPythonStringInformation(
        Python,
        StringOrUnicodeObject,
        &Length,
        &Width,
        &Buffer
    );

    if (!Success) {
        return FALSE;
    }

    if (Width == 2) {
        Length = Length << 1;
    } else if (Width != 1) {
        __debugbreak();
    }

    String->MaximumLength = String->Length = (USHORT)Length;
    String->Buffer = Buffer;

    return TRUE;
}

FORCEINLINE
BOOL
IsModuleDirectoryW(
    _In_      PRTL Rtl,
    _In_      PUNICODE_STRING Directory,
    _Out_     PBOOL IsModule
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
BOOL
IsModuleDirectoryA(
    _In_      PRTL Rtl,
    _In_      PSTRING Directory,
    _Out_     PBOOL IsModule
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

BOOL
RegisterDirectory(
    _In_      PPYTHON Python,
    _In_      PSTRING Directory,
    _In_opt_  PSTRING DirectoryName,
    _In_opt_  PPYTHON_PATH_TABLE_ENTRY AncestorEntry,
    _Out_opt_ PPPYTHON_PATH_TABLE_ENTRY EntryPointer,
    _In_      BOOL IsRoot
    )
{
    PRTL Rtl;

    PPREFIX_TABLE PrefixTable;
    PPREFIX_TABLE_ENTRY PrefixTableEntry;

    PVOID Buffer;
    ULONG AllocationSize;
    PPYTHON_PATH_TABLE_ENTRY Entry;

    PSTRING AncestorModuleName;
    PSTRING Name;
    PSTRING ModuleName;
    USHORT Offset;
    USHORT NameLength;

    PSTRING DirectoryPrefix;

    PSTRING Match = NULL;
    BOOL CaseInsensitive = TRUE;
    BOOL Success = FALSE;
    BOOL IsModule;
    BOOL AncestorIsRoot;

    PALLOCATION_ROUTINE AllocationRoutine;
    PVOID AllocationContext;

    PFREE_ROUTINE FreeRoutine;
    PVOID FreeContext;

    if (!ARGUMENT_PRESENT(Python)) {
        return FALSE;
    }

    if (!ARGUMENT_PRESENT(Directory)) {
        return FALSE;
    }

    AllocationRoutine = Python->AllocationRoutine;
    AllocationContext = Python->AllocationContext;

    FreeRoutine = Python->FreeRoutine;
    FreeContext = Python->FreeContext;

    //
    // Non-root nodes must have a name and ancestor provided.
    //

    if (IsRoot) {

        AllocationSize = sizeof(*Entry);

        IsModule = FALSE;
        AncestorIsRoot = FALSE;

    } else {

        if (!ARGUMENT_PRESENT(DirectoryName)) {
            return FALSE;
        }

        if (!ARGUMENT_PRESENT(AncestorEntry)) {
            return FALSE;
        }

        IsModule = TRUE;

        AncestorIsRoot = (BOOL)AncestorEntry->IsNonModuleDirectory;

        if (AncestorEntry->Name.Length != 0 ||
            AncestorEntry->ModuleName.Length != 0)
        {
            __debugbreak();
        }

        AncestorModuleName = &AncestorEntry->ModuleName;

        NameLength = DirectoryName->Length;

        AllocationSize = (
            sizeof(*Entry)                    +
            AncestorModuleName->MaximumLength + // includes trailing NUL
            NameLength
        );

        if (!AncestorIsRoot) {

            //
            // Account for the joining slash + NUL.
            //

            AllocationSize += 2;

        } else {

            //
            // Account for just the trailing NUL.
            //

            AllocationSize += 1;
        }

    }

    Rtl = Python->Rtl;

    AllocationSize = ALIGN_UP(AllocationSize, sizeof(ULONG_PTR));

    Buffer = AllocationRoutine(AllocationContext, AllocationSize);
    if (!Buffer) {
        return FALSE;
    }

    Entry = (PPYTHON_PATH_TABLE_ENTRY)Buffer;

    if (IsModule) {
        Entry->IsModuleDirectory = TRUE;
    } else {
        Entry->IsNonModuleDirectory = TRUE;
    }

    if (IsRoot) {

        ClearString(&Entry->ModuleName);
        ClearString(&Entry->Name);

    } else {

        PSTR Dest;
        PSTR Source;
        USHORT Count;

        ModuleName = &Entry->ModuleName;
        Name = &Entry->Name;

        if (!AncestorIsRoot) {
            Offset = AncestorModuleName->Length + 1;
        } else {
            Offset = 0;
        }

        ModuleName->Length = Offset + NameLength;
        ModuleName->MaximumLength = ModuleName->Length + 1;

        Name->Length = NameLength;
        Name->MaximumLength = NameLength + 1;

        //
        // The new module name lives at the end of the structure.
        //

        ModuleName->Buffer = (PSTR)RtlOffsetToPointer(Entry, sizeof(*Entry));

        //
        // Point the name into the relevant offset of the ModuleName.
        //

        Name->Buffer = &ModuleName->Buffer[Offset];

        if (!AncestorIsRoot) {

            //
            // Copy the ModuleName over.
            //

            __movsb(ModuleName->Buffer,
                    AncestorModuleName->Buffer,
                    AncestorModuleName->Length);

            //
            // Add the slash.
            //

            ModuleName->Buffer[Offset-1] = '\\';

        }

        //
        // Copy the name over.
        //

        Count = NameLength;
        Dest = Name->Buffer;
        Source = DirectoryName->Buffer;

        __movsb(Dest, Source, Count);

        Dest += Count;

        //
        // And add the final trailing NUL.
        //

        *Dest++ = '\0';
    }

    DirectoryPrefix = &Entry->Path;

    DirectoryPrefix->Length = Directory->Length;
    DirectoryPrefix->MaximumLength = Directory->MaximumLength;
    DirectoryPrefix->Buffer = Directory->Buffer;

    PrefixTable = &Python->PathTable.PrefixTable;
    PrefixTableEntry = (PPREFIX_TABLE_ENTRY)Entry;

    Success = Rtl->PfxInsertPrefix(PrefixTable,
                                   DirectoryPrefix,
                                   PrefixTableEntry);

    if (!Success) {
        FreeRoutine(FreeContext, Buffer);
    } else if (ARGUMENT_PRESENT(EntryPointer)) {
        *EntryPointer = Entry;
    }

    return Success;
}

BOOL
GetSelf(
    _In_    PPYTHON             Python,
    _In_    PPYTHON_FUNCTION    Function,
    _In_    PPYFRAMEOBJECT      FrameObject,
    _Out_   PPPYOBJECT          SelfPointer
    )
{
    BOOL Success = FALSE;
    PPYOBJECT Self = NULL;
    PPYOBJECT Locals = FrameObject->Locals;
    PPYOBJECT CodeObject = Function->CodeObject;

    LONG ArgumentCount;
    PPYTUPLEOBJECT ArgumentNames;

    //
    // Attempt to resolve self from the locals dictionary.
    //

    if (Locals && Locals->Type == Python->PyDict.Type) {
        if ((Self = Python->PyDict_GetItemString(Locals, "self"))) {
            Success = TRUE;
            goto End;
        }
    }

    //
    // If that didn't work, see if the first argument's name was "self", and
    // if so, use that.
    //

    ArgumentCount = *(
        (PLONG)RtlOffsetToPointer(
            CodeObject,
            Python->PyCodeObjectOffsets->ArgumentCount
        )
    );

    ArgumentNames = *(
        (PPPYTUPLEOBJECT)RtlOffsetToPointer(
            CodeObject,
            Python->PyCodeObjectOffsets->LocalVariableNames
        )
    );

    if (ArgumentCount != 0 && ArgumentNames->Type == Python->PyTuple.Type) {
        static const STRING SelfString = RTL_CONSTANT_STRING("self");
        PRTL_EQUAL_STRING EqualString = Python->Rtl->RtlEqualString;
        STRING ArgumentName;

        Success = WrapPythonStringAsString(
            Python,
            ArgumentNames->Item[0],
            &ArgumentName
        );

        if (!Success) {
            return FALSE;
        }

        if (EqualString(&ArgumentName, &SelfString, FALSE)) {
            Self = *(
                (PPPYOBJECT)RtlOffsetToPointer(
                    FrameObject,
                    Python->PyFrameObjectOffsets->LocalsPlusStack
                )
            );
        }
    }

    Success = TRUE;

End:
    *SelfPointer = Self;
    return Success;
}

BOOL
GetClassNameFromSelf(
    _In_    PPYTHON             Python,
    _In_    PPYTHON_FUNCTION    Function,
    _In_    PPYFRAMEOBJECT      FrameObject,
    _In_    PPYOBJECT           Self,
    _In_    PSTRING             FunctionName,
    _Out_   PPCHAR              ClassNameBuffer
    )
{
    BOOL Success;
    ULONG Index;
    PPYTUPLEOBJECT Mro;
    PPYTYPEOBJECT TypeObject;
    PPYOBJECT TypeDict;
    PPYOBJECT CodeObject;
    PYOBJECTEX Func;
    PPYDICT_GETITEMSTRING PyDict_GetItemString;
    PCHAR FuncName;

    if (Python->PyInstance.Type && Self->Type == Python->PyInstance.Type) {
        STRING Class;
        PPYINSTANCEOBJECT Instance = (PPYINSTANCEOBJECT)Self;
        PPYOBJECT ClassNameObject = Instance->OldStyleClass->Name;

        Success = WrapPythonStringAsString(Python, ClassNameObject, &Class);
        if (!Success) {
            return FALSE;
        }

        *ClassNameBuffer = Class.Buffer;
        return TRUE;
    }

    Mro = (PPYTUPLEOBJECT)Self->Type->MethodResolutionOrder;
    if (!Mro || Mro->Type != Python->PyTuple.Type) {

        //
        // We should always have an MRO for new-style classes.
        //

        return FALSE;
    }

    FuncName = FunctionName->Buffer;
    CodeObject = FrameObject->Code;
    PyDict_GetItemString = Python->PyDict_GetItemString;

    //
    // Walk the MRO looking for our method.
    //

    for (Index = 0; Index < Mro->ObjectSize; Index++) {

        TypeObject = (PPYTYPEOBJECT)Mro->Item[Index];

        if (TypeObject->Type != Python->PyType.Type) {
            continue;
        }

        TypeDict = TypeObject->Dict;
        if (TypeDict->Type != Python->PyDict.Type) {
            continue;
        }

        Func.Object = PyDict_GetItemString(TypeDict, FuncName);

        if (!Func.Object || Func.Object->Type != Python->PyFunction.Type) {
            continue;
        }

        if (Func.Function->Code != CodeObject) {
            continue;
        }

        //
        // We've found the function in the MRO whose code object matches the
        // code object of our frame, so use the class name of this type.
        //

        *ClassNameBuffer = (PCHAR)TypeObject->Name;
        return TRUE;
    }

    return FALSE;
}

BOOL
RegisterFunction(
    _In_ PPYTHON Python,
    _In_ PPYTHON_FUNCTION Function,
    _In_ PPYFRAMEOBJECT FrameObject
    )
/*++

Routine Description:

    This method is responsible for finalizing details about a function, such
    as the function name, class name (if any), module name and first line
    number.  A frame object is provided to assist with resolution of names.

    It is called once per function after a new PYTHON_PATH_TABLE_ENTRY has been
    inserted into the path prefix table.


--*/
{
    BOOL Success;
    PRTL Rtl;
    PCHAR Dest;
    PCHAR Start;
    PPYOBJECT Self = NULL;
    PPYOBJECT FunctionNameObject;
    PPYOBJECT CodeObject = Function->CodeObject;
    PCPYCODEOBJECTOFFSETS CodeObjectOffsets = Python->PyCodeObjectOffsets;
    PSTRING Path;
    PSTRING FunctionName;
    PSTRING ClassName;
    PSTRING FullName;
    PSTRING ModuleName;
    PCHAR ClassNameBuffer = NULL;
    USHORT FullNameAllocSize;
    USHORT FullNameLength;
    PVOID Buffer;
    PSTRING ParentModuleName;
    PSTRING ParentName;
    PPYTHON_PATH_TABLE_ENTRY PathEntry;
    PPYTHON_PATH_TABLE_ENTRY ParentPathEntry;

    PathEntry = (PPYTHON_PATH_TABLE_ENTRY)Function;
    ParentPathEntry = Function->ParentPathEntry;

    Function->FirstLineNumber = (USHORT)*(
        (PULONG)RtlOffsetToPointer(
            CodeObject,
            CodeObjectOffsets->FirstLineNumber
        )
    );

    FunctionNameObject = *(
        (PPPYOBJECT)RtlOffsetToPointer(
            CodeObject,
            CodeObjectOffsets->Name
        )
    );

    Rtl = Python->Rtl;
    FunctionName = &PathEntry->FunctionName;

    Success = WrapPythonStringAsString(Python,
                                       FunctionNameObject,
                                       FunctionName);

    if (!Success) {
        return FALSE;
    }

    Success = GetSelf(Python, Function, FrameObject, &Self);

    if (!Success) {
        return FALSE;
    }

    ClassName = &PathEntry->ClassName;
    ClearString(ClassName);

    if (Self) {

        Success = GetClassNameFromSelf(Python,
                                       Function,
                                       FrameObject,
                                       Self,
                                       FunctionName,
                                       &ClassNameBuffer);

        if (!Success) {
            return FALSE;
        }

        if (!ClassNameBuffer) {
            return FALSE;
        }

        ClassName->Length = (USHORT)strlen((PCSZ)ClassNameBuffer);
        ClassName->MaximumLength = ClassName->Length;
        ClassName->Buffer = ClassNameBuffer;
    }

    //
    // Initialize the module name, initially pointing at our parent's
    // buffer.
    //

    ParentModuleName = &ParentPathEntry->ModuleName;
    ModuleName = &PathEntry->ModuleName;
    ModuleName->Length = ParentModuleName->Length;
    ModuleName->MaximumLength = ParentModuleName->Length;
    ModuleName->Buffer = ParentModuleName->Buffer;

    ParentName = &ParentPathEntry->Name;

    //
    // Calculate the length of the full name.  The extra +1s are accounting
    // for joining slashes and the final trailing NUL.
    //

    FullNameLength = (
        ModuleName->Length                                +
        1                                                 +
        (ParentName->Length ? ParentName->Length + 1 : 0) +
        (ClassName->Length ? ClassName->Length + 1 : 0)   +
        FunctionName->Length                              +
        1
    );

    if (FullNameLength > MAX_USTRING) {
        return FALSE;
    }

    FullNameAllocSize = ALIGN_UP_USHORT_TO_POINTER_SIZE(FullNameLength);

    if (FullNameAllocSize > MAX_STRING) {
        FullNameAllocSize = (USHORT)MAX_STRING;
    }

    Buffer = Python->AllocationRoutine(
        Python->AllocationContext,
        FullNameLength
    );

    if (!Buffer) {
        return FALSE;
    }

    //
    // Construct the final full name.  After each part has been copied, update
    // the corresponding Buffer pointer to the relevant point within the newly-
    // allocated buffer for the full name.
    //

    FullName = &PathEntry->FullName;
    ClearString(FullName);
    FullName->Buffer = (PCHAR)Buffer;

    Dest = FullName->Buffer;

    __movsb(Dest, (PBYTE)ModuleName->Buffer, ModuleName->Length);
    Dest += ModuleName->Length;
    ModuleName->Buffer = FullName->Buffer;

    *Dest++ = '\\';

    if (ParentName->Length) {
        __movsb(Dest, (PBYTE)ParentName->Buffer, ParentName->Length);
        Dest += ParentName->Length;
        *Dest++ = '\\';
    }

    if (ClassName->Length) {
        Start = Dest;
        __movsb(Dest, (PBYTE)ClassName->Buffer, ClassName->Length);
        ClassName->Buffer = Start;
        Dest += ClassName->Length;
        *Dest++ = '\\';
    }

    Start = Dest;
    __movsb(Dest, (PBYTE)FunctionName->Buffer, FunctionName->Length);
    FunctionName->Buffer = Start;
    Dest += FunctionName->Length;
    *Dest++ = '\0';

    //
    // Omit trailing NUL from Length.
    //

    FullName->Length = FullNameLength-1;
    FullName->MaximumLength = FullNameLength;

    //
    // Point our path at our parent.
    //

    Path = &PathEntry->Path;
    Path->Length = ParentPathEntry->Path.Length;
    Path->MaximumLength = Path->Length;
    Path->Buffer = ParentPathEntry->Path.Buffer;

    PathEntry->IsFunction = TRUE;

    return TRUE;
}

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

BOOL
QualifyPath(
        _In_        PPYTHON             Python,
        _In_        PSTRING             SourcePath,
        _Out_       PPSTRING            DestinationPathPointer
    )
{
    ULARGE_INTEGER Size;
    CONST ULARGE_INTEGER MaxSize = { MAX_USTRING - 2 };
    ULONG AllocSizeInBytes;

    ULONG CurDirLength;
    ULONG RequiredSizeInBytes;

    PSTRING String;
    PSTR Dest;

    PALLOCATION_ROUTINE AllocationRoutine;
    PVOID AllocationContext;

    PFREE_ROUTINE FreeRoutine;
    PVOID FreeContext;

    if (!ARGUMENT_PRESENT(Python)) {
        return FALSE;
    }

    if (!ARGUMENT_PRESENT(SourcePath)) {
        return FALSE;
    }


    if (!ARGUMENT_PRESENT(DestinationPathPointer)) {
        return FALSE;
    }

    AllocationRoutine = Python->AllocationRoutine;
    AllocationContext = Python->AllocationContext;

    FreeRoutine = Python->FreeRoutine;
    FreeContext = Python->FreeContext;

    Size.QuadPart = SourcePath->Length;

    //
    // Get the length (in characters) of the current directory and verify it
    // fits within our unicode string size constraints.
    //

    RequiredSizeInBytes = GetCurrentDirectoryW(0, NULL);
    if (RequiredSizeInBytes == 0) {
        return FALSE;
    }

    //
    // Update the allocation size with the current directory size plus
    // a joining backslash character.
    //

    Size.QuadPart = Size.LowPart + sizeof(CHAR) + RequiredSizeInBytes;

    //
    // Verify it's within our limits.
    //

    if (Size.HighPart != 0 || Size.LowPart > MaxSize.LowPart) {
        return FALSE;
    }

    //
    // Account for the STRING struct size.
    //

    AllocSizeInBytes = Size.LowPart + sizeof(STRING);

    String = (PSTRING)AllocationRoutine(AllocationContext, AllocSizeInBytes);

    if (!String) {
        return FALSE;
    }

    //
    // Point the buffer to the memory immediately after the struct.
    //

    String->Buffer = (PSTR)(
        RtlOffsetToPointer(
            String,
            sizeof(STRING)
        )
    );

    //
    // CurDirLength will represent the length (number of characters, not
    // bytes) of the string copied into our buffer.
    //

    CurDirLength = GetCurrentDirectoryA(
        RequiredSizeInBytes,
        String->Buffer
    );

    if (CurDirLength == 0) {
        FreeRoutine(FreeContext, String);
        return FALSE;
    }

    Dest = &String->Buffer[CurDirLength];

    //
    // Add the joining backslash and update the destination pointer.
    //

    *Dest++ = '\\';

    //
    // Initialize the maximum string length (number of bytes, including NUL),
    // and the string length (number of bytes, not including NUL).
    //

    String->MaximumLength = (USHORT)Size.LowPart;
    String->Length = ((USHORT)Size.LowPart) - sizeof(CHAR);

    //
    // Copy the rest of the name over.
    //

    __movsb(Dest, SourcePath->Buffer, SourcePath->Length);

    //
    // Add terminating NUL.
    //

    Dest[String->Length] = '\0';

    //
    // And finally, update the user's pointer.
    //

    *DestinationPathPointer = String;

    return TRUE;
}

BOOL
GetPathEntryForDirectory(
    _In_     PPYTHON             Python,
    _In_     PSTRING             Directory,
    _In_     PRTL_BITMAP         Backslashes,
    _In_     PUSHORT             BitmapHintIndex,
    _In_     PUSHORT             NumberOfBackslashesRemaining,
    _Out_    PPPYTHON_PATH_TABLE_ENTRY PathEntryPointer
    )
{
    PRTL Rtl;

    PPREFIX_TABLE PrefixTable;
    PPREFIX_TABLE_ENTRY PrefixTableEntry;

    PPYTHON_PATH_TABLE_ENTRY PathEntry = NULL;
    PPYTHON_PATH_TABLE_ENTRY RootEntry = NULL;
    PPYTHON_PATH_TABLE_ENTRY ParentEntry = NULL;
    PPYTHON_PATH_TABLE_ENTRY AncestorEntry = NULL;

    PSTRING Match = NULL;
    BOOL CaseInsensitive = TRUE;
    BOOL Success;

    STRING NextName;
    STRING AncestorName;
    STRING PreviousName;
    STRING DirectoryName;

    STRING NextDirectory;
    STRING RootDirectory;
    STRING ParentDirectory;
    STRING AncestorDirectory;
    STRING PreviousDirectory;

    PSTRING NextPrefix;
    PSTRING RootPrefix;
    PSTRING ParentPrefix;
    PSTRING AncestorPrefix;
    PSTRING PreviousPrefix;

    USHORT Offset;
    USHORT ReversedIndex;
    USHORT NumberOfChars;
    USHORT LastReversedIndex;
    USHORT CumulativeReversedIndex;
    BOOL IsModule = FALSE;

    //
    // Validate arguments.
    //

    if (!ARGUMENT_PRESENT(Python)) {
        return FALSE;
    }

    if (!ARGUMENT_PRESENT(Directory)) {
        return FALSE;
    }

    if (!ARGUMENT_PRESENT(Backslashes)) {
        return FALSE;
    }

    if (!ARGUMENT_PRESENT(PathEntryPointer)) {
        return FALSE;
    }

    Rtl = Python->Rtl;

    //
    // Initialize pointer to the PREFIX_TABLE.
    //

    PrefixTable = &Python->PathTable.PrefixTable;

    //
    // Seach for the directory in the prefix table.
    //

    PrefixTableEntry = Rtl->PfxFindPrefix(PrefixTable, Directory);
    PathEntry = (PPYTHON_PATH_TABLE_ENTRY)PrefixTableEntry;

    if (PathEntry) {

        //
        // A match was found, see if it matches our entire directory string.
        //

        Match = PathEntry->Prefix;

        if (Match->Length == Directory->Length) {

            //
            // The match is exact.  The directory has already been added to
            // our path table.
            //

            Success = TRUE;
            goto End;

        }


        if (Match->Length > Directory->Length) {

            //
            // We should never get a longer match than the directory name
            // we searched for.
            //

            __debugbreak();
        }

        //
        // A shorter entry was found.  Fall through to the following code
        // which will handle inserting a new prefix table entry for the
        // directory.  Note that we just set the shorter directory as our
        // parent directory; later on, once we've actually calculated our
        // parent directory, we may revise this to be an ancestory entry.
        //

        ParentEntry = PathEntry;
        PathEntry = NULL;

    }

    //
    // Final step: make sure directory entries have been added for all of our
    // parent directories, up to and including the first one we find without
    // an __init__.py file present.  The directory after this becomes the start
    // of our module name.
    //

    //
    // Get the index of the next backslash if possible.
    //

    if (!*NumberOfBackslashesRemaining) {
        __debugbreak();

        //
        // This will be hit if the directory we just added was a root volume,
        // e.g. the file was c:\foo.py and we added c:\.  Set the module name
        // to be empty and return.
        //

        PathEntry->IsNonModuleDirectory = TRUE;
        ClearString(&PathEntry->ModuleName);
        return TRUE;

    }

    //
    // We have at least one parent directory to process.  Extract it.
    //

    LastReversedIndex = (*BitmapHintIndex)++;

    ReversedIndex = (USHORT)(
        Rtl->RtlFindSetBits(
            Backslashes,
            1,
            *BitmapHintIndex
        )
    );

    if (ReversedIndex == BITS_NOT_FOUND) {

        //
        // Should never happen.
        //

        __debugbreak();
    }

    NumberOfChars = Directory->Length;
    Offset = NumberOfChars - ReversedIndex + LastReversedIndex + 1;
    CumulativeReversedIndex = LastReversedIndex;

    //
    // Extract the directory name, and the parent directory's full path.
    //

    DirectoryName.Length = (
        (ReversedIndex - CumulativeReversedIndex) -
        sizeof(CHAR)
    );
    DirectoryName.MaximumLength = DirectoryName.Length;
    DirectoryName.Buffer = &Directory->Buffer[Offset];

    ParentDirectory.Length = Offset - 1;
    ParentDirectory.MaximumLength = Offset - 1;
    ParentDirectory.Buffer = Directory->Buffer;

    //
    // Special-case fast-path: if ParentEntry is defined and the length matches
    // ParentDirectory, we can assume all paths have already been added.
    //

    if (ParentEntry) {
        ParentPrefix = ParentEntry->Prefix;

        if (ParentPrefix->Length == ParentDirectory.Length) {

            //
            // Parent has been added.
            //

            goto FoundParent;

        }

        //
        // An ancestor, not our immediate parent, has been added.
        //

        AncestorEntry = ParentEntry;
        AncestorPrefix = ParentPrefix;
        ParentEntry = NULL;
        ParentPrefix = NULL;

        goto FoundAncestor;
    }

    //
    // No parent entry was found at all, we need to find the first directory
    // in our hierarchy that *doesn't* have an __init__.py file, then add that
    // and all subsequent directories up-to and including our parent.
    //

    PreviousDirectory.Length = ParentDirectory.Length;
    PreviousDirectory.MaximumLength = ParentDirectory.MaximumLength;
    PreviousDirectory.Buffer = ParentDirectory.Buffer;

    AncestorDirectory.Length = ParentDirectory.Length;
    AncestorDirectory.MaximumLength = ParentDirectory.MaximumLength;
    AncestorDirectory.Buffer = ParentDirectory.Buffer;

    PreviousName.Length = DirectoryName.Length;
    PreviousName.MaximumLength = DirectoryName.MaximumLength;
    PreviousName.Buffer = DirectoryName.Buffer;

    AncestorName.Length = DirectoryName.Length;
    AncestorName.MaximumLength = DirectoryName.MaximumLength;
    AncestorName.Buffer = DirectoryName.Buffer;

    do {

        if (!*NumberOfBackslashesRemaining) {

            IsModule = FALSE;

        } else {

            Success = IsModuleDirectoryA(Rtl, &AncestorDirectory, &IsModule);

            if (!Success) {

                //
                // We treat failure of the IsModuleDirectory() call the same
                // as if it indicated that the directory wasn't a module (i.e.
                // didn't have an __init__.py file), mainly because there's
                // not really any other sensible option.  (Unwinding all of
                // the entries we may have added seems like overkill at this
                // point.)
                //

                IsModule = FALSE;
            }
        }

        if (!IsModule) {

            //
            // We've found the first non-module directory we're looking for.
            // This becomes our root directory.
            //

            RootDirectory.Length = AncestorDirectory.Length;
            RootDirectory.MaximumLength = AncestorDirectory.MaximumLength;
            RootDirectory.Buffer = AncestorDirectory.Buffer;
            RootPrefix = &RootDirectory;

            Success = RegisterDirectory(Python,
                                        &RootDirectory,
                                        NULL,
                                        NULL,
                                        &RootEntry,
                                        TRUE);

            if (!Success) {
                return FALSE;
            }

            if (PreviousDirectory.Length > RootDirectory.Length) {

                //
                // Add the previous directory as the first "module" directory.
                //

                Success = RegisterDirectory(Python,
                                            &PreviousDirectory,
                                            &PreviousName,
                                            RootEntry,
                                            &AncestorEntry,
                                            FALSE);

                if (!Success) {
                    return FALSE;
                }

                //
                // Determine if we need to process more ancestors, or if that
                // was actually the parent path.
                //

                if (AncestorEntry->Prefix->Length == ParentDirectory.Length) {

                    ParentPrefix = AncestorEntry->Prefix;
                    ParentEntry = AncestorEntry;

                    goto FoundParent;

                } else {

                    goto FoundAncestor;

                }

            } else {

                //
                // Our parent directory is the root directory.
                //

                ParentPrefix = RootPrefix;
                ParentEntry = RootEntry;

                goto FoundParent;

            }

            break;
        }

        //
        // Parent directory is also a module.  Make sure we're not on the
        // root level.
        //

        if (!--(*NumberOfBackslashesRemaining)) {

            //
            // Force the loop to continue, which will trigger the check at the
            // top for number of remaining backslashes, which will fail, which
            // causes the path to be added as a root, which is what we want.
            //

            continue;
        }

        PreviousPrefix = &PreviousDirectory;

        PreviousDirectory.Length = AncestorDirectory.Length;
        PreviousDirectory.MaximumLength = AncestorDirectory.MaximumLength;
        PreviousDirectory.Buffer = AncestorDirectory.Buffer;

        PreviousName.Length = AncestorName.Length;
        PreviousName.MaximumLength = AncestorName.MaximumLength;
        PreviousName.Buffer = AncestorName.Buffer;

        LastReversedIndex = (*BitmapHintIndex)++;

        ReversedIndex = (USHORT)Rtl->RtlFindSetBits(Backslashes, 1,
                                                    *BitmapHintIndex);

        if (ReversedIndex == BITS_NOT_FOUND) {

            //
            // Should never happen.
            //

            __debugbreak();
        }

        CumulativeReversedIndex += LastReversedIndex;
        Offset = NumberOfChars - ReversedIndex + CumulativeReversedIndex + 1;

        //
        // Extract the ancestor name and directory full path.
        //

        AncestorName.Length = (
            (ReversedIndex - CumulativeReversedIndex) -
            sizeof(CHAR)
        );
        AncestorName.MaximumLength = DirectoryName.Length;
        AncestorName.Buffer = &Directory->Buffer[Offset];

        AncestorDirectory.Length = Offset - 1;
        AncestorDirectory.MaximumLength = Offset - 1;
        AncestorDirectory.Buffer = Directory->Buffer;

        //
        // Continue the loop.
        //

    } while (1);

FoundAncestor:

    //
    // Keep adding entries for the next directories as long as they're
    // not the parent directory.
    //

    do {

        //
        // Fill out the NextDirectory et al structures based on the next
        // directory after the current ancestor.  (Reversing the bitmap
        // may help here.)
        //

        __debugbreak();

        NextName;
        NextPrefix;
        NextDirectory;

    } while (1);

FoundParent:


    //
    // Add a new entry for the parent directory.
    //

    Success = RegisterDirectory(Python,
                                Directory,
                                &DirectoryName,
                                ParentEntry,
                                &PathEntry,
                                FALSE);

    if (!Success) {
        return FALSE;
    }

End:
    *PathEntryPointer = PathEntry;

    return TRUE;
}

BOOL
RegisterFile(
    _In_  PPYTHON    Python,
    _In_  PSTRING    QualifiedPath,
    _In_  PPYFRAMEOBJECT FrameObject,
    _Out_ PPPYTHON_PATH_TABLE_ENTRY PathEntryPointer
    )
/*++

Routine Description:

    This routine registers a new Python source code filename.

--*/
{
    CHAR Char;

    USHORT Index;
    USHORT Limit = 0;
    USHORT Offset = 0;
    USHORT ReversedIndex;
    USHORT BitmapHintIndex;
    USHORT NumberOfChars;
    USHORT InitPyNumberOfChars;
    USHORT NumberOfBackslashes;
    USHORT NumberOfBackslashesRemaining;
    USHORT ModuleLength;

    BOOL Success;
    BOOL IsInitPy = FALSE;
    BOOL CaseSensitive = TRUE;
    BOOL Reversed = TRUE;
    BOOL WeOwnPathBuffer;

    ULONG AllocationSize;

    HANDLE HeapHandle = NULL;

    PRTL Rtl;
    PPYOBJECT CodeObject;
    PPYOBJECT FilenameObject;
    PPREFIX_TABLE PrefixTable;
    PPREFIX_TABLE_ENTRY PrefixTableEntry;
    PPYTHON_PATH_TABLE_ENTRY PathEntry;
    PPYTHON_PATH_TABLE_ENTRY DirectoryEntry;
    PALLOCATION_ROUTINE AllocationRoutine;
    PVOID AllocationContext;
    PFREE_ROUTINE FreeRoutine;
    PVOID FreeContext;
    PVOID Buffer;
    PSTR ModuleBuffer;
    PSTRING Path = QualifiedPath;
    PSTRING Name;
    PSTRING FullName;
    PSTRING ModuleName;
    PSTRING DirectoryName;
    PSTRING DirectoryModuleName;
    PRTL_BITMAP BitmapPointer;

    STRING PathString;
    STRING Filename;
    STRING Directory;

    PSTR Dest;
    PSTR File;
    PSTR Start;
    USHORT PathAllocSize;
    USHORT FullNameLength;
    USHORT FullNameAllocSize;

    CHAR StackBitmapBuffer[_MAX_FNAME >> 3];
    RTL_BITMAP Bitmap = { _MAX_FNAME, (PULONG)&StackBitmapBuffer };
    BitmapPointer = &Bitmap;

    if (!ARGUMENT_PRESENT(Python)) {
        return FALSE;
    }

    if (!ARGUMENT_PRESENT(Path)) {
        return FALSE;
    }

    if (!ARGUMENT_PRESENT(FrameObject)) {
        return FALSE;
    }

    if (!ARGUMENT_PRESENT(PathEntryPointer)) {
        return FALSE;
    }

    AllocationRoutine = Python->AllocationRoutine;
    AllocationContext = Python->AllocationContext;

    FreeRoutine = Python->FreeRoutine;
    FreeContext = Python->FreeContext;

    CodeObject = FrameObject->Code;

    FilenameObject = *(
        (PPPYOBJECT)RtlOffsetToPointer(
            CodeObject,
            Python->PyCodeObjectOffsets->Filename
        )
    );

    Success = WrapPythonStringAsString(
        Python,
        FilenameObject,
        &PathString
    );

    if (!Success) {
        return FALSE;
    }

    //
    // If the path was qualified, we will have already allocated new space for
    // it.  If not, we'll need to account for additional space to store a copy
    // of the string later on in this method.
    //

    WeOwnPathBuffer = (Path->Buffer != QualifiedPath->Buffer);

    //
    // Initialize Rtl and character length variables.
    //

    Rtl = Python->Rtl;
    HeapHandle = Python->HeapHandle;

    NumberOfChars = Path->Length;
    InitPyNumberOfChars = A__init__py.Length;

    //
    // Create a reversed bitmap for the backslashes in the path.
    //

    Success = Rtl->CreateBitmapIndexForString(Rtl,
                                              Path,
                                              '\\',
                                              &HeapHandle,
                                              &BitmapPointer,
                                              Reversed,
                                              NULL);

    if (!Success) {
        return FALSE;
    }

    //
    // Make sure there's at least one backslash in the path.
    //

    NumberOfBackslashes = (USHORT)Rtl->RtlNumberOfSetBits(BitmapPointer);
    if (NumberOfBackslashes == 0) {
        goto Error;
    }

    //
    // Extract the filename from the path by finding the first backslash
    // and calculating the offset into the string buffer.
    //

    ReversedIndex = (USHORT)Rtl->RtlFindSetBits(BitmapPointer, 1, 0);
    Offset = NumberOfChars - ReversedIndex + 1;

    Filename.Length = ReversedIndex;
    Filename.MaximumLength = ReversedIndex + sizeof(CHAR);
    Filename.Buffer = &Path->Buffer[Offset];

    //
    // Extract the directory name.
    //

    Directory.Length = Offset - 1;
    Directory.MaximumLength = Directory.Length;
    Directory.Buffer = Path->Buffer;

    //
    // Get the module name from the directory.
    //

    BitmapHintIndex = ReversedIndex;
    NumberOfBackslashesRemaining = NumberOfBackslashes - 1;

    //
    // Get the PathEntry for the directory.  This will build up the path table
    // prefix tree with any intermediate module directories until we hit the
    // first "non-module" directory (i.e. the first directory that doesn't have
    // an __init__.py file).
    //

    DirectoryEntry = NULL;
    Success = GetPathEntryForDirectory(Python,
                                       &Directory,
                                       BitmapPointer,
                                       &BitmapHintIndex,
                                       &NumberOfBackslashesRemaining,
                                       &DirectoryEntry);

    if (!Success) {
        goto Error;
    }

    if (!DirectoryEntry) {

        //
        // We weren't able to create a parent path entry for the directory.
        // Not sure when this would happen at the moment, so __debugbreak().
        //

        __debugbreak();

    }

    //
    // Now that we have a DirectoryEntry for our parent directory, we can
    // continue creating a PathEntry for this filename and filling in the
    // relevant details.
    //

    //
    // Initialize convenience pointers.
    //

    DirectoryName = &DirectoryEntry->Name;
    DirectoryModuleName = &DirectoryEntry->ModuleName;
    ModuleBuffer = DirectoryModuleName->Buffer;
    ModuleLength = DirectoryModuleName->Length;

    //
    // Determine our final allocation size ahead of time so that we only have
    // to do a single allocation for the PathEntry struct plus any additional
    // string buffers.
    //

    AllocationSize = sizeof(*PathEntry);

    if (!WeOwnPathBuffer) {

        //
        // Account for path length plus trailing NUL.
        //

        PathAllocSize = ALIGN_UP_USHORT_TO_POINTER_SIZE(Path->Length + 1);
        AllocationSize += PathAllocSize;

    } else {

        PathAllocSize = 0;
    }

    //
    // Determine the length of the file part (filename sans extension).
    //

    for (Index = Filename.Length - 1; Index > 0; Index--) {
        Char = Filename.Buffer[Index];
        if (Char == '.') {

            //
            // We can re-use the Filename STRING here to truncate the length
            // such that the extension is omitted (i.e. there's no need to
            // allocate a new string as we're going to be copying into a new
            // string buffer shortly).
            //

            Filename.Length = Index;
            break;
        }
    }

    FullNameLength = (
        ModuleLength            +
        sizeof(CHAR)            + // joining slash
        Filename.Length         +
        sizeof(CHAR)              // terminating NUL
    );

    FullNameAllocSize = ALIGN_UP_USHORT_TO_POINTER_SIZE(FullNameLength);

    Buffer = AllocationRoutine(AllocationContext, AllocationSize);
    if (!Buffer) {
        goto Error;
    }

    PathEntry = (PPYTHON_PATH_TABLE_ENTRY)Buffer;

    PathEntry->IsFile = TRUE;

    Path = &PathEntry->Path;

    Path->Length = QualifiedPath->Length;
    Path->MaximumLength = PathAllocSize;

    //
    // Initialize shortcut pointers.
    //

    Name = &PathEntry->Name;
    FullName = &PathEntry->FullName;
    ModuleName = &PathEntry->ModuleName;

    //
    // Update the lengths of our strings.
    //

    FullName->Length = FullNameLength - 1; // exclude trailing NUL
    FullName->MaximumLength = FullNameAllocSize;

    Name->Length = Filename.Length;
    Name->MaximumLength = Filename.MaximumLength;

    ModuleName->Length = ModuleLength;
    ModuleName->MaximumLength = ModuleLength;
    ModuleName->Buffer = ModuleBuffer;

    //
    // Clear the strings we won't be using.
    //

    ClearString(&PathEntry->ClassName);

    if (!WeOwnPathBuffer) {

        //
        // If we didn't own the incoming path's buffer, point the STRING's
        // Buffer field to the end of our newly-allocated PathEntry struct
        // and copy the string over.
        //

        Path->Buffer = (PSTR)RtlOffsetToPointer(PathEntry, sizeof(*PathEntry));

        //
        // Copy the path string over to the new buffer.
        //

        __movsb((PBYTE)Path->Buffer,
                (PBYTE)QualifiedPath->Buffer,
                Path->Length);

        //
        // Add trailing NUL.
        //

        Path->Buffer[Path->Length] = '\0';

        //
        // Point the FullName->Buffer to after us.
        //

        FullName->Buffer = Path->Buffer + PathAllocSize;

    } else {

        //
        // We own the incoming path's buffer, so we can just point the new
        // field at the existing buffer.
        //

        Path->Buffer = QualifiedPath->Buffer;

        //
        // The FullName->Buffer will come straight after the PathEntry.
        //

        FullName->Buffer = (PSTR)(
            RtlOffsetToPointer(
                PathEntry,
                sizeof(*PathEntry)
            )
        );

    }

    //
    // Add the newly created PathEntry to our path table prefix tree.
    //

    PrefixTable = &Python->PathTable.PrefixTable;
    PrefixTableEntry = (PPREFIX_TABLE_ENTRY)PathEntry;

    Success = Rtl->PfxInsertPrefix(PrefixTable,
                                   Path,
                                   PrefixTableEntry);

    if (!Success) {
        if (WeOwnPathBuffer) {
            FreeRoutine(FreeContext, QualifiedPath->Buffer);
        }
        FreeRoutine(FreeContext, Buffer);
        goto Error;
    }


    //
    // Construct the final full name.  After each part has been copied, update
    // the corresponding Buffer pointer to the relevant point within the newly-
    // allocated buffer for the full name.
    //

    Dest = FullName->Buffer;

    //
    // Copy module name.
    //

    __movsb(Dest, (PBYTE)ModuleName->Buffer, ModuleName->Length);
    Dest += ModuleName->Length;
    ModuleName->Buffer = FullName->Buffer;

    //
    // Add joining slash.
    //

    *Dest++ = '\\';

    //
    // Copy the file name.
    //

    Start = Dest;
    File = Filename.Buffer;
    __movsb(Dest, File, Filename.Length);
    Name->Buffer = Start;
    Dest += Filename.Length;

    //
    // Add the trailing NUL.
    //

    *Dest++ = '\0';

    Success = TRUE;

    //
    // Intentional follow-on.
    //

Error:

    //
    // Update the caller's path entry pointer.
    //

    *PathEntryPointer = PathEntry;

    if ((ULONG_PTR)Bitmap.Buffer != (ULONG_PTR)BitmapPointer->Buffer) {

        //
        // We'll hit this point if a new bitmap had to be allocated because
        // our stack-allocated one was too small.  Make sure we free it here.
        //

        HeapFree(HeapHandle, 0, BitmapPointer->Buffer);
    }

    return Success;
}

BOOL
GetPathEntryFromFrame(
    _In_      PPYTHON         Python,
    _In_      PPYFRAMEOBJECT  FrameObject,
    _In_      LONG            EventType,
    _In_opt_  PPYOBJECT       ArgObject,
    _Out_opt_ PPPYTHON_PATH_TABLE_ENTRY PathEntryPointer
    )
{
    PRTL Rtl;
    PPYFRAMEOBJECT Frame = (PPYFRAMEOBJECT)FrameObject;
    PPYOBJECT FilenameObject;
    PSTRING Match;
    STRING PathString;
    PSTRING Path;
    PPREFIX_TABLE_ENTRY PrefixTableEntry;
    PPREFIX_TABLE PrefixTable;
    PPYTHON_PATH_TABLE_ENTRY PathEntry;
    BOOL TriedQualified = FALSE;
    BOOL Success;

    FilenameObject = *(
        (PPPYOBJECT)RtlOffsetToPointer(
            FrameObject->Code,
            Python->PyCodeObjectOffsets->Filename
        )
    );

    Path = &PathString;

    Success = WrapPythonStringAsString(
        Python,
        FilenameObject,
        Path
    );

    if (!Success) {
        return FALSE;
    }

    PrefixTable = &Python->PathTable.PrefixTable;
    Rtl = Python->Rtl;

Retry:
    PrefixTableEntry = Rtl->PfxFindPrefix(PrefixTable, Path);
    PathEntry = (PPYTHON_PATH_TABLE_ENTRY)PrefixTableEntry;

    if (PathEntry) {

        //
        // A match was found, see if it matches our entire path.
        //

        Match = PathEntry->Prefix;

        if (Match->Length == Path->Length) {

            //
            // An exact match was found, we've seen this filename before.
            //

            Success = TRUE;
            goto End;

        } else if (Match->Length > Path->Length) {

            //
            // We should never get a match that is longer than the path we
            // searched for.
            //

            __debugbreak();

        }

        //
        // A shorter entry was found.  Fall through to the following code which
        // will handle inserting a new prefix table entry for the file.
        //

    } else if (!TriedQualified) {

        //
        // See if we need to qualify the path and potentially do another prefix
        // tree search.  We don't do this up front because non-qualified paths
        // are pretty infrequent and the initial prefix tree lookup is on the
        // hot path.
        //

        if (!IsQualifiedPath(Path)) {

            Success = QualifyPath(Python, Path, &Path);

            if (!Success) {
                goto End;
            }

            TriedQualified = TRUE;
            goto Retry;
        }

    }

    Success = RegisterFile(Python,
                           Path,
                           FrameObject,
                           &PathEntry);

    //
    // Intentional follow-on to End (i.e. we let the success indicator from
    // RegisterFile() bubble back to the caller).
    //

End:

    //
    // Update the caller's PathEntryPointer (even if PathEntry is null).
    //

    if (ARGUMENT_PRESENT(PathEntryPointer)) {
        *PathEntryPointer = PathEntry;
    }

    return Success;
}

BOOL
RegisterFrame(
    _In_      PPYTHON         Python,
    _In_      PPYFRAMEOBJECT  FrameObject,
    _In_      LONG            EventType,
    _In_opt_  PPYOBJECT       ArgObject,
    _Out_opt_ PPPYTHON_FUNCTION FunctionPointer
    )
{
    PRTL Rtl;
    PPYFRAMEOBJECT Frame = (PPYFRAMEOBJECT)FrameObject;
    PPYOBJECT CodeObject;

    PYTHON_FUNCTION FunctionRecord;
    PPYTHON_FUNCTION Function;
    PPYTHON_FUNCTION_TABLE FunctionTable;
    BOOLEAN NewFunction;
    PPYTHON_PATH_TABLE_ENTRY ParentPathEntry;
    BOOL Success;

    CodeObject = Frame->Code;

    if (CodeObject->Type != Python->PyCode.Type) {
        return FALSE;
    }

    Rtl = Python->Rtl;

    FunctionRecord.CodeObject = CodeObject;

    FunctionTable = &Python->FunctionTable;

    Function = Rtl->RtlInsertElementGenericTable(
        &FunctionTable->GenericTable,
        &FunctionRecord,
        sizeof(FunctionRecord),
        &NewFunction
    );

    if (!NewFunction) {

        //
        // We've already seen this function.
        //

        return TRUE;
    }

    //
    // This is a new function; attempt to register the underlying filename for
    // this frame.
    //

    Success = GetPathEntryFromFrame(Python,
                                    FrameObject,
                                    EventType,
                                    ArgObject,
                                    &ParentPathEntry);

    if (!Success || !ParentPathEntry) {
        return FALSE;
    }

    Function->ParentPathEntry = ParentPathEntry;
    Function->CodeObject = CodeObject;

    Success = RegisterFunction(Python,
                               Function,
                               FrameObject);

    if (!Success) {
        return FALSE;
    }

    if (ARGUMENT_PRESENT(FunctionPointer)) {
        *FunctionPointer = Function;
    }

    return TRUE;
}

#ifdef __cpp
} // extern "C"
#endif

// vim:set ts=8 sw=4 sts=4 tw=80 expandtab                                     :
