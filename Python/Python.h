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
typedef __int64 Py_ssize_t;
typedef __int64 SSIZE_T, *PSSIZE_T;
#else
typedef _W64 int Py_ssize_t;
typedef _W64 int SSIZE_T, *PSSIZE_T;
#endif

#define _PYOBJECT_HEAD              \
    union {                         \
        Py_ssize_t  ob_refcnt;      \
        SSIZE_T     ReferenceCount; \
    };                              \
    union {                         \
        PVOID ob_type;              \
        PVOID TypeObject;           \
    };

#define _PYVAROBJECT_HEAD           \
    _PYOBJECT_HEAD                  \
    union {                         \
        Py_ssize_t  ob_size;        \
        SSIZE_T  ObjectSize;        \
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
            void            *tp_compare;
            PyAsyncMethods  *tp_as_async;
        };
        union {
            PVOID           Compare;
            PPYASYNCMETHODS AsyncMethods;
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
        void    *tp_hash;
        PVOID   Hash;
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
        void    *tp_richcompare;
        PVOID   RichCompare;
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

typedef struct _PYTUPLEOBJECT {
    _PYVAROBJECT_HEAD
    union {
        PyObject *ob_item[1];
        PPYOBJECT Item[1];
    };
} PYTUPLEOBJECT, *PPYTUPLEOBJECT;

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

// Python functions
typedef PCSTR (*PPY_GETVERSION)();
typedef VOID (*PPY_INCREF)(PPYOBJECT);
typedef VOID (*PPY_DECREF)(PPYOBJECT);
typedef LONG (*PPYFRAME_GETLINENUMBER)(PPYFRAMEOBJECT FrameObject);
typedef PWSTR (*PPYUNICODE_ASUNICODE)(PPYOBJECT Object);
typedef SSIZE_T (*PPYUNICODE_GETLENGTH)(PPYOBJECT Object);
typedef LONG (*PPYEVAL_SETTRACE)(PPYTRACEFUNC, PPYOBJECT);
typedef PPYOBJECT (*PPYDICT_GETITEMSTRING)(PPYOBJECT, PCCH);
typedef enum _PYGILSTATE {
    PyGILState_LOCKED,
    PyGILState_UNLOCKED
} PYGILSTATE, *PPYGILSTATE, PyGILState_STATE;
typedef PYGILSTATE (*PPYGILSTATE_ENSURE)();
typedef VOID (*PPYGILSTATE_RELEASE)(PYGILSTATE);

#define _PYTHONFUNCTIONS_HEAD                                   \
    PPY_GETVERSION          Py_GetVersion;                      \
    PPYDICT_GETITEMSTRING   PyDict_GetItemString;               \
    PPYFRAME_GETLINENUMBER  PyFrame_GetLineNumber;              \
    PPYEVAL_SETTRACE        PyEval_SetTrace;                    \
    PPYUNICODE_ASUNICODE    PyUnicode_AsUnicode;                \
    PPYUNICODE_GETLENGTH    PyUnicode_GetLength;                \
    PPY_INCREF              Py_IncRef;                          \
    PPY_DECREF              Py_DecRef;                          \
    PPYGILSTATE_ENSURE      PyGILState_Ensure;                  \
    PPYGILSTATE_RELEASE     PyGILState_Release;

typedef struct _PYTHONFUNCTIONS {
    _PYTHONFUNCTIONS_HEAD
} PYTHONFUNCTIONS, *PPYTHONFUNCTIONS;

#define _PYTHONDATA_HEAD                                        \
    PPYOBJECT PyCode_Type;                                      \
    PPYOBJECT PyDict_Type;                                      \
    PPYOBJECT PyTuple_Type;                                     \
    PPYOBJECT PyType_Type;                                      \
    PPYOBJECT PyFunction_Type;                                  \
    PPYOBJECT PyString_Type;                                    \
    PPYOBJECT PyBytes_Type;                                     \
    PPYOBJECT PyUnicode_Type;                                   \
    PPYOBJECT PyCFunction_Type;                                 \
    PPYOBJECT PyInstance_Type;                                  \
    PPYOBJECT PyModule_Type;

typedef struct _PYTHONDATA {
    _PYTHONDATA_HEAD
} PYTHONDATA, *PPYTHONDATA;


#define _PYTHONVERSION_HEAD                                     \
    PCSTR   VersionString;                                      \
    USHORT  MajorVersion;                                       \
    USHORT  MinorVersion;                                       \
    USHORT  PatchLevel;

typedef struct _PYTHONVERSION {
    _PYTHONVERSION_HEAD
} PYTHONVERSION, *PPYTHONVERSION;

typedef struct _PYTHON PYTHON, *PPYTHON, **PPPYTHON;

typedef BOOL (*PGETUNICODELENGTHFORPYTHONSTRING)(
    _In_    PPYTHON     Python,
    _In_    PPYOBJECT   StringOrUnicodeObject,
    _Out_   PULONG      UnicodeLength
);

typedef BOOL (*PCONVERTPYSTRINGTOUNICODESTRING)(
    _In_    PPYTHON             Python,
    _In_    PPYOBJECT           StringObject,
    _Inout_ PPUNICODE_STRING    UnicodeString,
    _In_    BOOL                AllocateMaximumSize
);

typedef BOOL (*PRESOLVEFRAMEOBJECTDETAILS)(
    _In_    PPYTHON         Python,
    _In_    PPYFRAMEOBJECT  FrameObject,
    _Inout_ PPPYOBJECT      CodeObject,
    _Inout_ PPPYOBJECT      ModuleFilenameStringObject,
    _Inout_ PPPYOBJECT      FunctionNameStringObject,
    _Inout_ PULONG          LineNumber
);

#define _PYTHONEXFUNCTIONS_HEAD                                             \
    PGETUNICODELENGTHFORPYTHONSTRING    GetUnicodeLengthForPythonString;    \
    PCONVERTPYSTRINGTOUNICODESTRING     ConvertPythonStringToUnicodeString; \
    PRESOLVEFRAMEOBJECTDETAILS          ResolveFrameObjectDetails;

typedef struct _PYTHONEXFUNCTIONS {
    _PYTHONEXFUNCTIONS_HEAD
} PYTHONEXFUNCTIONS, *PPYTHONEXFUNCTIONS;

#define _PYTHONEXRUNTIME_HEAD \
    HANDLE  StringsHeap;

typedef struct _PYTHONEXRUNTIME {
    _PYTHONEXRUNTIME_HEAD
} PYTHONEXRUNTIME, *PPYTHONEXRUNTIME;

#define _PYTHONOBJECTOFFSETS_HEAD                       \
    PCPYCODEOBJECTOFFSETS     PyCodeObjectOffsets;

typedef struct _PYTHONOBJECTOFFSETS {
    _PYTHONOBJECTOFFSETS_HEAD
} PYTHONOBJECTOFFSETS, *PPYTHONOBJECTOFFSETS, **PPPYTHONOBJECTOFFSETS;

#define _PYTHONEXDATA_HEAD                              \
    BOOLEAN IsDebug;                                    \
    USHORT  FilenameStringObjectOffsetFromCodeObject;

typedef struct _PYTHONEXDATA {
    _PYTHONEXDATA_HEAD
} PYTHONEXDATA, *PPYTHONEXDATA;

typedef struct _PYTHON {
    DWORD Size;
    HMODULE PythonModule;
    HMODULE PythonExModule;

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
} PYTHON, *PPYTHON, **PPPYTHON;

TRACER_API
BOOL
InitializePython(
    _In_                         HMODULE     PythonModule,
    _Out_bytecap_(*SizeOfPython) PPYTHON     Python,
    _Inout_                      PDWORD      SizeOfPython
);

TRACER_API
BOOL
GetUnicodeLengthForPythonString(
    _In_    PPYTHON         Python,
    _In_    PPYOBJECT       StringOrUnicodeObject,
    _Out_   PULONG          UnicodeLength
);

TRACER_API
BOOL
ConvertPythonStringToUnicodeString(
    _In_    PPYTHON             Python,
    _In_    PPYOBJECT           StringOrUnicodeObject,
    _Out_   PPUNICODE_STRING    UnicodeString,
    _In_    BOOL                AllocateMaximumSize
);

TRACER_API
BOOL
GetModuleFilenameStringObjectFromCodeObject(
    _In_    PPYTHON     Python,
    _In_    PPYOBJECT   CodeObject,
    _Inout_ PPPYOBJECT  FilenameStringObject
);

TRACER_API
BOOL
ResolveFrameObjectDetails(
    _In_    PPYTHON         Python,
    _In_    PPYFRAMEOBJECT  FrameObject,
    _Inout_ PPPYOBJECT      CodeObject,
    _Inout_ PPPYOBJECT      ModuleFilenameStringObject,
    _Inout_ PPPYOBJECT      FunctionNameStringObject,
    _Inout_ PULONG          LineNumber
);

#ifdef __cpp
} // extern "C"
#endif
