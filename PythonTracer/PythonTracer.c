////////////////////////////////////////////////////////////////////////////////
// PythonTracer.c
////////////////////////////////////////////////////////////////////////////////
#ifdef __cplusplus
extern "C" {
#endif

#include "PythonTracer.h"

PVOID
TraceStoreAllocationRoutine(
    _In_opt_ PVOID AllocationContext,
    _In_ const ULONG ByteSize
    )
{
    PTRACE_STORE TraceStore = (PTRACE_STORE)AllocationContext;
    ULARGE_INTEGER NumberOfRecords = { 1 };
    ULARGE_INTEGER RecordSize = { ByteSize };

    return TraceStore->AllocateRecords(
        TraceStore->TraceContext,
        TraceStore,
        &RecordSize,
        &NumberOfRecords
    );
}

PPYTHON_TRACE_EVENT
AllocatePythonTraceEvent(
    _In_ PTRACE_STORE TraceStore
    )
{
    ULARGE_INTEGER NumberOfRecords = { 1 };
    ULARGE_INTEGER RecordSize = { sizeof(PYTHON_TRACE_EVENT) };

    return (PPYTHON_TRACE_EVENT)(
        TraceStore->AllocateRecords(
            TraceStore->TraceContext,
            TraceStore,
            &RecordSize,
            &NumberOfRecords
        )
    );
}

VOID
TraceStoreFreeRoutine(
    _In_opt_ PVOID FreeContext,
    _In_     PVOID Buffer
    )
{
    PTRACE_STORE TraceStore = (PTRACE_STORE)FreeContext;

    TraceStore->FreeRecords(TraceStore->TraceContext,
                            TraceStore,
                            Buffer);
}

FORCEINLINE
VOID
PrepareTraceEvent(
    _In_    PPYTHON_TRACE_CONTEXT Context,
    _Inout_ PPYTHON_TRACE_EVENT   Event,
    _In_    PPYTHON_FUNCTION      Function
    )
{
    Context->LastTimestamp.QuadPart = Context->ThisTimestamp.QuadPart;
    QueryPerformanceCounter(&Context->ThisTimestamp);

}

LONG
PyTraceCallback(
    _In_        PPYTHON_TRACE_CONTEXT   Context,
    _In_        PPYFRAMEOBJECT          FrameObject,
    _In_opt_    LONG                    EventType,
    _In_opt_    PPYOBJECT               ArgObject
    )
{
    BOOL Success;
    BOOL IsFirstTrace;
    BOOL StartedTracing;
    BOOL IsCall;
    BOOL IsReturn;
    BOOL IsLine;
    BOOL IsException;
    BOOL IsC;

    PRTL Rtl;
    PPYTHON Python;
    PTRACE_CONTEXT TraceContext;
    PTRACE_STORES TraceStores;
    PTRACE_STORE Events;
    PYTHON_TRACE_EVENT Event;
    PPYTHON_TRACE_EVENT LastEvent;
    PPYTHON_TRACE_EVENT ThisEvent;
    PPYTHON_FUNCTION Function;
    PSTRING ModuleName;
    PPREFIX_TABLE Table;
    PPREFIX_TABLE_ENTRY Entry;
    LARGE_INTEGER Elapsed;

    IsFirstTrace = FALSE;
    StartedTracing = (BOOL)Context->StartedTracing;

    IsCall = (
        EventType == TraceEventType_PyTrace_CALL   ||
        EventType == TraceEventType_PyTrace_C_CALL
    );

    IsReturn = (
        EventType == TraceEventType_PyTrace_RETURN   ||
        EventType == TraceEventType_PyTrace_C_RETURN
    );

    IsLine = (
        EventType == TraceEventType_PyTrace_LINE
    );

    IsException = (
        EventType == TraceEventType_PyTrace_EXCEPTION   ||
        EventType == TraceEventType_PyTrace_C_EXCEPTION
    );

    IsC = (
        EventType == TraceEventType_PyTrace_C_CALL      ||
        EventType == TraceEventType_PyTrace_C_RETURN    ||
        EventType == TraceEventType_PyTrace_C_EXCEPTION
    );

    if (!StartedTracing) {

        if (!IsCall) {

            //
            // If we haven't started tracing yet, we can ignore any events that
            // aren't PyTrace_CALL or PyTrace_C_CALL.
            //

            return 0;
        }

        //
        // We haven't started tracing yet, and this is a call event.  See if
        // we need to skip the frame.
        //

        if (++Context->Depth <= Context->SkipFrames) {

            //
            // Skip the frame.
            //

            return 0;
        }

        //
        // Indicate we've started tracing by toggling the context flag.
        //

        StartedTracing = Context->StartedTracing = TRUE;

        IsFirstTrace = TRUE;

    } else {

        //
        // We've already started tracing.
        //

        if (IsReturn) {

            //
            // Context->Depth should never be 0 here if StartedTracing is set.
            //

            if (!Context->Depth) {
                __debugbreak();
            }

            if (--Context->Depth <= Context->SkipFrames) {

                //
                // We're returning from a skipped frame.  Stop tracing.
                //

                Context->StartedTracing = FALSE;

                return 0;

            }
        }
    }

    //
    // Attempt to register the frame and get the underlying function object.
    //

    Rtl = Context->Rtl;
    Python = Context->Python;

    Success = Python->RegisterFrame(Python,
                                    FrameObject,
                                    EventType,
                                    ArgObject,
                                    &Function);

    if (!Success) {

        //
        // We can't do anything more if we weren't able to resolve the
        // function for this frame.
        //

        return 0;
    }

    //
    // We obtained the PYTHON_FUNCTION for this frame; use the module name to
    // determine if we should keep tracing.
    //

    ModuleName = &Function->PathEntry.ModuleName;
    Table = &Context->ModuleFilterTable;

    if (IsFirstTrace && !Context->FirstFunction) {

        //
        // This is the first function we've traced.  Use the function's module
        // name as our initial filter.
        //

        Context->FirstFunction = Function;
        Entry = &Function->ModuleNameEntry;
        Rtl->PfxInsertPrefix(Table, ModuleName, Entry);

    } else {

        Entry = Rtl->PfxFindPrefix(Table, ModuleName);

        if (!Entry) {

            //
            // The function doesn't reside in a module we're tracing.
            //

            return 0;
        }
    }

    //
    // The function resides in a module (or submodule) we're tracing, continue.
    //


    //
    // Load the events trace store and previous event record, if any.
    //

    TraceContext = Context->TraceContext;
    TraceStores = TraceContext->TraceStores;
    Events = &TraceStores->Stores[TRACE_STORE_EVENTS_INDEX];

    LastEvent = (PPYTHON_TRACE_EVENT)Events->PrevAddress;

    //
    // Save the timestamp for this event.
    //

    QueryPerformanceCounter(&Event.Timestamp);

    //
    // Fill out the function.
    //

    Event.Function = Function;
    Event.Flags = 0;
    Event.IsC = IsC;

    if (IsException) {

        //
        // We don't do anything for exceptions at the moment.
        //

        Event.IsException = TRUE;

    } else if (IsCall) {

        Event.IsCall = TRUE;

    } else if (IsReturn) {

        Event.IsReturn = TRUE;

    } else if (IsLine) {

        Event.IsLine = TRUE;

        //
        // Update the line number for this event.
        //

        Event.LineNumber = Python->PyFrame_GetLineNumber(FrameObject);

        if (LastEvent && LastEvent->IsLine) {

            //
            // Update the duration for the line event.
            //

            Elapsed.QuadPart = (
                Event.Timestamp.QuadPart -
                LastEvent->Timestamp.QuadPart
            );

            //
            // Microseconds to seconds.
            //

            Elapsed.QuadPart *= 1000000;

            //
            // Divide by frequency to get elapsed microseconds.
            //

            Elapsed.QuadPart /= Context->Frequency.QuadPart;

            //
            // Copy the elapsed microsecond value back to the last event.
            //

            LastEvent->Elapsed.QuadPart = Elapsed.QuadPart;

            //
            // If the last event's line number was greater than this line
            // number, we've jumped backwards, presumably as part of a loop.
            //

            if (LastEvent->LineNumber > Event.LineNumber) {
                Event.IsReverseJump = TRUE;
            }

        }
    }

    //
    // Allocate a new event record, then copy our temporary event over.
    //

    ThisEvent = AllocatePythonTraceEvent(Events);

    if (!ThisEvent) {
        return 0;
    }

    Rtl->CopyToMemoryMappedMemory(Rtl, ThisEvent, &Event, sizeof(Event));

    return 0;
}

PVOID
NTAPI
CodeObjectAllocateFromStore(
    _In_ PRTL_GENERIC_TABLE Table,
    _In_ CLONG ByteSize
    )
{
    PPYTHON_TRACE_CONTEXT PythonTraceContext;
    if (!Table) {
        return NULL;
    }

    if (ByteSize <= 0) {
        return NULL;
    }

    PythonTraceContext = (PPYTHON_TRACE_CONTEXT)Table->TableContext;

    return NULL;
}

PVOID
NTAPI
CodeObjectAllocateFromHeap(
    _In_ PRTL_GENERIC_TABLE Table,
    _In_ CLONG ByteSize
    )
{
    PPYTHON_TRACE_CONTEXT PythonTraceContext;
    HANDLE HeapHandle;
    if (!Table) {
        return NULL;
    }

    if (ByteSize <= 0) {
        return NULL;
    }

    PythonTraceContext = (PPYTHON_TRACE_CONTEXT)Table->TableContext;

    HeapHandle = GetProcessHeap();
    if (!HeapHandle) {
        return NULL;
    }

    return HeapAlloc(HeapHandle, HEAP_ZERO_MEMORY, ByteSize);
}

VOID
NTAPI
CodeObjectFreeFromHeap(
    _In_ PRTL_GENERIC_TABLE Table,
    _In_ PVOID Buffer
    )
{
    PPYTHON_TRACE_CONTEXT PythonTraceContext;
    HANDLE HeapHandle;

    if (!Table) {
        return;
    }

    if (Buffer == NULL) {
        return;
    }

    PythonTraceContext = (PPYTHON_TRACE_CONTEXT)Table->TableContext;

    HeapHandle = GetProcessHeap();
    if (!HeapHandle) {
        return;
    }

    HeapFree(HeapHandle, 0, Buffer);
}

FORCEINLINE
RTL_GENERIC_COMPARE_RESULTS
GenericComparePointer(
    _In_ PRTL_GENERIC_TABLE Table,
    _In_ PVOID FirstStruct,
    _In_ PVOID SecondStruct
    )
{
    PULONG_PTR First = (PULONG_PTR)FirstStruct;
    PULONG_PTR Second = (PULONG_PTR)SecondStruct;

    if (First < Second) {
        return GenericLessThan;
    }
    else if (First > Second) {
        return GenericGreaterThan;
    }
    else {
        return GenericEqual;
    }
}

FORCEINLINE
RTL_GENERIC_COMPARE_RESULTS
GenericComparePyObjectHash(
    _In_ PPYTHON Python,
    _In_ PVOID FirstStruct,
    _In_ PVOID SecondStruct
    )
{
    PPYOBJECT First = (PPYOBJECT)FirstStruct;
    PPYOBJECT Second = (PPYOBJECT)SecondStruct;

    LONG FirstHash = Python->PyObject_Hash(First);
    LONG SecondHash = Python->PyObject_Hash(Second);

    if (First < Second) {
        return GenericLessThan;
    }
    else if (First > Second) {
        return GenericGreaterThan;
    }
    else {
        return GenericEqual;
    }
}

RTL_GENERIC_COMPARE_RESULTS
NTAPI
CodeObjectCompare(
    _In_ PRTL_GENERIC_TABLE Table,
    _In_ PVOID FirstStruct,
    _In_ PVOID SecondStruct
    )
{
    return GenericComparePointer(Table, FirstStruct, SecondStruct);
}

RTL_GENERIC_COMPARE_RESULTS
NTAPI
FunctionCompare(
    _In_ PRTL_GENERIC_TABLE Table,
    _In_ PVOID FirstStruct,
    _In_ PVOID SecondStruct
    )
{
    PPYTHON_FUNCTION First = (PPYTHON_FUNCTION)FirstStruct;
    PPYTHON_FUNCTION Second = (PPYTHON_FUNCTION)SecondStruct;

    //PPYTHON Python;
    //PPYTHON_TRACE_CONTEXT Context;

    /*
    Context = CONTAINING_RECORD(Table->TableContext,
                                PYTHON_TRACE_CONTEXT,
                                FunctionTable);

    Python = Context->Python;
    */

    return GenericComparePointer(Table,
                                 First->CodeObject,
                                 Second->CodeObject);
}


BOOL
InitializePythonTraceContext(
    _In_ PRTL Rtl,
    _Out_bytecap_(*SizeOfContext) PPYTHON_TRACE_CONTEXT Context,
    _Inout_ PULONG SizeOfContext,
    _In_ PPYTHON Python,
    _In_ PTRACE_CONTEXT TraceContext,
    _In_opt_ PPYTRACEFUNC PythonTraceFunction,
    _In_opt_ PVOID UserData
    )
{
    PTRACE_STORE TraceStore;
    PTRACE_STORES TraceStores;

    if (!Context) {
        if (SizeOfContext) {
            *SizeOfContext = sizeof(*Context);
        }
        return FALSE;
    }

    if (!SizeOfContext) {
        return FALSE;
    }

    if (*SizeOfContext < sizeof(*Context)) {
        *SizeOfContext = sizeof(*Context);
        return FALSE;
    }

    if (!Python) {
        return FALSE;
    };

    if (!Rtl) {
        return FALSE;
    }

    SecureZeroMemory(Context, sizeof(*Context));

    Context->Size = *SizeOfContext;
    Context->Rtl = Rtl;
    Context->Python = Python;
    Context->TraceContext = TraceContext;
    Context->PythonTraceFunction = PythonTraceFunction;
    Context->UserData = UserData;

    if (!Context->PythonTraceFunction) {
        Context->PythonTraceFunction = (PPYTRACEFUNC)PyTraceCallback;
    }

    Context->SkipFrames = 1;

    TraceStores = TraceContext->TraceStores;
    TraceStore = &TraceStores->Stores[TRACE_STORE_FUNCTIONS_INDEX];

    Python->InitializePythonRuntimeTables(
        Python,
        TraceStoreAllocationRoutine,
        TraceStore,
        TraceStoreFreeRoutine,
        TraceStore
    );

    Context->FirstFunction = NULL;
    QueryPerformanceFrequency(&Context->Frequency);

    Rtl->PfxInitialize(&Context->ModuleFilterTable);

    InitializeListHead(&Context->Functions);

    return TRUE;
}

BOOL
StartTracing(
    _In_    PPYTHON_TRACE_CONTEXT   PythonTraceContext
)
{
    PPYTHON Python;
    PPYTRACEFUNC PythonTraceFunction;

    if (!PythonTraceContext) {
        return FALSE;
    }

    Python = PythonTraceContext->Python;

    if (!Python) {
        return FALSE;
    }

    PythonTraceFunction = PythonTraceContext->PythonTraceFunction;

    if (!PythonTraceFunction) {
        return FALSE;
    }

    Python->PyEval_SetTrace(
        PythonTraceFunction,
        (PPYOBJECT)PythonTraceContext
    );

    return TRUE;
}

BOOL
StopTracing(
    _In_    PPYTHON_TRACE_CONTEXT   PythonTraceContext
)
{
    PPYTHON Python;

    if (!PythonTraceContext) {
        return FALSE;
    }

    Python = PythonTraceContext->Python;

    if (!Python) {
        return FALSE;
    }

    Python->PyEval_SetTrace(NULL, NULL);

    return TRUE;
}

BOOL
StartProfiling(
    _In_    PPYTHON_TRACE_CONTEXT   PythonTraceContext
)
{
    PPYTHON Python;
    PPYTRACEFUNC PythonTraceFunction;

    if (!PythonTraceContext) {
        return FALSE;
    }

    Python = PythonTraceContext->Python;

    if (!Python) {
        return FALSE;
    }

    PythonTraceFunction = PythonTraceContext->PythonTraceFunction;

    if (!PythonTraceFunction) {
        return FALSE;
    }

    Python->PyEval_SetProfile(PythonTraceFunction, (PPYOBJECT)PythonTraceContext);

    return TRUE;
}

BOOL
StopProfiling(
    _In_    PPYTHON_TRACE_CONTEXT   PythonTraceContext
)
{
    PPYTHON Python;

    if (!PythonTraceContext) {
        return FALSE;
    }

    Python = PythonTraceContext->Python;

    if (!Python) {
        return FALSE;
    }

    Python->PyEval_SetProfile(NULL, NULL);

    return TRUE;
}

BOOL
AddFunction(
    _In_    PPYTHON_TRACE_CONTEXT   PythonTraceContext,
    _In_    PVOID                   FunctionObject
)
{
    if (!PythonTraceContext) {
        return FALSE;
    }

    if (!FunctionObject) {
        return FALSE;
    }

    //PythonTraceContext->FunctionObject = (PPYFUNCTIONOBJECT)FunctionObject;

    return TRUE;
}

#ifdef __cplusplus
} // extern "C"
#endif

// vim:set ts=8 sw=4 sts=4 tw=80 expandtab                                     :
