/*++

Copyright (c) 2016 Trent Nelson <trent@trent.me>

Module Name:

    PythonTracer.h

Abstract:

    This is the main header file for the PythonTracer component.  It defines
    structures and functions related to tracing and profiling the Python
    interpreter within the context of the general tracer machinery.

--*/

#pragma once

#ifdef __cplusplus
extern "C" {
#endif

#ifdef _PYTHON_TRACER_INTERNAL_BUILD

//
// This is an internal build of the PythonTracer component.
//

#define PYTHON_TRACER_API __declspec(dllexport)
#define PYTHON_TRACER_DATA extern __declspec(dllexport)

#include "stdafx.h"

#else

//
// We're being included by an external component.
//

#define PYTHON_TRACER_API __declspec(dllimport)
#define PYTHON_TRACER_DATA extern __declspec(dllimport)

#include <Windows.h>
#include "../Rtl/Rtl.h"
#include "../Python/Python.h"
#include "../TraceStore/TraceStore.h"
#include "../StringTable/StringTable.h"

#endif

typedef struct _EVENT_TYPE_NAME {
    TraceEventType  Id;
    PCWSTR          Name;
    PCSTR           NameA;
} EVENT_TYPE_NAME, *PEVENT_TYPE_NAME;

static const EVENT_TYPE_NAME EventTypeNames[] = {

    {
        TraceEventType_PyTrace_CALL,
        L"PyTrace_CALL",
        "PyTrace_CALL"
    },

    {
        TraceEventType_PyTrace_EXCEPTION,
        L"PyTrace_EXCEPTION",
        "PyTrace_EXCEPTION"
    },

    {
        TraceEventType_PyTrace_LINE,
        L"PyTrace_LINE",
        "PyTrace_LINE"
    },

    {
        TraceEventType_PyTrace_RETURN,
        L"PyTrace_RETURN",
        "PyTrace_RETURN"
    },

    {
        TraceEventType_PyTrace_C_CALL,
        L"PyTrace_C_CALL",
        "PyTrace_C_CALL"
    },

    {
        TraceEventType_PyTrace_C_EXCEPTION,
        L"PyTrace_C_EXCEPTION",
        "PyTrace_C_EXCEPTION"
    },

    {
        TraceEventType_PyTrace_C_RETURN,
        L"PyTrace_C_RETURN",
        "PyTrace_C_RETURN"
    },
};

static const DWORD NumberOfTraceEventTypes = (
    sizeof(EventTypeNames) /
    sizeof(EVENT_TYPE_NAME)
);

typedef enum _PYTHON_TRACE_EVENT_TYPE {
    Call        =         1,
    Exception   =   1 <<  1, // 2
    Line        =   1 <<  2, // 4
    Return      =   1 <<  3, // 8
    Invalid     =   1 << 31
} PYTHON_TRACE_EVENT_TYPE, *PPYTHON_TRACE_EVENT_TYPE;

#pragma pack(push, DefaultAlignment, 2)

typedef struct _PYTHON_TRACE_EVENT {
    LARGE_INTEGER Timestamp;            // 8
    PPYTHON_FUNCTION Function;          // 8        16

    ULONGLONG WorkingSetSize;           // 8        24
    ULONGLONG PageFaultCount;           // 8        32
    ULONGLONG CommittedSize;            // 8        40

    ULONGLONG ReadTransferCount;        // 8        48
    ULONGLONG WriteTransferCount;       // 8        56

    ULONG HandleCount;                  // 4        60
    ULONG ThreadId;                     // 4        64
    ULONG ElapsedMicroseconds;          // 4        68

    LONG WorkingSetDelta;               // 4        72
    LONG CommittedDelta;                // 4        76
    ULONG ReadTransferDelta;            // 4        80
    ULONG WriteTransferDelta;           // 4        84

    ULONG CodeObjectHash;               // 4        88
    ULONG FunctionHash;                 // 4        92

    ULONG PathHash;                     // 4        96
    ULONG FullNameHash;                 // 4        100
    ULONG ModuleNameHash;               // 4        104
    ULONG ClassNameHash;                // 4        108
    ULONG NameHash;                     // 4        112

    union {
        ULONG Flags;                    // 4        116
        PYTHON_TRACE_EVENT_TYPE Type;
        struct {
            ULONG IsCall:1;         // PyTrace_CALL
            ULONG IsException:1;    // PyTrace_EXCEPTION
            ULONG IsLine:1;         // PyTrace_LINE
            ULONG IsReturn:1;       // PyTrace_RETURN
            ULONG IsC:1;
            ULONG IsReverseJump:1;
            ULONG UnusedLow:2;      // 8 bits, 1 byte (24 bits, 3 bytes remain)

            ULONG :24;  // Unused bits
        };
        struct {
            BYTE FlagsByte1;
            BYTE FlagsByte2;
            BYTE FlagsByte3;
            BYTE FlagsByte4;
        };
        struct {
            USHORT FlagsLow;
            USHORT FlagsHigh;
        };
    };

    SHORT HandleDelta;                  // 2        118
    USHORT PageFaultDelta;              // 2        120

    USHORT LineNumber;                  // 2        122
    USHORT FirstLineNumber;             // 2        124
    USHORT NumberOfLines;               // 2        126
    USHORT NumberOfCodeLines;           // 2        128

} PYTHON_TRACE_EVENT, *PPYTHON_TRACE_EVENT, **PPPYTHON_TRACE_EVENT;

#pragma pack(pop, DefaultAlignment)

C_ASSERT(sizeof(PYTHON_TRACE_EVENT) == 128);

typedef struct _PYTHON_TRACE_EVENT_SLIM {
    PPYTHON_FUNCTION Function;          // 8        16
    USHORT CallStackDepth;
    USHORT LineNumber;
    TraceEventType EventType;

} PYTHON_TRACE_EVENT_SLIM, *PPYTHON_TRACE_EVENT_SLIM;

typedef struct _PYTHON_TRACE_CALL_EVENT {
    LIST_ENTRY ListEntry;
    union {
        LARGE_INTEGER Timestamp;
        LARGE_INTEGER Elapsed;
    };
    PPYTHON_TRACE_EVENT CallEvent;
    PPYTHON_TRACE_EVENT ReturnEvent;
} PYTHON_TRACE_CALL_EVENT, *PPYTHON_TRACE_CALL_EVENT;

//
// Forward declarations.
//

typedef struct _PYTHON_TRACE_CONTEXT PYTHON_TRACE_CONTEXT;
typedef PYTHON_TRACE_CONTEXT *PPYTHON_TRACE_CONTEXT;

typedef struct _PYTHON_TRACE_CONTEXT_FLAGS {
    union {
        ULONG Flags;
        struct {
            ULONG IsProfile:1;
            ULONG HasStarted:1;
            ULONG TraceMemory:1;
            ULONG TraceIoCounters:1;
            ULONG TraceHandleCount:1;
        };
    };
} PYTHON_TRACE_CONTEXT_FLAGS, *PPYTHON_TRACE_CONTEXT_FLAGS;

typedef struct _EVENT_TYPE {
    union {
        ULONG EventType;
    };
    struct {
        ULONG IsCall:1;
        ULONG IsReturn:1;
        ULONG IsLine:1;
        ULONG IsException:1;
        ULONG IsC:1;
    };
} EVENT_TYPE, *PEVENT_TYPE;

typedef struct _PYTHON_TRACE_CONTEXT PYTHON_TRACE_CONTEXT;
typedef PYTHON_TRACE_CONTEXT *PPYTHON_TRACE_CONTEXT;

typedef
PPYTHON_TRACE_CONTEXT
(GET_CURRENT_PYTHON_TRACE_CONTEXT)(VOID);
typedef GET_CURRENT_PYTHON_TRACE_CONTEXT *PGET_CURRENT_PYTHON_TRACE_CONTEXT;

typedef
VOID
(ENABLE_MEMORY_TRACING)(
    _In_ PPYTHON_TRACE_CONTEXT PythonTraceContext
    );
typedef ENABLE_MEMORY_TRACING *PENABLE_MEMORY_TRACING;
PYTHON_TRACER_API ENABLE_MEMORY_TRACING EnableMemoryTracing;

typedef
VOID
(DISABLE_MEMORY_TRACING)(
    _In_ PPYTHON_TRACE_CONTEXT PythonTraceContext
    );
typedef DISABLE_MEMORY_TRACING *PDISABLE_MEMORY_TRACING;
PYTHON_TRACER_API DISABLE_MEMORY_TRACING DisableMemoryTracing;

typedef
VOID
(ENABLE_IO_COUNTERS_TRACING)(
    _In_ PPYTHON_TRACE_CONTEXT PythonTraceContext
    );
typedef ENABLE_IO_COUNTERS_TRACING *PENABLE_IO_COUNTERS_TRACING;
PYTHON_TRACER_API ENABLE_IO_COUNTERS_TRACING EnableIoCountersTracing;

typedef
VOID
(DISABLE_IO_COUNTERS_TRACING)(
    _In_ PPYTHON_TRACE_CONTEXT PythonTraceContext
    );
typedef DISABLE_IO_COUNTERS_TRACING *PDISABLE_IO_COUNTERS_TRACING;
PYTHON_TRACER_API DISABLE_IO_COUNTERS_TRACING DisableIoCountersTracing;

typedef
VOID
(ENABLE_HANDLE_COUNT_TRACING)(
    _In_ PPYTHON_TRACE_CONTEXT PythonTraceContext
    );
typedef ENABLE_HANDLE_COUNT_TRACING *PENABLE_HANDLE_COUNT_TRACING;
PYTHON_TRACER_API ENABLE_HANDLE_COUNT_TRACING EnableHandleCountTracing;

typedef
VOID
(DISABLE_HANDLE_COUNT_TRACING)(
    _In_ PPYTHON_TRACE_CONTEXT PythonTraceContext
    );
typedef DISABLE_HANDLE_COUNT_TRACING *PDISABLE_HANDLE_COUNT_TRACING;
PYTHON_TRACER_API DISABLE_HANDLE_COUNT_TRACING DisableHandleCountTracing;

typedef
BOOL
(START_TRACING)(
    _In_ PPYTHON_TRACE_CONTEXT PythonTraceContext
    );
typedef START_TRACING *PSTART_TRACING;
PYTHON_TRACER_API START_TRACING StartTracing;

typedef
BOOL
(STOP_TRACING)(
    _In_ PPYTHON_TRACE_CONTEXT PythonTraceContext
    );
typedef STOP_TRACING *PSTOP_TRACING;
PYTHON_TRACER_API STOP_TRACING StopTracing;

typedef
BOOL
(START_PROFILING)(
    _In_ PPYTHON_TRACE_CONTEXT PythonTraceContext
    );
typedef START_PROFILING *PSTART_PROFILING;
PYTHON_TRACER_API START_PROFILING StartProfiling;

typedef
BOOL
(STOP_PROFILING)(
    _In_ PPYTHON_TRACE_CONTEXT PythonTraceContext
    );
typedef STOP_PROFILING *PSTOP_PROFILING;
PYTHON_TRACER_API STOP_PROFILING StopProfiling;

typedef
BOOL
(START)(
    _In_ PPYTHON_TRACE_CONTEXT PythonTraceContext
    );
typedef START *PSTART;
PYTHON_TRACER_API START Start;

typedef
BOOL
(STOP)(
    _In_ PPYTHON_TRACE_CONTEXT PythonTraceContext
    );
typedef STOP *PSTOP;
PYTHON_TRACER_API STOP Stop;

typedef
BOOL
(ADD_MODULE_NAME)(
    _In_ PPYTHON_TRACE_CONTEXT PythonTraceContext,
    _In_ PPYOBJECT ModuleNameObject
    );
typedef ADD_MODULE_NAME *PADD_MODULE_NAME;
PYTHON_TRACER_API ADD_MODULE_NAME AddModuleName;

typedef
_Success_(return != 0)
BOOL
(SET_MODULE_NAMES_STRING_TABLE)(
    _In_ PPYTHON_TRACE_CONTEXT Context,
    _In_ PSTRING_TABLE StringTable
    );
typedef SET_MODULE_NAMES_STRING_TABLE *PSET_MODULE_NAMES_STRING_TABLE;
PYTHON_TRACER_API SET_MODULE_NAMES_STRING_TABLE \
                  SetModuleNamesStringTable;

typedef struct _PYTHON_TRACE_CONTEXT {

    ULONG             Size;                                 // 4    0   4

    //
    // Inline PYTHON_TRACE_CONTEXT_FLAGS.
    //

    union {
        PYTHON_TRACE_CONTEXT_FLAGS Flags;                   // 4    4   8
        struct {
            ULONG IsProfile:1;
            ULONG HasStarted:1;
            ULONG TraceMemory:1;
            ULONG TraceIoCounters:1;
            ULONG TraceHandleCount:1;
            ULONG HasModuleFilter:1;
            ULONG TrackMaxRefCounts:1;
        };
    };

    PRTL              Rtl;                                  // 8    8   16
    PPYTHON           Python;                               // 8    16  24
    PTRACE_CONTEXT    TraceContext;                         // 8    24  32
    PPYTRACEFUNC      PythonTraceFunction;                  // 8    32  40
    PVOID             UserData;                             // 8    40  48
    PALLOCATOR        Allocator;

    ULONGLONG         Depth;
    ULARGE_INTEGER    MaxDepth;

    ULONG             SkipFrames;
    ULONG             Padding;

    LARGE_INTEGER Frequency;
    LARGE_INTEGER StartTimestamp;
    LARGE_INTEGER StopTimestamp;
    LARGE_INTEGER LastTimestamp;
    LARGE_INTEGER ThisTimestamp;

    LARGE_INTEGER MaxNoneRefCount;
    LARGE_INTEGER MaxTrueRefCount;
    LARGE_INTEGER MaxZeroRefCount;
    LARGE_INTEGER MaxFalseRefCount;

    PPYTHON_FUNCTION FirstFunction;

    PSTRING_TABLE ModuleFilterStringTable;

    PREFIX_TABLE ModuleFilterPrefixTree;

    LIST_ENTRY Functions;

    PSTART_TRACING StartTracing;
    PSTOP_TRACING StopTracing;

    PSTART_PROFILING StartProfiling;
    PSTOP_PROFILING StopProfiling;

    PSTART Start;
    PSTOP Stop;

    PENABLE_MEMORY_TRACING EnableMemoryTracing;
    PDISABLE_MEMORY_TRACING DisableMemoryTracing;

    PENABLE_IO_COUNTERS_TRACING EnableIoCountersTracing;
    PDISABLE_IO_COUNTERS_TRACING DisableIoCountersTracing;

    PENABLE_HANDLE_COUNT_TRACING EnableHandleCountTracing;
    PDISABLE_HANDLE_COUNT_TRACING DisableHandleCountTracing;

    PADD_MODULE_NAME AddModuleName;
    PSET_MODULE_NAMES_STRING_TABLE SetModuleNamesStringTable;

} PYTHON_TRACE_CONTEXT, *PPYTHON_TRACE_CONTEXT;

typedef
_Success_(return == 0)
LONG
(PY_TRACE_CALLBACK)(
    _In_        PPYTHON_TRACE_CONTEXT   Context,
    _In_        PPYFRAMEOBJECT          FrameObject,
    _In_opt_    LONG                    EventType,
    _In_opt_    PPYOBJECT               ArgObject
    );
typedef PY_TRACE_CALLBACK *PPY_TRACE_CALLBACK;
PYTHON_TRACER_API PY_TRACE_CALLBACK PyTraceCallback;

typedef
_Check_return_
_Success_(return != 0)
BOOL
(INITIALIZE_PYTHON_TRACE_CONTEXT)(
    _In_ PRTL Rtl,
    _In_ PALLOCATOR Allocator,
    _Out_bytecap_(*SizeOfPythonTraceContext) PPYTHON_TRACE_CONTEXT
        PythonTraceContext,
    _Inout_ PULONG SizeOfPythonTraceContext,
    _In_ PPYTHON Python,
    _In_ PTRACE_CONTEXT TraceContext,
    _In_opt_ PPYTRACEFUNC PythonTraceFunction,
    _In_opt_ PVOID UserData
    );
typedef INITIALIZE_PYTHON_TRACE_CONTEXT *PINITIALIZE_PYTHON_TRACE_CONTEXT;
PYTHON_TRACER_API INITIALIZE_PYTHON_TRACE_CONTEXT InitializePythonTraceContext;

FORCEINLINE
VOID
UpdateMaxRefCounts(
    _In_ PPYTHON_TRACE_CONTEXT Context
    )
{
    PPYTHON Python;
    PPYOBJECT _Py_NoneStruct;
    PPYOBJECT _Py_TrueStruct;
    PPYOBJECT _Py_ZeroStruct;
    PPYOBJECT _Py_FalseStruct;

    Python = Context->Python;
    _Py_NoneStruct = Python->_Py_NoneStruct.Object;
    _Py_TrueStruct = Python->_Py_TrueStruct.Object;
    _Py_ZeroStruct = Python->_Py_ZeroStruct.Object;
    _Py_FalseStruct = Python->_Py_FalseStruct.Object;

    if (_Py_NoneStruct->ReferenceCount > Context->MaxNoneRefCount.QuadPart) {
        Context->MaxNoneRefCount.QuadPart = _Py_NoneStruct->ReferenceCount;
        if (Context->MaxNoneRefCount.HighPart) {
            __debugbreak();
        }
    }

    if (_Py_TrueStruct->ReferenceCount > Context->MaxTrueRefCount.QuadPart) {
        Context->MaxTrueRefCount.QuadPart = _Py_TrueStruct->ReferenceCount;
        if (Context->MaxTrueRefCount.HighPart) {
            __debugbreak();
        }
    }

    if (_Py_ZeroStruct->ReferenceCount > Context->MaxZeroRefCount.QuadPart) {
        Context->MaxZeroRefCount.QuadPart = _Py_ZeroStruct->ReferenceCount;
        if (Context->MaxZeroRefCount.HighPart) {
            __debugbreak();
        }
    }

    if (!_Py_FalseStruct) {
        return;
    }

    if (_Py_FalseStruct->ReferenceCount > Context->MaxFalseRefCount.QuadPart) {
        Context->MaxFalseRefCount.QuadPart = _Py_FalseStruct->ReferenceCount;
        if (Context->MaxFalseRefCount.HighPart) {
            __debugbreak();
        }
    }

}

//
// Trace Store relocation information.
//

PYTHON_TRACER_DATA TRACE_STORE_FIELD_RELOC \
    PythonFunctionTableRelocations[];

PYTHON_TRACER_DATA TRACE_STORE_FIELD_RELOC \
    PythonFunctionTableEntryRelocations[];

PYTHON_TRACER_DATA TRACE_STORE_FIELD_RELOC \
    PythonPathTableRelocations[];

PYTHON_TRACER_DATA TRACE_STORE_FIELD_RELOC \
    PythonPathTableEntryRelocations[];

PYTHON_TRACER_DATA TRACE_STORE_FIELD_RELOC \
    PythonTraceEventRelocations[];

PYTHON_TRACER_DATA TRACE_STORE_FIELD_RELOCS \
    PythonTracerTraceStoreRelocations[];


// vim:set ts=8 sw=4 sts=4 tw=80 expandtab                                     :
