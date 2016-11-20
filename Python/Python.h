/*++

Copyright (c) 2016 Trent Nelson <trent@trent.me>

Module Name:

    Python.h

Abstract:

    This is the main header file for the Python component.

    This component is responsible for providing a single facade, by way of the
    PYTHON struct, to all supported versions of Python runtimes.  The component
    also provides additional functionality to facilitate runtime tracing of the
    interpreter via the standard line tracing and profiling hooks.

--*/

#pragma once

#ifdef __cplusplus
extern "C" {
#endif

#ifdef _PYTHON_INTERNAL_BUILD

#define PYTHON_API __declspec(dllexport)
#define PYTHON_DATA extern __declspec(dllexport)

#include "stdafx.h"

#else

#define PYTHON_API __declspec(dllimport)
#define PYTHON_DATA extern __declspec(dllimport)

#include <Windows.h>
#include "../Rtl/Rtl.h"

#endif

#define PYTHON_EX_API PYTHON_API
#define PYTHON_EX_DATA PYTHON_DATA

enum PythonVersion {
    PythonVersion_Unknown,
    PythonVersion_25 = 0x0205,
    PythonVersion_26 = 0x0206,
    PythonVersion_27 = 0x0207,
    PythonVersion_30 = 0x0300,
    PythonVersion_31 = 0x0301,
    PythonVersion_32 = 0x0302,
    PythonVersion_33 = 0x0303,
    PythonVersion_34 = 0x0304,
    PythonVersion_35 = 0x0305
};

typedef union _PYTHON_EVENT_TRAITS {
    BYTE AsByte;
    struct {
        BYTE IsCall:1;
        BYTE IsException:1;
        BYTE IsLine:1;
        BYTE IsReturn:1;
        BYTE IsC:1;
        BYTE AsEventType:3;
    };
} PYTHON_EVENT_TRAITS, *PPYTHON_EVENT_TRAITS;

typedef
_Success_(return != 0)
BOOL
(FIND_PYTHON_DLL_AND_EXE)(
    _In_ PRTL Rtl,
    _In_ PALLOCATOR Allocator,
    _In_ PUNICODE_STRING Directory,
    _Outptr_opt_ PPUNICODE_STRING PythonDllPath,
    _Outptr_opt_ PPUNICODE_STRING PythonExePath,
    _Outptr_opt_ PUSHORT NumberOfPathEntries,
    _Outptr_opt_ PPUNICODE_STRING PathEntries,
    _Out_opt_ PCHAR MajorVersion,
    _Out_opt_ PCHAR MinorVersion
    );
typedef FIND_PYTHON_DLL_AND_EXE *PFIND_PYTHON_DLL_AND_EXE;

#ifdef _M_AMD64
typedef __int64 Py_ssize_t, PY_SSIZE;
typedef __int64 Py_hash_t, PY_HASH;
typedef __int64 SSIZE_T, *PSSIZE_T;
#else
typedef _W64 int Py_ssize_t, Py_SSIZE;
typedef _W64 int Py_hash_t, PY_HASH;
//typedef _W64 int SSIZE_T, *PSSIZE_T;
#endif

typedef struct _PYTYPEOBJECT PYTYPEOBJECT, *PPYTYPEOBJECT, PyTypeObject;

typedef union _PY_OBJECT_REFCOUNTEX {
    Py_ssize_t ob_refcnt;
    SSIZE_T ReferenceCount;
    struct {
        ULONG LowPart;
        struct {
            ULONG UpperBits:28;
            ULONG Seen:1;
            ULONG Unused:2;
        };
    };
} PY_OBJECT_REFCOUNTEX, *PPY_OBJECT_REFCOUNTEX;


#define _PYOBJECT_HEAD                   \
    union {                              \
        Py_ssize_t  ob_refcnt;           \
        SSIZE_T     ReferenceCount;      \
        PY_OBJECT_REFCOUNTEX RefCountEx; \
    };                                   \
    union {                              \
        PyTypeObject *ob_type;           \
        PPYTYPEOBJECT Type;              \
    };

#define _PYVAROBJECT_HEAD    \
    _PYOBJECT_HEAD           \
    union {                  \
        Py_ssize_t  ob_size; \
        SSIZE_T  ObjectSize; \
    };


// Types
typedef struct _PYOBJECT {
    _PYOBJECT_HEAD
} PYOBJECT, *PPYOBJECT, **PPPYOBJECT, PyObject;

typedef struct _PYVAROBJECT {
    _PYVAROBJECT_HEAD
} PYVAROBJECT, *PPYVAROBJECT, PyVarObject;

typedef struct _PYTHREADSTATE PYTHREADSTATE, *PPYTHREADSTATE, PyThreadState;

typedef struct _PYOLDSTYLECLASS {
    _PYOBJECT_HEAD
    PPYOBJECT Bases;
    PPYOBJECT Dict;
    PPYOBJECT Name;
    PPYOBJECT GetAttr;
    PPYOBJECT SetAttr;
    PPYOBJECT DelAttr;
} PYOLDSTYLECLASS, *PPYOLDSTYLECLASS;

typedef struct _PYINSTANCEOBJECT {
    _PYOBJECT_HEAD
    PPYOLDSTYLECLASS OldStyleClass;
    PPYOBJECT Dict;
    PPYOBJECT WeakRefList;
} PYINSTANCEOBJECT, *PPYINSTANCEOBJECT;

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

typedef struct _PYBYTESOBJECT {
    _PYVAROBJECT_HEAD

    union {
        long ob_shash;
        LONG Hash;
    };
    union {
        char ob_sval[1];
        CHAR Value[1];
    };

} PYBYTESOBJECT, *PPYBYTESOBJECT;

typedef struct _PYUNICODEOBJECT {
    _PYOBJECT_HEAD
    union {
        Py_ssize_t  length;
        SSIZE_T     Length;
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
        const USHORT co_argcount;
        const USHORT ArgumentCount;
    };
    union {
        const USHORT co_kwonlyargcount;
        const USHORT KeywordOnlyArgumentCount;
    };
    union {
        const USHORT co_nlocals;
        const USHORT NumberOfLocals;
    };
    union {
        const USHORT co_stacksize;
        const USHORT StackSize;
    };
    union {
        const USHORT co_flags;
        const USHORT Flags;
    };
    union {
        const USHORT co_code;
        const USHORT Code;
    };
    union {
        const USHORT co_consts;
        const USHORT Constants;
    };
    union {
        const USHORT co_names;
        const USHORT Names;
    };
    union {
        const USHORT co_varnames;
        const USHORT LocalVariableNames;
    };
    union {
        const USHORT co_freevars;
        const USHORT FreeVariableNames;
    };
    union {
        const USHORT co_cellvars;
        const USHORT CellVariableNames;
    };
    union {
        const USHORT co_filename;
        const USHORT Filename;
    };
    union {
        const USHORT co_name;
        const USHORT Name;
    };
    union {
        const USHORT co_firstlineno;
        const USHORT FirstLineNumber;
    };
    union {
        const USHORT co_lnotab;
        const USHORT LineNumberTable;
    };
    union {
        const USHORT co_zombieframe;
        const USHORT ZombieFrame;
    };
} PYCODEOBJECTOFFSETS, *PPYCODEOBJECTOFFSETS;
typedef const PYCODEOBJECTOFFSETS CPYCODEOBJECTOFFSETS, *PCPYCODEOBJECTOFFSETS;
//typedef const PPYCODEOBJECTOFFSETS *PCPYCODEOBJECTOFFSETS;

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
} PYFUNCTIONOBJECT, *PPYFUNCTIONOBJECT;

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

#define _PYFRAMEOBJECT_HEAD           \
    _PYVAROBJECT_HEAD                 \
    union {                           \
        PyFrameObject *f_back;        \
        PPYFRAMEOBJECT Back;          \
    };                                \
    union {                           \
        PyObject *f_code;             \
        PPYOBJECT Code;               \
    };                                \
    union {                           \
        PyObject *f_builtins;         \
        PPYOBJECT Builtins;           \
    };                                \
    union {                           \
        PyObject *f_globals;          \
        PPYOBJECT Globals;            \
    };                                \
    union {                           \
        PyObject *f_locals;           \
        PPYOBJECT Locals;             \
    };                                \
    union {                           \
        PyObject **f_valuestack;      \
        PPPYOBJECT ValueStack;        \
    };                                \
    union {                           \
        PyObject **f_stacktop;        \
        PPPYOBJECT StackTop;          \
    };                                \
    union {                           \
        PyObject *f_trace;            \
        PPYOBJECT Trace;              \
    };                                \
    union {                           \
        PyObject *f_exc_type;         \
        PPYOBJECT ExceptionType;      \
    };                                \
    union {                           \
        PyObject *f_exc_value;        \
        PPYOBJECT ExceptionValue;     \
    };                                \
    union {                           \
        PyObject *f_exc_traceback;    \
        PPYOBJECT ExceptionTraceback; \
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
} PYFRAMEOBJECT25_33, *PPYFRAMEOBJECT25_33;

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
        CHAR StillExecuting;
    };
    union {
        PyTryBlock f_blockstack[CO_MAXBLOCKS];
        PYTRYBLOCK BlockStack[CO_MAXBLOCKS];
    };
    union {
        PyObject *f_localsplus[1];
        PPYOBJECT LocalsPlusStack[1];
    };
} PYFRAMEOBJECT34_35, *PPYFRAMEOBJECT34_35;

//
// We only capture the offsets for the struct members that are not consistent
// between 2.7-3.x.
//

typedef struct _PYFRAMEOBJECTOFFSETS {

    //
    // PYFRAMEOBJECT25_33
    //

    union {
        const USHORT f_state;
        const USHORT ThreadState;
    };
    union {
        const USHORT f_lasti;
        const USHORT LastInstruction;
    };
    union {
        const USHORT f_lineno;
        const USHORT LineNumber;
    };
    union {
        const USHORT f_iblock;
        const USHORT BlockIndex;
    };
    union {
        const USHORT f_blockstack;
        const USHORT BlockStack;
    };
    union {
        const USHORT f_localsplus;
        const USHORT LocalsPlusStack;
    };

    //
    // PYFRAMEOBJECT34_35
    //

    union {
        const USHORT f_gen;
        const USHORT Generator;
    };
    union {
        const USHORT f_executing;
        const USHORT StillExecuting;
    };

} PYFRAMEOBJECTOFFSETS, *PPYFRAMEOBJECTOFFSETS;

typedef const PYFRAMEOBJECTOFFSETS CPYFRAMEOBJECTOFFSETS;
typedef const PYFRAMEOBJECTOFFSETS *PCPYFRAMEOBJECTOFFSETS;

typedef
PPYOBJECT
(PYCFUNCTION)(
    _In_ PPYOBJECT Self,
    _In_ PPYOBJECT Args
    );
typedef PYCFUNCTION *PPYCFUNCTION;

typedef
PPYOBJECT
(PYCFUNCTIONWITHKEYWORDS)(
    _In_ PPYOBJECT Self,
    _In_ PPYOBJECT Args,
    _In_ PPYOBJECT Keywords
    );
typedef PYCFUNCTIONWITHKEYWORDS *PPYCFUNCTIONWITHKEYWORDS;
typedef PYCFUNCTIONWITHKEYWORDS *PyCFunctionWithKeywords;

typedef
PPYOBJECT
(PYCFUNCTIONNOARGS)(
    _In_ PPYOBJECT Self
    );
typedef PYCFUNCTIONNOARGS *PPYCFUNCTIONNOARGS;
typedef PYCFUNCTIONNOARGS *PyCFunctionNoArgs;

typedef struct _PYMETHODDEF {

    union {
        char *ml_name;
        PCHAR Name;
    };

    union {
        PPYCFUNCTION ml_meth;
        PPYCFUNCTION FunctionPointer;
    };

    union {
        int ml_flags;
        union {
            ULONG Flags;
            struct {
                ULONG VarArgs:1;
                ULONG KeywordArgs:1;
                ULONG NoArgs:1;
                ULONG ObjectArg:1;
                ULONG ClassMethod:1;
                ULONG StaticMethod:1;
                ULONG CoexistMethod:1;
            };
        };
    };

    union {
        const char *ml_doc;
        PCCH Doc;
    };

} PYMETHODDEF, *PPYMETHODDEF, PyMethodDef;

typedef PPYOBJECT (*PPYGETTER)(PPYOBJECT, PVOID), (*getter);
typedef LONG (*PPYSETTER)(PPYOBJECT, PPYOBJECT, PVOID), (*setter);
typedef struct _PYGETSETDEF {
    union {
        char *name;
        PCHAR Name;
    };
    union {
        getter      get;
        PPYGETTER   Get;
    };
    union {
        setter      set;
        PPYSETTER   Set;
    };
    union {
        char *doc;
        PCHAR Doc;
    };
    union {
        void *closure;
        PVOID Closure;
    };
} PYGETSETDEF, *PPYGETSETDEF, PyGetSetDef;

