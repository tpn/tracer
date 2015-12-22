// Copyright(c) Trent Nelson <trent@trent.me>
// All rights reserved.

#pragma once

#ifdef __cpplus
extern "C" {
#endif

#include <Windows.h>
#include "../Tracer/Tracer.h"

typedef struct _PYOBJECT {
    union {
        SIZE_T  ob_refcnt;
        SIZE_T  ReferenceCount;
    };
    union {
        PVOID ob_type;
        PVOID TypeObject;
    };
} PYOBJECT, *PPYOBJECT;

typedef struct _PYVAROBJECT {
    union {
        SIZE_T  ob_refcnt;
        SIZE_T  ReferenceCount;
    };
    union {
        PVOID ob_type;
        PVOID TypeObject;
    };
    union {
        SIZE_T  ob_size;
        SIZE_T  ObjectSize;
    };
} PYVAROBJECT, *PPYVAROBJECT;

typedef PVOID *PPYFRAMEOBJECT;

// Functions
typedef PCSTR (*PPY_GETVERSION)();
typedef VOID (*PPY_INCREF)(PPYOBJECT);
typedef VOID (*PPY_DECREF)(PPYOBJECT);
typedef LONG (*PPYFRAME_GETLINENUMBER)(PPYFRAMEOBJECT FrameObject);
typedef PWSTR (*PPYUNICODE_ASUNICODE)(PPYOBJECT Object);
typedef SIZE_T (*PPYUNICODE_GETLENGTH)(PPYOBJECT Object);
typedef LONG (*PPY_TRACEFUNC)(PPYOBJECT, PPYFRAMEOBJECT, LONG, PPYOBJECT);
typedef LONG (*PPYEVAL_SETTRACEFUNC)(PPY_TRACEFUNC, PPYOBJECT);

typedef struct _PYTHON {
    DWORD Size;

    // Functions
    PPY_GETVERSION          Py_GetVersion;
    PPYFRAME_GETLINENUMBER  PyFrame_GetLineNumber;
    PPYEVAL_SETTRACEFUNC    PyEval_SetTraceFunc;
    PPYUNICODE_ASUNICODE    PyUnicode_AsUnicode;
    PPYUNICODE_GETLENGTH    PyUnicode_GetLength;
    PPY_INCREF              Py_IncRef;
    PPY_DECREF              Py_DecRef;

    // Data Types
    PPYOBJECT PyCode_Type;
    PPYOBJECT PyDict_Type;
    PPYOBJECT PyTuple_Type;
    PPYOBJECT PyType_Type;
    PPYOBJECT PyFunction_Type;
    PPYOBJECT PyString_Type;
    PPYOBJECT PyBytes_Type;
    PPYOBJECT PyUnicode_Type;
    PPYOBJECT PyCFunction_Type;
    PPYOBJECT PyInstance_Type;
    PPYOBJECT PyModule_Type;

    // Computed/Derived Values
    PCSTR   VersionString;
    USHORT  MajorVersion;
    USHORT  MinorVersion;

} PYTHON, *PPYTHON, **PPPYTHON;

TRACER_API
BOOL
InitializePython(
    _In_                         HMODULE     PythonModule,
    _Out_bytecap_(*SizeOfPython) PPYTHON     Python,
    _Inout_                      PDWORD      SizeOfPython
);

#ifdef __cpp
} // extern "C"
#endif
