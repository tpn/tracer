
#include "stdafx.h"

#pragma intrinsic(strlen)

static CONST USHORT TargetSizeOfPythonPathTableEntry = 128;
static CONST USHORT TargetSizeOfPythonFunctionTableEntry = 256;

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

static const PYTHON_PATH_TABLE_ENTRY_OFFSETS PythonPathTableEntryOffsets = {
    sizeof(PYTHON_PATH_TABLE_ENTRY),
    (sizeof(PYTHON_PATH_TABLE_ENTRY_OFFSETS) / sizeof(USHORT)) - 2,

    FIELD_OFFSET(PYTHON_PATH_TABLE_ENTRY, NodeTypeCode),
    FIELD_OFFSET(PYTHON_PATH_TABLE_ENTRY, PrefixNameLength),
    FIELD_OFFSET(PYTHON_PATH_TABLE_ENTRY, PathEntryType),
    FIELD_OFFSET(PYTHON_PATH_TABLE_ENTRY, NextPrefixTree),
    FIELD_OFFSET(PYTHON_PATH_TABLE_ENTRY, Links),
    FIELD_OFFSET(PYTHON_PATH_TABLE_ENTRY, Parent),
    FIELD_OFFSET(PYTHON_PATH_TABLE_ENTRY, LeftChild),
    FIELD_OFFSET(PYTHON_PATH_TABLE_ENTRY, RightChild),
    FIELD_OFFSET(PYTHON_PATH_TABLE_ENTRY, Prefix),

    FIELD_OFFSET(PYTHON_PATH_TABLE_ENTRY, Path),
    FIELD_OFFSET(PYTHON_PATH_TABLE_ENTRY, PathLength),
    FIELD_OFFSET(PYTHON_PATH_TABLE_ENTRY, PathMaximumLength),
    FIELD_OFFSET(PYTHON_PATH_TABLE_ENTRY, PathHash),
    FIELD_OFFSET(PYTHON_PATH_TABLE_ENTRY, PathBuffer),

    FIELD_OFFSET(PYTHON_PATH_TABLE_ENTRY, FullName),
    FIELD_OFFSET(PYTHON_PATH_TABLE_ENTRY, FullNameLength),
    FIELD_OFFSET(PYTHON_PATH_TABLE_ENTRY, FullNameMaximumLength),
    FIELD_OFFSET(PYTHON_PATH_TABLE_ENTRY, FullNameHash),
    FIELD_OFFSET(PYTHON_PATH_TABLE_ENTRY, FullNameBuffer),

    FIELD_OFFSET(PYTHON_PATH_TABLE_ENTRY, ModuleName),
    FIELD_OFFSET(PYTHON_PATH_TABLE_ENTRY, ModuleNameLength),
    FIELD_OFFSET(PYTHON_PATH_TABLE_ENTRY, ModuleNameMaximumLength),
    FIELD_OFFSET(PYTHON_PATH_TABLE_ENTRY, ModuleNameHash),
    FIELD_OFFSET(PYTHON_PATH_TABLE_ENTRY, ModuleNameBuffer),

    FIELD_OFFSET(PYTHON_PATH_TABLE_ENTRY, Name),
    FIELD_OFFSET(PYTHON_PATH_TABLE_ENTRY, NameLength),
    FIELD_OFFSET(PYTHON_PATH_TABLE_ENTRY, NameMaximumLength),
    FIELD_OFFSET(PYTHON_PATH_TABLE_ENTRY, NameHash),
    FIELD_OFFSET(PYTHON_PATH_TABLE_ENTRY, NameBuffer),

    FIELD_OFFSET(PYTHON_PATH_TABLE_ENTRY, ClassName),
    FIELD_OFFSET(PYTHON_PATH_TABLE_ENTRY, ClassNameLength),
    FIELD_OFFSET(PYTHON_PATH_TABLE_ENTRY, ClassNameMaximumLength),
    FIELD_OFFSET(PYTHON_PATH_TABLE_ENTRY, ClassNameHash),
    FIELD_OFFSET(PYTHON_PATH_TABLE_ENTRY, ClassNameBuffer),
};