typedef VOID (*PPYDESTRUCTOR)(PPYOBJECT), (*destructor);
typedef PPYOBJECT (*PPYUNARYFUNC)(PPYOBJECT), (*unaryfunc);
typedef PPYOBJECT (*PPYBINARYFUNC)(PPYOBJECT, PPYOBJECT), (*binaryfunc);
typedef PPYOBJECT (*PPYTERNARYFUNC)(PPYOBJECT, PPYOBJECT, PPYOBJECT), (*ternaryfunc);
typedef LONG (*PPYINQUIRYFUNC)(PPYOBJECT), (*inquiry);
typedef SSIZE_T (*PPYLENFUNC)(PPYOBJECT), (*lenfunc);
typedef PPYOBJECT (*PPYSSIZEARGFUNC)(PPYOBJECT, SSIZE_T), (*ssizeargfunc);
typedef PPYOBJECT (*PPYSSIZESSIZEARGFUNC)(PPYOBJECT, SSIZE_T SSIZE_T), (*ssizessizeargfunc);
typedef LONG (*PPYSSIZEOBJARGPROC)(PPYOBJECT, SSIZE_T, PPYOBJECT), (*ssizeobjargproc);
typedef LONG (*PPYSSIZESSIZEOBJARGPROC)(PPYOBJECT, SSIZE_T, SSIZE_T, PPYOBJECT), (*ssizeobjargproc);
typedef LONG (*PPYOBJOBJARGPROC)(PPYOBJECT, PPYOBJECT, PPYOBJECT), (*objobjargproc);
typedef INT (*PCOMPARE_FUNCTION)(PPYOBJECT, PPYOBJECT), (*cmpfunc);
typedef LONG (*PHASH_FUNCTION)(PPYOBJECT), (*hashfunc);
typedef PPYOBJECT (*PRICH_COMPARE_FUNCTION)(PPYOBJECT, PPYOBJECT, INT), (*richcmpfunc);

typedef struct _PYBUFFER {
    union {
        void *buf;
        PVOID Buffer;
    };
    union {
        PyObject *obj;
        PPYOBJECT Object;
    };
    union {
        Py_ssize_t  len;
        SSIZE_T     Length;
    };
    union {
        Py_ssize_t  itemsize;
        SSIZE_T     ItemSize;
    };
    union {
        int readonly;
        LONG ReadOnly;
    };
    union {
        int ndim;
        LONG NumberOfDimensions;
    };
    union {
        char *format;
        PCHAR Format;
    };
    union {
        Py_ssize_t  *shape;
        PSSIZE_T     Shape;
    };
    union {
        Py_ssize_t  *strides;
        PSSIZE_T     Strides;
    };
    union {
        Py_ssize_t  *suboffsets;
        PSSIZE_T     SubOffsets;
    };
    union {
        void    *internal;
        PVOID    Internal;
    };
} PYBUFFER, *PPYBUFFER, Py_buffer;

typedef LONG (*PPYGETBUFFERPROC)(PPYOBJECT, PPYBUFFER, LONG), (*getbufferproc);
typedef
VOID (PYRELEASEBUFFERPROC)(
    PPYOBJECT,
    PPYBUFFER
    );
typedef PYRELEASEBUFFERPROC *PPYRELEASEBUFFERPROC;
typedef PYRELEASEBUFFERPROC *releasebufferproc;

typedef struct _PYBUFFERPROCS {
    union {
        getbufferproc           bf_getbuffer;
        PPYGETBUFFERPROC        GetBuffer;
    };
    union {
        releasebufferproc       bf_releasebuffer;
        PPYRELEASEBUFFERPROC    ReleaseBuffer;
    };
} PYBUFFERPROCS, *PPYBUFFERPROCS, PyBufferProcs;

typedef LONG (*PPYOBJOBJPROC)(PPYOBJECT, PPYOBJECT), (*objobjproc);
typedef LONG (*PPYVISITPROC)(PPYOBJECT, PPYOBJECT), (*visitproc);

typedef LONG (*PYFREEFUNC)(PVOID), (*freefunc);

typedef struct _PYASYNCMETHODS {
    union {
        unaryfunc       am_await;
        PPYUNARYFUNC    AsyncAwait;
    };
    union {
        unaryfunc       am_aiter;
        PPYUNARYFUNC    AsyncIter;
    };
    union {
        unaryfunc       am_next;
        PPYUNARYFUNC    AsyncNext;
    };
} PYASYNCMETHODS, *PPYASYNCMETHODS, PyAsyncMethods;

typedef struct _PYTYPEOBJECT PYTYPEOBJECT, *PPYTYPEOBJECT, PyTypeObject;

typedef struct _PYTYPEOBJECT {
    _PYVAROBJECT_HEAD
    union {
        const char *tp_name;
        PCCH        Name;
    };
    union {
        Py_ssize_t  tp_basicsize;
        SSIZE_T     BasicSize;
    };
    union {
        Py_ssize_t  tp_itemsize;
        SSIZE_T     ItemSize;
    };
    union {
        destructor      tp_dealloc;
        PPYDESTRUCTOR   Dealloc;
    };
    union {
        void    *tp_print;
        PVOID    Print;
    };
    union {
        void    *tp_getattr;
        PVOID    GetAttribute;
    };
    union {
        void    *tp_setattr;
        PVOID    SetAttribute;
    };
    union {
        union {
            hashfunc         tp_compare;
            PyAsyncMethods  *tp_as_async;
        };
        union {
            PCOMPARE_FUNCTION Compare;
            PPYASYNCMETHODS   AsyncMethods;
        };
    };
    union {
        void    *tp_repr;
        PVOID    Repr;
    };
    union {
        void    *tp_as_number;
        PVOID   NumberMethods;
    };
    union {
        void    *tp_as_sequence;
        PVOID   SequenceMethods;
    };
    union {
        void    *tp_as_mapping;
        PVOID   MappingMethods;
    };
    union {
        hashfunc        tp_hash;
        PHASH_FUNCTION  Hash;
    };
    union {
        void    *tp_call;
        PVOID   Call;
    };
    union {
        void    *tp_str;
        PVOID   Str;
    };
    union {
        void    *tp_getattro;
        PVOID   GetAttributeObject;
    };
    union {
        void    *tp_setattro;
        PVOID   SetAttributeObject;
    };
    union {
        PyBufferProcs   *tp_as_buffer;
        PPYBUFFERPROCS  BufferMethods;
    };
    union {
        long    tp_flags;
        LONG    Flags;
    };
    union {
        const char *tp_doc;
        PCCH Doc;
    };
    union {
        void    *tp_traverse;
        PVOID   Traverse;
    };
    union {
        void    *tp_clear;
        PVOID   Clear;
    };
    union {
        richcmpfunc            tp_richcompare;
        PRICH_COMPARE_FUNCTION RichCompare;
    };
    union {
        Py_ssize_t  tp_weaklistoffset;
        SSIZE_T     WeakListOffset;
    };
    union {
        void    *tp_iter;
        PVOID   Iter;
    };
    union {
        void    *tp_iternext;
        PVOID   IterNext;
    };
    union {
        PyMethodDef *tp_methods;
        PPYMETHODDEF Methods;
    };
    union {
        PyMethodDef *tp_members;
        PPYMETHODDEF MemberMethods;
    };
    union {
        PyGetSetDef *tp_getset;
        PPYGETSETDEF GetSetMethods;
    };
    union {
        PyTypeObject *tp_base;
        PPYTYPEOBJECT BaseType;
    };
    union {
        PyObject *tp_dict;
        PPYOBJECT Dict;
    };
    union {
        void    *tp_descr_get;
        PVOID   DescriptorGet;
    };
    union {
        void    *tp_descr_set;
        PVOID   DescriptorSet;
    };
    union {
        Py_ssize_t  tp_dictoffset;
        SSIZE_T     DictOffset;
    };
    union {
        void    *tp_init;
        PVOID   Init;
    };
    union {
        void    *tp_alloc;
        PVOID   Alloc;
    };
    union {
        void    *tp_new;
        PVOID   New;
    };
    union {
        void    *tp_free;
        PVOID   Free;
    };
    union {
        void    *tp_is_gc;
        PVOID   IsGC;
    };
    union {
        PyObject    *tp_bases;
        PPYOBJECT   Bases;
    };
    union {
        PyObject    *tp_mro;
        PPYOBJECT   MethodResolutionOrder;
    };
    union {
        PyObject    *tp_cache;
        PPYOBJECT   Cache;
    };
    union {
        PyObject    *tp_subclasses;
        PPYOBJECT   Subclasses;
    };
    union {
        PyObject    *tp_weaklist;
        PPYOBJECT   WeakList;
    };
    union {
        PyObject    *tp_del;
        PVOID       Del;
    };
    union {
        unsigned int tp_version_tag;
        ULONG        VersionTag;
    };
} PYTYPEOBJECT, *PPYTYPEOBJECT, PyTypeObject;

typedef struct _PYTYPEOBJECTEX {
    union {
        PPYOBJECT Object;
        PPYTYPEOBJECT Type;
    };
} PYTYPEOBJECTEX, *PPYTYPEOBJECTEX, **PPPYTYPEOBJECTEX;

typedef struct _PYTUPLEOBJECT {
    _PYVAROBJECT_HEAD
    union {
        PyObject *ob_item[1];
        PPYOBJECT Item[1];
    };
} PYTUPLEOBJECT, *PPYTUPLEOBJECT, **PPPYTUPLEOBJECT;

//
// PyDictObject - 2.x to 3.2
//

#define PYDICT_MIN_SIZE 8

typedef struct _PYDICTENTRY25_32 {

    union {
        Py_ssize_t me_hash;
        SSIZE_T Hash;
    };

    union {
        PyObject *me_key;
        PPYOBJECT Key;
    };

    union {
        PyObject *me_value;
        PPYOBJECT Value;
    };

} PYDICTENTRY25_32, *PPYDICTENTRY25_32, PyDictEntry25_32;

typedef
PPYDICTENTRY25_32
(PYDICTENTRY25_32_MA_LOOKUP)(
    _In_ struct _PYDICTOBJECT25_32 *Dict,
    _In_ PPYOBJECT Key,
    _In_opt_ LONG Hash
    );
typedef PYDICTENTRY25_32_MA_LOOKUP *PPYDICTENTRY25_32_MA_LOOKUP;

typedef struct _PYDICTOBJECT25_32 {
    _PYOBJECT_HEAD

    union {
        Py_ssize_t  ma_fill;
        SSIZE_T     Fill;
    };

    union {
        Py_ssize_t ma_used;
        SSIZE_T    Used;
    };

    union {
        Py_ssize_t ma_mask;
        SSIZE_T    Mask;
    };

    union {
        PyDictEntry25_32 *ma_table;
        PPYDICTENTRY25_32 Table;
    };

    union {
        PPYDICTENTRY25_32 ma_lookup;
        PPYDICTENTRY25_32 Lookup;
    };

    union {
        PYDICTENTRY25_32 ma_smalltable[PYDICT_MIN_SIZE];
        PYDICTENTRY25_32 SmallTable[PYDICT_MIN_SIZE];
    };

} PYDICTOBJECT25_32, *PPYDICTOBJECT25_32;

//
// PyCFunctionObject
//

typedef struct _PYCFUNCTIONOBJECT25_34 {
    _PYOBJECT_HEAD
    union {
        PyMethodDef *m_ml;
        PPYMETHODDEF MethodDef;
    };
    union {
        PyObject    *m_self;
        PPYOBJECT    Self;
    };
    union {
        PyObject    *m_module;
        PPYOBJECT    Module;
    };
} PYCFUNCTIONOBJECT25_34, *PPYCFUNCTIONOBJECT25_34;

typedef PYCFUNCTIONOBJECT25_34 PYCFUNCTIONOBJECT;
typedef PYCFUNCTIONOBJECT25_34 *PPYCFUNCTIONOBJECT;

typedef struct _PYCFUNCTIONOBJECT35_35 {
    _PYOBJECT_HEAD
    union {
        PyMethodDef *m_ml;
        PPYMETHODDEF MethodDef;
    };
    union {
        PyObject    *m_self;
        PPYOBJECT    Self;
    };
    union {
        PyObject    *m_module;
        PPYOBJECT    Module;
    };
    union {
        PyObject    *m_weakreflist;
        PPYOBJECT    WeakRefList;
    };
} PYCFUNCTIONOBJECT35_35, *PPYCFUNCTIONOBJECT35_35;

typedef struct _PYINTERPRETERSTATE PYINTERPRETERSTATE, *PPYINTERPRETERSTATE, PyInterpreterState;

typedef LONG (*PPYTRACEFUNC)(PPYOBJECT, PPYFRAMEOBJECT, LONG, PPYOBJECT), (*Py_tracefunc);

