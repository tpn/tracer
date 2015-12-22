
#ifdef __cplusplus
extern "C" {
#endif

#include "PythonTracer.h"

LONG
PyTraceCallbackBasic(
    _In_        PTRACE_CONTEXT  TraceContext,
    _In_        PPYFRAMEOBJECT  FrameObject,
    _In_opt_    LONG            EventType,
    _In_opt_    PPYOBJECT       ArgObject
)
{
    PSYSTEM_TIMER_FUNCTION SystemTimerFunction;
    PTRACE_STORES TraceStores;
    PTRACE_STORE Events;
    PTRACE_EVENT Event, LastEvent = NULL;
    ULARGE_INTEGER RecordSize = { sizeof(*Event) };
    ULARGE_INTEGER NumberOfRecords = { 1 };

    if (!TraceContext) {
        return 1;
    }

    TraceStores = TraceContext->TraceStores;
    if (!TraceStores) {
        return 1;
    }

    Events = &TraceStores->Events;

    Event = (PTRACE_EVENT)Events->AllocateRecords(TraceContext, Events, RecordSize, NumberOfRecords);

    SystemTimerFunction = TraceContext->SystemTimerFunction;

    if (SystemTimerFunction->GetSystemTimePreciseAsFileTime) {
        SystemTimerFunction->GetSystemTimePreciseAsFileTime(&Event->ftSystemTime);
    } else if (SystemTimerFunction->NtQuerySystemTime) {
        SystemTimerFunction->NtQuerySystemTime(&Event->liSystemTime);
    }

    Event->Version = 1;
    Event->EventType = (USHORT)EventType;

    if (sizeof(FrameObject) == sizeof(Event->uliFramePointer.QuadPart)) {
        // 64-bit
        Event->ullObjPointer = (ULONGLONG)ArgObject;
        Event->ullFramePointer = (ULONGLONG)FrameObject;
        Event->ProcessId = __readgsdword(0x40);
        Event->ThreadId = __readgsdword(0x48);
    } else {
        // 32-bit
        Event->uliObjPointer.LowPart = (DWORD)ArgObject;
        Event->uliFramePointer.LowPart = (DWORD)FrameObject;
        Event->ProcessId = __readgsdword(0x20);
        Event->ThreadId = __readgsdword(0x24);
    }

    Event->SequenceId = TraceContext->SequenceId;

    switch (EventType) {
        case TraceEventType_PyTrace_CALL:
            break;
        case TraceEventType_PyTrace_EXCEPTION:
            break;
        case TraceEventType_PyTrace_LINE:
            break;
        case TraceEventType_PyTrace_RETURN:
            break;
        case TraceEventType_PyTrace_C_CALL:
            break;
        case TraceEventType_PyTrace_C_EXCEPTION:
            break;
        case TraceEventType_PyTrace_C_RETURN:
            break;
    };

    return 0;
}

#ifdef __cplusplus
} // extern "C"
#endif