static const PYTHON_FUNCTION_OFFSETS PythonFunctionOffsets = {
    sizeof(PYTHON_FUNCTION),
    (sizeof(PYTHON_FUNCTION_OFFSETS) / sizeof(USHORT)) - 2,

    FIELD_OFFSET(PYTHON_FUNCTION, PathEntry),
    FIELD_OFFSET(PYTHON_FUNCTION, ParentPathEntry),
    FIELD_OFFSET(PYTHON_FUNCTION, CodeObject),

    FIELD_OFFSET(PYTHON_FUNCTION, LineNumbersBitmap),
    FIELD_OFFSET(PYTHON_FUNCTION, Histogram),
    FIELD_OFFSET(PYTHON_FUNCTION, CodeLineNumbers),

    FIELD_OFFSET(PYTHON_FUNCTION, ReferenceCount),
    FIELD_OFFSET(PYTHON_FUNCTION, CodeObjectHash),
    FIELD_OFFSET(PYTHON_FUNCTION, FunctionHash),
    FIELD_OFFSET(PYTHON_FUNCTION, Unused1),

    FIELD_OFFSET(PYTHON_FUNCTION, FirstLineNumber),
    FIELD_OFFSET(PYTHON_FUNCTION, NumberOfLines),
    FIELD_OFFSET(PYTHON_FUNCTION, NumberOfCodeLines),
    FIELD_OFFSET(PYTHON_FUNCTION, SizeOfByteCode),

    FIELD_OFFSET(PYTHON_FUNCTION, Unused2),
    FIELD_OFFSET(PYTHON_FUNCTION, Unused3),
    FIELD_OFFSET(PYTHON_FUNCTION, Unused4),
};

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

    PrefixTable = &Python->ModuleNameTable;
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


_Success_(return != 0)
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

    ULONG StringBufferSize;
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

    if (!ARGUMENT_PRESENT(Python)) {
        return FALSE;
    }

    if (!ARGUMENT_PRESENT(Directory)) {
        return FALSE;
    }

    //
    // Non-root nodes must have a name and ancestor provided.
    //

    if (IsRoot) {

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

        AncestorModuleName = &AncestorEntry->ModuleName;

        NameLength = DirectoryName->Length;

        StringBufferSize = (
            AncestorModuleName->MaximumLength + // includes trailing NUL
            NameLength
        );

        if (!AncestorIsRoot) {

            //
            // Account for the joining slash + NUL.
            //

            StringBufferSize += 2;

        } else {

            //
            // Account for just the trailing NUL.
            //

            StringBufferSize += 1;
        }

        if (StringBufferSize > MAX_USTRING) {
            return FALSE;
        }

    }

    Rtl = Python->Rtl;

    if (!AllocatePythonPathTableEntry(Python, &Entry)) {
        return FALSE;
    }

    if (IsModule) {
        Entry->IsModuleDirectory = TRUE;
    } else {
        Entry->IsNonModuleDirectory = TRUE;
    }

    if (!IsRoot) {

        PSTR Dest;
        PSTR Source;
        USHORT Count;

        //
        // We verified that StringBufferSizes was < MAX_USTRING above.
        //

        USHORT Size = (USHORT)StringBufferSize;

        ModuleName = &Entry->ModuleName;

        if (!AllocateStringBuffer(Python, Size, ModuleName)) {
            FreePythonPathTableEntry(Python, Entry);
            return FALSE;
        }

        Name = &Entry->Name;

        if (!AncestorIsRoot) {
            Offset = AncestorModuleName->Length + 1;
        } else {
            Offset = 0;
        }

        ModuleName->Length = Offset + NameLength;

        if (ModuleName->MaximumLength <= ModuleName->Length) {

            //
            // There should be space for at least the trailing NUL.
            //

            __debugbreak();
        }

        Name->Length = NameLength;
        Name->MaximumLength = NameLength + 1;

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

    PrefixTable = &Python->PathTable->PrefixTable;
    PrefixTableEntry = (PPREFIX_TABLE_ENTRY)Entry;

    Success = Rtl->PfxInsertPrefix(PrefixTable,
                                   DirectoryPrefix,
                                   PrefixTableEntry);

    if (!Success) {
        FreeStringBuffer(Python, &Entry->ModuleName);
        FreePythonPathTableEntry(Python, Entry);
    } else if (ARGUMENT_PRESENT(EntryPointer)) {
        *EntryPointer = Entry;
    }

    return Success;
}