typedef struct _PYTHREADSTATE25_27 PYTHREADSTATE25_27, *PPYTHREADSTATE25_27, PyThreadState25_27;

typedef struct _PYTHREADSTATE25_27 {
    union {
        PyThreadState25_27 *next;
        PPYTHREADSTATE25_27 Next;
    };
    union {
        PyInterpreterState *interp;
        PPYINTERPRETERSTATE Interpreter;
    };
    union {
        PyFrameObject   *frame;
        PPYFRAMEOBJECT  Frame;
    };
    union {
        int recursion_depth;
        LONG RecursionDepth;
    };
    union {
        int tracing;
        LONG Tracing;
    };
    union {
        int use_tracing;
        LONG UseTracing;
    };
    union {
        Py_tracefunc    c_profilefunc;
        PPYTRACEFUNC    ProfileFunction;
    };
    union {
        Py_tracefunc    c_tracefunc;
        PPYTRACEFUNC    TraceFunction;
    };
    union {
        PyObject    *c_profileobj;
        PPYOBJECT    ProfileObject;
    };
    union {
        PyObject    *c_traceobj;
        PPYOBJECT    TraceObject;
    };
    union {
        PyObject    *curexc_type;
        PPYOBJECT    CurrentExceptionType;
    };
    union {
        PyObject    *curexc_value;
        PPYOBJECT    CurrentExceptionValue;
    };
    union {
        PyObject    *curexc_traceback;
        PPYOBJECT    CurrentExceptionTraceback;
    };
    union {
        PyObject    *exc_type;
        PPYOBJECT    ExceptionType;
    };
    union {
        PyObject    *exc_value;
        PPYOBJECT    ExceptionValue;
    };
    union {
        PyObject    *exc_traceback;
        PPYOBJECT    ExceptionTraceback;
    };
    union {
        PyObject    *dict;
        PPYOBJECT   Dict;
    };
    union {
        int tick_counter;
        LONG TickCounter;
    };
    union {
        int gilstate_counter;
        LONG GilstateCounter;
    };
    union {
        PyObject *async_exc;
        PPYOBJECT AsyncException;
    };
    union {
        long thread_id;
        LONG ThreadId;
    };
} PYTHREADSTATE25_27, *PPYTHREADSTATE25_27, PyThreadState25_27;

typedef struct _PYTHREADSTATE30_33 PYTHREADSTATE30_33, *PPYTHREADSTATE30_33, PyThreadState30_33;

typedef struct _PYTHREADSTATE30_33 {
    union {
        PyThreadState30_33 *next;
        PPYTHREADSTATE30_33 Next;
    };
    union {
        PyInterpreterState *interp;
        PPYINTERPRETERSTATE Interpreter;
    };
    union {
        PyFrameObject   *frame;
        PPYFRAMEOBJECT  Frame;
    };
    union {
        int recursion_depth;
        LONG RecursionDepth;
    };
    union {
        char overflowed;
        CHAR Overflowed;
    };
    union {
        char recursion_critical;
        CHAR RecursionCritical;
    };
    union {
        int tracing;
        LONG Tracing;
    };
    union {
        int use_tracing;
        LONG UseTracing;
    };
    union {
        Py_tracefunc    c_profilefunc;
        PPYTRACEFUNC    ProfileFunction;
    };
    union {
        Py_tracefunc    c_tracefunc;
        PPYTRACEFUNC    TraceFunction;
    };
    union {
        PyObject    *c_profileobj;
        PPYOBJECT    ProfileObject;
    };
    union {
        PyObject    *c_traceobj;
        PPYOBJECT    TraceObject;
    };
    union {
        PyObject    *curexc_type;
        PPYOBJECT    CurrentExceptionType;
    };
    union {
        PyObject    *curexc_value;
        PPYOBJECT    CurrentExceptionValue;
    };
    union {
        PyObject    *curexc_traceback;
        PPYOBJECT    CurrentExceptionTraceback;
    };
    union {
        PyObject    *exc_type;
        PPYOBJECT    ExceptionType;
    };
    union {
        PyObject    *exc_value;
        PPYOBJECT    ExceptionValue;
    };
    union {
        PyObject    *exc_traceback;
        PPYOBJECT    ExceptionTraceback;
    };
    union {
        PyObject    *dict;
        PPYOBJECT   Dict;
    };
    union {
        int tick_counter;
        LONG TickCounter;
    };
    union {
        int gilstate_counter;
        LONG GilstateCounter;
    };
    union {
        PyObject *async_exc;
        PPYOBJECT AsyncException;
    };
    union {
        long thread_id;
        LONG ThreadId;
    };
} PYTHREADSTATE30_33, *PPYTHREADSTATE30_33, PyThreadState30_33;

typedef struct _PYTHREADSTATE34_35 PYTHREADSTATE34_35, *PPYTHREADSTATE34_35;
typedef struct _PYTHREADSTATE34_35 PyThreadState34_35;

typedef struct _PYTHREADSTATE34_35 {
    union {
        PyThreadState34_35 *prev;
        PPYTHREADSTATE34_35 Prev;
    };
    union {
        PyThreadState34_35 *next;
        PPYTHREADSTATE34_35 Next;
    };
    union {
        PyInterpreterState *interp;
        PPYINTERPRETERSTATE Interpreter;
    };
    union {
        PyFrameObject   *frame;
        PPYFRAMEOBJECT  Frame;
    };
    union {
        int recursion_depth;
        LONG RecursionDepth;
    };
    union {
        char overflowed;
        CHAR Overflowed;
    };
    union {
        char recursion_critical;
        CHAR RecursionCritical;
    };
    union {
        int tracing;
        LONG Tracing;
    };
    union {
        int use_tracing;
        LONG UseTracing;
    };
    union {
        Py_tracefunc    c_profilefunc;
        PPYTRACEFUNC    ProfileFunction;
    };
    union {
        Py_tracefunc    c_tracefunc;
        PPYTRACEFUNC    TraceFunction;
    };
    union {
        PyObject    *c_profileobj;
        PPYOBJECT    ProfileObject;
    };
    union {
        PyObject    *c_traceobj;
        PPYOBJECT    TraceObject;
    };
    union {
        PyObject    *curexc_type;
        PPYOBJECT    CurrentExceptionType;
    };
    union {
        PyObject    *curexc_value;
        PPYOBJECT    CurrentExceptionValue;
    };
    union {
        PyObject    *curexc_traceback;
        PPYOBJECT    CurrentExceptionTraceback;
    };
    union {
        PyObject    *exc_type;
        PPYOBJECT    ExceptionType;
    };
    union {
        PyObject    *exc_value;
        PPYOBJECT    ExceptionValue;
    };
    union {
        PyObject    *exc_traceback;
        PPYOBJECT    ExceptionTraceback;
    };
    union {
        PyObject    *dict;
        PPYOBJECT   Dict;
    };
    union {
        int tick_counter;
        LONG TickCounter;
    };
    union {
        int gilstate_counter;
        LONG GilstateCounter;
    };
    union {
        PyObject *async_exc;
        PPYOBJECT AsyncException;
    };
    union {
        long thread_id;
        LONG ThreadId;
    };
} PYTHREADSTATE34_35, *PPYTHREADSTATE34_35, PyThreadState34_35;

typedef struct _PYINTOBJECT {
    _PYOBJECT_HEAD
    union {
        long ob_ival;
        LONG Value;
    };
} PYINTOBJECT, *PPYINTOBJECT, PyIntObject;

typedef struct _PYLONGOBJECT {
    _PYVAROBJECT_HEAD
    union {
        unsigned int ob_digit[1];
        DWORD Digit[1];
    };
} PYLONGOBJECT, *PPYLONGOBJECT, PyLongObject;

typedef struct _PY_HASH_SECRET {
    LONG Prefix;
    LONG Suffix;
} PY_HASH_SECRET, *PPY_HASH_SECRET;


//
// Our custom helper types
//

typedef struct _PYOBJECTEX {
    union {
        PPYOBJECT               Object;
        PPYVAROBJECT            VarObject;
        PPYTYPEOBJECT           Type;
        PPYCODEOBJECT25_27      Code25_27;
        PPYCODEOBJECT30_32      Code30_32;
        PPYCODEOBJECT33_35      Code33_35;
        PPYFRAMEOBJECT          Frame;
        PPYFRAMEOBJECT25_33     Frame25_33;
        PPYFRAMEOBJECT34_35     Frame34_35;
        PPYLONGOBJECT           Long;
        PPYINTOBJECT            Int;
        PPYTUPLEOBJECT          Tuple;
        PPYFUNCTIONOBJECT       Function;
        PPYBYTESOBJECT          Bytes;
        PPYSTRINGOBJECT         String;
        PPYUNICODEOBJECT        Unicode;
        PPYINSTANCEOBJECT       Instance;
    };
} PYOBJECTEX, *PPYOBJECTEX, **PPPYOBJECTEX;

typedef struct _LINE_NUMBER {
    union {
        WCHAR Pair;
        CHAR Value[2];
        struct {
            BYTE ByteIncrement;
            BYTE LineIncrement;
        };
    };
} LINE_NUMBER, *PLINE_NUMBER, **PPLINE_NUMBER;

//
// Python 2.x uses a PyString type for the line number table.
//

typedef struct _LINE_NUMBER_TABLE2 {

    //
    // Inline PYOBJECT.
    //

    SSIZE_T ReferenceCount;
    PPYTYPEOBJECT Type;

    //
    // Inline PYVAROBJECT.
    //

    SSIZE_T ObjectSize;

    //
    // Inline PYSTRINGOBJECT.
    //

    LONG Hash;
    LONG State;

    LINE_NUMBER Table[1];

} LINE_NUMBER_TABLE2, *PLINE_NUMBER_TABLE2;

//
// Python 3.x uses a PyBytes type for the line number table.
//

typedef struct _LINE_NUMBER_TABLE3 {

    //
    // Inline PYOBJECT.
    //

    SSIZE_T ReferenceCount;
    PPYTYPEOBJECT Type;

    //
    // Inline PYVAROBJECT.
    //

    SSIZE_T ObjectSize;

    //
    // Inline PYBYTESOBJECT.
    //

    LONG Hash;

    LINE_NUMBER Table[1];

} LINE_NUMBER_TABLE3, *PLINE_NUMBER_TABLE3;

typedef union _PYGC_HEAD PYGC_HEAD, *PPYGC_HEAD, **PPPYGC_HEAD;
typedef union _PYGC_HEAD {
    struct {
        PPYGC_HEAD Next;
        PPYGC_HEAD Prev;
        PY_SSIZE ReferenceState;
    } Gc;
    long double Dummy;
} PYGC_HEAD, *PPYGC_HEAD, **PPPYGC_HEAD;

typedef enum _PYGILSTATE {
    PyGILState_LOCKED,
    PyGILState_UNLOCKED
} PYGILSTATE, *PPYGILSTATE, PyGILState_STATE;


// Typedefs for Python function pointers.
//

typedef PCSTR (*PPY_GETVERSION)();
typedef VOID (*PPY_INCREF)(PPYOBJECT);
typedef VOID (*PPY_DECREF)(PPYOBJECT);
typedef LONG (*PPYFRAME_GETLINENUMBER)(PPYFRAMEOBJECT FrameObject);
typedef LONG (*PPYCODE_ADDR2LINE)(PPYOBJECT CodeObject, LONG);
typedef PWSTR (*PPYUNICODE_ASUNICODE)(PPYOBJECT Object);
typedef SSIZE_T (*PPYUNICODE_GETLENGTH)(PPYOBJECT Object);
typedef LONG (*PPYEVAL_SETTRACE)(PPYTRACEFUNC, PPYOBJECT);
typedef LONG (*PPYEVAL_SETPROFILE)(PPYTRACEFUNC, PPYOBJECT);
typedef PPYOBJECT (*PPYDICT_GETITEMSTRING)(PPYOBJECT, PCCH);
typedef PYGILSTATE (*PPYGILSTATE_ENSURE)();
typedef VOID (*PPYGILSTATE_RELEASE)(PYGILSTATE);
typedef LONG (*PPYOBJECT_HASH)(PPYOBJECT);
typedef INT (*PPYOBJECT_COMPARE)(PPYOBJECT, PPYOBJECT, PINT);
typedef PVOID (*PPYMEM_MALLOC)(SIZE_T);
typedef PVOID (*PPYMEM_REALLOC)(PVOID, SIZE_T);
typedef VOID (*PPYMEM_FREE)(PVOID);
typedef PVOID (*PPYOBJECT_MALLOC)(SIZE_T);
typedef PVOID (*PPYOBJECT_REALLOC)(PVOID, SIZE_T);
typedef VOID (*PPYOBJECT_FREE)(PVOID);
typedef SSIZE_T (*PPYGC_COLLECT)(VOID);
typedef PPYOBJECT (*PPYOBJECT_GC_MALLOC)(SIZE_T);
typedef PPYOBJECT (*PPYOBJECT_GC_NEW)(PPYTYPEOBJECT);
typedef PPYVAROBJECT (*PPYOBJECT_GC_NEWVAR)(PPYTYPEOBJECT, SSIZE_T);
typedef PPYVAROBJECT (*PPYOBJECT_GC_RESIZE)(PPYVAROBJECT, SSIZE_T);
typedef VOID (*PPYOBJECT_GC_TRACK)(PVOID);
typedef VOID (*PPYOBJECT_GC_UNTRACK)(PVOID);
typedef VOID (*PPYOBJECT_GC_DEL)(PVOID);
typedef PPYOBJECT (*PPYOBJECT_INIT)(PPYOBJECT, PPYTYPEOBJECT);
typedef PPYVAROBJECT (*PPYOBJECT_INITVAR)(PPYVAROBJECT,
                                          PPYTYPEOBJECT,
                                          SSIZE_T);
