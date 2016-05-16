// Copyright(c) Trent Nelson <trent@trent.me>
// All rights reserved.

#pragma once

#ifdef __cpplus
extern "C" {
#endif

#include <Windows.h>
#include "../Rtl/Rtl.h"
#include "../Tracer/Tracer.h"

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

#define _PYOBJECT_HEAD              \
    union {                         \
        Py_ssize_t  ob_refcnt;      \
        SSIZE_T     ReferenceCount; \
    };                              \
    union {                         \
        PyTypeObject *ob_type;      \
        PPYTYPEOBJECT Type;         \
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

typedef struct _PYMETHODDEF {
    union {
        char *ml_name;
        PCHAR Name;
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
typedef VOID (*PPYRELEASEBUFFERPROC)(PPYOBJECT, PPYBUFFER), (*releasebufferproc);

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

typedef struct _PYCFUNCTIONOBJECT {
    union {
        PyMethodDef *m_ml;
        PPYMETHODDEF Method;
    };
    union {
        PyObject    *m_self;
        PPYOBJECT    Self;
    };
    union {
        PyObject    *m_module;
        PPYOBJECT    Module;
    };
} PYCFUNCTIONOBJECT, *PPYCFUNCTIONOBJECT, PyCFunctionObject;

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

typedef struct _PYTHREADSTATE34_35 PYTHREADSTATE34_35, *PPYTHREADSTATE34_35, PyThreadState34_35;

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

// Python functions
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
typedef enum _PYGILSTATE {
    PyGILState_LOCKED,
    PyGILState_UNLOCKED
} PYGILSTATE, *PPYGILSTATE, PyGILState_STATE;
typedef PYGILSTATE (*PPYGILSTATE_ENSURE)();
typedef VOID (*PPYGILSTATE_RELEASE)(PYGILSTATE);
typedef LONG (*PPYOBJECT_HASH)(PPYOBJECT);
typedef INT (*PPYOBJECT_COMPARE)(PPYOBJECT, PPYOBJECT, PINT);
typedef PVOID (*PPYMEM_MALLOC)(SIZE_T);
typedef PVOID (*PPYMEM_REALLOC)(PVOID, SIZE_T);
typedef VOID (*PPYMEM_FREE)(PVOID);
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

#define _PYTHONFUNCTIONS_HEAD                      \
    PPY_GETVERSION          Py_GetVersion;         \
    PPYDICT_GETITEMSTRING   PyDict_GetItemString;  \
    PPYFRAME_GETLINENUMBER  PyFrame_GetLineNumber; \
    PPYCODE_ADDR2LINE       PyCode_Addr2Line;      \
    PPYEVAL_SETTRACE        PyEval_SetProfile;     \
    PPYEVAL_SETTRACE        PyEval_SetTrace;       \
    PPYUNICODE_ASUNICODE    PyUnicode_AsUnicode;   \
    PPYUNICODE_GETLENGTH    PyUnicode_GetLength;   \
    PPY_INCREF              Py_IncRef;             \
    PPY_DECREF              Py_DecRef;             \
    PPYGILSTATE_ENSURE      PyGILState_Ensure;     \
    PPYGILSTATE_RELEASE     PyGILState_Release;    \
    PPYOBJECT_HASH          PyObject_Hash;         \
    PPYOBJECT_COMPARE       PyObject_Compare;      \
    PPYMEM_MALLOC           PyMem_Malloc;          \
    PPYMEM_REALLOC          PyMem_Realloc;         \
    PPYMEM_FREE             PyMem_Free;            \
    PPYOBJECT_MALLOC        PyObject_Malloc;       \
    PPYOBJECT_REALLOC       PyObject_Realloc;      \
    PPYOBJECT_FREE          PyObject_Free;         \
    PPYGC_COLLECT           PyGC_Collect;          \
    PPYOBJECT_GC_MALLOC     _PyObject_GC_Malloc;   \
    PPYOBJECT_GC_NEW        _PyObject_GC_New;      \
    PPYOBJECT_GC_NEWVAR     _PyObject_GC_NewVar;   \
    PPYOBJECT_GC_RESIZE     _PyObject_GC_Resize;   \
    PPYOBJECT_GC_TRACK      PyObject_GC_Track;     \
    PPYOBJECT_GC_UNTRACK    PyObject_GC_UnTrack;   \
    PPYOBJECT_GC_DEL        PyObject_GC_Del;


typedef struct _PYTHONFUNCTIONS {
    _PYTHONFUNCTIONS_HEAD
} PYTHONFUNCTIONS, *PPYTHONFUNCTIONS;

#define _PYTHONDATA_HEAD        \
    PYTYPEOBJECTEX PyString;    \
    PYTYPEOBJECTEX PyBytes;     \
    PYTYPEOBJECTEX PyCode;      \
    PYTYPEOBJECTEX PyDict;      \
    PYTYPEOBJECTEX PyTuple;     \
    PYTYPEOBJECTEX PyType;      \
    PYTYPEOBJECTEX PyFunction;  \
    PYTYPEOBJECTEX PyUnicode;   \
    PYTYPEOBJECTEX PyCFunction; \
    PYTYPEOBJECTEX PyInstance;  \
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

typedef struct _PYTHON_PATH_TABLE_ENTRY PYTHON_PATH_TABLE_ENTRY;
typedef PYTHON_PATH_TABLE_ENTRY *PPYTHON_PATH_TABLE_ENTRY;
typedef PYTHON_PATH_TABLE_ENTRY **PPPYTHON_PATH_TABLE_ENTRY;

typedef struct _PYTHON_FUNCTION PYTHON_FUNCTION;
typedef PYTHON_FUNCTION *PPYTHON_FUNCTION;
typedef PYTHON_FUNCTION **PPPYTHON_FUNCTION;

typedef BOOL (*PREGISTER_FRAME)(
    _In_      PPYTHON         Python,
    _In_      PPYFRAMEOBJECT  FrameObject,
    _In_      LONG            EventType,
    _In_opt_  PPYOBJECT       ArgObject,
    _Out_opt_ PVOID           Token
    );

typedef BOOL (*PINITIALIZE_PYTHON_RUNTIME_TABLES)(
    _In_      PPYTHON             Python,
    _In_opt_  PALLOCATION_ROUTINE AllocationRoutine,
    _In_opt_  PVOID               AllocationContext,
    _In_opt_  PFREE_ROUTINE       FreeRoutine,
    _In_opt_  PVOID               FreeContext,
    _In_      BOOL                Reuse
    );

#define _PYTHONEXFUNCTIONS_HEAD                                      \
    PREGISTER_FRAME RegisterFrame;                                   \
    PINITIALIZE_PYTHON_RUNTIME_TABLES InitializePythonRuntimeTables;

typedef struct _PYTHONEXFUNCTIONS {
    _PYTHONEXFUNCTIONS_HEAD
} PYTHONEXFUNCTIONS, *PPYTHONEXFUNCTIONS;

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

    //
    // Inline the PREFIX_TABLE_ENTRY struct.
    //

    union {
        PREFIX_TABLE_ENTRY PrefixTableEntry;
        struct {
            CSHORT NodeTypeCode;
            CSHORT NameLength;
            PPYTHON_PATH_TABLE_ENTRY NextPrefixTree;
            RTL_SPLAY_LINKS Links;
            PSTRING Prefix;
        };
    };

    DECLSPEC_ALIGN(8)
    union {
        PYTHON_PATH_ENTRY_TYPE PathEntryType;
        struct {
            ULONG IsModuleDirectory:1;
            ULONG IsNonModuleDirectory:1;
            ULONG IsFile:1;
            ULONG IsClass:1;
            ULONG IsFunction:1;
            ULONG IsBuiltin:1;
            ULONG IsValid:1;
        };
    };
    ULONG Unused1;

    //
    // Fully-qualified path name.  Prefix will point to &Path.  (The underlying
    // Path->Buffer is usually allocated straight after this struct in memory.)
    //

    STRING Path;

    //
    // Full name, using backslashes instead of periods, allowing it to be used
    // in other prefix trees.  The underlying FullName->Buffer will always be
    // allocated uniquely for each PathEntry.
    //

    STRING FullName;

    //
    // Full module name, using backslashes instead of periods, allowing it
    // to be used in other prefix trees.  The underlying buffer will be unique
    // to this path entry.
    //

    STRING ModuleName;

    //
    // Name of the entry.  If the entry is a file, this will exclude the file
    // extension (and period).  This will always be a view into the FullName
    // buffer above (i.e. Name->Buffer will be advanced to the relevant offset
    // and Length/MaximumLength set accordingly).
    //

    union {
        STRING Name;
        STRING FunctionName;
    };

    //
    // ClassName will only be filled out if the entry type is Function and
    // there's a class name available.  It will be a view into FullName.
    //

    STRING ClassName;

} PYTHON_PATH_TABLE_ENTRY, *PPYTHON_PATH_TABLE_ENTRY;

typedef struct _PYTHON_FUNCTION {
    PYTHON_PATH_TABLE_ENTRY PathEntry;
    PREFIX_TABLE_ENTRY ModuleNameEntry;

    union {
        PPYOBJECT          CodeObject;
        PPYCODEOBJECT25_27 Code25_27;
        PPYCODEOBJECT30_32 Code30_32;
        PPYCODEOBJECT33_35 Code33_35;
    };

    ULONG CodeObjectHash;

    USHORT FirstLineNumber;
    USHORT LastLineNumber;
    USHORT NumberOfLines;
    USHORT SizeOfByteCode;
    USHORT LastByteCodeOffset;
    USHORT NumberOfByteCodes;

    RTL_BITMAP LineNumbersBitmap;

    PPYTHON_PATH_TABLE_ENTRY ParentPathEntry;

} PYTHON_FUNCTION, *PPYTHON_FUNCTION, **PPPYTHON_FUNCTION;

typedef struct _PYTHON_FUNCTION_TABLE {

    //
    // Inline RTL_GENERIC_TABLE.
    //

    union {
        RTL_GENERIC_TABLE GenericTable;
        struct {
            PRTL_SPLAY_LINKS              TableRoot;
            LIST_ENTRY                    InsertOrderList;
            PLIST_ENTRY                   OrderedPointer;
            ULONG                         WhichOrderedElement;
            ULONG                         NumberGenericTableElements;
            PRTL_GENERIC_COMPARE_ROUTINE  CompareRoutine;
            PRTL_GENERIC_ALLOCATE_ROUTINE AllocateRoutine;
            PRTL_GENERIC_FREE_ROUTINE     FreeRoutine;
            union {
                PVOID TableContext;
                PPYTHON_FUNCTION PythonFunction;
            };
        };
    };
} PYTHON_FUNCTION_TABLE, *PPYTHON_FUNCTION_TABLE;

#define IsValidFunction(Function) (                  \
    (BOOL)(Function && Function->PathEntry.IsValid)  \
)

typedef PREFIX_TABLE MODULE_NAME_TABLE;
typedef MODULE_NAME_TABLE *PMODULE_NAME_TABLE, **PPMODULE_NAME_TABLE;

#define _PYTHONEXRUNTIME_HEAD                                   \
    HANDLE HeapHandle;                                          \
    PREFIX_TABLE ModuleNameTable;                               \
    PPYTHON_PATH_TABLE PathTable;                               \
    PPYTHON_FUNCTION_TABLE FunctionTable;                       \
    PALLOCATION_ROUTINE AllocationRoutine;                      \
    PVOID AllocationContext;                                    \
    PFREE_ROUTINE FreeRoutine;                                  \
    PVOID FreeContext;                                          \
    PRTL_GENERIC_COMPARE_ROUTINE FunctionTableCompareRoutine;   \
    PRTL_GENERIC_ALLOCATE_ROUTINE FunctionTableAllocateRoutine; \
    PRTL_GENERIC_FREE_ROUTINE FunctionTableFreeRoutine;         \
    PTP_CALLBACK_ENVIRON ThreadpoolCallbackEnvironment;


typedef struct _PYTHONEXRUNTIME {
    _PYTHONEXRUNTIME_HEAD
} PYTHONEXRUNTIME, *PPYTHONEXRUNTIME;

#define _PYTHONOBJECTOFFSETS_HEAD                 \
    PCPYCODEOBJECTOFFSETS   PyCodeObjectOffsets;  \
    PCPYFRAMEOBJECTOFFSETS  PyFrameObjectOffsets;

typedef struct _PYTHONOBJECTOFFSETS {
    _PYTHONOBJECTOFFSETS_HEAD
} PYTHONOBJECTOFFSETS, *PPYTHONOBJECTOFFSETS, **PPPYTHONOBJECTOFFSETS;

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
        PYTHONEXRUNTIME PythonExRuntime;
        struct {
            _PYTHONEXRUNTIME_HEAD
        };
    };

    USHORT NumberOfCacheElements;
    PPYOBJECT CodeObjectCache[32];
} PYTHON, *PPYTHON, **PPPYTHON;