FORCEINLINE
BOOL
RegisterRoot(
    _In_      PPYTHON Python,
    _In_      PSTRING Directory,
    _Out_opt_ PPPYTHON_PATH_TABLE_ENTRY PathEntryPointer
    )
{
    return RegisterDirectory(Python,
                             Directory,
                             NULL,
                             NULL,
                             PathEntryPointer,
                             TRUE);
}

//
// This is currently identical to RegisterRoot().  In fact, we may deprecate
// RegisterRoot() in favor of this method, as it is more accurately named.
//

FORCEINLINE
BOOL
RegisterNonModuleDirectory(
    _In_      PPYTHON Python,
    _In_      PSTRING Directory,
    _Out_opt_ PPPYTHON_PATH_TABLE_ENTRY PathEntryPointer
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
BOOL
RegisterModuleDirectory(
    _In_      PPYTHON Python,
    _In_      PSTRING Directory,
    _In_      PSTRING DirectoryName,
    _In_      PPYTHON_PATH_TABLE_ENTRY AncestorEntry,
    _Out_opt_ PPPYTHON_PATH_TABLE_ENTRY PathEntryPointer
    )
{
    return RegisterDirectory(Python,
                             Directory,
                             DirectoryName,
                             AncestorEntry,
                             PathEntryPointer,
                             FALSE);
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

VOID
ResolveLineNumbersForPython2(
    _In_    PPYTHON             Python,
    _In_    PPYTHON_FUNCTION    Function
    )
{
    USHORT Index;
    USHORT Address;
    USHORT TableSize;
    USHORT LineNumber;
    USHORT SizeOfByteCode;
    USHORT NumberOfLines;
    USHORT FirstLineNumber;
    USHORT PreviousAddress;
    USHORT PreviousLineNumber;
    PPYSTRINGOBJECT ByteCodes;
    PLINE_NUMBER Table;
    PLINE_NUMBER_TABLE2 LineNumbers;
    PPYCODEOBJECT25_27 CodeObject;

    CodeObject = (PPYCODEOBJECT25_27)Function->CodeObject;
    ByteCodes = (PPYSTRINGOBJECT)CodeObject->Code;
    SizeOfByteCode = (USHORT)ByteCodes->ObjectSize;

    LineNumbers = (PLINE_NUMBER_TABLE2)CodeObject->LineNumberTable;

    //
    // The underlying ObjectSize will refer to the number of bytes; as each
    // entry is two bytes, shift right once.
    //

    TableSize = (USHORT)LineNumbers->ObjectSize >> 1;

    Address = 0;
    NumberOfLines = 0;
    PreviousAddress = 0;
    PreviousLineNumber = 0;
    FirstLineNumber = LineNumber = (USHORT)CodeObject->FirstLineNumber;

    for (Index = 0; Index < TableSize; Index++) {

        USHORT ByteIncrement;
        USHORT LineIncrement;

        Table = &LineNumbers->Table[Index];

        ByteIncrement = Table->ByteIncrement;
        LineIncrement = Table->LineIncrement;

        if (ByteIncrement) {
            if (LineNumber != PreviousLineNumber) {
                NumberOfLines++;
                PreviousLineNumber = LineNumber;
            }
        }

        if (LineIncrement) {
            if (Address != PreviousAddress) {
                PreviousAddress = Address;
            }
        }

        Address += ByteIncrement;
        LineNumber += LineIncrement;
    }

    if (LineNumber != PreviousLineNumber) {
        NumberOfLines++;
        PreviousLineNumber = LineNumber;
    }

    if (Address != PreviousAddress) {
        PreviousAddress = Address;
    }

    Function->FirstLineNumber = FirstLineNumber;
    Function->NumberOfLines = (
        PreviousLineNumber -
        FirstLineNumber
    );
    Function->SizeOfByteCode = SizeOfByteCode;
    Function->NumberOfCodeLines = NumberOfLines;

    //
    // Now that we know the total number of code lines and total number of
    // lines, we have enough information to create a line number bitmap,
    // a line-number-to-relative-index table, and a line-hit histogram.
    // All of which are an XXX todo.
    //

    return;
}

VOID
ResolveLineNumbers(
    _In_ PPYTHON Python,
    _In_ PPYTHON_FUNCTION Function
    )
{
    if (Python->MajorVersion == 2) {
        ResolveLineNumbersForPython2(Python, Function);
    } else {
        Function->FirstLineNumber = 0;
        Function->NumberOfLines = 0;
        Function->NumberOfCodeLines = 0;
        Function->SizeOfByteCode = 0;
    }
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
    PSTRING ParentModuleName;
    PSTRING ParentName;
    PPYTHON_PATH_TABLE_ENTRY PathEntry;
    PPYTHON_PATH_TABLE_ENTRY ParentPathEntry;

    PathEntry = (PPYTHON_PATH_TABLE_ENTRY)Function;
    ParentPathEntry = Function->ParentPathEntry;

    PathEntry->IsValid = FALSE;

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
    FunctionName = &PathEntry->Name;

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
        USHORT ClassNameLength;

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

        ClassNameLength = (USHORT)strlen((PCSZ)ClassNameBuffer);

        //
        // Ensure we've got a NUL-terminated string.
        //

        if (ClassNameBuffer[ClassNameLength] != '\0') {
            __debugbreak();
        }

        ClassName->Length = ClassNameLength;
        ClassName->MaximumLength = ClassNameLength;
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

    if (FullNameLength > MAX_STRING) {
        return FALSE;
    }

    FullNameAllocSize = ALIGN_UP_USHORT_TO_POINTER_SIZE(FullNameLength);

    if (FullNameAllocSize > MAX_STRING) {
        FullNameAllocSize = (USHORT)MAX_STRING;
    }

    //
    // Construct the final full name.  After each part has been copied, update
    // the corresponding Buffer pointer to the relevant point within the newly-
    // allocated buffer for the full name.
    //

    FullName = &PathEntry->FullName;

    Success = AllocateStringBuffer(Python, FullNameAllocSize, FullName);
    if (!Success) {
        return FALSE;
    }

    Dest = FullName->Buffer;

    __movsb(Dest, (PBYTE)ModuleName->Buffer, ModuleName->Length);
    Dest += ModuleName->Length;
    ModuleName->Buffer = FullName->Buffer;

    *Dest++ = '\\';

    if (ParentName->Length) {
        __movsb(Dest, (PBYTE)ParentName->Buffer, ParentName->Length);
        Dest += ParentName->Length;
        *Dest++ = '\\';

        //
        // If the parent is a file, update the module name to account
        // for the parent's module name, plus the joining slash.
        //

        if (ParentPathEntry->IsFile) {
            ModuleName->Length += ParentName->Length + 1;
            ModuleName->MaximumLength = ModuleName->Length;
        }
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

    FullName->Length = FullNameLength - 1;
    FullName->MaximumLength = FullNameAllocSize;

    //
    // Point our path at our parent.
    //

    Path = &PathEntry->Path;
    Path->Length = ParentPathEntry->Path.Length;
    Path->MaximumLength = Path->Length;
    Path->Buffer = ParentPathEntry->Path.Buffer;

    PathEntry->IsFunction = TRUE;

    ResolveLineNumbers(Python, Function);

    PathEntry->IsValid = TRUE;

    //
    // Calculate the code object's hash.
    //

    Function->CodeObjectHash = Python->PyObject_Hash(CodeObject);

    //
    // Hash the strings.
    //

    HashString(Python, &PathEntry->Name);
    HashString(Python, &PathEntry->Path);
    HashString(Python, &PathEntry->FullName);
    HashString(Python, &PathEntry->ModuleName);

    if (ClassName->Length) {
        HashString(Python, &PathEntry->ClassName);
    } else {
        PathEntry->ClassName.Hash = 0;
    }

    //
    // Calculate a final hash value.
    //

    Function->FunctionHash = (
        PathEntry->PathEntryType    ^
        PathEntry->PathHash         ^
        PathEntry->FullNameHash     ^
        Function->CodeObjectHash    ^
        Function->NumberOfCodeLines
    );

    //
    // Initialize the reference count.
    //

    Function->ReferenceCount = 1;

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
    BOOL Success;
    PRTL Rtl;

    CONST ULARGE_INTEGER MaxSize = { MAX_USTRING - 2 };

    ULONG CurDirLength;

    PSTRING String;
    PCHAR Dest;

    ULONG Remaining;
    USHORT NewLength;

    CHAR Buffer[_MAX_PATH];
    CHAR CanonPath[_MAX_PATH];

    if (!ARGUMENT_PRESENT(Python)) {
        return FALSE;
    }

    if (!ARGUMENT_PRESENT(SourcePath)) {
        return FALSE;
    }

    if (!ARGUMENT_PRESENT(DestinationPathPointer)) {
        return FALSE;
    }

    Rtl = Python->Rtl;

    Dest = (PCHAR)Buffer;
    CurDirLength = GetCurrentDirectoryA(_MAX_PATH, Dest);

    //
    // Initial validation of the directory length.
    //

    if (CurDirLength == 0) {

        return FALSE;

    } else if (CurDirLength > MaxSize.LowPart) {

        return FALSE;
    }

    //
    // Update the destination pointer.
    //

    Dest += CurDirLength;

    //
    // Append the joining slash and update the length if necessary.
    //

    if (*Dest != '\\') {
        *Dest++ = '\\';
        CurDirLength += 1;
    }

    //
    // Calculate remaining space (account for trailing NUL).
    //

    Remaining = _MAX_PATH - CurDirLength - 1;

    if (Remaining < SourcePath->Length) {

        //
        // Not enough space left for our source path to be appended.
        //

        return FALSE;

    }

    //
    // There's enough space, copy the source path over.
    //

    __movsb(Dest, SourcePath->Buffer, SourcePath->Length);

    Dest += SourcePath->Length;

    //
    // Add terminating NUL.
    //

    *Dest = '\0';

    //
    // Our temporary buffer now contains a concatenated current directory name
    // and source path plus trailing NUL.  Call PathCanonicalize() against it
    // and store the results in another stack-allocated _MAX_PATH-sized temp
    // buffer.
    //

    Success = Rtl->PathCanonicalizeA((PSTR)&CanonPath, (PSTR)&Buffer);

    if (!Success) {
        return FALSE;
    }

    //
    // Get the new length for the canonicalized path.
    //

    NewLength = (USHORT)strlen((PCSTR)&CanonPath);

    //
    // Sanity check that there is still a trailing NUL where we expect.
    //

    if (CanonPath[NewLength] != '\0') {
        __debugbreak();
    }

    //
    // Now allocate the string.
    //

    Success = AllocateStringAndBuffer(Python, NewLength, &String);
    if (!Success) {
        return FALSE;
    }

    //
    // Initialize sizes.  NewLength includes trailing NUL, so omit it for
    // String->Length.
    //

    String->Length = NewLength-1;

    if (String->MaximumLength <= String->Length) {
        __debugbreak();
    }

    //
    // And finally, copy over the canonicalized path.  We've already verified
    // that CanonPath is NUL terminated, so we can just use NewLength here to
    // pick up the terminating NUL from CanonPath.
    //

    __movsb((PBYTE)String->Buffer, (LPCBYTE)&CanonPath[0], NewLength);

    //
    // Update the caller's pointer and return.
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

    PRTL_BITMAP Bitmap;

    PPREFIX_TABLE PrefixTable;
    PPREFIX_TABLE_ENTRY PrefixTableEntry;

    PPYTHON_PATH_TABLE_ENTRY PathEntry = NULL;
    PPYTHON_PATH_TABLE_ENTRY RootEntry = NULL;
    PPYTHON_PATH_TABLE_ENTRY NextEntry = NULL;
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

    PSTRING RootPrefix;
    PSTRING ParentPrefix;
    PSTRING AncestorPrefix;
    PSTRING PreviousPrefix;

    PUSHORT ForwardHintIndex;

    USHORT Offset;
    USHORT ForwardIndex;
    USHORT ReversedIndex;
    USHORT NumberOfChars;
    USHORT LastForwardIndex;
    USHORT LastReversedIndex;
    USHORT CumulativeForwardIndex;
    USHORT CumulativeReversedIndex;
    USHORT RemainingAncestors;

    BOOL IsModule = FALSE;
    BOOL IsRoot = FALSE;

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

    PrefixTable = &Python->PathTable->PrefixTable;

    //
    // Search for the directory in the prefix table.
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
        // parent directory, we may revise this to be an ancestor entry.
        //

        ParentEntry = PathEntry;
        PathEntry = NULL;

    } else {

        //
        // No path entry was present.  If the directory *isn't* a module, it
        // becomes our root directory.  If it is a module, we let execution
        // fall through to the normal parent/ancestor directory processing
        // logic below.
        //

        Success = IsModuleDirectoryA(Rtl, Directory, &IsModule);

        if (!Success) {

            //
            // Treat failure equivalent to "IsModule = FALSE".  See comment
            // in while loop below for more explanation.
            //

            IsModule = FALSE;

        }

        if (!IsModule) {

            //
            // Register this directory as the root and return.
            //

            return RegisterRoot(Python, Directory, PathEntryPointer);

        }

    }

    //
    // If we get here, we will be in one of the following states:
    //
    //  1.  An ancestor path (i.e. a prefix match) of Directory already exists
    //      in the prefix tree.
    //
    //  2.  No common ancestor exists, and the Directory has an __init__.py
    //      file, indicating that it is a module.
    //
    // In either case, we need to ensure the relevant ancestor directories
    // have been added, up to the "root" directory (i.e. the first directory
    // we find that has no __init__.py file present).
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

            Success = RegisterRoot(Python, RootPrefix, &RootEntry);

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
    // not the parent directory.  AncestorEntry will be set here to the entry
    // for the initial matching ancestor path entry.  ParentDirectory will
    // still represent the target parent directory we need to add ancestor
    // entries up-to (but not including).
    //

    //
    // Re-use our reversed bitmap to create a forward bitmap of backslashes.
    //

    AncestorPrefix = AncestorEntry->Prefix;
    NextName.Length = ParentDirectory.Length - AncestorPrefix->Length - 1;
    NextName.MaximumLength = NextName.Length;
    NextName.Buffer = ParentDirectory.Buffer + AncestorPrefix->Length + 1;

    //
    // Truncate our existing bitmap to an aligned size matching the number of
    // remaining characters to scan through.
    //

    Bitmap = Backslashes;
    Bitmap->SizeOfBitMap = ALIGN_UP_USHORT_TO_POINTER_SIZE(NextName.Length);
    Rtl->RtlClearAllBits(Bitmap);

    InlineFindCharsInString(&NextName, '\\', Bitmap);

    //
    // Add one to account for the fact that our Remaining.Length omits the
    // last trailing backslash from the path.
    //

    RemainingAncestors = (USHORT)Rtl->RtlNumberOfSetBits(Bitmap) + 1;

    if (RemainingAncestors == 1) {

        //
        // We don't need to consult the bitmap at all if there is only one
        // ancestor directory remaining; the NextName string we prepared
        // above has all the details we need.
        //

        NextDirectory.Length = AncestorPrefix->Length + NextName.Length + 1;
        NextDirectory.MaximumLength = NextDirectory.Length;

    } else {

        if (RemainingAncestors == 0) {

            //
            // Shouldn't be possible.
            //

            __debugbreak();
        }

        //
        // As we have multiple ancestors to process, use the bitmap to find
        // out where the first one ends.
        //

        ForwardIndex = (USHORT)Rtl->RtlFindSetBits(Bitmap, 1, 0);
        ForwardHintIndex = &ForwardIndex;
        CumulativeForwardIndex = ForwardIndex;

        NextName.Length = ForwardIndex;
        NextName.MaximumLength = NextName.Length;

        NextDirectory.Length = AncestorPrefix->Length + ForwardIndex + 1;
        NextDirectory.MaximumLength = NextDirectory.Length;

    }

    NextDirectory.Buffer = ParentDirectory.Buffer;

    do {

        //
        // Use the same logic as above with regards to handling the failure
        // of IsModuleDirectory (i.e. treat it as if it were IsModule = FALSE).
        //

        Success = IsModuleDirectoryA(Rtl, &NextDirectory, &IsModule);

        if (!Success) {
            IsModule = FALSE;
        }

        if (!IsModule) {

            Success = RegisterNonModuleDirectory(Python,
                                                 &NextDirectory,
                                                 &NextEntry);

        } else {

            Success = RegisterModuleDirectory(Python,
                                              &NextDirectory,
                                              &NextName,
                                              AncestorEntry,
                                              &NextEntry);

        }

        if (!Success) {
            return FALSE;
        }

        //
        // See if that was the last ancestor directory we need to add.
        //

        if (!--RemainingAncestors) {

            //
            // Invariant check: if there are no more ancestors, the length of
            // the path prefix just added should match the length of our parent
            // directory.
            //

            if (NextEntry->Prefix->Length != ParentDirectory.Length) {
                __debugbreak();
            }

            //
            // This ancestor is our parent entry, continue to the final step.
            //

            ParentEntry = NextEntry;

            goto FoundParent;

        }

        //
        // There are still ancestor paths remaining.
        //

        if (RemainingAncestors == 1) {

            NextName.Length = (
                ParentDirectory.Length -
                NextDirectory.Length   -
                1
            );

            NextName.Buffer += (ForwardIndex + 1);

            NextDirectory.Length += (NextName.Length + 1);

        } else {

            LastForwardIndex = (*ForwardHintIndex)++;

            CumulativeForwardIndex += LastForwardIndex;

            ForwardIndex = (USHORT)(
                Rtl->RtlFindSetBits(Bitmap, 1,
                                    *ForwardHintIndex)
            );

            NextName.Length = CumulativeForwardIndex - ForwardIndex;
            NextName.Buffer += ForwardIndex;
            NextDirectory.Length += ForwardIndex;
        }

        if (ForwardIndex == BITS_NOT_FOUND ||
            ForwardIndex == LastForwardIndex) {

            //
            // Should never happen.
            //

            __debugbreak();

        }

        //
        // Isolate the name portion of the next ancestor.
        //

        NextName.MaximumLength = NextName.Length;

        //
        // Update the directory length.
        //

        NextDirectory.MaximumLength = NextDirectory.Length;

        //
        // Continue the loop.
        //

    } while (1);

FoundParent:

    //
    // Add a new entry for the parent directory.
    //

    Success = IsModuleDirectoryA(Rtl, Directory, &IsModule);
    if (!Success) {
        IsModule = FALSE;
    }

    IsRoot = (IsModule ? FALSE : TRUE);

    Success = RegisterDirectory(Python,
                                Directory,
                                &DirectoryName,
                                ParentEntry,
                                &PathEntry,
                                IsRoot);

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

    HANDLE HeapHandle = NULL;

    PRTL Rtl;
    PPYOBJECT CodeObject;
    PPYOBJECT FilenameObject;
    PPREFIX_TABLE PrefixTable;
    PPREFIX_TABLE_ENTRY PrefixTableEntry;
    PPYTHON_PATH_TABLE_ENTRY PathEntry;
    PPYTHON_PATH_TABLE_ENTRY DirectoryEntry;
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
    USHORT ExpectedMaximumLength;

    CHAR StackBitmapBuffer[_MAX_FNAME >> 3];
    RTL_BITMAP Bitmap = { _MAX_FNAME, (PULONG)&StackBitmapBuffer };
    BitmapPointer = &Bitmap;

    if (!ARGUMENT_PRESENT(Python)) {
        return FALSE;
    }

    if (!ARGUMENT_PRESENT(QualifiedPath)) {
        return FALSE;
    }

    if (!ARGUMENT_PRESENT(FrameObject)) {
        return FALSE;
    }

    if (!ARGUMENT_PRESENT(PathEntryPointer)) {
        return FALSE;
    }

    CodeObject = FrameObject->Code;

    FilenameObject = *(
        (PPPYOBJECT)RtlOffsetToPointer(
            CodeObject,
            Python->PyCodeObjectOffsets->Filename
        )
    );

    Success = WrapPythonFilenameStringAsString(
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

    WeOwnPathBuffer = (Path->Buffer != PathString.Buffer);

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
    // Invariant check: if the directory's module name length is 0, verify
    // the corresponding path entry indicates it is a non-module directory.
    //

    if (ModuleLength == 0) {
        if (!DirectoryEntry->IsNonModuleDirectory) {
            __debugbreak();
        }
    }

    if (!WeOwnPathBuffer) {

        //
        // Account for path length plus trailing NUL.
        //

        PathAllocSize = ALIGN_UP_USHORT_TO_POINTER_SIZE(Path->Length + 1);

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

    //
    // Calculate the length.  The first sizeof(CHAR) accounts for the joining
    // backslash if there's a module name, the second sizeof(CHAR) accounts
    // for the terminating NUL.
    //

    FullNameLength = (
        (ModuleLength ? ModuleLength + sizeof(CHAR) : 0)    +
        Filename.Length                                     +
        sizeof(CHAR)
    );

    FullNameAllocSize = FullNameLength;

    Success = AllocatePythonPathTableEntry(Python, &PathEntry);
    if (!Success) {
        goto Error;
    }

    PathEntry->IsFile = TRUE;

    Path = &PathEntry->Path;
    FullName = &PathEntry->FullName;

    //
    // Allocate the full name string and the path string if we don't already
    // own it.
    //

    if (!AllocateStringBuffer(Python, FullNameAllocSize, FullName)) {
        FreePythonPathTableEntry(Python, PathEntry);
        goto Error;
    }

    if (!WeOwnPathBuffer) {

        if (!AllocateStringBuffer(Python, PathAllocSize, Path)) {
            FreeStringBuffer(Python, FullName);
            FreePythonPathTableEntry(Python, PathEntry);
            goto Error;
        }

    } else {

        //
        // Re-use the qualified path's details.
        //

        Path->MaximumLength = QualifiedPath->MaximumLength;
        Path->Buffer = QualifiedPath->Buffer;
    }

    Path->Length = QualifiedPath->Length;

    ExpectedMaximumLength = (
        PathAllocSize ? PathAllocSize :
                        QualifiedPath->MaximumLength
    );

    if (Path->MaximumLength < ExpectedMaximumLength) {
        __debugbreak();
    }

    //
    // Initialize shortcut pointers.
    //

    Name = &PathEntry->Name;
    ModuleName = &PathEntry->ModuleName;

    //
    // Update the lengths of our strings.
    //

    FullName->Length = FullNameLength - 1; // exclude trailing NUL

    if (FullName->MaximumLength <= FullName->Length) {
        __debugbreak();
    }

    Name->Length = Filename.Length;
    Name->MaximumLength = Filename.MaximumLength;

    ModuleName->Length = ModuleLength;
    ModuleName->MaximumLength = ModuleLength;
    ModuleName->Buffer = ModuleBuffer;

    if (!WeOwnPathBuffer) {

        //
        // If we didn't own the incoming path's buffer, copy it over.
        //

        __movsb((PBYTE)Path->Buffer,
                (PBYTE)QualifiedPath->Buffer,
                Path->Length);

        //
        // Add trailing NUL.
        //

        Path->Buffer[Path->Length] = '\0';

    }

    //
    // Add the newly created PathEntry to our path table prefix tree.
    //

    PrefixTable = &Python->PathTable->PrefixTable;
    PrefixTableEntry = (PPREFIX_TABLE_ENTRY)PathEntry;

    Success = Rtl->PfxInsertPrefix(PrefixTable,
                                   Path,
                                   PrefixTableEntry);

    if (!Success) {

        FreeStringBuffer(Python, FullName);
        FreePythonPathTableEntry(Python, PathEntry);

        if (WeOwnPathBuffer) {
            FreeStringAndBuffer(Python, QualifiedPath);
        } else {
            FreeStringBuffer(Python, Path);
        }

        goto Error;
    }


    //
    // Construct the final full name.  After each part has been copied, update
    // the corresponding Buffer pointer to the relevant point within the newly-
    // allocated buffer for the full name.
    //

    Dest = FullName->Buffer;

    //
    // Copy module name if applicable.
    //

    if (ModuleName->Length) {
        __movsb(Dest, (PBYTE)ModuleName->Buffer, ModuleName->Length);
        Dest += ModuleName->Length;
        ModuleName->Buffer = FullName->Buffer;

        //
        // Add joining slash.
        //

        *Dest++ = '\\';
    }

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

    if (!Success) {
        PathEntry = NULL;
    }

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

    Success = WrapPythonFilenameStringAsString(
        Python,
        FilenameObject,
        Path
    );

    if (!Success) {
        return FALSE;
    }

    PrefixTable = &Python->PathTable->PrefixTable;
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

    FunctionTable = Python->FunctionTable;

    Function = Rtl->RtlInsertElementGenericTable(
        &FunctionTable->GenericTable,
        &FunctionRecord,
        sizeof(FunctionRecord),
        &NewFunction
    );

    if (!NewFunction) {

        //
        // We've already seen this function.  Increment the reference count.
        //

        Function->ReferenceCount++;
        goto End;
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

End:
    if (ARGUMENT_PRESENT(FunctionPointer)) {
        *FunctionPointer = Function;
    }

    return IsValidFunction(Function);
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