typedef PPYOBJECT (*PPYOBJECT_NEW)(PPYTYPEOBJECT);
typedef PPYVAROBJECT (*PPYOBJECT_NEWVAR)(PPYTYPEOBJECT, PY_SSIZE);

typedef VOID (PY_INITIALIZE)(VOID);
typedef PY_INITIALIZE *PPY_INITIALIZE;

typedef VOID (PY_INITIALIZE_EX)(INT);
typedef PY_INITIALIZE_EX *PPY_INITIALIZE_EX;

typedef VOID (PY_FINALIZE)(VOID);
typedef PY_FINALIZE *PPY_FINALIZE;

typedef INT (PY_IS_INITIALIZED)(VOID);
typedef PY_IS_INITIALIZED *PPY_IS_INITIALIZED;

typedef LONG (PY_MAINA)(_In_ LONG, _In_ PPCHAR);
typedef PY_MAINA *PPY_MAINA;

typedef LONG (PY_MAINW)(_In_ LONG, _In_ PPWCHAR);
typedef PY_MAINW *PPY_MAINW;

typedef PCHAR (PY_GET_PREFIXA)(VOID);
typedef PY_GET_PREFIXA *PPY_GET_PREFIXA;

typedef PWCHAR (PY_GET_PREFIXW)(VOID);
typedef PY_GET_PREFIXW *PPY_GET_PREFIXW;

typedef PCHAR (PY_GET_EXEC_PREFIXA)(VOID);
typedef PY_GET_EXEC_PREFIXA *PPY_GET_EXEC_PREFIXA;

typedef PWCHAR (PY_GET_EXEC_PREFIXW)(VOID);
typedef PY_GET_EXEC_PREFIXW *PPY_GET_EXEC_PREFIXW;

typedef PCHAR (PY_GET_PROGRAM_FULL_PATHA)(VOID);
typedef PY_GET_PROGRAM_FULL_PATHA *PPY_GET_PROGRAM_FULL_PATHA;

typedef PWCHAR (PY_GET_PROGRAM_FULL_PATHW)(VOID);
typedef PY_GET_PROGRAM_FULL_PATHW *PPY_GET_PROGRAM_FULL_PATHW;

typedef PCHAR (PY_GET_PROGRAM_NAMEA)(VOID);
typedef PY_GET_PROGRAM_NAMEA *PPY_GET_PROGRAM_NAMEA;

typedef PWCHAR (PY_GET_PROGRAM_NAMEW)(VOID);
typedef PY_GET_PROGRAM_NAMEW *PPY_GET_PROGRAM_NAMEW;

typedef VOID (PY_SET_PROGRAM_NAMEA)(PCHAR);
typedef PY_SET_PROGRAM_NAMEA *PPY_SET_PROGRAM_NAMEA;

typedef VOID (PY_SET_PROGRAM_NAMEW)(PWCHAR);
typedef PY_SET_PROGRAM_NAMEW *PPY_SET_PROGRAM_NAMEW;

typedef VOID (PY_SET_PYTHON_HOMEA)(PCHAR);
typedef PY_SET_PYTHON_HOMEA *PPY_SET_PYTHON_HOMEA;

typedef VOID (PY_SET_PYTHON_HOMEW)(PWCHAR);
typedef PY_SET_PYTHON_HOMEW *PPY_SET_PYTHON_HOMEW;

typedef PWCHAR (PY_GET_PYTHON_HOMEA)(VOID);
typedef PY_GET_PYTHON_HOMEA *PPY_GET_PYTHON_HOMEA;

typedef PWCHAR (PY_GET_PYTHON_HOMEW)(VOID);
typedef PY_GET_PYTHON_HOMEW *PPY_GET_PYTHON_HOMEW;

typedef VOID (PYSYS_SET_ARGVA)(INT, PPCHAR);
typedef PYSYS_SET_ARGVA *PPYSYS_SET_ARGVA;

typedef VOID (PYSYS_SET_ARGVW)(INT, PPWCHAR);
typedef PYSYS_SET_ARGVW *PPYSYS_SET_ARGVW;

typedef VOID (PYSYS_SET_ARGV_EXA)(INT, PPCHAR, INT);
typedef PYSYS_SET_ARGV_EXA *PPYSYS_SET_ARGV_EXA;

typedef VOID (PYSYS_SET_ARGV_EXW)(INT, PPWCHAR, INT);
typedef PYSYS_SET_ARGV_EXW *PPYSYS_SET_ARGV_EXW;

#define _PYTHONFUNCTIONS_HEAD                               \
    PPYSYS_SET_ARGVA                PySys_SetArgvA;         \
    PPYSYS_SET_ARGVW                PySys_SetArgvW;         \
    PPYSYS_SET_ARGV_EXA             PySys_SetArgvExA;       \
    PPYSYS_SET_ARGV_EXW             PySys_SetArgvExW;       \
    PPY_SET_PROGRAM_NAMEA           Py_SetProgramNameA;     \
    PPY_SET_PROGRAM_NAMEW           Py_SetProgramNameW;     \
    PPY_GET_PROGRAM_NAMEA           Py_GetProgramNameA;     \
    PPY_GET_PROGRAM_NAMEW           Py_GetProgramNameW;     \
    PPY_SET_PYTHON_HOMEA            Py_SetPythonHomeA;      \
    PPY_SET_PYTHON_HOMEW            Py_SetPythonHomeW;      \
    PPY_MAINA                       Py_MainA;               \
    PPY_MAINW                       Py_MainW;               \
    PPY_GET_PREFIXA                 Py_GetPrefixA;          \
    PPY_GET_PREFIXW                 Py_GetPrefixW;          \
    PPY_GET_EXEC_PREFIXA            Py_GetExecPrefixA;      \
    PPY_GET_EXEC_PREFIXW            Py_GetExecPrefixW;      \
    PPY_GET_PROGRAM_FULL_PATHA      Py_GetProgramFullPathA; \
    PPY_GET_PROGRAM_FULL_PATHW      Py_GetProgramFullPathW; \
    PPY_INITIALIZE                  Py_Initialize;          \
    PPY_INITIALIZE_EX               Py_InitializeEx;        \
    PPY_IS_INITIALIZED              Py_IsInitialized;       \
    PPY_FINALIZE                    Py_Finalize;            \
    PPY_GETVERSION                  Py_GetVersion;          \
    PPYDICT_GETITEMSTRING           PyDict_GetItemString;   \
    PPYFRAME_GETLINENUMBER          PyFrame_GetLineNumber;  \
    PPYCODE_ADDR2LINE               PyCode_Addr2Line;       \
    PPYEVAL_SETTRACE                PyEval_SetProfile;      \
    PPYEVAL_SETTRACE                PyEval_SetTrace;        \
    PPYUNICODE_ASUNICODE            PyUnicode_AsUnicode;    \
    PPYUNICODE_GETLENGTH            PyUnicode_GetLength;    \
    PPY_INCREF                      Py_IncRef;              \
    PPY_DECREF                      Py_DecRef;              \
    PPYGILSTATE_ENSURE              PyGILState_Ensure;      \
    PPYGILSTATE_RELEASE             PyGILState_Release;     \
    PPYOBJECT_HASH                  PyObject_Hash;          \
    PPYOBJECT_COMPARE               PyObject_Compare;       \
    PPYMEM_MALLOC                   PyMem_Malloc;           \
    PPYMEM_REALLOC                  PyMem_Realloc;          \
    PPYMEM_FREE                     PyMem_Free;             \
    PPYOBJECT_MALLOC                PyObject_Malloc;        \
    PPYOBJECT_REALLOC               PyObject_Realloc;       \
    PPYOBJECT_FREE                  PyObject_Free;          \
    PPYGC_COLLECT                   PyGC_Collect;           \
    PPYOBJECT_GC_MALLOC             _PyObject_GC_Malloc;    \
    PPYOBJECT_GC_NEW                _PyObject_GC_New;       \
    PPYOBJECT_GC_NEWVAR             _PyObject_GC_NewVar;    \
    PPYOBJECT_GC_RESIZE             _PyObject_GC_Resize;    \
    PPYOBJECT_GC_TRACK              PyObject_GC_Track;      \
    PPYOBJECT_GC_UNTRACK            PyObject_GC_UnTrack;    \
    PPYOBJECT_GC_DEL                PyObject_GC_Del;        \
    PPYOBJECT_INIT                  PyObject_Init;          \
    PPYOBJECT_INITVAR               PyObject_InitVar;       \
    PPYOBJECT_NEW                   _PyObject_New;          \
    PPYOBJECT_NEWVAR                _PyObject_NewVar;

typedef struct _PYTHONFUNCTIONS {
    _PYTHONFUNCTIONS_HEAD
} PYTHONFUNCTIONS, *PPYTHONFUNCTIONS;

#define _PYTHONDATA_HEAD           \
    PYOBJECTEX _Py_NoneStruct;     \
    PYOBJECTEX _Py_TrueStruct;     \
    PYOBJECTEX _Py_FalseStruct;    \
    PYOBJECTEX _Py_ZeroStruct;     \
    PY_HASH_SECRET _Py_HashSecret; \
    PYTYPEOBJECTEX PyString;       \
    PYTYPEOBJECTEX PyBytes;        \
    PYTYPEOBJECTEX PyCode;         \
    PYTYPEOBJECTEX PyDict;         \
    PYTYPEOBJECTEX PyTuple;        \
    PYTYPEOBJECTEX PyType;         \
    PYTYPEOBJECTEX PyFunction;     \
    PYTYPEOBJECTEX PyUnicode;      \
    PYTYPEOBJECTEX PyCFunction;    \
    PYTYPEOBJECTEX PyInstance;     \
    PYTYPEOBJECTEX PyModule;

typedef struct _PYTHONDATA {
    _PYTHONDATA_HEAD
} PYTHONDATA, *PPYTHONDATA;

#define _PYTHONVERSION_HEAD \
    PCSTR   VersionString;  \
    USHORT  MajorVersion;   \
    USHORT  MinorVersion;   \
    USHORT  PatchLevel;

typedef struct _PYTHONVERSION {
    _PYTHONVERSION_HEAD
} PYTHONVERSION, *PPYTHONVERSION;

typedef struct _PYTHON PYTHON, *PPYTHON, **PPPYTHON;

typedef struct _PYTHON_PATH_TABLE PYTHON_PATH_TABLE;
typedef PYTHON_PATH_TABLE *PPYTHON_PATH_TABLE;
typedef PYTHON_PATH_TABLE **PPPYTHON_PATH_TABLE;

typedef struct _PYTHON_PATH_TABLE_ENTRY PYTHON_PATH_TABLE_ENTRY;
typedef PYTHON_PATH_TABLE_ENTRY *PPYTHON_PATH_TABLE_ENTRY;
typedef PYTHON_PATH_TABLE_ENTRY **PPPYTHON_PATH_TABLE_ENTRY;

typedef struct _PYTHON_FUNCTION PYTHON_FUNCTION;
typedef PYTHON_FUNCTION *PPYTHON_FUNCTION;
typedef PYTHON_FUNCTION **PPPYTHON_FUNCTION;

typedef struct _PYTHON_FUNCTION_TABLE PYTHON_FUNCTION_TABLE;
typedef PYTHON_FUNCTION_TABLE *PPYTHON_FUNCTION_TABLE;
typedef PYTHON_FUNCTION_TABLE **PPPYTHON_FUNCTION_TABLE;

typedef struct _PYTHON_FUNCTION_TABLE_ENTRY PYTHON_FUNCTION_TABLE_ENTRY;
typedef PYTHON_FUNCTION_TABLE_ENTRY *PPYTHON_FUNCTION_TABLE_ENTRY;
typedef PYTHON_FUNCTION_TABLE_ENTRY **PPPYTHON_FUNCTION_TABLE_ENTRY;

typedef struct _PYTHON_ALLOCATOR {
    PALLOCATION_ROUTINE AllocationRoutine;
    PVOID AllocationContext;
    PFREE_ROUTINE FreeRoutine;
    PVOID FreeContext;
} PYTHON_ALLOCATOR, *PPYTHON_ALLOCATOR, **PPPYTHON_ALLOCATOR;

