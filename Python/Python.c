
#include "stdafx.h"
#include <Windows.h>
#include "Python.h"
#include "../Tracer/Tracing.h"

static CONST UNICODE_STRING __init__py  = RTL_CONSTANT_STRING(L"__init__.py");
static CONST UNICODE_STRING __init__pyc = RTL_CONSTANT_STRING(L"__init__.pyc");
static CONST UNICODE_STRING __init__pyo = RTL_CONSTANT_STRING(L"__init__.pyo");

static CONST PUNICODE_STRING InitPyFiles[] = {
    (PUNICODE_STRING)&__init__py,
    (PUNICODE_STRING)&__init__pyc,
    (PUNICODE_STRING)&__init__pyo
};

static CONST USHORT NumberOfInitPyFiles = 3;
static CONST USHORT MaxInitPyFileLengthInBytes = 13; // Exclude trailing NUL.

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

    PythonData->PyCode_Type = (PPYOBJECT)GetProcAddress(PythonModule, "PyCode_Type");
    if (!PythonData->PyCode_Type) {
        goto error;
    }

    PythonData->PyDict_Type = (PPYOBJECT)GetProcAddress(PythonModule, "PyDict_Type");
    if (!PythonData->PyDict_Type) {
        goto error;
    }

    PythonData->PyTuple_Type = (PPYOBJECT)GetProcAddress(PythonModule, "PyTuple_Type");
    if (!PythonData->PyTuple_Type) {
        goto error;
    }

    PythonData->PyType_Type = (PPYOBJECT)GetProcAddress(PythonModule, "PyType_Type");
    if (!PythonData->PyType_Type) {
        goto error;
    }

    PythonData->PyFunction_Type = (PPYOBJECT)GetProcAddress(PythonModule, "PyFunction_Type");
    if (!PythonData->PyFunction_Type) {
        goto error;
    }

    PythonData->PyString_Type = (PPYOBJECT)GetProcAddress(PythonModule, "PyString_Type");
    if (!PythonData->PyString_Type) {
        PythonData->PyBytes_Type = (PPYOBJECT)GetProcAddress(PythonModule, "PyBytes_Type");
        if (!PythonData->PyBytes_Type) {
            goto error;
        }
    }

    PythonData->PyUnicode_Type = (PPYOBJECT)GetProcAddress(PythonModule, "PyUnicode_Type");
    if (!PythonData->PyUnicode_Type) {
        goto error;
    }

    PythonData->PyCFunction_Type = (PPYOBJECT)GetProcAddress(PythonModule, "PyCFunction_Type");
    if (!PythonData->PyCFunction_Type) {
        goto error;
    }

    PythonData->PyInstance_Type = (PPYOBJECT)GetProcAddress(PythonModule, "PyInstance_Type");
    if (!PythonData->PyInstance_Type) {
        goto error;
    }

    PythonData->PyModule_Type = (PPYOBJECT)GetProcAddress(PythonModule, "PyModule_Type");
    if (!PythonData->PyModule_Type) {
        goto error;
    }

    return TRUE;

error:
    return FALSE;
}

