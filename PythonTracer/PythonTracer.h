// Copyright(c) Trent Nelson <trent@trent.me>
// All rights reserved.

#pragma once

#ifdef __cpplus
extern "C" {
#endif

#include "stdafx.h"

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
// Forward decls.
//
typedef struct _PYTHON_TRACE_CONTEXT PYTHON_TRACE_CONTEXT;
typedef PYTHON_TRACE_CONTEXT *PPYTHON_TRACE_CONTEXT;

typedef BOOLEAN (PYTHON_TRACE_CALL)(
    _Inout_ PPYTHON_TRACE_CONTEXT PythonTraceContext,
    _Inout_ PPYTHON_TRACE_EVENT          Event,
    _In_    PPYFRAMEOBJECT        FrameObject
    );
typedef PYTHON_TRACE_CALL *PPYTHON_TRACE_CALL;

typedef BOOLEAN (PYTHON_TRACE_LINE)(
    _Inout_ PPYTHON_TRACE_CONTEXT PythonTraceContext,
    _Inout_ PPYTHON_TRACE_EVENT          Event,
    _In_    PPYFRAMEOBJECT        FrameObject
    );
typedef PYTHON_TRACE_LINE *PPYTHON_TRACE_LINE;

typedef BOOLEAN (PYTHON_TRACE_RETURN)(
    _Inout_ PPYTHON_TRACE_CONTEXT PythonTraceContext,
    _Inout_ PPYTHON_TRACE_EVENT          Event,
    _In_    PPYFRAMEOBJECT        FrameObject
    );
typedef PYTHON_TRACE_RETURN *PPYTHON_TRACE_RETURN;

typedef BOOLEAN (REGISTER_PYTHON_FUNCTION)(
    _Inout_ PPYTHON_TRACE_CONTEXT PythonTraceContext,
    _Inout_ PPYTHON_TRACE_EVENT          Event,
    _In_    PPYFRAMEOBJECT        FrameObject
    );
typedef REGISTER_PYTHON_FUNCTION *PREGISTER_PYTHON_FUNCTION;

typedef BOOLEAN (PREPARE_TRACE_EVENT)(
    _Inout_  PPYTHON_TRACE_CONTEXT PythonTraceContext,
    _Inout_  PPYTHON_TRACE_EVENT   TraceEvent,
    _In_     PPYFRAMEOBJECT        FrameObject,
    _In_opt_ LONG                  EventType,
    _In_opt_ PPYOBJECT             ArgObject
    );
typedef PREPARE_TRACE_EVENT *PPREPARE_TRACE_EVENT;

typedef VOID (CONTINUE_TRACE_EVENT)(
    _Inout_ PPYTHON_TRACE_CONTEXT PythonTraceContext,
    _Inout_ PPYTHON_TRACE_EVENT          Event,
    _In_    PPYFRAMEOBJECT        FrameObject
    );
typedef CONTINUE_TRACE_EVENT *PCONTINUE_TRACE_EVENT;

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
VOID
(ENABLE_MEMORY_TRACING)(
    _In_ PPYTHON_TRACE_CONTEXT PythonTraceContext
    );
typedef ENABLE_MEMORY_TRACING *PENABLE_MEMORY_TRACING;

typedef
VOID
(DISABLE_MEMORY_TRACING)(
    _In_ PPYTHON_TRACE_CONTEXT PythonTraceContext
    );
typedef DISABLE_MEMORY_TRACING *PDISABLE_MEMORY_TRACING;

typedef
VOID
(ENABLE_IO_COUNTERS_TRACING)(
    _In_ PPYTHON_TRACE_CONTEXT PythonTraceContext
    );
typedef ENABLE_IO_COUNTERS_TRACING *PENABLE_IO_COUNTERS_TRACING;

typedef
VOID
(DISABLE_IO_COUNTERS_TRACING)(
    _In_ PPYTHON_TRACE_CONTEXT PythonTraceContext
    );
typedef DISABLE_IO_COUNTERS_TRACING *PDISABLE_IO_COUNTERS_TRACING;

typedef
VOID
(ENABLE_HANDLE_COUNT_TRACING)(
    _In_ PPYTHON_TRACE_CONTEXT PythonTraceContext
    );
typedef ENABLE_HANDLE_COUNT_TRACING *PENABLE_HANDLE_COUNT_TRACING;

typedef
VOID
(DISABLE_HANDLE_COUNT_TRACING)(
    _In_ PPYTHON_TRACE_CONTEXT PythonTraceContext
    );
typedef DISABLE_HANDLE_COUNT_TRACING *PDISABLE_HANDLE_COUNT_TRACING;

typedef
BOOL
(START_TRACING)(
    _In_ PPYTHON_TRACE_CONTEXT PythonTraceContext
    );
typedef START_TRACING *PSTART_TRACING;

typedef
BOOL
(STOP_TRACING)(
    _In_ PPYTHON_TRACE_CONTEXT PythonTraceContext
    );
typedef STOP_TRACING *PSTOP_TRACING;

typedef
BOOL
(START_PROFILING)(
    _In_ PPYTHON_TRACE_CONTEXT PythonTraceContext
    );
typedef START_PROFILING *PSTART_PROFILING;

typedef
BOOL
(STOP_PROFILING)(
    _In_ PPYTHON_TRACE_CONTEXT PythonTraceContext
    );
typedef STOP_PROFILING *PSTOP_PROFILING;

typedef
BOOL
(ADD_MODULE_NAME)(
    _In_ PPYTHON_TRACE_CONTEXT PythonTraceContext,
    _In_ PPYOBJECT ModuleNameObject
    );
typedef ADD_MODULE_NAME *PADD_MODULE_NAME;

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
        };
    };

    PRTL              Rtl;                                  // 8    8   16
    PPYTHON           Python;                               // 8    16  24
    PTRACE_CONTEXT    TraceContext;                         // 8    24  32
    PPYTRACEFUNC      PythonTraceFunction;                  // 8    32  40
    PVOID             UserData;                             // 8    40  48

    ULONG             Depth;
    ULONG             SkipFrames;

    ULONG Unused2;

    LARGE_INTEGER Frequency;
    LARGE_INTEGER StartTimestamp;
    LARGE_INTEGER StopTimestamp;
    LARGE_INTEGER LastTimestamp;
    LARGE_INTEGER ThisTimestamp;

    PPYTHON_FUNCTION FirstFunction;

    PREFIX_TABLE ModuleFilterTable;

    LIST_ENTRY Functions;

    PPREPARE_TRACE_EVENT PrepareTraceEvent;
    PCONTINUE_TRACE_EVENT ContinueTraceEvent;

    PREGISTER_PYTHON_FUNCTION RegisterPythonFunction;

    PPYTHON_TRACE_CALL TraceCall;
    PPYTHON_TRACE_LINE TraceLine;
    PPYTHON_TRACE_RETURN TraceReturn;

    PSTART_TRACING StartTracing;
    PSTOP_TRACING StopTracing;

    PSTART_PROFILING StartProfiling;
    PSTOP_PROFILING StopProfiling;

    PENABLE_MEMORY_TRACING EnableMemoryTracing;
    PDISABLE_MEMORY_TRACING DisableMemoryTracing;

    PENABLE_IO_COUNTERS_TRACING EnableIoCountersTracing;
    PDISABLE_IO_COUNTERS_TRACING DisableIoCountersTracing;

    PENABLE_HANDLE_COUNT_TRACING EnableHandleCountTracing;
    PDISABLE_HANDLE_COUNT_TRACING DisableHandleCountTracing;

    PADD_MODULE_NAME AddModuleName;

} PYTHON_TRACE_CONTEXT, *PPYTHON_TRACE_CONTEXT;