typedef PYTHON_ALLOCATOR PYTHON_STRING_ALLOCATOR;
typedef PYTHON_STRING_ALLOCATOR *PPYTHON_STRING_ALLOCATOR;
typedef PYTHON_STRING_ALLOCATOR **PPPYTHON_STRING_ALLOCATOR;

typedef PYTHON_ALLOCATOR PYTHON_HASHED_STRING_ALLOCATOR;
typedef PYTHON_HASHED_STRING_ALLOCATOR *PPYTHON_HASHED_STRING_ALLOCATOR;
typedef PYTHON_HASHED_STRING_ALLOCATOR **PPPYTHON_HASHED_STRING_ALLOCATOR;

typedef PYTHON_ALLOCATOR PYTHON_BUFFER_ALLOCATOR;
typedef PYTHON_BUFFER_ALLOCATOR *PPYTHON_BUFFER_ALLOCATOR;
typedef PYTHON_BUFFER_ALLOCATOR **PPPYTHON_BUFFER_ALLOCATOR;

typedef PYTHON_ALLOCATOR PYTHON_FUNCTION_TABLE_ALLOCATOR;
typedef PYTHON_FUNCTION_TABLE_ALLOCATOR *PPYTHON_FUNCTION_TABLE_ALLOCATOR;
typedef PYTHON_FUNCTION_TABLE_ALLOCATOR **PPPYTHON_FUNCTION_TABLE_ALLOCATOR;

typedef PYTHON_ALLOCATOR PYTHON_FUNCTION_TABLE_ENTRY_ALLOCATOR;
typedef PYTHON_FUNCTION_TABLE_ENTRY_ALLOCATOR \
       *PPYTHON_FUNCTION_TABLE_ENTRY_ALLOCATOR;
typedef PYTHON_FUNCTION_TABLE_ENTRY_ALLOCATOR \
      **PPPYTHON_FUNCTION_TABLE_ENTRY_ALLOCATOR;

typedef PYTHON_ALLOCATOR PYTHON_PATH_TABLE_ALLOCATOR;
typedef PYTHON_PATH_TABLE_ALLOCATOR *PPYTHON_PATH_TABLE_ALLOCATOR;
typedef PYTHON_PATH_TABLE_ALLOCATOR **PPPYTHON_PATH_TABLE_ALLOCATOR;

typedef PYTHON_ALLOCATOR PYTHON_PATH_TABLE_ENTRY_ALLOCATOR;
typedef PYTHON_PATH_TABLE_ENTRY_ALLOCATOR *PPYTHON_PATH_TABLE_ENTRY_ALLOCATOR;
typedef PYTHON_PATH_TABLE_ENTRY_ALLOCATOR **PPPYTHON_PATH_TABLE_ENTRY_ALLOCATOR;

typedef PYTHON_ALLOCATOR PYTHON_FILENAME_STRING_ALLOCATOR;
typedef PYTHON_FILENAME_STRING_ALLOCATOR *PPYTHON_FILENAME_STRING_ALLOCATOR;
typedef PYTHON_FILENAME_STRING_ALLOCATOR **PPPYTHON_FILENAME_STRING_ALLOCATOR;

typedef PYTHON_ALLOCATOR PYTHON_DIRECTORY_STRING_ALLOCATOR;
typedef PYTHON_DIRECTORY_STRING_ALLOCATOR *PPYTHON_DIRECTORY_STRING_ALLOCATOR;
typedef PYTHON_DIRECTORY_STRING_ALLOCATOR **PPPYTHON_DIRECTORY_STRING_ALLOCATOR;

typedef PYTHON_ALLOCATOR PYTHON_STRING_ARRAY_ALLOCATOR;
typedef PYTHON_STRING_ARRAY_ALLOCATOR *PPYTHON_STRING_ARRAY_ALLOCATOR;
typedef PYTHON_STRING_ARRAY_ALLOCATOR **PPPYTHON_STRING_ARRAY_ALLOCATOR;

typedef PYTHON_ALLOCATOR PYTHON_STRING_TABLE_ALLOCATOR;
typedef PYTHON_STRING_TABLE_ALLOCATOR *PPYTHON_STRING_TABLE_ALLOCATOR;
typedef PYTHON_STRING_TABLE_ALLOCATOR **PPPYTHON_STRING_TABLE_ALLOCATOR;

typedef struct _PYTHON_ALLOCATORS {
    ULONG SizeInBytes;
    ULONG NumberOfAllocators;

    PYTHON_STRING_ALLOCATOR               String;
    PYTHON_BUFFER_ALLOCATOR               StringBuffer;
    PYTHON_HASHED_STRING_ALLOCATOR        HashedString;
    PYTHON_BUFFER_ALLOCATOR               HashedStringBuffer;
    PYTHON_BUFFER_ALLOCATOR               Buffer;
    PYTHON_FUNCTION_TABLE_ALLOCATOR       FunctionTable;
    PYTHON_FUNCTION_TABLE_ENTRY_ALLOCATOR FunctionTableEntry;
    PYTHON_PATH_TABLE_ALLOCATOR           PathTable;
    PYTHON_PATH_TABLE_ENTRY_ALLOCATOR     PathTableEntry;
    PYTHON_FILENAME_STRING_ALLOCATOR      FilenameString;
    PYTHON_BUFFER_ALLOCATOR               FilenameStringBuffer;
    PYTHON_DIRECTORY_STRING_ALLOCATOR     DirectoryString;
    PYTHON_BUFFER_ALLOCATOR               DirectoryStringBuffer;
    PYTHON_STRING_ARRAY_ALLOCATOR         StringArray;
    PYTHON_STRING_TABLE_ALLOCATOR         StringTable;

} PYTHON_ALLOCATORS, *PPYTHON_ALLOCATORS, **PPPYTHON_ALLOCATORS;

#define ALLOCATE(Name, Size)                           \
    Python->Allocators.##Name##.AllocationRoutine(     \
        Python->Allocators.##Name##.AllocationContext, \
        Size                                           \
    )

#define FREE(Name, Pointer)                      \
    Python->Allocators.##Name##.FreeRoutine(     \
        Python->Allocators.##Name##.FreeContext, \
        Pointer                                  \
    )

typedef struct _PYTHON_HASHED_STRING {
    ULONG Atom;                             //  4
    LONG  Hash;                             //  4       8

    //
    // Inline STRING struct.
    //

    union {
        STRING String;
        struct {
            USHORT Length;
            USHORT MaximumLength;
            PCHAR Buffer;
        };
    };

} PYTHON_HASHED_STRING, *PPYTHON_HASHED_STRING, **PPPYTHON_HASHED_STRING;

//
// This structure is used to communicate information about a filename path
// between frame and path registration.
//

typedef union _FILENAME_FLAGS {
    ULONG AsLong;
    struct _Struct_size_bytes_(sizeof(ULONG)) {
        ULONG IsSpecialName:1;
        ULONG IsFullyQualified:1;
        ULONG WeOwnPathBuffer:1;
        ULONG IsDll:1;
    };
} FILENAME_FLAGS, *PFILENAME_FLAGS;
C_ASSERT(sizeof(FILENAME_FLAGS) == sizeof(ULONG));

//
// Constants
//

PYTHON_EX_DATA CONST USHORT TargetSizeOfPythonPathTableEntry;
PYTHON_EX_DATA CONST USHORT TargetSizeOfPythonFunctionTableEntry;

PYTHON_EX_DATA CONST UNICODE_STRING W__init__py;
PYTHON_EX_DATA CONST UNICODE_STRING W__init__pyc;
PYTHON_EX_DATA CONST UNICODE_STRING W__init__pyo;

PYTHON_EX_DATA CONST PUNICODE_STRING InitPyFilesW[3];

PYTHON_EX_DATA CONST STRING A__init__py;
PYTHON_EX_DATA CONST STRING A__init__pyc;
PYTHON_EX_DATA CONST STRING A__init__pyo;

PYTHON_EX_DATA CONST PUNICODE_STRING InitPyFilesA[3];

PYTHON_EX_DATA CONST USHORT NumberOfInitPyFiles;

PYTHON_EX_DATA CONST STRING SELF_A;
PYTHON_EX_DATA CONST STRING __BUILTIN__A;
PYTHON_EX_DATA CONST STRING BUILTINS_A;

PYTHON_EX_DATA CONST PYCODEOBJECTOFFSETS PyCodeObjectOffsets25_27;
PYTHON_EX_DATA CONST PYCODEOBJECTOFFSETS PyCodeObjectOffsets30_32;
PYTHON_EX_DATA CONST PYCODEOBJECTOFFSETS PyCodeObjectOffsets33_35;

PYTHON_EX_DATA CONST PYFRAMEOBJECTOFFSETS PyFrameObjectOffsets25_33;
PYTHON_EX_DATA CONST PYFRAMEOBJECTOFFSETS PyFrameObjectOffsets34_35;

//
// PythonAllocator-related functions.
//

typedef
VOID
(FREE_PYTHON_PATH_TABLE_ENTRY)(
    _In_        PPYTHON                  Python,
    _In_opt_    PPYTHON_PATH_TABLE_ENTRY PathTableEntry
    );
typedef FREE_PYTHON_PATH_TABLE_ENTRY *PFREE_PYTHON_PATH_TABLE_ENTRY;
typedef FREE_PYTHON_PATH_TABLE_ENTRY **PPFREE_PYTHON_PATH_TABLE_ENTRY;

//
// PythonFunction-related functions.
//

typedef
_Success_(return != 0)
BOOL
(REGISTER_FUNCTION)(
    _In_ PPYTHON Python,
    _In_ PPYTHON_FUNCTION Function,
    _In_ PPYFRAMEOBJECT FrameObject
    );
typedef REGISTER_FUNCTION *PREGISTER_FUNCTION, **PPREGISTER_FUNCTION;

typedef
_Success_(return != 0)
BOOL
(GET_SELF)(
    _In_ PPYTHON Python,
    _In_ PPYTHON_FUNCTION Function,
    _In_ PPYFRAMEOBJECT FrameObject,
    _Outptr_result_nullonfailure_ PPPYOBJECT SelfPointer
    );
typedef GET_SELF *PGET_SELF, **PPGET_SELF;

typedef
_Success_(return != 0)
BOOL
(GET_CLASS_NAME_FROM_SELF)(
    _In_    PPYTHON             Python,
    _In_    PPYTHON_FUNCTION    Function,
    _In_    PPYFRAMEOBJECT      FrameObject,
    _In_    PPYOBJECT           Self,
    _In_    PSTRING             FunctionName,
    _Outptr_result_nullonfailure_ PPCHAR ClassNameBuffer
    );
typedef GET_CLASS_NAME_FROM_SELF *PGET_CLASS_NAME_FROM_SELF;
typedef GET_CLASS_NAME_FROM_SELF **PPGET_CLASS_NAME_FROM_SELF;

//
// PythonLineNumbers-related functions.
//

typedef
VOID
(RESOLVE_LINE_NUMBERS)(
    _In_ PPYTHON Python,
    _In_ PPYTHON_FUNCTION Function
    );
typedef RESOLVE_LINE_NUMBERS *PRESOLVE_LINE_NUMBERS;
typedef RESOLVE_LINE_NUMBERS **PPRESOLVE_LINE_NUMBERS;

//
// PythonPathTableEntry-related functions.
//

typedef
_Success_(return != 0)
BOOL
(REGISTER_FRAME)(
    _In_      PPYTHON             Python,
    _In_      PPYFRAMEOBJECT      FrameObject,
    _In_      PYTHON_EVENT_TRAITS EventTraits,
    _In_opt_  PPYOBJECT           ArgObject,
    _Outptr_result_nullonfailure_ PPPYTHON_FUNCTION FunctionPointer
    );
typedef REGISTER_FRAME *PREGISTER_FRAME;
typedef REGISTER_FRAME *PPREGISTER_FRAME;

typedef
_Success_(return != 0)
BOOL
(GET_PATH_ENTRY_FOR_DIRECTORY)(
    _In_     PPYTHON             Python,
    _In_     PSTRING             Directory,
    _In_     PRTL_BITMAP         Backslashes,
    _In_     PUSHORT             BitmapHintIndex,
    _In_     PUSHORT             NumberOfBackslashesRemaining,
    _Outptr_result_nullonfailure_ PPPYTHON_PATH_TABLE_ENTRY PathEntryPointer
    );
typedef GET_PATH_ENTRY_FOR_DIRECTORY *PGET_PATH_ENTRY_FOR_DIRECTORY;
typedef GET_PATH_ENTRY_FOR_DIRECTORY **PPGET_PATH_ENTRY_FOR_DIRECTORY;

