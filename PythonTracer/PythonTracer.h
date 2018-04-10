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
#include "../StringTable.h"
#include "PythonTracer.h"

#endif

typedef enum _PYTHON_TRACE_EVENT_TYPE {
    PythonTraceEventNull = 0,
    PythonTraceEvent1 = 1,
    PythonTraceEvent2,
    PythonTraceEvent3,
    PythonTraceEvent4,
    PythonTraceEventInvalid
} PYTHON_TRACE_EVENT_TYPE, *PPYTHON_TRACE_EVENT_TYPE;

#define MAX_PYTHON_TRACE_EVENT_TYPE PythonTraceEventInvalid - 1

typedef union _PYTHON_EVENT_TRAITS_EX {
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
    LONG AsLong;
    ULONG AsULong;
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

typedef struct _PYTHON_CALL_STACK_ENTRY {
    LIST_ENTRY CallStackListEntry;      // 16
    LIST_ENTRY FunctionListEntry;       // 16   32
    PPYTHON_FUNCTION Function;          //  8   40
    LARGE_INTEGER EnterTimestamp;       //  8   48
    LARGE_INTEGER ReturnTimestamp;      //  8   56
    ULONGLONG Padding1;                 //  8   64
    ULONGLONG Padding2[8];              // 64  128
    ULONGLONG Padding3[16];             // 128 256
} PYTHON_CALL_STACK_ENTRY, *PPYTHON_CALL_STACK_ENTRY;
C_ASSERT(sizeof(PYTHON_CALL_STACK_ENTRY) == 256);

//
// Forward declarations.
//

typedef struct _PYTHON_TRACE_CONTEXT PYTHON_TRACE_CONTEXT;
typedef PYTHON_TRACE_CONTEXT *PPYTHON_TRACE_CONTEXT;

//
// Trace context flags.
//

typedef union _PYTHON_TRACE_CONTEXT_FLAGS {
    struct _Struct_size_bytes_(sizeof(ULONG)) {

        //
        // When set, the tracer will start off profiling instead of tracing.
        //

        ULONG ProfileOnly:1;

        //
        // When set, the tracer will *only* register for Python trace events,
        // which means it won't get called for C events.
        //

        ULONG TraceOnly:1;

        //
        // When set, tracks the maximum reference count values observed for
        // _Py_NoneStruct, _Py_TrueStruct, _Py_ZeroStruct, and _Py_FalseStruct
        // if it's available.  These values are saved to the LastRun registry
        // key at exit.
        //

        ULONG TrackMaxRefCounts:1;

        //
        // When set, a counter is incremented for every trace event type and
        // saved to the LastRun registry key at exit.
        //

        ULONG CountEvents:1;

        //
        // When set, enable call stack tracing for applicable event types.
        //

        ULONG TraceCallStack:1;

        //
        // The following three flags only apply when using event type 1.
        //

        //
        // When set, virtual memory stats will be tracked during tracing.
        // Defaults to FALSE.
        //

        ULONG TraceMemory:1;

        //
        // When set, I/O counters will be tracked during tracing.
        // Defaults to FALSE.
        //

        ULONG TraceIoCounters:1;

        //
        // When set, handle counts will be tracked during tracing.
        // Defaults to FALSE.
        //

        ULONG TraceHandleCount:1;

        //
        // End of event type 1 flags.
        //

        //
        // The following three flags control the logic that determines whether
        // a function is of interest for tracing.  The precendence is the same
        // order as the flags (i.e. TraceEverything > TraceNothing > ...).
        //

        ULONG TraceEverything:1;
        ULONG TraceNothing:1;
        ULONG TraceEverythingWhenNoModuleFilterSet:1;

        //
        // Disables the asynchronous threadpool processing of new Python path
        // table entries.
        //

        ULONG DisablePathTableEntryProcessing:1;
    };

    LONG AsLong;
    ULONG AsULong;

} PYTHON_TRACE_CONTEXT_FLAGS, *PPYTHON_TRACE_CONTEXT_FLAGS;
C_ASSERT(sizeof(PYTHON_TRACE_CONTEXT_FLAGS) == sizeof(ULONG));

//
// Runtime Parameters.
//

typedef struct _PYTHON_TRACER_RUNTIME_PARAMETERS {
    ULONG TraceEventType;
    ULONG CallbackWorkerType;
} PYTHON_TRACER_RUNTIME_PARAMETERS, *PPYTHON_TRACER_RUNTIME_PARAMETERS;

//
// Runtime State.
//

typedef union _PYTHON_TRACER_RUNTIME_STATE {
    ULONG AsLong;
    struct {
        ULONG IsProfiling:1;
        ULONG IsTracing:1;
        ULONG HasStarted:1;
        ULONG HasModuleFilter:1;
    };
} PYTHON_TRACER_RUNTIME_STATE, *PPYTHON_TRACER_RUNTIME_STATE;

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
_Check_return_
_Success_(return != 0)
BOOL
(SET_SYSTEM_TIME_FOR_RUN_HISTORY)(
    _In_ PPYTHON_TRACE_CONTEXT Context,
    _In_ PSYSTEMTIME SystemTime
    );
typedef SET_SYSTEM_TIME_FOR_RUN_HISTORY *PSET_SYSTEM_TIME_FOR_RUN_HISTORY;
PYTHON_TRACER_API SET_SYSTEM_TIME_FOR_RUN_HISTORY SetSystemTimeForRunHistory;

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

typedef
_Check_return_
_Maybe_raises_SEH_exception_
_Success_(return != 0)
BOOL
(PY_TRACE_EVENT)(
    _In_ PPYTHON_TRACE_CONTEXT PythonTraceContext,
    _In_ PPYTHON_FUNCTION Function,
    _In_ PPYTHON_EVENT_TRAITS EventTraits,
    _In_ PPYFRAMEOBJECT FrameObject,
    _In_opt_ PPYOBJECT ArgObject
    );
typedef PY_TRACE_EVENT *PPY_TRACE_EVENT;

PYTHON_TRACER_API PY_TRACE_EVENT PyTraceEvent1;
PYTHON_TRACER_API PY_TRACE_EVENT PyTraceEvent2;
PYTHON_TRACER_API PY_TRACE_EVENT PyTraceEvent3;
PYTHON_TRACER_API PY_TRACE_EVENT PyTraceEvent4;

typedef struct _Struct_size_bytes_(Size) _PYTHON_TRACE_CONTEXT {

    _Field_range_(==, sizeof(struct _PYTHON_TRACE_CONTEXT)) ULONG Size;

    PYTHON_TRACE_CONTEXT_FLAGS Flags;
    PYTHON_TRACER_RUNTIME_PARAMETERS RuntimeParameters;
    PYTHON_TRACER_RUNTIME_STATE RuntimeState;

    PRTL               Rtl;
    PPYTHON            Python;
    PTRACE_CONTEXT     TraceContext;
    PPY_TRACE_CALLBACK CallbackWorker;
    PPY_TRACE_EVENT    TraceEventFunction;
    PVOID              UserData;
    PALLOCATOR         Allocator;
    SYSTEMTIME         SystemTime;
    HKEY               RunHistoryRegistryKey;

    ULONGLONG          Depth;
    ULARGE_INTEGER     MaxDepth;
    ULONGLONG          FramesTraced;
    ULONGLONG          FramesSkipped;

    union {
        ULONGLONG Counters[7];
        struct {
            ULONGLONG          NumberOfPythonCalls;
            ULONGLONG          NumberOfPythonExceptions;
            ULONGLONG          NumberOfPythonLines;
            ULONGLONG          NumberOfPythonReturns;
            ULONGLONG          NumberOfCCalls;
            ULONGLONG          NumberOfCExceptions;
            ULONGLONG          NumberOfCReturns;
        };
    };

    ULONG SkipFrames;
    ULONG LastError;

    PYTHON_TRACE_EVENT_TYPE EventType;

    LARGE_INTEGER Frequency;
    LARGE_INTEGER Multiplicand;
    LARGE_INTEGER StartTimestamp;
    LARGE_INTEGER StopTimestamp;
    LARGE_INTEGER LastTimestamp;
    LARGE_INTEGER ThisTimestamp;

    LARGE_INTEGER MaxNoneRefCount;
    LARGE_INTEGER MaxTrueRefCount;
    LARGE_INTEGER MaxZeroRefCount;
    LARGE_INTEGER MaxFalseRefCount;

    PSTRING_TABLE_API StringTableApi;
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
    PSET_SYSTEM_TIME_FOR_RUN_HISTORY SetSystemTimeForRunHistory;

    struct _RTL_ATEXIT_ENTRY *SaveMaxCountsAtExitEntry;
    struct _RTL_ATEXIT_ENTRY *SaveCountsToRunHistoryAtExitEntry;
    struct _RTL_ATEXIT_ENTRY *SavePerformanceMetricsAtExitEntry;

    //
    // Custom allocators.  These are all backed by trace stores with the
    // concurrent allocations trait.
    //

    ALLOCATOR BitmapAllocator;
    ALLOCATOR UnicodeStringBufferAllocator;

    //
    // Threadpool callback environments.
    //

    PTP_CALLBACK_ENVIRON ThreadpoolCallbackEnvironment;
    PTP_CALLBACK_ENVIRON CancellationThreadpoolCallbackEnvironment;

    //
    // List head for calls stacks.
    //

    LIST_ENTRY CallStackListHead;

    //
    // Interlocked list heads.
    //

    SLIST_HEADER NewPythonPathTableEntryListHead;

    //
    // Threadpool work.
    //

    PTP_WORK NewPythonPathTableEntryWork;

    volatile ULONG ActiveNewPythonPathTableEntryCallbacks;

    //
    // Cleanup group.
    //

    PTP_CLEANUP_GROUP ThreadpoolCleanupGroup;

} PYTHON_TRACE_CONTEXT, *PPYTHON_TRACE_CONTEXT;

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

typedef union _CALL_STACK_ENTRY {
    PPYTHON_FUNCTION Function;
    struct {
        union {
            struct {
                ULONG IsCall:1;
                ULONG IsReturn:1;
                ULONG LowPartUnused:30;
            };
            struct {
                ULONG TaggedBits:2;
            };
        };
        ULONG HighPart;
    };
    ULONGLONG AsULongLong;
} CALL_STACK_ENTRY, *PCALL_STACK_ENTRY, **PPCALL_STACK_ENTRY;

FORCEINLINE
PPYTHON_FUNCTION
CallStackEntryToPythonFunction(
    _In_ PCALL_STACK_ENTRY CallStackEntry
    )
{
    CALL_STACK_ENTRY NewEntry;

    NewEntry.AsULongLong = CallStackEntry->AsULongLong;
    NewEntry.TaggedBits = 0;
    return NewEntry.Function;
}

FORCEINLINE
VOID
WriteCallStackEntry(
    _In_ PCALL_STACK_ENTRY Entry,
    _In_ PPYTHON_FUNCTION Function,
    _In_ BOOL IsCall
    )
{
    Entry->Function = Function;
    Entry->IsCall = (IsCall == TRUE);
    Entry->IsReturn = (IsCall != TRUE);
}

FORCEINLINE
_Check_return_
_Success_(return != 0)
BOOL
AllocateAndWriteCallStackEntry(
    _In_ PTRACE_STORE TraceStore,
    _In_ PLARGE_INTEGER Timestamp,
    _Out_ PPCALL_STACK_ENTRY EntryPointer,
    _In_ PPYTHON_FUNCTION Function,
    _In_ BOOL IsCall
    )
{
    PCALL_STACK_ENTRY Entry;
    ULONG_PTR NumberOfRecords = 1;
    ULONG_PTR RecordSize = sizeof(CALL_STACK_ENTRY);

    Entry = (PCALL_STACK_ENTRY)(
        TraceStore->AllocateRecordsWithTimestamp(
            TraceStore->TraceContext,
            TraceStore,
            NumberOfRecords,
            RecordSize,
            Timestamp
        )
    );

    if (!Entry) {
        return FALSE;
    }

    WriteCallStackEntry(Entry, Function, IsCall);
    *EntryPointer = Entry;

    return TRUE;
}

typedef
_Check_return_
_Success_(return != 0)
BOOL
(INITIALIZE_PYTHON_TRACE_CONTEXT)(
    _In_opt_ PRTL Rtl,
    _In_opt_ PALLOCATOR Allocator,
    _Out_bytecap_(*SizeOfPythonTraceContext)
        PPYTHON_TRACE_CONTEXT PythonTraceContext,
    _Inout_ PULONG SizeOfPythonTraceContext,
    _In_opt_ PPYTHON Python,
    _In_opt_ PTRACE_CONTEXT TraceContext,
    _In_opt_ PSTRING_TABLE_API StringTableApi,
    _In_opt_ PVOID UserData
    );
typedef INITIALIZE_PYTHON_TRACE_CONTEXT *PINITIALIZE_PYTHON_TRACE_CONTEXT;
PYTHON_TRACER_API INITIALIZE_PYTHON_TRACE_CONTEXT InitializePythonTraceContext;

typedef
VOID
(CLOSE_PYTHON_TRACE_CONTEXT)(
    _In_ PPYTHON_TRACE_CONTEXT PythonTraceContext
    );
typedef CLOSE_PYTHON_TRACE_CONTEXT *PCLOSE_PYTHON_TRACE_CONTEXT;
PYTHON_TRACER_API CLOSE_PYTHON_TRACE_CONTEXT ClosePythonTraceContext;

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