_Check_return_
BOOL
LoadPythonFunctions(
    _In_    HMODULE             PythonModule,
    _Inout_ PPYTHONFUNCTIONS    PythonFunctions
)
{
    if (!PythonModule) {
        return FALSE;
    }

    if (!PythonFunctions) {
        return FALSE;
    }

    PythonFunctions->PyFrame_GetLineNumber = (PPYFRAME_GETLINENUMBER)GetProcAddress(PythonModule, "PyFrame_GetLineNumber");
    if (!PythonFunctions->PyFrame_GetLineNumber) {
        goto error;
    }

    PythonFunctions->PyEval_SetProfile = (PPYEVAL_SETPROFILE)GetProcAddress(PythonModule, "PyEval_SetProfile");
    if (!PythonFunctions->PyEval_SetProfile) {
        goto error;
    }

    PythonFunctions->PyEval_SetTrace = (PPYEVAL_SETTRACE)GetProcAddress(PythonModule, "PyEval_SetTrace");
    if (!PythonFunctions->PyEval_SetTrace) {
        goto error;
    }

    PythonFunctions->PyDict_GetItemString = (PPYDICT_GETITEMSTRING)GetProcAddress(PythonModule, "PyDict_GetItemString");
    if (!PythonFunctions->PyDict_GetItemString) {
        goto error;
    }

    PythonFunctions->Py_IncRef = (PPY_INCREF)GetProcAddress(PythonModule, "Py_IncRef");
    if (!PythonFunctions->Py_IncRef) {
        goto error;
    }

    PythonFunctions->Py_DecRef = (PPY_DECREF)GetProcAddress(PythonModule, "Py_DecRef");
    if (!PythonFunctions->Py_DecRef) {
        goto error;
    }

    PythonFunctions->PyGILState_Ensure = (PPYGILSTATE_ENSURE)GetProcAddress(PythonModule, "PyGILState_Ensure");
    if (!PythonFunctions->PyGILState_Ensure) {
        goto error;
    }

    PythonFunctions->PyGILState_Release = (PPYGILSTATE_RELEASE)GetProcAddress(PythonModule, "PyGILState_Release");
    if (!PythonFunctions->PyGILState_Release) {
        goto error;
    }

    PythonFunctions->PyUnicode_AsUnicode = (PPYUNICODE_ASUNICODE)GetProcAddress(PythonModule, "PyUnicode_AsUnicode");
    PythonFunctions->PyUnicode_GetLength = (PPYUNICODE_GETLENGTH)GetProcAddress(PythonModule, "PyUnicode_GetLength");

    return TRUE;

error:
    return FALSE;
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

    PythonExFunctions->GetUnicodeLengthForPythonString = (PGETUNICODELENGTHFORPYTHONSTRING)GetProcAddress(PythonExModule, "GetUnicodeLengthForPythonString");

    PythonExFunctions->ConvertPythonStringToUnicodeString = (PCONVERTPYSTRINGTOUNICODESTRING)GetProcAddress(PythonExModule, "ConvertPythonStringToUnicodeString");

    PythonExFunctions->CopyPythonStringToUnicodeString = (PCOPY_PYTHON_STRING_TO_UNICODE_STRING)GetProcAddress(PythonExModule, "CopyPythonStringToUnicodeString");

    PythonExFunctions->ResolveFrameObjectDetails = (PRESOLVEFRAMEOBJECTDETAILS)GetProcAddress(PythonExModule, "ResolveFrameObjectDetails");

    PythonExFunctions->ResolveFrameObjectDetailsFast = (PRESOLVEFRAMEOBJECTDETAILS)GetProcAddress(PythonExModule, "ResolveFrameObjectDetailsFast");

    PythonExFunctions->GetModuleNameAndQualifiedPathFromModuleFilename = (PGET_MODULE_NAME_AND_QUALIFIED_PATH_FROM_MODULE_FILENAME)GetProcAddress(PythonExModule, "GetModuleNameAndQualifiedPathFromModuleFilename");

    PythonExFunctions->AddDirectoryEntry = (PADD_DIRECTORY_ENTRY)GetProcAddress(PythonExModule, "AddDirectoryEntry");

    return TRUE;
}

_Check_return_
BOOL
LoadPythonExRuntime(
    _In_opt_    HMODULE PythonExModule,
    _Inout_     PPYTHONEXRUNTIME PythonExRuntime
)
{
    PPYTHON Python;
    PRTL Rtl;
    USHORT Offset;
    DWORD HeapFlags;
    PUNICODE_PREFIX_TABLE PrefixTable;

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

    Offset = FIELD_OFFSET(PYTHON, PythonExRuntime); 
    Python = (PPYTHON)(RtlOffsetFromPointer(PythonExRuntime, Offset));
    Rtl = Python->Rtl;

    PrefixTable = &PythonExRuntime->DirectoryPrefixTable.PrefixTable;
    Rtl->RtlInitializeUnicodePrefix(PrefixTable);

    return TRUE;
}

