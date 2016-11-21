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
#include "../Rtl/__C_specific_handler.h"
#include "../Python/Python.h"
#include "../TraceStore/TraceStore.h"
#include "../StringTable/StringTable.h"
#include "PythonTracer.h"

#endif

typedef enum _PYTHON_TRACE_EVENT_TYPE {
    PythonTraceEventNull = 0,
    PythonTraceEvent1 = 1,
    PythonTraceEvent2,
    PythonTraceEventInvalid
} PYTHON_TRACE_EVENT_TYPE, *PPYTHON_TRACE_EVENT_TYPE;

#define MAX_PYTHON_TRACE_EVENT_TYPE PythonTraceEventInvalid - 1

typedef union _PYTHON_EVENT_TRAITS_EX {
    ULONG AsLong;
    struct _Struct_size_bytes_(sizeof(ULONG)) {

        //
        // Inline the PYTHON_EVENT_TRAITS structure.
        //

        ULONG IsCall:1;
        ULONG IsException:1;
        ULONG IsLine:1;
        ULONG IsReturn:1;
        ULONG IsC:1;
        ULONG AsEventType:3;
        ULONG IsReverseJump:1;
        ULONG LineNumberOrCallStackDepth:23;
    };
} PYTHON_EVENT_TRAITS_EX, *PPYTHON_EVENT_TRAITS_EX;

C_ASSERT(sizeof(PYTHON_EVENT_TRAITS_EX) == sizeof(ULONG));

#pragma pack(push, DefaultAlignment, 2)

typedef struct _PYTHON_TRACE_EVENT1 {
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

    PYTHON_EVENT_TRAITS_EX EventTraitsEx; // 4      116

    SHORT HandleDelta;                  // 2        118
    USHORT PageFaultDelta;              // 2        120

    USHORT LineNumber;                  // 2        122
    USHORT FirstLineNumber;             // 2        124
    USHORT NumberOfLines;               // 2        126
    USHORT NumberOfCodeLines;           // 2        128

} PYTHON_TRACE_EVENT1, *PPYTHON_TRACE_EVENT1, **PPPYTHON_TRACE_EVENT1;

#pragma pack(pop, DefaultAlignment)

C_ASSERT(sizeof(PYTHON_TRACE_EVENT1) == 128);

typedef struct _PYTHON_TRACE_EVENT2 {
    PPYTHON_FUNCTION Function;
} PYTHON_TRACE_EVENT2, *PPYTHON_TRACE_EVENT2;

C_ASSERT(sizeof(PYTHON_TRACE_EVENT2) == 8);

//
// Forward declarations.
//

typedef struct _PYTHON_TRACE_CONTEXT PYTHON_TRACE_CONTEXT;
typedef PYTHON_TRACE_CONTEXT *PPYTHON_TRACE_CONTEXT;