TRACER_API
LONG
PyTraceCallbackDummy(
    _In_        PPYTHON_TRACE_CONTEXT   PythonTraceContext,
    _In_        PPYFRAMEOBJECT          FrameObject,
    _In_opt_    LONG                    EventType,
    _In_opt_    PPYOBJECT               ArgObject
);

TRACER_API
LONG
PyTraceCallbackBasic(
    _In_        PPYTHON_TRACE_CONTEXT   PythonTraceContext,
    _In_        PPYFRAMEOBJECT          FrameObject,
    _In_opt_    LONG                    EventType,
    _In_opt_    PPYOBJECT               ArgObject
);

TRACER_API
LONG
PyTraceCallbackFast(
    _In_        PPYTHON_TRACE_CONTEXT   PythonTraceContext,
    _In_        PPYFRAMEOBJECT          FrameObject,
    _In_opt_    LONG                    EventType,
    _In_opt_    PPYOBJECT               ArgObject
);

TRACER_API
BOOL
PyTracePrepareTraceEvent(
    _In_        PPYTHON_TRACE_CONTEXT   PythonTraceContext,
    _Inout_     PPYTHON_TRACE_EVENT     TraceEvent,
    _In_        PPYFRAMEOBJECT          FrameObject,
    _In_opt_    LONG                    EventType,
    _In_opt_    PPYOBJECT               ArgObject
    );