typedef
_Success_(return != 0)
BOOL
(REGISTER_FILE)(
    _In_  PPYTHON Python,
    _In_  PPYFRAMEOBJECT FrameObject,
    _In_  PSTRING FilenameString,
    _In_  PFILENAME_FLAGS FilenameFlags,
    _Outptr_result_nullonfailure_ PPPYTHON_PATH_TABLE_ENTRY PathEntryPointer
    );
typedef REGISTER_FILE *PREGISTER_FILE, **PPREGISTER_FILE;

typedef
_Success_(return != 0)
BOOL
(GET_PATH_ENTRY_FROM_FRAME)(
    _In_      PPYTHON             Python,
    _In_      PPYFRAMEOBJECT      FrameObject,
    _In_      PYTHON_EVENT_TRAITS EventTraits,
    _In_opt_  PPYOBJECT           ArgObject,
    _In_      PSTRING             FilenameString,
    _In_      PFILENAME_FLAGS     FilenameFlags,
    _Outptr_result_nullonfailure_ PPPYTHON_PATH_TABLE_ENTRY PathEntryPointer
    );
typedef GET_PATH_ENTRY_FROM_FRAME *PGET_PATH_ENTRY_FROM_FRAME;
typedef GET_PATH_ENTRY_FROM_FRAME **PPGET_PATH_ENTRY_FROM_FRAME;

typedef
_Success_(return != 0)
BOOL
(REGISTER_DIRECTORY)(
    _In_      PPYTHON Python,
    _In_      PSTRING Directory,
    _In_opt_  PSTRING DirectoryName,
    _In_opt_  PPYTHON_PATH_TABLE_ENTRY AncestorEntry,
    _Outptr_result_nullonfailure_ PPPYTHON_PATH_TABLE_ENTRY EntryPointer,
    _In_      BOOL IsRoot
    );
typedef REGISTER_DIRECTORY *PREGISTER_DIRECTORY;
typedef REGISTER_DIRECTORY **PPREGISTER_DIRECTORY;

typedef
_Success_(return != 0)
BOOL
(QUALIFY_PATH)(
    _In_ PPYTHON Python,
    _In_ PSTRING SourcePath,
    _Outptr_result_nullonfailure_ PPSTRING DestinationPathPointer
    );
typedef QUALIFY_PATH *PQUALIFY_PATH, **PPQUALIFY_PATH;

//
// Initializer functions.
//

typedef
_Success_(return != 0)
BOOL
(INITIALIZE_PYTHON_RUNTIME_TABLES)(
    _In_ PPYTHON Python
    );
typedef INITIALIZE_PYTHON_RUNTIME_TABLES *PINITIALIZE_PYTHON_RUNTIME_TABLES;
typedef INITIALIZE_PYTHON_RUNTIME_TABLES **PPINITIALIZE_PYTHON_RUNTIME_TABLES;

typedef
_Success_(return != 0)
BOOL
(INITIALIZE_PYTHON)(
    _In_                         PRTL                Rtl,
    _In_                         HMODULE             PythonModule,
    _Out_bytecap_(*SizeOfPython) PPYTHON             Python,
    _Inout_                      PULONG              SizeOfPython
    );
typedef INITIALIZE_PYTHON *PINITIALIZE_PYTHON;

//
// Allocator function type definitions.
//

typedef
_Success_(return != 0)
BOOL
(ALLOCATE_STRING)(
    _In_  PPYTHON  Python,
    _Outptr_result_nullonfailure_ PPSTRING StringPointer
    );
typedef ALLOCATE_STRING *PALLOCATE_STRING;
typedef ALLOCATE_STRING **PPALLOCATE_STRING;

typedef
_Success_(return != 0)
BOOL
(ALLOCATE_BUFFER)(
    _In_  PPYTHON Python,
    _In_  ULONG   SizeInBytes,
    _Outptr_result_nullonfailure_ PPVOID BufferPointer
    );
typedef ALLOCATE_BUFFER *PALLOCATE_BUFFER;
typedef ALLOCATE_BUFFER **PPALLOCATE_BUFFER;

typedef
_Success_(return != 0)
BOOL
(ALLOCATE_STRING_BUFFER)(
    _In_  PPYTHON  Python,
    _In_  USHORT   StringBufferSizeInBytes,
    _In_  PSTRING  String
    );
typedef ALLOCATE_STRING_BUFFER *PALLOCATE_STRING_BUFFER;
typedef ALLOCATE_STRING_BUFFER **PPALLOCATE_STRING_BUFFER;

typedef
_Success_(return != 0)
BOOL
(ALLOCATE_STRING_AND_BUFFER)(
    _In_  PPYTHON  Python,
    _In_  USHORT   StringBufferSizeInBytes,
    _Outptr_result_nullonfailure_ PPSTRING StringPointer
    );
typedef ALLOCATE_STRING_AND_BUFFER *PALLOCATE_STRING_AND_BUFFER;
typedef ALLOCATE_STRING_AND_BUFFER **PPALLOCATE_STRING_AND_BUFFER;

typedef
_Success_(return != 0)
BOOL
(ALLOCATE_HASHED_STRING)(
    _In_  PPYTHON Python,
    _Outptr_result_nullonfailure_ PPPYTHON_HASHED_STRING HashedStringPointer
    );
typedef ALLOCATE_HASHED_STRING *PALLOCATE_HASHED_STRING;
typedef ALLOCATE_HASHED_STRING **PPALLOCATE_HASHED_STRING;

typedef
_Success_(return != 0)
BOOL
(ALLOCATE_HASHED_STRING_AND_BUFFER)(
    _In_  PPYTHON                Python,
    _In_  USHORT                 StringBufferSizeInBytes,
    _Outptr_result_nullonfailure_ PPPYTHON_HASHED_STRING HashedStringPointer
    );
typedef ALLOCATE_HASHED_STRING_AND_BUFFER *PALLOCATE_HASHED_STRING_AND_BUFFER;
typedef ALLOCATE_HASHED_STRING_AND_BUFFER **PPALLOCATE_HASHED_STRING_AND_BUFFER;

typedef
_Success_(return != 0)
BOOL
(FINALIZE_HASHED_STRING)(
    _In_ PPYTHON Python,

    _Outptr_result_nullonfailure_ PPYTHON_HASHED_STRING
                                  HashedString,

    _Outptr_result_nullonfailure_ PPPYTHON_HASHED_STRING
                                  ExistingHashedStringPointer
    );
typedef FINALIZE_HASHED_STRING *PFINALIZE_HASHED_STRING;
typedef FINALIZE_HASHED_STRING **PPFINALIZE_HASHED_STRING;

typedef
_Success_(return != 0)
BOOL
(ALLOCATE_PYTHON_FUNCTION_TABLE)(
    _In_  PPYTHON Python,
    _Outptr_result_nullonfailure_ PPPYTHON_FUNCTION_TABLE FunctionTablePointer
    );
typedef ALLOCATE_PYTHON_FUNCTION_TABLE *PALLOCATE_PYTHON_FUNCTION_TABLE;

typedef
_Success_(return != 0)
BOOL
(ALLOCATE_PYTHON_PATH_TABLE)(
    _In_  PPYTHON Python,
    _Outptr_result_nullonfailure_ PPPYTHON_PATH_TABLE PathTablePointer
    );
typedef ALLOCATE_PYTHON_PATH_TABLE *PALLOCATE_PYTHON_PATH_TABLE;

typedef
_Success_(return != 0)
BOOL
(ALLOCATE_PYTHON_PATH_TABLE_ENTRY)(
    _In_  PPYTHON Python,
    _Outptr_result_nullonfailure_ PPPYTHON_PATH_TABLE_ENTRY
                                  PathTableEntryPointer
    );
typedef ALLOCATE_PYTHON_PATH_TABLE_ENTRY *PALLOCATE_PYTHON_PATH_TABLE_ENTRY;

typedef
_Success_(return != 0)
BOOL
(ALLOCATE_PYTHON_PATH_TABLE_ENTRY_AND_STRING)(
    _In_  PPYTHON Python,

    _Outptr_result_nullonfailure_ PPPYTHON_PATH_TABLE_ENTRY
                                  PathTableEntryPointer,

    _Outptr_result_nullonfailure_ PPSTRING StringPointer
    );
typedef ALLOCATE_PYTHON_PATH_TABLE_ENTRY_AND_STRING \
      *PALLOCATE_PYTHON_PATH_TABLE_ENTRY_AND_STRING;

typedef
_Success_(return != 0)
BOOL
(ALLOCATE_PYTHON_PATH_TABLE_ENTRY_AND_STRING_WITH_BUFFER)(
    _In_  PPYTHON  Python,
    _In_  ULONG    StringBufferSize,

    _Outptr_result_nullonfailure_ PPPYTHON_PATH_TABLE_ENTRY
                                  PathTableEntryPointer,

    _Outptr_result_nullonfailure_ PPSTRING StringPointer
    );
typedef ALLOCATE_PYTHON_PATH_TABLE_ENTRY_AND_STRING_WITH_BUFFER \
      *PALLOCATE_PYTHON_PATH_TABLE_ENTRY_AND_STRING_WITH_BUFFER;

typedef
_Success_(return != 0)
BOOL
(SET_PYTHON_ALLOCATORS)(
    _In_ PPYTHON             Python,
    _In_ PPYTHON_ALLOCATORS  Allocators
    );
typedef SET_PYTHON_ALLOCATORS *PSET_PYTHON_ALLOCATORS;

typedef
VOID
(FREE_STRING)(
    _In_        PPYTHON Python,
    _In_opt_    PSTRING String
    );
typedef FREE_STRING *PFREE_STRING;
typedef FREE_STRING **PPFREE_STRING;

typedef
VOID
(FREE_STRING_BUFFER)(
    _In_        PPYTHON  Python,
    _In_opt_    PSTRING  String
    );
typedef FREE_STRING_BUFFER *PFREE_STRING_BUFFER;

typedef
VOID
(FREE_STRING_AND_BUFFER)(
    _In_        PPYTHON  Python,
    _In_opt_    PSTRING  String
    );
typedef FREE_STRING_AND_BUFFER *PFREE_STRING_AND_BUFFER;

typedef
VOID
(FREE_STRING_BUFFER_DIRECT)(
    _In_        PPYTHON  Python,
    _In_opt_    PVOID    Buffer
    );
typedef FREE_STRING_BUFFER_DIRECT *PFREE_STRING_BUFFER_DIRECT;

typedef
VOID
(FREE_BUFFER)(
    _In_        PPYTHON Python,
    _In_opt_    PVOID   Buffer
    );
typedef FREE_BUFFER *PFREE_BUFFER;

typedef
_Success_(return != 0)
BOOL
(HASH_AND_ATOMIZE_ANSI)(
    _In_ PPYTHON Python,
    _In_ PSTR String,
    _In_ PLONG HashPointer,
    _In_ PULONG AtomPointer
    );
typedef HASH_AND_ATOMIZE_ANSI *PHASH_AND_ATOMIZE_ANSI;

//
// End of allocator function type definitions.
//

#define _PYTHONEXFUNCTIONS_HEAD                                      \
    PALLOCATE_STRING AllocateString;                                 \
    PALLOCATE_STRING_BUFFER AllocateStringBuffer;                    \
    PALLOCATE_STRING_AND_BUFFER AllocateStringAndBuffer;             \
    PFREE_STRING FreeString;                                         \
    PFREE_STRING_AND_BUFFER FreeStringAndBuffer;                     \
    PFREE_STRING_BUFFER FreeStringBuffer;                            \
    PFREE_STRING_BUFFER_DIRECT FreeStringBufferDirect;               \
    PALLOCATE_BUFFER AllocateBuffer;                                 \
    PFREE_BUFFER FreeBuffer;                                         \
    PREGISTER_FRAME RegisterFrame;                                   \
    PSET_PYTHON_ALLOCATORS SetPythonAllocators;                      \
    PHASH_AND_ATOMIZE_ANSI HashAndAtomizeAnsi;                       \
    PINITIALIZE_PYTHON_RUNTIME_TABLES InitializePythonRuntimeTables;

typedef struct _PYTHONEXFUNCTIONS {
    _PYTHONEXFUNCTIONS_HEAD
} PYTHONEXFUNCTIONS, *PPYTHONEXFUNCTIONS;

#pragma pack(push, 8)

typedef struct _PYTHON_PATH_TABLE {

    //
    // Inline the PREFIX_TABLE struct.
    //

    union {
        PREFIX_TABLE PrefixTable;
        struct {
            CSHORT NodeTypeCode;
            CSHORT NameLength;
            PPYTHON_PATH_TABLE_ENTRY NextPrefixTree;
        };
    };

} PYTHON_PATH_TABLE, *PPYTHON_PATH_TABLE;

