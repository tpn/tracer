// Copyright(c) Trent Nelson <trent@trent.me>
// All rights reserved.

#pragma once

#ifdef __cpplus
extern "C" {
#endif

#include <Windows.h>
#include "../Tracer/Tracer.h"

#define _PYOBJECT_HEAD              \
    union {                         \
        SIZE_T  ob_refcnt;          \
        SIZE_T  ReferenceCount;     \
    };                              \
    union {                         \
        PVOID ob_type;              \
        PVOID TypeObject;           \
    };

#define _PYVAROBJECT_HEAD           \
    union {                         \
        SIZE_T  ob_refcnt;          \
        SIZE_T  ReferenceCount;     \
    };                              \
    union {                         \
        PVOID ob_type;              \
        PVOID TypeObject;           \
    };                              \
    union {                         \
        SIZE_T  ob_size;            \
        SIZE_T  ObjectSize;         \
    };


// Types
typedef struct _PYOBJECT {
    _PYOBJECT_HEAD
} PYOBJECT, *PPYOBJECT, **PPPYOBJECT, PyObject;

typedef struct _PYVAROBJECT {
    _PYVAROBJECT_HEAD
} PYVAROBJECT, *PPYVAROBJECT, PyVarObject;

typedef struct _PYTYPEOBJECT PYTYPEOBJECT, *PPYTYPEOBJECT, PyTypeObject;
typedef struct _PYTHREADSTATE PYTHREADSTATE, *PPYTHREADSTATE, PyThreadState;

typedef struct _PYSTRINGOBJECT {
    _PYVAROBJECT_HEAD
    union {
        long ob_shash;
        LONG Hash;
    };
    union {
        int ob_sstate;
        LONG State;
    };
    union {
        char ob_sval[1];
        CHAR Value[1];
    };
} PYSTRINGOBJECT, *PPYSTRINGOBJECT;

typedef struct _PYUNICODEOBJECT {
    _PYOBJECT_HEAD
    union {
        size_t length;
        SIZE_T Length;
    };
    union {
        wchar_t *str;
        PWCHAR   String;
    };
    union {
        long hash;
        LONG Hash;
    };
} PYUNICODEOBJECT, *PPYUNICODEOBJECT, PyUnicodeObject;

typedef struct _PYCODEOBJECT25_27 {
    _PYOBJECT_HEAD
    union {
        int co_argcount;
        LONG ArgumentCount;
    };
    union {
        int co_nlocals;
        LONG NumberOfLocals;
    };
    union {
        int co_stacksize;
        LONG StackSize;
    };
    union {
        int co_flags;
        LONG Flags;
    };
    union {
        PyObject *co_code;
        PPYOBJECT Code;
    };
    union {
        PyObject *co_consts;
        PPYOBJECT Constants;
    };
    union {
        PyObject *co_names;
        PPYOBJECT Names;
    };
    union {
        PyObject *co_varnames;
        PPYOBJECT LocalVariableNames;
    };
    union {
        PyObject *co_freevars;
        PPYOBJECT FreeVariableNames;
    };
    union {
        PyObject *co_cellvars;
        PPYOBJECT CellVariableNames;
    };
    union {
        PyObject *co_filename;
        PPYOBJECT Filename;
    };
    union {
        PyObject *co_name;
        PPYOBJECT Name;
        PPYSTRINGOBJECT NameString;
    };
    union {
        int co_firstlineno;
        LONG FirstLineNumber;
    };
    union {
        PyObject *co_lnotab;
        PPYOBJECT LineNumberTable;
    };
} PYCODEOBJECT25_27, *PPYCODEOBJECT25_27;

typedef struct _PYCODEOBJECT30_32 {
    _PYOBJECT_HEAD
    union {
        int co_argcount;
        LONG ArgumentCount;
    };
    union {
        int co_kwonlyargcount;
        LONG KeywordOnlyArgumentCount;
    };
    union {
        int co_nlocals;
        LONG NumberOfLocals;
    };
    union {
        int co_stacksize;
        LONG StackSize;
    };
    union {
        int co_flags;
        LONG Flags;
    };
    union {
        PyObject *co_code;
        PPYOBJECT Code;
    };
    union {
        PyObject *co_consts;
        PPYOBJECT Constants;
    };
    union {
        PyObject *co_names;
        PPYOBJECT Names;
    };
    union {
        PyObject *co_varnames;
        PPYOBJECT LocalVariableNames;
    };
    union {
        PyObject *co_freevars;
        PPYOBJECT FreeVariableNames;
    };
    union {
        PyObject *co_cellvars;
        PPYOBJECT CellVariableNames;
    };
    union {
        PyObject *co_filename;
        PPYOBJECT Filename;
    };
    union {
        PyObject *co_name;
        PPYOBJECT Name;
    };
    union {
        int co_firstlineno;
        LONG FirstLineNumber;
    };
    union {
        PyObject *co_lnotab;
        PPYOBJECT LineNumberTable;
    };
    union {
        PyObject *co_zombieframe;
        PPYOBJECT ZombieFrame;
    };
} PYCODEOBJECT30_32, *PPYCODEOBJECT30_32;


typedef struct _PYCODEOBJECT33_35 {
    _PYOBJECT_HEAD
    union {
        int co_argcount;
        LONG ArgumentCount;
    };
    union {
        int co_kwonlyargcount;
        LONG KeywordOnlyArgumentCount;
    };
    union {
        int co_nlocals;
        LONG NumberOfLocals;
    };
    union {
        int co_stacksize;
        LONG StackSize;
    };
    union {
        int co_flags;
        LONG Flags;
    };
    union {
        PyObject *co_code;
        PPYOBJECT Code;
    };
    union {
        PyObject *co_consts;
        PPYOBJECT Constants;
    };
    union {
        PyObject *co_names;
        PPYOBJECT Names;
    };
    union {
        PyObject *co_varnames;
        PPYOBJECT LocalVariableNames;
    };
    union {
        PyObject *co_freevars;
        PPYOBJECT FreeVariableNames;
    };
    union {
        PyObject *co_cellvars;
        PPYOBJECT CellVariableNames;
    };
    union {
        unsigned char *co_cell2arg;
        PUCHAR CellVariableToArgument;
    };
    union {
        PyObject *co_filename;
        PPYOBJECT Filename;
    };
    union {
        PyObject *co_name;
        PPYOBJECT Name;
    };
    union {
        int co_firstlineno;
        LONG FirstLineNumber;
    };
    union {
        PyObject *co_lnotab;
        PPYOBJECT LineNumberTable;
    };
    union {
        PyObject *co_zombieframe;
        PPYOBJECT ZombieFrame;
    };
} PYCODEOBJECT33_35, *PPYCODEOBJECT33_35;

typedef struct _PYCODEOBJECTOFFSETS {
    union {
        USHORT co_argcount;
        USHORT ArgumentCount;
    };
    union {
        USHORT co_kwonlyargcount;
        USHORT KeywordOnlyArgumentCount;
    };
    union {
        USHORT co_nlocals;
        USHORT NumberOfLocals;
    };
    union {
        USHORT co_stacksize;
        USHORT StackSize;
    };
    union {
        USHORT co_flags;
        USHORT Flags;
    };
    union {
        USHORT co_code;
        USHORT Code;
    };
    union {
        USHORT co_consts;
        USHORT Constants;
    };
    union {
        USHORT co_names;
        USHORT Names;
    };
    union {
        USHORT co_varnames;
        USHORT LocalVariableNames;
    };
    union {
        USHORT co_freevars;
        USHORT FreeVariableNames;
    };
    union {
        USHORT co_cellvars;
        USHORT CellVariableNames;
    };
    union {
        USHORT co_filename;
        USHORT Filename;
    };
    union {
        USHORT co_name;
        USHORT Name;
    };
    union {
        USHORT co_firstlineno;
        USHORT FirstLineNumber;
    };
    union {
        USHORT co_lnotab;
        USHORT LineNumberTable;
    };
    union {
        USHORT co_zombieframe;
        USHORT ZombieFrame;
    };
} PYCODEOBJECTOFFSETS, *PPYCODEOBJECTOFFSETS;

typedef struct _PYFUNCTIONOBJECT {
    _PYOBJECT_HEAD
    union {
        PyObject *func_code;
        PPYOBJECT Code;
        PPYCODEOBJECT25_27 Code25_27;
        PPYCODEOBJECT30_32 Code30_32;
        PPYCODEOBJECT33_35 Code33_35;
    };
    union {
        PyObject *func_globals;
        PPYOBJECT Globals;
    };
    union {
        PyObject *func_defaults;
        PPYOBJECT Defaults;
    };
};

#define CO_MAXBLOCKS 20
typedef struct _PYTRYBLOCK {
    union {
        int b_type;
        LONG Type;
    };
    union {
        int b_handler;
        LONG Handler;
    };
    union {
        int b_level;
        LONG Level;
    };
} PYTRYBLOCK, *PPYTRYBLOCK, PyTryBlock;

typedef struct _PYFRAMEOBJECT PYFRAMEOBJECT, *PPYFRAMEOBJECT, PyFrameObject;

#define _PYFRAMEOBJECT_HEAD             \
    _PYVAROBJECT_HEAD                   \
    union {                             \
        PyFrameObject *f_back;          \
        PPYFRAMEOBJECT Back;            \
    };                                  \
    union {                             \
        PyObject *f_code;               \
        PPYOBJECT Code;                 \
    };                                  \
    union {                             \
        PyObject *f_builtins;           \
        PPYOBJECT Builtins;             \
    };                                  \
    union {                             \
        PyObject *f_globals;            \
        PPYOBJECT Globals;              \
    };                                  \
    union {                             \
        PyObject *f_locals;             \
        PPYOBJECT Locals;               \
    };                                  \
    union {                             \
        PyObject **f_valuestack;        \
        PPPYOBJECT ValueStack;          \
    };                                  \
    union {                             \
        PyObject **f_stacktop;          \
        PPPYOBJECT StackTop;            \
    };                                  \
    union {                             \
        PyObject *f_trace;              \
        PPYOBJECT Trace;                \
    };                                  \
    union {                             \
        PyObject *f_exc_type;           \
        PPYOBJECT ExceptionType;        \
    };                                  \
    union {                             \
        PyObject *f_exc_value;          \
        PPYOBJECT ExceptionValue;       \
    };                                  \
    union {                             \
        PyObject *f_exc_traceback;      \
        PPYOBJECT ExceptionTraceback;   \
    };

typedef struct _PYFRAMEOBJECT {
    _PYFRAMEOBJECT_HEAD
} PYFRAMEOBJECT, *PPYFRAMEOBJECT, PyFrameObject;

typedef struct _PYFRAMEOBJECT25_33 {
    _PYFRAMEOBJECT_HEAD
    union {
        PyThreadState   *f_state;
        PPYTHREADSTATE   ThreadState;
    };
    union {
        int f_lasti;
        LONG LastInstruction;
    };
    union {
        int f_lineno;
        LONG LineNumber;
    };
    union {
        int f_iblock;
        LONG BlockIndex;
    };
    union {
        PyTryBlock f_blockstack[CO_MAXBLOCKS];
        PYTRYBLOCK BlockStack[CO_MAXBLOCKS];
    };
    union {
        PyObject *f_localsplus[1];
        PPYOBJECT LocalsPlusStack[1];
    };
};

typedef struct _PYFRAMEOBJECT34_35 {
    _PYFRAMEOBJECT_HEAD
    union {
        PyObject *f_gen;
        PPYOBJECT Generator;
    };
    union {
        int f_lasti;
        LONG LastInstruction;
    };
    union {
        int f_lineno;
        LONG LineNumber;
    };
    union {
        int f_iblock;
        LONG BlockIndex;
    };
    union {
        char f_executing;
        BOOLEAN StillExecuting;
    };
    union {
        PyTryBlock f_blockstack[CO_MAXBLOCKS];
        PYTRYBLOCK BlockStack[CO_MAXBLOCKS];
    };
    union {
        PyObject *f_localsplus[1];
        PPYOBJECT LocalsPlusStack[1];
    };
};

typedef struct _PYMETHODDEF {
    union {
        char *ml_name;
        PCHAR Name;
    };
} PYMETHODDEF, *PPYMETHODDEF, PyMethodDef;

typedef VOID (*PPYDESTRUCTOR)(PPYOBJECT);
typedef PPYDESTRUCTOR destructor;

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
