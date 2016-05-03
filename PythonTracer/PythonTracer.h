// Copyright(c) Trent Nelson <trent@trent.me>
// All rights reserved.

#pragma once

#ifdef __cpplus
extern "C" {
#endif

#include <Windows.h>
#include "../Rtl/Rtl.h"
#include "../Tracer/Tracer.h"
#include "../Tracer/Tracing.h"
#include "../Python/Python.h"

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

typedef struct _EVENT_TYPE {
    TraceEventType  Id;
    PCWSTR          Name;
    PCSTR           NameA;
} EVENT_TYPE, *PEVENT_TYPE;

static const EVENT_TYPE EventTypes[] = {
    { TraceEventType_PyTrace_CALL,          L"PyTrace_CALL",        "PyTrace_CALL" },
    { TraceEventType_PyTrace_EXCEPTION,     L"PyTrace_EXCEPTION",   "PyTrace_EXCEPTION" },
    { TraceEventType_PyTrace_LINE,          L"PyTrace_LINE",        "PyTrace_LINE" },
    { TraceEventType_PyTrace_RETURN,        L"PyTrace_RETURN",      "PyTrace_RETURN" },
    { TraceEventType_PyTrace_C_CALL,        L"PyTrace_C_CALL",      "PyTrace_C_CALL" },
    { TraceEventType_PyTrace_C_EXCEPTION,   L"PyTrace_C_EXCEPTION", "PyTrace_C_EXCEPTION" },
    { TraceEventType_PyTrace_C_RETURN,      L"PyTrace_C_RETURN",    "PyTrace_C_RETURN" },
};

static const DWORD NumberOfTraceEventTypes = (
    sizeof(EventTypes) /
    sizeof(EVENT_TYPE)
);

//
// Forward decls.
//
typedef struct _PYTHON_TRACE_CONTEXT PYTHON_TRACE_CONTEXT;
typedef PYTHON_TRACE_CONTEXT *PPYTHON_TRACE_CONTEXT;

typedef BOOLEAN (PYTHON_TRACE_CALL)(
    _Inout_ PPYTHON_TRACE_CONTEXT PythonTraceContext,
    _Inout_ PTRACE_EVENT          Event,
    _In_    PPYFRAMEOBJECT        FrameObject
    );
typedef PYTHON_TRACE_CALL *PPYTHON_TRACE_CALL;

typedef BOOLEAN (PYTHON_TRACE_LINE)(
    _Inout_ PPYTHON_TRACE_CONTEXT PythonTraceContext,
    _Inout_ PTRACE_EVENT          Event,
    _In_    PPYFRAMEOBJECT        FrameObject
    );
typedef PYTHON_TRACE_LINE *PPYTHON_TRACE_LINE;

typedef BOOLEAN (PYTHON_TRACE_RETURN)(
    _Inout_ PPYTHON_TRACE_CONTEXT PythonTraceContext,
    _Inout_ PTRACE_EVENT          Event,
    _In_    PPYFRAMEOBJECT        FrameObject
    );
typedef PYTHON_TRACE_RETURN *PPYTHON_TRACE_RETURN;

typedef BOOLEAN (REGISTER_PYTHON_FUNCTION)(
    _Inout_ PPYTHON_TRACE_CONTEXT PythonTraceContext,
    _Inout_ PTRACE_EVENT          Event,
    _In_    PPYFRAMEOBJECT        FrameObject
    );
typedef REGISTER_PYTHON_FUNCTION *PREGISTER_PYTHON_FUNCTION;

typedef BOOLEAN (PREPARE_TRACE_EVENT)(
    _Inout_  PPYTHON_TRACE_CONTEXT PythonTraceContext,
    _Inout_  PTRACE_EVENT          TraceEvent,
    _In_     PPYFRAMEOBJECT        FrameObject,
    _In_opt_ LONG                  EventType,
    _In_opt_ PPYOBJECT             ArgObject
    );
typedef PREPARE_TRACE_EVENT *PPREPARE_TRACE_EVENT;

typedef VOID (CONTINUE_TRACE_EVENT)(
    _Inout_ PPYTHON_TRACE_CONTEXT PythonTraceContext,
    _Inout_ PTRACE_EVENT          Event,
    _In_    PPYFRAMEOBJECT        FrameObject
    );
typedef CONTINUE_TRACE_EVENT *PCONTINUE_TRACE_EVENT;

typedef struct _PYTHON_TRACE_CONTEXT {
    ULONG             Size;
    PRTL              Rtl;
    PPYTHON           Python;
    PTRACE_CONTEXT    TraceContext;
    PPYTRACEFUNC      PythonTraceFunction;
    PVOID             UserData;

    ULONG             Depth;
    ULONG             SkipFrames;
    union {
        ULONG Flags;
        struct {
            ULONG StartedTracing:1;
        };
    };
    ULONG Unused1;

    ULONGLONG LastTimestamp;


    PPREPARE_TRACE_EVENT PrepareTraceEvent;
    PCONTINUE_TRACE_EVENT ContinueTraceEvent;

    PREGISTER_PYTHON_FUNCTION RegisterPythonFunction;

    PPYTHON_TRACE_CALL TraceCall;
    PPYTHON_TRACE_LINE TraceLine;
    PPYTHON_TRACE_RETURN TraceReturn;

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
    _Inout_     PTRACE_EVENT            TraceEvent,
    _In_        PPYFRAMEOBJECT          FrameObject,
    _In_opt_    LONG                    EventType,
    _In_opt_    PPYOBJECT               ArgObject
    );

TRACER_API
VOID
PyTraceContinueTraceEvent(
    _In_        PPYTHON_TRACE_CONTEXT   PythonTraceContext,
    _Inout_     PTRACE_EVENT            TraceEvent,
    _In_        PPYFRAMEOBJECT          FrameObject
    );

TRACER_API
BOOL
PyTraceRegisterPythonFunction(
    _In_        PPYTHON_TRACE_CONTEXT   PythonTraceContext,
    _Inout_     PTRACE_EVENT            TraceEvent,
    _In_        PPYFRAMEOBJECT          FrameObject
    );

TRACER_API
VOID
PyTraceCall(
    _In_        PPYTHON_TRACE_CONTEXT   PythonTraceContext,
    _Inout_     PTRACE_EVENT            TraceEvent,
    _In_        PPYFRAMEOBJECT          FrameObject
    );

TRACER_API
VOID
PyTraceLine(
    _In_        PPYTHON_TRACE_CONTEXT   PythonTraceContext,
    _Inout_     PTRACE_EVENT            TraceEvent,
    _In_        PPYFRAMEOBJECT          FrameObject
    );

TRACER_API
VOID
PyTraceReturn(
    _In_        PPYTHON_TRACE_CONTEXT   PythonTraceContext,
    _Inout_     PTRACE_EVENT            TraceEvent,
    _In_        PPYFRAMEOBJECT          FrameObject
    );


TRACER_API
BOOL
InitializePythonTraceContext(
    _In_                                        PRTL                    Rtl,
    _Out_bytecap_(*SizeOfPythonTraceContext)    PPYTHON_TRACE_CONTEXT   PythonTraceContext,
    _Inout_                                     PULONG                  SizeOfPythonTraceContext,
    _In_                                        PPYTHON                 Python,
    _In_                                        PTRACE_CONTEXT          TraceContext,
    _In_opt_                                    PPYTRACEFUNC            PythonTraceFunction,
    _In_opt_                                    PVOID                   UserData
);

TRACER_API
BOOL
StartTracing(
    _In_    PPYTHON_TRACE_CONTEXT   PythonTraceContext
);

TRACER_API
BOOL
StopTracing(
    _In_    PPYTHON_TRACE_CONTEXT   PythonTraceContext
);

TRACER_API
BOOL
StartProfiling(
    _In_    PPYTHON_TRACE_CONTEXT   PythonTraceContext
);

TRACER_API
BOOL
StopProfiling(
    _In_    PPYTHON_TRACE_CONTEXT   PythonTraceContext
);

TRACER_API
BOOL
AddFunction(
    _In_    PPYTHON_TRACE_CONTEXT   PythonTraceContext,
    _In_    PVOID                   FunctionObject
);