typedef union _PYTHON_TRACE_CONTEXT_FLAGS {
    ULONG AsLong;
    struct {
        ULONG ProfileOnly:1;
        ULONG IsProfiling:1;
        ULONG IsTracing:1;
        ULONG HasStarted:1;
        ULONG TraceMemory:1;
        ULONG TraceIoCounters:1;
        ULONG TraceHandleCount:1;
        ULONG TrackMaxRefCounts:1;
        ULONG HasModuleFilter:1;
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
    _In_ PPYTHON_TRACE_CONTEXT Context
    );
typedef ENABLE_MEMORY_TRACING *PENABLE_MEMORY_TRACING;
PYTHON_TRACER_API ENABLE_MEMORY_TRACING EnableMemoryTracing;

typedef
VOID
(DISABLE_MEMORY_TRACING)(
    _In_ PPYTHON_TRACE_CONTEXT Context
    );
typedef DISABLE_MEMORY_TRACING *PDISABLE_MEMORY_TRACING;
PYTHON_TRACER_API DISABLE_MEMORY_TRACING DisableMemoryTracing;

typedef
VOID
(ENABLE_IO_COUNTERS_TRACING)(
    _In_ PPYTHON_TRACE_CONTEXT Context
    );
typedef ENABLE_IO_COUNTERS_TRACING *PENABLE_IO_COUNTERS_TRACING;
PYTHON_TRACER_API ENABLE_IO_COUNTERS_TRACING EnableIoCountersTracing;

typedef
VOID
(DISABLE_IO_COUNTERS_TRACING)(
    _In_ PPYTHON_TRACE_CONTEXT Context
    );
typedef DISABLE_IO_COUNTERS_TRACING *PDISABLE_IO_COUNTERS_TRACING;
PYTHON_TRACER_API DISABLE_IO_COUNTERS_TRACING DisableIoCountersTracing;

typedef
VOID
(ENABLE_HANDLE_COUNT_TRACING)(
    _In_ PPYTHON_TRACE_CONTEXT Context
    );
typedef ENABLE_HANDLE_COUNT_TRACING *PENABLE_HANDLE_COUNT_TRACING;
PYTHON_TRACER_API ENABLE_HANDLE_COUNT_TRACING EnableHandleCountTracing;

typedef
VOID
(DISABLE_HANDLE_COUNT_TRACING)(
    _In_ PPYTHON_TRACE_CONTEXT Context
    );
typedef DISABLE_HANDLE_COUNT_TRACING *PDISABLE_HANDLE_COUNT_TRACING;
PYTHON_TRACER_API DISABLE_HANDLE_COUNT_TRACING DisableHandleCountTracing;

typedef
BOOL
(START)(
    _In_ PPYTHON_TRACE_CONTEXT Context
    );
typedef START *PSTART;
PYTHON_TRACER_API START Start;

typedef
BOOL
(STOP)(
    _In_ PPYTHON_TRACE_CONTEXT Context
    );
typedef STOP *PSTOP;
PYTHON_TRACER_API STOP Stop;

typedef
BOOL
(ADD_MODULE_NAME)(
    _In_ PPYTHON_TRACE_CONTEXT Context,
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

typedef
_Check_return_
_Maybe_raises_SEH_exception_
_Success_(return != 0)
BOOL
(PY_TRACE_CALLBACK)(
    _In_        PPYTHON_TRACE_CONTEXT   Context,
    _In_        PPYFRAMEOBJECT          FrameObject,
    _In_opt_    LONG                    EventType,
    _In_opt_    PPYOBJECT               ArgObject
    );
typedef PY_TRACE_CALLBACK *PPY_TRACE_CALLBACK;

PYTHON_TRACER_API PY_TRACE_CALLBACK PyTraceEvent1;
PYTHON_TRACER_API PY_TRACE_CALLBACK PyTraceEvent2;

typedef struct _PYTHON_TRACE_CONTEXT {

    ULONG              Size;                                 // 4    0   4

    PYTHON_TRACE_CONTEXT_FLAGS Flags;                        // 4    4   8

    PRTL               Rtl;                                  // 8    8   16
    PPYTHON            Python;                               // 8    16  24
    PTRACE_CONTEXT     TraceContext;                         // 8    24  32
    PPY_TRACE_CALLBACK PythonTraceFunction;                  // 8    32  40
    PVOID              UserData;                             // 8    40  48
    PALLOCATOR         Allocator;

    ULONGLONG          Depth;
    ULARGE_INTEGER     MaxDepth;

    ULONG              SkipFrames;
    ULONG              Padding;

    PYTHON_TRACE_EVENT_TYPE EventType;

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
_Check_return_
_Success_(return != 0)
PPY_TRACE_CALLBACK
(GET_CALLBACK_FOR_TRACE_EVENT_TYPE)(
    _In_ PYTHON_TRACE_EVENT_TYPE TraceEventType
    );
typedef GET_CALLBACK_FOR_TRACE_EVENT_TYPE *PGET_CALLBACK_FOR_TRACE_EVENT_TYPE;
PYTHON_TRACER_API GET_CALLBACK_FOR_TRACE_EVENT_TYPE \
                  GetCallbackForTraceEventType;

typedef
_Check_return_
_Success_(return != 0)
PPYTHON_TRACE_EVENT1
(ALLOCATE_PYTHON_TRACE_EVENT1)(
    _In_ PTRACE_STORE TraceStore,
    _In_ PLARGE_INTEGER Timestamp
    );
typedef ALLOCATE_PYTHON_TRACE_EVENT1 *PALLOCATE_PYTHON_TRACE_EVENT1;
PYTHON_TRACER_API ALLOCATE_PYTHON_TRACE_EVENT1 AllocatePythonTraceEvent1;

typedef
_Check_return_
_Success_(return != 0)
PPYTHON_TRACE_EVENT2
(ALLOCATE_PYTHON_TRACE_EVENT2)(
    _In_ PTRACE_STORE TraceStore,
    _In_ PLARGE_INTEGER Timestamp
    );
typedef ALLOCATE_PYTHON_TRACE_EVENT2 *PALLOCATE_PYTHON_TRACE_EVENT2;
PYTHON_TRACER_API ALLOCATE_PYTHON_TRACE_EVENT2 AllocatePythonTraceEvent2;


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
    _In_ PYTHON_TRACE_EVENT_TYPE PythonTraceEventType,
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