TRACER_API
VOID
PyTraceContinueTraceEvent(
    _In_        PPYTHON_TRACE_CONTEXT   PythonTraceContext,
    _Inout_     PPYTHON_TRACE_EVENT     TraceEvent,
    _In_        PPYFRAMEOBJECT          FrameObject
    );

TRACER_API
BOOL
PyTraceRegisterPythonFunction(
    _In_        PPYTHON_TRACE_CONTEXT   PythonTraceContext,
    _Inout_     PPYTHON_TRACE_EVENT     TraceEvent,
    _In_        PPYFRAMEOBJECT          FrameObject
    );

TRACER_API
VOID
PyTraceCall(
    _In_        PPYTHON_TRACE_CONTEXT   PythonTraceContext,
    _Inout_     PPYTHON_TRACE_EVENT     TraceEvent,
    _In_        PPYFRAMEOBJECT          FrameObject
    );

TRACER_API
VOID
PyTraceLine(
    _In_        PPYTHON_TRACE_CONTEXT   PythonTraceContext,
    _Inout_     PPYTHON_TRACE_EVENT     TraceEvent,
    _In_        PPYFRAMEOBJECT          FrameObject
    );

TRACER_API
VOID
PyTraceReturn(
    _In_        PPYTHON_TRACE_CONTEXT   PythonTraceContext,
    _Inout_     PPYTHON_TRACE_EVENT     TraceEvent,
    _In_        PPYFRAMEOBJECT          FrameObject
    );


typedef BOOL (INITIALIZE_PYTHON_TRACE_CONTEXT)(
    _In_ PRTL Rtl,
    _Out_bytecap_(*SizeOfPythonTraceContext) PPYTHON_TRACE_CONTEXT
                                             PythonTraceContext,
    _Inout_ PULONG SizeOfPythonTraceContext,
    _In_ PPYTHON Python,
    _In_ PTRACE_CONTEXT TraceContext,
    _In_opt_ PPYTRACEFUNC PythonTraceFunction,
    _In_opt_ PVOID UserData
    );

typedef INITIALIZE_PYTHON_TRACE_CONTEXT *PINITIALIZE_PYTHON_TRACE_CONTEXT;

TRACER_API
BOOL
AddFunction(
    _In_    PPYTHON_TRACE_CONTEXT   PythonTraceContext,
    _In_    PVOID                   FunctionObject
    );

// vim:set ts=8 sw=4 sts=4 tw=80 expandtab                                     :
