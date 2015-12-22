// Python Tools for Visual Studio
// Copyright(c) Microsoft Corporation
// All rights reserved.
//
// Licensed under the Apache License, Version 2.0 (the License); you may not use
// this file except in compliance with the License. You may obtain a copy of the
// License at http://www.apache.org/licenses/LICENSE-2.0
//
// THIS CODE IS PROVIDED ON AN  *AS IS* BASIS, WITHOUT WARRANTIES OR CONDITIONS
// OF ANY KIND, EITHER EXPRESS OR IMPLIED, INCLUDING WITHOUT LIMITATION ANY
// IMPLIED WARRANTIES OR CONDITIONS OF TITLE, FITNESS FOR A PARTICULAR PURPOSE,
// MERCHANTABLITY OR NON-INFRINGEMENT.
//
// See the Apache Version 2.0 License for specific language governing
// permissions and limitations under the License.

#include <Windows.h>
#include "Python.h"
#include "../Tracer/Tracing.h"

BOOL
InitializePython(
    _In_        HMODULE     PythonModule,
    _Out_       PPYTHON     Python,
    _Inout_     PDWORD      SizeOfPython
)
{
    PCCH Version;
    ULONG MajorVersion;
    ULONG MinorVersion;
    HMODULE NtdllModule;
    BOOLEAN IsSupportedVersion;
    CHAR VersionString[2] = { 0 };
    PRTLCHARTOINTEGER RtlCharToInteger;

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

    SecureZeroMemory(Python, sizeof(*Python));

    Python->Py_GetVersion = (PPY_GETVERSION)GetProcAddress(PythonModule, "Py_GetVersion");
    if (!Python->Py_GetVersion) {
        goto error;
    }
    Python->VersionString = Python->Py_GetVersion();
    if (!Python->VersionString) {
        goto error;
    }

    NtdllModule = LoadLibraryW(L"ntdll");
    if (NtdllModule == INVALID_HANDLE_VALUE) {
        goto error;
    }

    RtlCharToInteger = (PRTLCHARTOINTEGER)GetProcAddress(NtdllModule, "RtlCharToInteger");
    if (!RtlCharToInteger) {
        goto error;
    }

    VersionString[0] = Python->VersionString[0];
    if (FAILED(RtlCharToInteger(VersionString, 0, &MajorVersion))) {
        goto error;
    }
    Python->MajorVersion = (USHORT)MajorVersion;

    Python->MinorVersion = 0;
    Version = Python->VersionString;
    while (*Version && *Version >= '0' && *Version <= '9') {
        Version++;
    }
    if (*Version == '.') {
        Version++;
        VersionString[0] = *Version;
        if (FAILED(RtlCharToInteger(VersionString, 0, &MinorVersion))) {
            goto error;
        }
        Python->MinorVersion = (USHORT)MinorVersion;
    }

    IsSupportedVersion = (
        (Python->MajorVersion == 2 && (Python->MinorVersion >= 4 && Python->MinorVersion <= 7)) ||
        (Python->MajorVersion == 3 && (Python->MinorVersion >= 0 && Python->MinorVersion <= 5))
    );

    if (!IsSupportedVersion) {
        goto error;
    }

    Python->PyCode_Type = (PPYOBJECT)GetProcAddress(PythonModule, "PyCode_Type");
    if (!Python->PyCode_Type) {
        goto error;
    }

    Python->PyDict_Type = (PPYOBJECT)GetProcAddress(PythonModule, "PyDict_Type");
    if (!Python->PyDict_Type) {
        goto error;
    }

    Python->PyTuple_Type = (PPYOBJECT)GetProcAddress(PythonModule, "PyTuple_Type");
    if (!Python->PyTuple_Type) {
        goto error;
    }

    Python->PyType_Type = (PPYOBJECT)GetProcAddress(PythonModule, "PyType_Type");
    if (!Python->PyType_Type) {
        goto error;
    }

    Python->PyFunction_Type = (PPYOBJECT)GetProcAddress(PythonModule, "PyFunction_Type");
    if (!Python->PyFunction_Type) {
        goto error;
    }

    Python->PyString_Type = (PPYOBJECT)GetProcAddress(PythonModule, "PyString_Type");
    if (!Python->PyString_Type) {
        Python->PyBytes_Type = (PPYOBJECT)GetProcAddress(PythonModule, "PyBytes_Type");
        if (!Python->PyBytes_Type) {
            goto error;
        }
    }

    Python->PyUnicode_Type = (PPYOBJECT)GetProcAddress(PythonModule, "PyUnicode_Type");
    if (!Python->PyUnicode_Type) {
        goto error;
    }

    Python->PyCFunction_Type = (PPYOBJECT)GetProcAddress(PythonModule, "PyCFunction_Type");
    if (!Python->PyCFunction_Type) {
        goto error;
    }

    Python->PyInstance_Type = (PPYOBJECT)GetProcAddress(PythonModule, "PyInstance_Type");
    if (!Python->PyInstance_Type) {
        goto error;
    }

    Python->PyModule_Type = (PPYOBJECT)GetProcAddress(PythonModule, "PyModule_Type");
    if (!Python->PyModule_Type) {
        goto error;
    }

    Python->PyFrame_GetLineNumber = (PPYFRAME_GETLINENUMBER)GetProcAddress(PythonModule, "PyFrame_GetLineNumber");
    if (!Python->PyFrame_GetLineNumber) {
        goto error;
    }

    Python->PyUnicode_AsUnicode = (PPYUNICODE_ASUNICODE)GetProcAddress(PythonModule, "PyUnicode_AsUnicode");
    if (!Python->PyUnicode_AsUnicode) {
        goto error;
    }

    // Added in 3.3
    Python->PyUnicode_GetLength = (PPYUNICODE_GETLENGTH)GetProcAddress(PythonModule, "PyUnicode_GetLength");

    Python->PyEval_SetTraceFunc = (PPYEVAL_SETTRACEFUNC)GetProcAddress(PythonModule, "PyEval_SetTraceFunc");
    if (!Python->PyEval_SetTraceFunc) {
        goto error;
    }

    Python->Py_IncRef = (PPY_INCREF)GetProcAddress(PythonModule, "Py_IncRef");
    if (!Python->Py_IncRef) {
        goto error;
    }

    Python->Py_DecRef = (PPY_DECREF)GetProcAddress(PythonModule, "Py_DecRef");
    if (!Python->Py_DecRef) {
        goto error;
    }

    Python->Size = *SizeOfPython;
    return TRUE;

error:
    // Clear any partial state.
    SecureZeroMemory(Python, sizeof(*Python));
    return FALSE;
}

#ifdef __cpp
} // extern "C"
#endif