typedef enum _PYTHON_PATH_ENTRY_TYPE {
    ModuleDirectory     =        1,
    NonModuleDirectory  =   1 << 1, // 2
    File                =   1 << 2, // 4
    Class               =   1 << 3, // 8
    Function            =   1 << 4, // 16
    Builtin             =   1 << 5, // 32
    CFunction           =   1 << 6, // 64
    PythonLine          =   1 << 7  // 128
} PYTHON_PATH_ENTRY_TYPE, *PPYTHON_PATH_ENTRY_TYPE;

typedef struct _PYTHON_PATH_TABLE_ENTRY {

    //                                                      //      s
    // Inline the PREFIX_TABLE_ENTRY struct.                //  s   t
    //                                                      //  i   a   e
                                                            //  z   r   n
    union {                                                 //  e   t   d
        PREFIX_TABLE_ENTRY PrefixTableEntry;                //  ^   ^   ^
        struct {

            //
            // Stash our flags in here given that NextPrefixTree needs to be
            // 8-byte aligned and we're only 4-byte aligned at this point.
            //

            CSHORT NodeTypeCode;                            //  2   0   2
            CSHORT PrefixNameLength;                        //  2   2   4

            union {
                ULONG Flags;                                //  4   4   8
                PYTHON_PATH_ENTRY_TYPE PathEntryType;
                struct {
                    ULONG IsModuleDirectory:1;      // 1
                    ULONG IsNonModuleDirectory:1;   // 2
                    ULONG IsFile:1;                 // 4
                    ULONG IsClass:1;                // 8
                    ULONG IsFunction:1;             // 16
                    ULONG IsSpecial:1;              // 32
                    ULONG IsValid:1;                // 64
                    ULONG IsDll:1;                  // 128
                    ULONG IsC:1;                    // 256
                    ULONG IsBuiltin:1;              // 512
                };
            };

            PPYTHON_PATH_TABLE_ENTRY NextPrefixTree;        // 8    8   16
            union {
                RTL_SPLAY_LINKS Links;                      // 24   16  40
                struct {
                    PRTL_SPLAY_LINKS    Parent;             // 8    16  24
                    PRTL_SPLAY_LINKS    LeftChild;          // 8    24  32
                    PRTL_SPLAY_LINKS    RightChild;         // 8    32  40
                };
            };
            PSTRING Prefix;                                 // 8    40  48
        };
    };

    //
    // Fully-qualified path name.  Prefix will typically point here.
    //

    union {
        STRING Path;                                        // 16   48  64
        struct {
            USHORT PathLength;                              // 2    48  50
            USHORT PathMaximumLength;                       // 2    50  52
            LONG   PathHash;                                // 4    52  56
            PCHAR  PathBuffer;                              // 8    56  64
        };
    };

    //
    // Full name, using backslashes instead of periods, allowing it to be used
    // in other prefix trees.  The underlying FullName->Buffer will always be
    // allocated uniquely for each PathEntry.
    //

    union {
        STRING FullName;                                    // 16   64  80
        struct {
            USHORT FullNameLength;                          // 2    64  66
            USHORT FullNameMaximumLength;                   // 2    66  68
            LONG   FullNameHash;                            // 4    68  72
            PCHAR  FullNameBuffer;                          // 8    72  80
        };
    };

    //
    // Full module name, using backslashes instead of periods, allowing it
    // to be used in other prefix trees.  The underlying buffer will be unique
    // to this path entry.
    //

    union {
        STRING ModuleName;                                  // 16   80  96
        struct {
            USHORT ModuleNameLength;                        // 2    80  82
            USHORT ModuleNameMaximumLength;                 // 2    82  84
            LONG   ModuleNameHash;                          // 4    84  88
            PCHAR  ModuleNameBuffer;                        // 8    88  96
        };
    };

    //
    // Name of the entry.  If the entry is a file, this will exclude the file
    // extension (and period).  This will always be a view into the FullName
    // buffer above (i.e. Name->Buffer will be advanced to the relevant offset
    // and Length/MaximumLength set accordingly).
    //

    union {
        STRING Name;                                        // 16   96  112
        struct {
            USHORT NameLength;                              // 2    96  98
            USHORT NameMaximumLength;                       // 2    98  100
            LONG   NameHash;                                // 4    100 104
            PCHAR  NameBuffer;                              // 8    104 112
        };
    };

    //
    // ClassName will only be filled out if the entry type is Function and
    // there's a class name available.  It will be a view into FullName.
    //

    union {
        STRING ClassName;                                    // 16  112 128
        struct {
            USHORT ClassNameLength;                          // 2   112 114
            USHORT ClassNameMaximumLength;                   // 2   114 116
            LONG   ClassNameHash;                            // 4   116 120
            PCHAR  ClassNameBuffer;                          // 8   120 128
        };
    };

} PYTHON_PATH_TABLE_ENTRY, *PPYTHON_PATH_TABLE_ENTRY;

C_ASSERT(sizeof(PYTHON_PATH_TABLE_ENTRY) == 128);

#pragma pack(pop)

typedef struct _PYTHON_PATH_TABLE_ENTRY_OFFSETS {
    USHORT Size;
    USHORT NumberOfFields;

    USHORT NodeTypeCodeOffset;
    USHORT PrefixNameLengthOffset;
    USHORT PathEntryTypeOffset;
    USHORT NextPrefixTreeOffset;
    USHORT LinksOffset;
    USHORT LinksParentOffset;
    USHORT LinksLeftChildOffset;
    USHORT LinksRightChildOffset;
    USHORT PrefixOffset;

    USHORT PathOffset;
    USHORT PathLengthOffset;
    USHORT PathMaximumLengthOffset;
    USHORT PathAtomOffset;
    USHORT PathBufferOffset;

    USHORT FullNameOffset;
    USHORT FullNameLengthOffset;
    USHORT FullNameMaximumLengthOffset;
    USHORT FullNameAtomOffset;
    USHORT FullNameBufferOffset;

    USHORT ModuleNameOffset;
    USHORT ModuleNameLengthOffset;
    USHORT ModuleNameMaximumLengthOffset;
    USHORT ModuleNameAtomOffset;
    USHORT ModuleNameBufferOffset;

    USHORT NameOffset;
    USHORT NameLengthOffset;
    USHORT NameMaximumLengthOffset;
    USHORT NameAtomOffset;
    USHORT NameBufferOffset;

    USHORT ClassNameOffset;
    USHORT ClassNameLengthOffset;
    USHORT ClassNameMaximumLengthOffset;
    USHORT ClassNameAtomOffset;
    USHORT ClassNameBufferOffset;

} PYTHON_PATH_TABLE_ENTRY_OFFSETS, *PPYTHON_PATH_TABLE_ENTRY_OFFSETS;

#pragma pack(push, 8)

//
// This structure needs to be 216 bytes in size.  This is because it is
// created indirectly as part of RtlInsertElementGenericTable(), and will
// always be preceded by TABLE_ENTRY_HEADER, which is 40 bytes in size.
// We want our total size, including the header, to be a power of 2, as
// this affords optimal trace store allocation behavior.
//

typedef struct _PYTHON_FUNCTION {

    //
    // The path entry for the function.  This captures naming details and
    // consumes the first 128 bytes of this structure.
    //

    PYTHON_PATH_TABLE_ENTRY PathEntry;

    //
    // A pointer to our parent path entry, if any.
    //

    PPYTHON_PATH_TABLE_ENTRY ParentPathEntry;

    //
    // This is used as the key for the generic table.  It will either be the
    // value of the code object if we're a Python code object, or the method
    // (the C function pointer residing in a DLL) if we're a Python C method.
    //

    ULONG_PTR Key;

    //
    // A unique value for this function that is independent to the given trace
    // session.  E.g. something that could be used as a repeatable hash key.
    //

    ULONG_PTR Signature;

    //
    // The code object associated with this function.  This will be set for
    // both Python and Python C functions.
    //

    union {
        PPYOBJECT               CodeObject;
        PPYCODEOBJECT25_27      Code25_27;
        PPYCODEOBJECT30_32      Code30_32;
        PPYCODEOBJECT33_35      Code33_35;
    };

    //
    // The PyCFunctionObject associated with this function.  This will only
    // be set if we're a Python C function.
    //

    union {
        PPYCFUNCTIONOBJECT      PyCFunctionObject;
        PPYCFUNCTIONOBJECT25_34 PyCFunction25_34;
        PPYCFUNCTIONOBJECT35_35 PyCFunction35_35;
    };

    //
    // The maximum call stack depth observed during tracing for this function.
    //

    ULONG MaxCallStackDepth;

    //
    // The number of times this function was seen during tracing.
    //

    ULONG CallCount;

    union {

        //
        // PyCodeObject fields.
        //

        struct {
            PUSHORT CodeLineNumbers;
            ULONG FirstLineNumber;
            ULONG NumberOfLines;
            ULONG NumberOfCodeLines;
            ULONG SizeOfByteCodeInBytes;
        };

        //
        // PyCFunction fields.
        //

        struct {

            //
            // The module handle.  This is the base address the
            //

            union {
                HANDLE ModuleHandle;
            };
        };
    };

    LIST_ENTRY ListEntry;

} PYTHON_FUNCTION, *PPYTHON_FUNCTION, **PPPYTHON_FUNCTION;

//C_ASSERT(sizeof(PYTHON_FUNCTION) == 216);

typedef struct _PYTHON_FUNCTION_TABLE_ENTRY {

    //
    // Inline TABLE_ENTRY_HEADER.
    //
    // The generic table routines manage this otherwise opaque structure --
    // however, we need to be aware of it given that we want to keep the entire
    // structure that is written to the backing trace store to 256 bytes.
    //

    //
    // Note: we don't use a union here like we normally do, e.g.:
    //      union {
    //          TABLE_ENTRY_HEADER TableEntryHeader;
    //          struct {
    //              // Inline definition of struct.
    //              ...
    //          };
    //
    // This is because TABLE_ENTRY_HEADER has a 'LONGLONG UserData' field that
    // actually represents the start of the user data, i.e. our PYTHON_FUNCTION
    // struct.
    //

    struct {

        //
        // Inline RTL_SPLAY_LINKS.
        //

        union {
            RTL_SPLAY_LINKS Links;                      // 24   0   24
            struct {
                PRTL_SPLAY_LINKS    Parent;             // 8    0   8
                PRTL_SPLAY_LINKS    LeftChild;          // 8    8   16
                PRTL_SPLAY_LINKS    RightChild;         // 8    16  24
            };
        };

        //
        // Inline LIST_ENTRY.
        //

        union {
            LIST_ENTRY ListEntry;                       // 16   24  40
            struct {
                PLIST_ENTRY Flink;                      // 8    24  32
                PLIST_ENTRY Blink;                      // 8    32  40
            };
        };

        //
        // This is where we omit LONGLONG UserData in lieu of the Function
        // field below.
        //
        // LONGLONG UserData;
        //
    };

    PYTHON_FUNCTION Function;

} PYTHON_FUNCTION_TABLE_ENTRY, *PPYTHON_FUNCTION_TABLE_ENTRY;

//C_ASSERT(sizeof(PYTHON_FUNCTION_TABLE_ENTRY) == 256);

typedef PYTHON_FUNCTION_TABLE_ENTRY **PPPYTHON_FUNCTION_TABLE_ENTRY;

typedef struct _PYTHON_FUNCTION_OFFSETS {
    USHORT Size;
    USHORT NumberOfFields;

    USHORT PathEntryOffset;
    USHORT ParentPathEntryOffset;
    USHORT KeyOffset;
    USHORT CodeObjectOffset;
    USHORT PyCFunctionObjectOffset;

} PYTHON_FUNCTION_OFFSETS, *PPYTHON_FUNCTION_OFFSETS;

#pragma pack(pop)

//
// Offset table constants.
//

PYTHON_EX_DATA CONST PYTHON_PATH_TABLE_ENTRY_OFFSETS \
                     PythonPathTableEntryOffsets;

PYTHON_EX_DATA CONST PYTHON_FUNCTION_OFFSETS PythonFunctionOffsets;

typedef struct _PYTHON_FUNCTION_TABLE {

    //
    // Inline RTL_GENERIC_TABLE.
    //

    union {
        RTL_GENERIC_TABLE GenericTable;
        struct {
            PRTL_SPLAY_LINKS              TableRoot;

            //
            // Inline the InsertOrderList LIST_ENTRY to ease the job of writing
            // relocation structure definitions using FIELD_OFFSET().
            //

            union {
                LIST_ENTRY InsertOrderList;
                struct {
                    PLIST_ENTRY InsertOrderFlink;
                    PLIST_ENTRY InsertOrderBlink;
                };
            };

            PLIST_ENTRY                   OrderedPointer;
            ULONG                         WhichOrderedElement;
            ULONG                         NumberGenericTableElements;
            PRTL_GENERIC_COMPARE_ROUTINE  CompareRoutine;
            PRTL_GENERIC_ALLOCATE_ROUTINE AllocateRoutine;
            PRTL_GENERIC_FREE_ROUTINE     FreeRoutine;
            union {
                PVOID TableContext;
                PPYTHON Python;
            };
        };
    };
} PYTHON_FUNCTION_TABLE, *PPYTHON_FUNCTION_TABLE, **PPPYTHON_FUNCTION_TABLE;