typedef BOOL (*PINITIALIZE_PYTHON)(
    _In_                         PRTL                Rtl,
    _In_                         HMODULE             PythonModule,
    _Out_bytecap_(*SizeOfPython) PPYTHON             Python,
    _Inout_                      PULONG              SizeOfPython
    );

TRACER_API
BOOL
InitializePython(
    _In_                         PRTL                Rtl,
    _In_                         HMODULE             PythonModule,
    _Out_bytecap_(*SizeOfPython) PPYTHON             Python,
    _Inout_                      PULONG              SizeOfPython
    );

TRACER_API
VOID
SetPythonThreadpoolCallbackEnvironment(
    _In_ PPYTHON              Python,
    _In_ PTP_CALLBACK_ENVIRON ThreadpoolCallbackEnvironment
    );


TRACER_API
BOOL
InitializePythonRuntimeTables(
    _In_      PPYTHON             Python,
    _In_opt_  PALLOCATION_ROUTINE AllocationRoutine,
    _In_opt_  PVOID               AllocationContext,
    _In_opt_  PFREE_ROUTINE       FreeRoutine,
    _In_opt_  PVOID               FreeContext,
    _In_      BOOL                Reuse
);

TRACER_API
BOOL
RegisterFrame(
    _In_      PPYTHON         Python,
    _In_      PPYFRAMEOBJECT  FrameObject,
    _In_      LONG            EventType,
    _In_opt_  PPYOBJECT       ArgObject,
    _Out_opt_ PVOID           Token
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
        Length = Length << 1;
    } else if (Width != 1) {
        __debugbreak();
    }

    String->MaximumLength = String->Length = (USHORT)Length;
    String->Buffer = Buffer;

    return TRUE;
}

#ifdef __cpp
} // extern "C"
#endif