_Check_return_
BOOL
LoadPythonExSymbols(
    _In_opt_    HMODULE PythonExModule,
    _Inout_     PPYTHON Python
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

        if (!GetModuleHandleEx(Flags, (LPCTSTR)&LoadPythonExFunctions, &Module)) {
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
        (Python->MajorVersion == 2 && (Python->MinorVersion >= 4 && Python->MinorVersion <= 7)) ||
        (Python->MajorVersion == 3 && (Python->MinorVersion >= 0 && Python->MinorVersion <= 5))
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

    Python->Py_GetVersion = (PPY_GETVERSION)GetProcAddress(PythonModule, "Py_GetVersion");
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
                Python->PatchLevel = (USHORT)((Python->PatchLevel * 10) + PatchLevel);
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
    _In_        PRTL        Rtl,
    _In_        HMODULE     PythonModule,
    _Out_       PPYTHON     Python,
    _Inout_     PULONG      SizeOfPython
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
    return TRUE;

error:
    // Clear any partial state.
    SecureZeroMemory(Python, sizeof(*Python));
    return FALSE;
}

BOOL
GetUnicodeLengthForPythonString(
    _In_    PPYTHON         Python,
    _In_    PPYOBJECT       StringOrUnicodeObject,
    _Out_   PULONG          UnicodeLength
)
{
    if (!Python) {
        return FALSE;
    }

    if (!StringOrUnicodeObject) {
        return FALSE;
    }

    if (!UnicodeLength) {
        return FALSE;
    }

    if (StringOrUnicodeObject->TypeObject == Python->PyString_Type) {


    } else if (StringOrUnicodeObject->TypeObject == Python->PyUnicode_Type) {

    } else {
        return FALSE;
    }

    return FALSE;
}

_Check_return_
BOOL
ConvertPythonStringToUnicodeString(
    _In_    PPYTHON             Python,
    _In_    PPYOBJECT           StringOrUnicodeObject,
    _Out_   PPUNICODE_STRING    UnicodeString,
    _In_    BOOL                AllocateMaximumSize
)
{
    PUNICODE_STRING String;
    LARGE_INTEGER RequiredSizeInBytes;
    LARGE_INTEGER AllocationSizeInBytes;

    if (!Python) {
        return FALSE;
    }

    if (!StringOrUnicodeObject) {
        return FALSE;
    }

    if (!UnicodeString) {
        return FALSE;
    }

    if (StringOrUnicodeObject->TypeObject == Python->PyString_Type) {
        ULONG Index;
        PPYSTRINGOBJECT StringObject = (PPYSTRINGOBJECT)StringOrUnicodeObject;
        RequiredSizeInBytes.QuadPart = StringObject->ObjectSize * sizeof(WCHAR);
        if (RequiredSizeInBytes.QuadPart > (MAX_USTRING-2-sizeof(UNICODE_STRING))) {
            return FALSE;
        }

        if (RequiredSizeInBytes.HighPart != 0) {
            return FALSE;
        }

        if (AllocateMaximumSize) {
            AllocationSizeInBytes.QuadPart = MAX_USTRING - 2;
        } else {
            AllocationSizeInBytes.QuadPart = RequiredSizeInBytes.LowPart + sizeof(UNICODE_STRING);
        }

        if (AllocationSizeInBytes.HighPart != 0) {
            return FALSE;
        }


        String = (PUNICODE_STRING)HeapAlloc(Python->HeapHandle,
                                            HEAP_ZERO_MEMORY,
                                            AllocationSizeInBytes.LowPart);
        if (!String) {
            return FALSE;
        }

        String->Length = (USHORT)RequiredSizeInBytes.LowPart;
        String->MaximumLength = (USHORT)AllocationSizeInBytes.LowPart-sizeof(UNICODE_STRING);
        String->Buffer = (PWSTR)RtlOffsetToPointer(String, sizeof(UNICODE_STRING));

        for (Index = 0; Index < StringObject->ObjectSize; Index++) {
            String->Buffer[Index] = (WCHAR)StringObject->Value[Index];
        }

        *UnicodeString = String;
        return TRUE;

    } else if (StringOrUnicodeObject->TypeObject == Python->PyUnicode_Type) {

        if (Python->PyUnicode_AsUnicode && Python->PyUnicode_GetLength) {

        }

        return FALSE;

    } else {
        return FALSE;
    }

    return FALSE;

}

BOOL
GetPythonStringInformation(
    _In_     PPYTHON             Python,
    _In_     PPYOBJECT           StringOrUnicodeObject,
    _Out_    PSIZE_T             Length,
    _Out_    PUSHORT             Width,
    _Out_    PPVOID              Buffer
)
{
    if (StringOrUnicodeObject->TypeObject == Python->PyString_Type) {

        PPYSTRINGOBJECT StringObject = (PPYSTRINGOBJECT)StringOrUnicodeObject;

        *Length = StringObject->ObjectSize;
        *Buffer = StringObject->Value;

        *Width = sizeof(CHAR);

    } else if (StringOrUnicodeObject->TypeObject == Python->PyUnicode_Type) {

        PPYUNICODEOBJECT UnicodeObject = (PPYUNICODEOBJECT)StringOrUnicodeObject;

        if (Python->PyUnicode_AsUnicode && Python->PyUnicode_GetLength) {

            *Length = Python->PyUnicode_GetLength(StringOrUnicodeObject);
            *Buffer = Python->PyUnicode_AsUnicode(StringOrUnicodeObject);

        } else {

            *Length = UnicodeObject->Length;
            *Buffer = UnicodeObject->String;

        }

        *Width = sizeof(WCHAR);

    } else {

        return FALSE;

    }

    return TRUE;
}

BOOL
CopyPythonStringToUnicodeString(
    _In_     PPYTHON             Python,
    _In_     PPYOBJECT           StringOrUnicodeObject,
    _Inout_  PPUNICODE_STRING    UnicodeString,
    _In_opt_ USHORT              AllocationSize,
    _In_     PALLOCATION_ROUTINE AllocationRoutine,
    _In_opt_ PVOID               AllocationContext
)
{
    PRTL Rtl;
    PUNICODE_STRING String;
    LARGE_INTEGER RequiredSizeInBytes;
    LARGE_INTEGER AllocationSizeInBytes;

    if (!Python) {
        return FALSE;
    }

    if (!StringOrUnicodeObject) {
        return FALSE;
    }

    if (!UnicodeString) {
        return FALSE;
    }

    Rtl = Python->Rtl;

    if (!Rtl) {
        return FALSE;
    }

    if (StringOrUnicodeObject->TypeObject == Python->PyString_Type) {
        ULONG Index;
        PPYSTRINGOBJECT StringObject = (PPYSTRINGOBJECT)StringOrUnicodeObject;
        RequiredSizeInBytes.QuadPart = StringObject->ObjectSize * sizeof(WCHAR);

        if (RequiredSizeInBytes.QuadPart > (MAX_USTRING-2-sizeof(UNICODE_STRING))) {
            return FALSE;
        }

        if (RequiredSizeInBytes.HighPart != 0) {
            return FALSE;
        }

        if (AllocationSize && (AllocationSize > (RequiredSizeInBytes.LowPart + sizeof(UNICODE_STRING)))) {

            AllocationSizeInBytes.QuadPart = AllocationSize + sizeof(UNICODE_STRING);

        } else {

            AllocationSizeInBytes.QuadPart = RequiredSizeInBytes.LowPart + sizeof(UNICODE_STRING);
        }

        if (AllocationSizeInBytes.QuadPart > (MAX_USTRING-2-sizeof(UNICODE_STRING))) {
            return FALSE;
        }

        if (AllocationSizeInBytes.HighPart != 0) {
            return FALSE;
        }

        String = (PUNICODE_STRING)AllocationRoutine(
            AllocationContext,
            AllocationSizeInBytes.LowPart
        );

        if (!String) {
            return FALSE;
        }

        String->Length = (USHORT)RequiredSizeInBytes.LowPart;
        String->MaximumLength = (USHORT)AllocationSizeInBytes.LowPart-sizeof(UNICODE_STRING);
        String->Buffer = (PWSTR)RtlOffsetToPointer(String, sizeof(UNICODE_STRING));

        for (Index = 0; Index < StringObject->ObjectSize; Index++) {
            String->Buffer[Index] = (WCHAR)StringObject->Value[Index];
        }

        *UnicodeString = String;
        return TRUE;

    } else if (StringOrUnicodeObject->TypeObject == Python->PyUnicode_Type) {

        PPYUNICODEOBJECT UnicodeObject = (PPYUNICODEOBJECT)StringOrUnicodeObject;
        UNICODE_STRING Source;

        if (Python->PyUnicode_AsUnicode && Python->PyUnicode_GetLength) {

            RequiredSizeInBytes.QuadPart = Python->PyUnicode_GetLength(StringOrUnicodeObject) * sizeof(WCHAR);
            Source.Buffer = Python->PyUnicode_AsUnicode(StringOrUnicodeObject);

        } else {

            RequiredSizeInBytes.QuadPart = UnicodeObject->Length * sizeof(WCHAR);
            Source.Buffer = UnicodeObject->String;
        }

        if (RequiredSizeInBytes.QuadPart > (MAX_USTRING-2-sizeof(UNICODE_STRING))) {
            return FALSE;
        }

        if (RequiredSizeInBytes.HighPart != 0) {
            return FALSE;
        }

        Source.Length = (USHORT)RequiredSizeInBytes.LowPart;
        Source.MaximumLength = Source.Length;

        if (AllocationSize && (AllocationSize > (RequiredSizeInBytes.LowPart + sizeof(UNICODE_STRING)))) {

            AllocationSizeInBytes.QuadPart = AllocationSize + sizeof(UNICODE_STRING);

        } else {

            AllocationSizeInBytes.QuadPart = RequiredSizeInBytes.LowPart + sizeof(UNICODE_STRING);
        }


        if (AllocationSizeInBytes.QuadPart > (MAX_USTRING-2-sizeof(UNICODE_STRING))) {
            return FALSE;
        }

        if (AllocationSizeInBytes.HighPart != 0) {
            return FALSE;
        }

        String = (PUNICODE_STRING)AllocationRoutine(
            AllocationContext,
            AllocationSizeInBytes.LowPart
        );

        if (!String) {
            return FALSE;
        }

        String->Length = Source.Length;
        String->MaximumLength = (USHORT)AllocationSizeInBytes.LowPart - sizeof(UNICODE_STRING);
        String->Buffer = (PWSTR)RtlOffsetToPointer(String, sizeof(UNICODE_STRING));
        Rtl->RtlCopyUnicodeString(String, &Source);

        return TRUE;

    } else {
        return FALSE;
    }

    return FALSE;
}

BOOL
AddDirectoryEntry(
    _In_      PPYTHON Python,
    _In_      PUNICODE_STRING Directory,
    _Out_opt_ PPPYTHON_DIRECTORY_PREFIX_TABLE_ENTRY EntryPointer,
    _In_      PALLOCATION_ROUTINE AllocationRoutine,
    _In_opt_  PVOID AllocationContext
    )
{
    PRTL Rtl;

    PUNICODE_PREFIX_TABLE PrefixTable;
    PUNICODE_PREFIX_TABLE_ENTRY PrefixTableEntry;
    
    PPYTHON_DIRECTORY_PREFIX_TABLE_ENTRY Entry;
    PCHAR EntryRawBuffer = NULL;

    USHORT NumberOfFilenames = NumberOfInitPyFiles;
    USHORT MaxFilenameLength = MaxInitPyFileLengthInBytes;
    PPUNICODE_STRING Filenames = (PPUNICODE_STRING)InitPyFiles;
    
    PUNICODE_STRING Match = NULL;
    BOOL CaseInsensitive = TRUE;
    BOOL Success = FALSE;
    BOOL Exists = FALSE;

    if (!ARGUMENT_PRESENT(Python)) {
        return FALSE;
    }

    if (!ARGUMENT_PRESENT(Directory)) {
        return FALSE;
    }

    Rtl = Python->Rtl;

    Success = Rtl->FilesExist(
        Rtl,
        Directory,
        NumberOfFilenames,
        Filenames,
        MaxFilenameLength,
        &Exists,
        &Match
        );

    if (!Success) {
        return FALSE;
    }

    EntryRawBuffer = AllocationRoutine(AllocationContext, sizeof(*Entry));
    if (!EntryRawBuffer) {
        return FALSE;
    }

    Entry = (PPYTHON_DIRECTORY_PREFIX_TABLE_ENTRY)EntryRawBuffer;

    Entry->IsModule = Exists;

    PrefixTable = &Python->DirectoryPrefixTable.PrefixTable;
    PrefixTableEntry = (PUNICODE_PREFIX_TABLE_ENTRY)Entry;

    Success = Rtl->RtlInsertUnicodePrefix(PrefixTable,
                                          Directory,
                                          PrefixTableEntry);

    if (!Success) {
        // XXX TODO: free memory.
        return FALSE;
    }

    //
    // Set the module name... potentially inheriting the parent's module name.
    //

    // (Keep adding parents until we don't find any more directories?)

    return TRUE;
}


BOOL
GetModuleNameFromDirectory(
    _In_     PPYTHON             Python,
    _In_     PUNICODE_STRING     Directory,
    _In_     PRTL_BITMAP         Backslashes,
    _Out_    PPSTRING            ModuleName,
    _In_     PALLOCATION_ROUTINE AllocationRoutine,
    _In_opt_ PVOID               AllocationContext
    )
{
    PRTL Rtl;

    PUNICODE_PREFIX_TABLE PrefixTable;
    PUNICODE_PREFIX_TABLE_ENTRY PrefixTableEntry;
    
    PPYTHON_DIRECTORY_PREFIX_TABLE_ENTRY Entry;
    
    PUNICODE_STRING Match = NULL;
    BOOL CaseInsensitive = TRUE;
    BOOL Success;

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

    if (!ARGUMENT_PRESENT(ModuleName) || !ARGUMENT_PRESENT(*ModuleName)) {
        return FALSE;
    }

    //
    // Initialize pointer to the UNICODE_PREFIX_TABLE.
    //
    PrefixTable = &Python->DirectoryPrefixTable.PrefixTable;

    //
    // Seach for the directory in the prefix table.
    //
    PrefixTableEntry = Rtl->RtlFindUnicodePrefix(PrefixTable,
                                                 Directory,
                                                 CaseInsensitive);

    if (PrefixTableEntry) {
        PUNICODE_STRING Match;

        //
        // A match was found, see if it matches our entire directory string.
        //
        Match = PrefixTableEntry->Prefix;

        if (Match->Length == Directory->Length) {

            //
            // The match is exact.  Fill in the user's module name pointer
            // and return success.
            //
            Entry = (PPYTHON_DIRECTORY_PREFIX_TABLE_ENTRY)Match;
            *ModuleName = Entry->ModuleName;
            return TRUE;
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
        // directory.
        //

    }

    //
    // We need to add a new entry for the directory.
    //
    Success = Python->AddDirectoryEntry(Python, Directory, &Entry);


    return FALSE;
}

BOOL
GetModuleNameFromQualifiedPath(
    _In_     PPYTHON             Python,
    _In_     PUNICODE_STRING     Path,
    _Out_    PPSTRING            ModuleName,
    _In_     PALLOCATION_ROUTINE AllocationRoutine,
    _In_opt_ PVOID               AllocationContext
    )
{
    BOOL Success = FALSE;
    USHORT Offset = 0;
    USHORT Limit = 0;
    USHORT Index;
    USHORT ReversedIndex;
    USHORT NumberOfChars;
    USHORT InitPyNumberOfChars;
    USHORT NumberOfBackslashes;
    PRTL Rtl;
    HANDLE HeapHandle = NULL;
    RTL_BITMAP Bitmap = { 0, 0 };
    PRTL_BITMAP BitmapPointer = &Bitmap;
    PRTL_TEST_BIT TestBit;
    PRTL_FIND_SET_BITS FindSetBits;
    PCREATE_BITMAP_INDEX_FOR_UNICODE_STRING CreateBitmapIndexForUnicodeString;
    UNICODE_STRING Package;
    UNICODE_STRING Filename;
    UNICODE_STRING Directory;
    //UNICODE_STRING BaseDirectory;
    //UNICODE_STRING ModuleDirectory;
    BOOL IsInitPy = FALSE;
    BOOL CaseSensitive = TRUE;
    //UNICODE_STRING FinalName;


    if (!ARGUMENT_PRESENT(Python)) {
        return FALSE;
    }

    if (!ARGUMENT_PRESENT(Path)) {
        return FALSE;
    }

    if (!ARGUMENT_PRESENT(ModuleName)) {
        return FALSE;
    }

    //
    // Load the Rtl/RtlEx functions we'll be using.
    //

    Rtl = Python->Rtl;
    TestBit = Rtl->RtlTestBit;
    FindSetBits = Rtl->RtlFindSetBits;
    CreateBitmapIndexForUnicodeString = Rtl->CreateBitmapIndexForUnicodeString;

    NumberOfChars = Path->Length >> 1;
    InitPyNumberOfChars = __init__py.Length >> 1;

    //
    // Create a reversed bitmap for the backslashes in the path.
    //
    Success = CreateBitmapIndexForUnicodeString(
        Rtl,
        Path,
        L'\\',
        &HeapHandle,
        &BitmapPointer,
        TRUE
        );

    if (!Success) {
        return FALSE;
    }

    //
    // Make sure there's at least one backslash in the path.
    //

    NumberOfBackslashes = (USHORT)Rtl->RtlNumberOfSetBits(&Bitmap);
    if (NumberOfBackslashes == 0) {
        HeapFree(HeapHandle, 0, Bitmap.Buffer);
        return FALSE;
    }

    //
    // Extract the filename from the path by finding the first backslash
    // and calculating the offset into the string buffer.
    //

    ReversedIndex = (USHORT)FindSetBits(&Bitmap, 1, 0);
    Offset = NumberOfChars - ReversedIndex + 1;

    Filename.Length = ReversedIndex;
    Filename.MaximumLength = ReversedIndex + sizeof(WCHAR);
    Filename.Buffer = &Path->Buffer[Offset];

    //
    // Extract the directory name.
    //
    Directory.Length = Offset - 1;
    Directory.MaximumLength = Directory.Length;
    Directory.Buffer = Path->Buffer;

    //Success = GetModuleNameFromDirectory()

    IsInitPy = Rtl->RtlPrefixUnicodeString(&Filename, &__init__py, CaseSensitive);

    if (!IsInitPy) {


    }



    //
    // Determine if the path ends with __init__.py.
    //


    if (Path->Length >= __init__py.Length &&
        (TestBit(&Bitmap, InitPyNumberOfChars) ||
         TestBit(&Bitmap, InitPyNumberOfChars-1)))
    {
        SIZE_T Size = __init__py.Length;
        SIZE_T Matched;
        PWCH Filename;

        //
        // Get the offset into the path where "__init__.py" would be found,
        // if present.
        //

        Offset = NumberOfChars - InitPyNumberOfChars + 1;
        Filename = &Path->Buffer[Offset];

        //
        // Compare the filename with our "\\__init__.py" string.
        //

        Matched = Rtl->RtlCompareMemory(Filename, __init__py.Buffer, Size);

        //
        // Allow the 
        if (Matched == Size || Matched - 1 == Size) {

            //
            // It's a match; the fully qualified path is a filename ending in
            // "__init__.py", so we use the parent directory name for the module
            // name.
            //

            Limit = Offset - 1;

        }
        else {

            //
            // We're not dealing with an __init__.py file, so the current filename
            // will be used in the module name.  If the filename has an extension,
            // we remove it (by adjusting Limit to point to the character before
            // the period).
            //

            WCHAR Char;
            BOOL Found = FALSE;

            Index = NumberOfChars - 1;

            do {
                //
                // Reverse through the path looking for either a
                // period or backslash.
                //

                Char = Path->Buffer[Index];

                if (Char == L'.') {
                    Found = TRUE;
                    break;
                }
                else if (Char == L'\\') {
                    break;
                }

            } while (Index--);

            if (Found) {
                Limit = Index;
            }

            //
            // FinalName captures the name of this file, excluding extension.
            // E.g. if our fully qualified path was "C:\Source\Foo\bar.py",
            // FinalName will be "bar".

            //FinalName.Buffer = 

        }
    }

    //
    // Limit marks the end of the path we're using for the module name.
    // If it hasn't been set at this point, default to using the number
    // of characters of the entire path.
    //

    if (!Limit) {
        Limit = NumberOfChars;
    }

    //
    // Identify the starting offset into the path that we're going to use
    // as the starting point for the module name.  Reversing through the
    // path string, this will be the last directory we find with a file
    // named "__init__.py" (indicating that the directory is a module).
    //



    Package.Buffer = (PWCH)HeapAlloc(HeapHandle,
                                     HEAP_ZERO_MEMORY,
                                     Path->MaximumLength);

    if (!Package.Buffer) {
        HeapFree(HeapHandle, 0, Bitmap.Buffer);
        return FALSE;
    }

    Package.Length = Path->Length;
    Package.MaximumLength = Path->MaximumLength;

    __movsw(Package.Buffer, Path->Buffer, Path->Length);
    //RtlCopyMemory(Package.Buffer, Path->Buffer, Path->Length);

    //
    // Example path: "c:\\users\\trent\\home\\src\\tpn\\lib\\tpn\\util.py".
    // The .py will have been removed, leaving "..\\util".  We test for the
    // presence of "..\\tpn\\util\\__init__.py",
    //
    // XXX - remove util from name... 
    //  "c:\\users\\trent\\home\\src\\tpn\\lib\\tpn\\util.py"       - Original.
    //  "c:\\users\\trent\\home\\src\\tpn\\lib\\tpn\\__init__.py"   - Exists.
    //  "c:\\users\\trent\\home\\src\\tpn\\lib\\__init__.py"        - Doesn't exist.
    // Therefore the module name will be tpn.util.
    //

    ReversedIndex = 0;

    do {
        ULONG NextBackslashReversed;
        USHORT NextBackslash;

        //
        // Find the next backslash.
        //
        NextBackslashReversed = FindSetBits(&Bitmap, 1, ReversedIndex);

        if (NextBackslashReversed == BITS_NOT_FOUND) {
            //
            // We shouldn't ever get here.
            //
            __debugbreak();
        }

        //
        // Convert the reversed bitmap position into the offset into the
        // buffer.
        //
        NextBackslash = NumberOfChars - (USHORT)NextBackslashReversed;

    } while (Index--);


    HeapFree(HeapHandle, 0, Bitmap.Buffer);
    HeapFree(HeapHandle, 0, Package.Buffer);

    return FALSE;

}

BOOL
GetModuleNameAndQualifiedPathFromModuleFilename(
    _In_     PPYTHON             Python,
    _In_     PPYOBJECT           ModuleFilenameObject,
    _Inout_  PPUNICODE_STRING    Path,
    _Inout_  PPSTRING            ModuleName,
    _In_     PALLOCATION_ROUTINE AllocationRoutine,
    _In_opt_ PVOID               AllocationContext,
    _In_     PFREE_ROUTINE       FreeRoutine,
    _In_opt_ PVOID               FreeContext
    )
{
    BOOL Success;
    BOOL Qualify = FALSE;

    SSIZE_T Length;
    USHORT  Width;
    PVOID   Buffer;

    ULARGE_INTEGER Size;
    CONST ULARGE_INTEGER MaxSize = { MAX_USTRING - 2 };
    ULONG AllocSizeInBytes;

    PWSTR Dest;

    PUNICODE_STRING String;

    Success = GetPythonStringInformation(
        Python,
        ModuleFilenameObject,
        &Length,
        &Width,
        &Buffer
    );

    if (!Success) {
        return FALSE;
    }

    //
    // Calculate the size in bytes from the length (characters) and width.
    //

    if (Width == sizeof(CHAR)) {

        Size.QuadPart = (Length << 1);

    } else if (Width == sizeof(WCHAR)) {

        Size.QuadPart = Length;

    } else {

        //
        // Shouldn't ever get here.
        //

        __debugbreak();
    }

    //
    // Account for the trailing NUL.
    //

    Size.QuadPart += sizeof(WCHAR);

    //
    // Verify string size doesn't exceed our maximum unicode object size.
    //

    if (Size.HighPart != 0 || Size.LowPart > MaxSize.LowPart) {
        return FALSE;
    }

    //
    // Determine if we need to qualify the path.
    //

    if (Length >= 3) {

        if (Width == sizeof(CHAR)) {

            PSTR Buf = (PVOID)Buffer;

            Qualify = (Buf[1] != ':' || Buf[2] != '\\');

        } else {

            PWSTR Buf = (PWSTR)Buffer;

            Qualify = (Buf[1] != L':' || Buf[2] != '\\');

        }

    } else if (Length >= 2) {

        if (Width == sizeof(CHAR)) {

            PSTR Buf = (PSTR)Buffer;

            Qualify = (Buf[0] != '\\' || Buf[1] != '\\');

        } else {

            PWSTR Buf = (PWSTR)Buffer;

            Qualify = (Buf[0] != L'\\' || Buf[1] != L'\\');

        }
    }

    if (Qualify) {

        //
        // Get the length (in characters) of the current directory and verify it
        // fits within our unicode string size constraints.
        //

        ULONG CurDirLength;
        ULONG RequiredSizeInBytes;

        RequiredSizeInBytes = GetCurrentDirectoryW(0, NULL);
        if (RequiredSizeInBytes == 0) {
            return FALSE;
        }

        //
        // Update the allocation size with the current directory size plus
        // a joining backslash character.
        //

        Size.QuadPart = Size.LowPart + sizeof(WCHAR) + RequiredSizeInBytes;

        //
        // Verify it's within our limits.
        //

        if (Size.HighPart != 0 || Size.LowPart > MaxSize.LowPart) {
            return FALSE;
        }

        //
        // Account for the UNICODE_STRING struct size.
        //

        AllocSizeInBytes = Size.LowPart + sizeof(UNICODE_STRING);

        String = (PUNICODE_STRING)AllocationRoutine(AllocationContext, AllocSizeInBytes);

        if (!String) {
            return FALSE;
        }

        //
        // Point the buffer to the memory immediately after the struct.
        //

        String->Buffer = (PWSTR)RtlOffsetToPointer(String, sizeof(UNICODE_STRING));

        //
        // CurDirLength will represent the length (number of characters, not
        // bytes) of the string copied into our buffer.
        //

        CurDirLength = GetCurrentDirectoryW(RequiredSizeInBytes, String->Buffer);
        if (CurDirLength == 0) {
            FreeRoutine(FreeContext, String);
            return FALSE;
        }

        Dest = &String->Buffer[CurDirLength];

        //
        // Add the joining backslash and update the destination pointer.
        //

        *Dest++ = L'\\';


    } else {

        //
        // Path is already qualified; just allocate a new unicode string, accounting
        // for the additional UNICODE_STRING struct overhead.
        //

        AllocSizeInBytes = Size.LowPart + sizeof(UNICODE_STRING);

        String = (PUNICODE_STRING)AllocationRoutine(AllocationContext, AllocSizeInBytes);

        if (!String) {
            return FALSE;
        }

        String->Buffer = (PWSTR)RtlOffsetToPointer(String, sizeof(UNICODE_STRING));

        Dest = String->Buffer;

    }

    if (!String) {
        __debugbreak();
    }

    //
    // Initialize the maximum string length (number of bytes, including NUL),
    // and the string length (number of bytes, not including NUL).
    //

    String->MaximumLength = (USHORT)Size.LowPart;
    String->Length = ((USHORT)Size.LowPart) - sizeof(WCHAR);

    //
    // Copy the rest of the name over.
    //
   
    if (Width == sizeof(CHAR)) {
        PSTR Source = (PSTR)Buffer;
        USHORT Count = ((USHORT)Length) + 1;
        
        while (--Count) {
            *Dest++ = (WCHAR)*Source++;
        }

    } else {

        //
        // Both source and destination are WCHAR, so we can use __movsw() here.
        // (This will generate a `rep movsw` instruction.)
        //

        __movsw(Dest, (PWSTR)Buffer, Length);

    }

    //
    // Add terminating NUL.
    //

    *Dest++ = UNICODE_NULL;

    //
    // We've handled the path, now construct the module name.
    //

    Success = GetModuleNameFromQualifiedPath(
        Python,
        String,
        ModuleName,
        AllocationRoutine,
        AllocationContext
        );

    if (!Success) {
        FreeRoutine(FreeContext, String);
        return FALSE;
    }

    //
    // Update the user's path pointer.
    //

    *Path = String;

    return FALSE;

}

BOOL
GetModuleFilenameStringObjectFromCodeObject(
    _In_    PPYTHON     Python,
    _In_    PPYOBJECT   CodeObject,
    _Inout_ PPPYOBJECT  ModuleFilenameStringObject
)
{
    if (!Python) {
        return FALSE;
    }

    if (!CodeObject) {
        return FALSE;
    }

    if (!ModuleFilenameStringObject) {
        return FALSE;
    }

    *ModuleFilenameStringObject = (PPYOBJECT)RtlOffsetToPointer(CodeObject, Python->PyCodeObjectOffsets->Filename);
    return TRUE;
}

BOOL
GetFunctionNameStringObjectAndLineNumberFromCodeObject(
    _In_    PPYTHON     Python,
    _In_    PPYOBJECT   CodeObject,
    _Inout_ PPPYOBJECT  FunctionNameStringObject,
    _Inout_ PDWORD      LineNumber
)
{
    if (!Python) {
        return FALSE;
    }

    if (!CodeObject) {
        return FALSE;
    }

    if (!FunctionNameStringObject) {
        return FALSE;
    }

    if (!LineNumber) {
        return FALSE;
    }

    *FunctionNameStringObject = *((PPPYOBJECT)RtlOffsetToPointer(CodeObject, Python->PyCodeObjectOffsets->Name));
    *LineNumber = *((PULONG)RtlOffsetToPointer(CodeObject, Python->PyCodeObjectOffsets->FirstLineNumber));

    return TRUE;
}

BOOL
ResolveFrameObjectDetailsFast(
    _In_    PPYTHON         Python,
    _In_    PPYFRAMEOBJECT  FrameObject,
    _Inout_ PPPYOBJECT      CodeObject,
    _Inout_ PPPYOBJECT      ModuleFilenameStringObject,
    _Inout_ PPPYOBJECT      FunctionNameStringObject,
    _Inout_ PULONG          LineNumber
)
{
    ResolveFrameObjectDetailsInline(
        Python,
        FrameObject,
        CodeObject,
        ModuleFilenameStringObject,
        FunctionNameStringObject,
        LineNumber
    );

    return TRUE;
}


BOOL
ResolveFrameObjectDetails(
    _In_    PPYTHON         Python,
    _In_    PPYFRAMEOBJECT  FrameObject,
    _Inout_ PPPYOBJECT      CodeObject,
    _Inout_ PPPYOBJECT      ModuleFilenameStringObject,
    _Inout_ PPPYOBJECT      FunctionNameStringObject,
    _Inout_ PULONG          LineNumber
)
{
    if (!Python) {
        return FALSE;
    }

    if (!CodeObject) {
        return FALSE;
    }

    if (!ModuleFilenameStringObject) {
        return FALSE;
    }

    if (!FunctionNameStringObject) {
        return FALSE;
    }

    if (!LineNumber) {
        return FALSE;
    }

    return ResolveFrameObjectDetailsFast(
        Python,
        FrameObject,
        CodeObject,
        ModuleFilenameStringObject,
        FunctionNameStringObject,
        LineNumber
    );
}

BOOL
GetClassNameStringObjectFromFrameObject(
    _In_    PPYTHON         Python,
    _In_    PPYFRAMEOBJECT  FrameObject,
    _In_    PPPYOBJECT      ClassNameStringObject
)
{
    return FALSE;
}


#ifdef __cpp
} // extern "C"
#endif