#define IsValidFunction(Function) (                 \
    (BOOL)(Function && Function->PathEntry.IsValid) \
)


typedef PREFIX_TABLE MODULE_NAME_TABLE;
typedef MODULE_NAME_TABLE *PMODULE_NAME_TABLE, **PPMODULE_NAME_TABLE;

#define _PYTHONEXRUNTIME_HEAD                                   \
    HANDLE HeapHandle;                                          \
    PREFIX_TABLE ModuleNameTable;                               \
    PPYTHON_PATH_TABLE PathTable;                               \
    PPYTHON_FUNCTION_TABLE FunctionTable;                       \
    PRTL_GENERIC_COMPARE_ROUTINE FunctionTableCompareRoutine;   \
    PRTL_GENERIC_ALLOCATE_ROUTINE FunctionTableAllocateRoutine; \
    PRTL_GENERIC_FREE_ROUTINE FunctionTableFreeRoutine;         \
    PTP_CALLBACK_ENVIRON ThreadpoolCallbackEnvironment;         \
    PYTHON_ALLOCATORS Allocators;


typedef struct _PYTHONEXRUNTIME {
    _PYTHONEXRUNTIME_HEAD
} PYTHONEXRUNTIME, *PPYTHONEXRUNTIME;

#define _PYTHONOBJECTOFFSETS_HEAD                 \
    PCPYCODEOBJECTOFFSETS   PyCodeObjectOffsets;  \
    PCPYFRAMEOBJECTOFFSETS  PyFrameObjectOffsets;

typedef struct _PYTHONOBJECTOFFSETS {
    _PYTHONOBJECTOFFSETS_HEAD
} PYTHONOBJECTOFFSETS, *PPYTHONOBJECTOFFSETS, **PPPYTHONOBJECTOFFSETS;

typedef const  PYTHON_PATH_TABLE_ENTRY_OFFSETS  \
              CPYTHON_PATH_TABLE_ENTRY_OFFSETS, \
            *PCPYTHON_PATH_TABLE_ENTRY_OFFSETS;

typedef const  PYTHON_FUNCTION_OFFSETS  \
              CPYTHON_FUNCTION_OFFSETS, \
            *PCPYTHON_FUNCTION_OFFSETS;

#define _PYTHONEX_OFFSETS_HEAD                                     \
    PCPYTHON_PATH_TABLE_ENTRY_OFFSETS PythonPathTableEntryOffsets; \
    PCPYTHON_FUNCTION_OFFSETS PythonFunctionOffsets;

typedef struct _PYTHONEX_OFFSETS {
    _PYTHONEX_OFFSETS_HEAD
} PYTHONEX_OFFSETS, *PPYTHONEX_OFFSETS, **PPPYTHONEX_OFFSETSA;

#define _PYTHONEXDATA_HEAD                            \
    BOOLEAN IsDebug;                                  \
    USHORT  FilenameStringObjectOffsetFromCodeObject;

typedef struct _PYTHONEXDATA {
    _PYTHONEXDATA_HEAD
} PYTHONEXDATA, *PPYTHONEXDATA;

typedef struct _PYTHON {
    ULONG Size;
    HMODULE PythonModule;
    HMODULE PythonExModule;
    PRTL Rtl;

    union {
        PYTHONFUNCTIONS PythonFunctions;
        struct {
            _PYTHONFUNCTIONS_HEAD
        };
    };

    union {
        PYTHONDATA PythonData;
        struct {
            _PYTHONDATA_HEAD
        };
    };

    union {
        PYTHONVERSION PythonVersion;
        struct {
            _PYTHONVERSION_HEAD
        };
    };

    union {
        PYTHONEXFUNCTIONS PythonExFunctions;
        struct {
            _PYTHONEXFUNCTIONS_HEAD
        };
    };

    union {
        PYTHONEXDATA PythonExData;
        struct {
            _PYTHONEXDATA_HEAD
        };
    };

    union {
        PYTHONOBJECTOFFSETS PythonObjectOffsets;
        struct {
            _PYTHONOBJECTOFFSETS_HEAD
        };
    };

    union {
        PYTHONEX_OFFSETS PythonExOffsets;
        struct {
            _PYTHONEX_OFFSETS_HEAD
        };
    };

    union {
        PYTHONEXRUNTIME PythonExRuntime;
        struct {
            _PYTHONEXRUNTIME_HEAD
        };
    };

    USHORT NumberOfCacheElements;
    PPYOBJECT CodeObjectCache[32];
} PYTHON, *PPYTHON, **PPPYTHON;

PYTHON_API FIND_PYTHON_DLL_AND_EXE FindPythonDllAndExe;

typedef enum _TraceEventType {
    // PyTrace_* constants.
    TraceEventType_PyTrace_CALL = 0,
    TraceEventType_PyTrace_EXCEPTION = 1,
    TraceEventType_PyTrace_LINE = 2,
    TraceEventType_PyTrace_RETURN = 3,
    TraceEventType_PyTrace_C_CALL = 4,
    TraceEventType_PyTrace_C_EXCEPTION = 5,
    TraceEventType_PyTrace_C_RETURN = 6,
} TraceEventType;

PYTHON_API
VOID
SetPythonThreadpoolCallbackEnvironment(
    _In_ PPYTHON              Python,
    _In_ PTP_CALLBACK_ENVIRON ThreadpoolCallbackEnvironment
    );


_Success_(return != 0)
FORCEINLINE
BOOL
GetPythonStringInformation(
    _In_     PPYTHON             Python,
    _In_     PPYOBJECT           StringishObject,
    _Out_    PSIZE_T             Length,
    _Out_    PUSHORT             Width,
    _Out_    PPVOID              Buffer
    )
{
    PYOBJECTEX Object;
    Object.Object = StringishObject;

    if (StringishObject->Type == Python->PyString.Type) {

        *Length = Object.String->ObjectSize;
        *Buffer = Object.String->Value;

        *Width = sizeof(CHAR);

    } else if (StringishObject->Type == Python->PyUnicode.Type) {

        if (Python->PyUnicode_AsUnicode && Python->PyUnicode_GetLength) {

            *Length = Python->PyUnicode_GetLength(StringishObject);
            *Buffer = Python->PyUnicode_AsUnicode(StringishObject);

        } else {

            *Length = Object.Unicode->Length;
            *Buffer = Object.Unicode->String;

        }

        *Width = sizeof(WCHAR);

    } else if (StringishObject->Type == Python->PyBytes.Type) {

        *Length = Object.Bytes->ObjectSize;
        *Buffer = Object.Bytes->Value;

        *Width = sizeof(CHAR);

    } else {

        return FALSE;

    }

    return TRUE;
}

_Success_(return != 0)
FORCEINLINE
BOOL
ConvertPythonUtf16StringToUtf8String(
    _In_     PPYTHON    Python,
    _In_     SIZE_T     Length,
    _In_     PWCHAR     Buffer,
    _Out_    PSTRING    String
    )
{
    LONG BufferSizeInBytes;

    BufferSizeInBytes = WideCharToMultiByte(
        CP_UTF8,
        0,
        Buffer,
        -1,
        NULL,
        0,
        NULL,
        NULL
    );

    if (BufferSizeInBytes <= 0) {
        return FALSE;
    }

    //
    // Sanity check the buffer size isn't over MAX_USHORT or under the number
    // of bytes for the unicode buffer.
    //

    String->Buffer = ALLOCATE(StringBuffer, BufferSizeInBytes);
    if (!String->Buffer) {
        return FALSE;
    }

    BufferSizeInBytes = WideCharToMultiByte(
        CP_UTF8,
        0,
        Buffer,
        -1,
        String->Buffer,
        BufferSizeInBytes,
        NULL,
        NULL
    );

    if (BufferSizeInBytes <= 0) {
        FREE(String, String->Buffer);
        return FALSE;
    }

    String->Length = (USHORT)BufferSizeInBytes - 1;
    String->MaximumLength = (USHORT)BufferSizeInBytes;

    return TRUE;
}

_Success_(return != 0)
FORCEINLINE
BOOL
WrapPythonStringAsString(
    _In_     PPYTHON    Python,
    _In_     PPYOBJECT  StringishObject,
    _Out_    PSTRING    String
    )
{
    SIZE_T Length;
    USHORT Width;
    PVOID  Buffer;

    BOOL Success;

    Success = GetPythonStringInformation(
        Python,
        StringishObject,
        &Length,
        &Width,
        &Buffer
    );

    if (!Success) {
        return FALSE;
    }

    if (Width == 2) {
        return ConvertPythonUtf16StringToUtf8String(
            Python,
            Length,
            Buffer,
            String
        );
    } else if (Width != 1) {
        __debugbreak();
    }

    String->MaximumLength = String->Length = (USHORT)Length;
    String->Buffer = (PCHAR)Buffer;

    return TRUE;
}

_Success_(return != 0)
FORCEINLINE
BOOL
WrapPythonFilenameStringAsString(
    _In_     PPYTHON    Python,
    _In_     PPYOBJECT  StringishObject,
    _Out_    PSTRING    String,
    _Out_    PBOOL      IsSpecialName
    )
{
    SIZE_T Length;
    USHORT Width;
    PVOID  Buffer;

    BOOL Success;

    Success = GetPythonStringInformation(
        Python,
        StringishObject,
        &Length,
        &Width,
        &Buffer
    );

    if (!Success) {
        return FALSE;
    }

    //
    // If the name starts with a '<', it's a special name.  Examples are things
    // like <string> and <frozenlib>.
    //

    if (*((PCHAR)Buffer) == '<') {
        *IsSpecialName = TRUE;
    } else {
        *IsSpecialName = FALSE;
    }

    if (Width == 2) {

        //
        // Python 3.x will use PyUNICODE structs for paths, which will have wide
        // characters, which we'll need to convert to a utf-8 string in order to
        // put into our filename prefix tree.
        //

        return ConvertPythonUtf16StringToUtf8String(
            Python,
            Length,
            Buffer,
            String
        );

    } else if (Width != 1) {
        __debugbreak();
    }

    String->MaximumLength = String->Length = (USHORT)Length;
    String->Buffer = (PCHAR)Buffer;

    return TRUE;
}

FORCEINLINE
LONG
PythonStringHashInline(
    _In_    PPYTHON Python,
    _In_    PSTRING String
    )
{
    PCHAR Char;
    SHORT Length;
    LONG Hash;

    if (String->Length == 0) {
        return 0;
    }

    Length = String->Length;
    Char = &String->Buffer[0];

    Hash = Python->_Py_HashSecret.Prefix;

    Hash ^= *Char << 7;

    while (--Length >= 0) {
        Hash = (1000003*Hash) ^ *Char++;
    }

    Hash ^= String->Length;
    Hash ^= Python->_Py_HashSecret.Suffix;

    if (Hash == -1) {
        Hash = -2;
    }

    return Hash;
}

FORCEINLINE
VOID
HashString(
    _In_ PPYTHON Python,
    _In_ PSTRING String
    )
{
    String->Hash = PythonStringHashInline(Python, String);
}

FORCEINLINE
BOOL
PythonAnsiHashInline(
    _In_    PPYTHON Python,
    _In_    PSTR    String,
    _Out_   PLONG  HashPointer
    )
{
    SHORT Length = 0;
    LONG Hash;
    PCHAR Char;

    if (!ARGUMENT_PRESENT(Python)) {
        return FALSE;
    }

    if (!ARGUMENT_PRESENT(String)) {
        return FALSE;
    }

    if (!ARGUMENT_PRESENT(HashPointer)) {
        return FALSE;
    }

    Char = String;

    Hash = Python->_Py_HashSecret.Prefix;

    Hash ^= *Char << 7;

    while (*Char != '\0') {
        Length++;
        Hash = (1000003*Hash) ^ *Char++;
    }

    Hash ^= Length;
    Hash ^= Python->_Py_HashSecret.Suffix;

    if (Hash == -1) {
        Hash = -2;
    }

    *HashPointer = Hash;

    return TRUE;
}

_Success_(return != 0)
FORCEINLINE
BOOL
HashAndAtomizeAnsiInline(
    _In_    PPYTHON Python,
    _In_    PSTR String,
    _Out_   PLONG HashPointer,
    _Out_   PULONG AtomPointer
    )
{
    ULONG Atom;

    if (!ARGUMENT_PRESENT(AtomPointer)) {
        return FALSE;
    }

    if (!PythonAnsiHashInline(Python, String, HashPointer)) {
        return FALSE;
    }

    Atom = HashAnsiToAtom(String);

    *AtomPointer = Atom;

    return TRUE;
}

#ifdef __cplusplus
}; // extern "C"
#endif

// vim:set ts=8 sw=4 sts=4 tw=80 expandtab                                     :
