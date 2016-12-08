/*++

Copyright (c) 2016 Trent Nelson <trent@trent.me>

Module Name:

    PythonTracerCallbackWorkers.c

Abstract:

    This module contains routines that are set as the Python C function
    callbacks via PyEval_SetTrace() and PyEval_SetProfile().  They are
    responsible for establishing the initial trace context and then calling
    the specific event handler (e.g. PyTraceEvent2()).

    New routines are introduced by simply adding a new PyTraceCallbackWorker[N]
    function versus changing a single implementation -- this affords maximum
    flexibility for experimenting with new tracing approaches without losing
    past "working" functionality.

--*/

#include "stdafx.h"

//
// Change this definition to alter the callback function (until we have
// something more sophisticated in place; ideally reading from the registry).
//

PPY_TRACE_CALLBACK PyTraceCallbackWorker = PyTraceCallbackWorker1;

_Use_decl_annotations_
BOOL
PyTraceCallbackWorker1(
    PPYTHON_TRACE_CONTEXT Context,
    PPYFRAMEOBJECT FrameObject,
    LONG EventType,
    PPYOBJECT ArgObject
    )
/*++

Routine Description:

    This routine calls RegisterFrame() and IsFunctionOfInterest() on every
    invocation, regardless of event type.

Arguments:

    Context - Supplies a pointer to a PYTHON_TRACE_CONTEXT structure.

    FrameObject - Supplies a pointer to a PYFRAMEOBJECT structure.

    EventType - Supplies a long value representing the event type.

    ArgObject - Supplies a pointer to a PYOBJECT that contains additional
        information in certain circumstances.

Return Value:

    TRUE on success, FALSE on failure.

--*/
{
    BOOL Success;

    PYTHON_EVENT_TRAITS EventTraits;

    PRTL Rtl;
    PPYTHON Python;
    PPYTHON_FUNCTION Function = NULL;

    //
    // Initialize the event traits.
    //

    if (!InitializePythonTraceEvent(Context, EventType, &EventTraits)) {
        return TRUE;
    }

    //
    // Initialize local aliases/variables.
    //

    Rtl = Context->Rtl;
    Python = Context->Python;

    //
    // Attempt to register the frame and get the underlying function object.
    //

    Success = Python->RegisterFrame(Python,
                                    FrameObject,
                                    EventTraits,
                                    ArgObject,
                                    &Function);

    if (!Success) {

        //
        // We can't do anything more if we weren't able to resolve the
        // function for this frame.
        //

        __debugbreak();
        return FALSE;
    }

    if (!Function->PathEntry.IsValid) {

        //
        // The function's path entry should always be valid if RegisterFrame()
        // succeeded.
        //

        __debugbreak();
        return FALSE;
    }

#ifdef _DEBUG
    if (!Function->PathEntry.FullName.Length) {
        __debugbreak();
    }

    if (Function->PathEntry.FullName.Buffer[0] == '\0') {
        __debugbreak();
    }
#endif

    if (Context->Depth > Function->MaxCallStackDepth) {
        Function->MaxCallStackDepth = (ULONG)Context->Depth;
    }

    //
    // We obtained the PYTHON_FUNCTION for this frame.
    //

    //
    // Check to see if the function is of interest to this session.
    //

    if (!IsFunctionOfInterest(Rtl, Context, Function)) {

        //
        // Function isn't of interest (i.e. doesn't reside in a module we're
        // tracing).  Set the ignore bit in this object's reference count and
        // return.
        //

        Context->FramesSkipped++;
        return TRUE;
    }

    //
    // The function resides in a module (or submodule) we're tracing, call the
    // actual trace function.
    //

    Context->FramesTraced++;

    Success = Context->TraceEventFunction(Context,
                                          Function,
                                          &EventTraits,
                                          FrameObject,
                                          ArgObject);

    if (!Success) {
        __debugbreak();
    }

    return Success;

}


_Check_return_
_Success_(return != 0)
PPYTHON_CALL_STACK_ENTRY
AllocatePythonCallStackEntry(
    _In_ PPYTHON_TRACE_CONTEXT Context,
    _In_ PLARGE_INTEGER Timestamp
    )
{
    TRACE_STORE_ID TraceStoreId;
    PTRACE_CONTEXT TraceContext;
    PTRACE_STORES TraceStores;
    PTRACE_STORE TraceStore;

    ULARGE_INTEGER NumberOfRecords = { 1 };
    ULARGE_INTEGER RecordSize = { sizeof(PYTHON_CALL_STACK_ENTRY) };

    TraceContext = Context->TraceContext;
    TraceStores = TraceContext->TraceStores;
    TraceStoreId = TraceStorePythonCallStackTableEntryId;
    TraceStore = TraceStoreIdToTraceStore(TraceStores, TraceStoreId);

    return (PPYTHON_CALL_STACK_ENTRY)(
        TraceStore->AllocateRecordsWithTimestamp(
            TraceStore->TraceContext,
            TraceStore,
            &RecordSize,
            &NumberOfRecords,
            Timestamp
        )
    );
}


_Use_decl_annotations_
BOOL
PyTraceCallbackWorker2(
    PPYTHON_TRACE_CONTEXT Context,
    PPYFRAMEOBJECT FrameObject,
    LONG EventType,
    PPYOBJECT ArgObject
    )
/*++

Routine Description:

    This routine is a work-in-progress experiment; it introduces the notion
    of call stack entries and only calling RegisterFrame() for trace events
    of type 'Call'.

Arguments:

    Context - Supplies a pointer to a PYTHON_TRACE_CONTEXT structure.

    FrameObject - Supplies a pointer to a PYFRAMEOBJECT structure.

    EventType - Supplies a long value representing the event type.

    ArgObject - Supplies a pointer to a PYOBJECT that contains additional
        information in certain circumstances.

Return Value:

    TRUE on success, FALSE on failure.

--*/
{
    BOOL Success;
    BOOL IsRecursiveCall;

    PYTHON_EVENT_TRAITS EventTraits;

    PRTL Rtl;
    PPYTHON Python;
    PPYTHON_FUNCTION Function;
    PPYTHON_CALL_STACK_ENTRY CallStackEntry;

    PTRACE_CONTEXT TraceContext;

    LARGE_INTEGER Elapsed;
    LARGE_INTEGER Timestamp;

    //
    // Initialize the event traits.
    //

    InitializeEventTraits(EventType, &EventTraits);

    if (!Context->RuntimeState.HasStarted) {

        if (!EventTraits.IsCall) {

            //
            // If we haven't started profiling/tracing yet, we can ignore any
            // event that isn't a call event.  (In practice, there will usually
            // be one return event/frame before we get a call event we're
            // interested in.)
            //

            return TRUE;
        }

        //
        // We've received our first profile/trace event of interest.  Toggle
        // our 'HasStarted' flag.
        //

        Context->RuntimeState.HasStarted = TRUE;
    }

    //
    // See if we've been configured to count the number of individual trace
    // event types, and if so, update the relevant counter.  (We can abuse the
    // fact EventTraits.AsEventType gives us a 0-based contiguous index, which
    // allows us to index directly into the array of ULONGLONG counters.)
    //

    if (Context->Flags.CountEvents) {
        ++(Context->Counters[EventTraits.AsEventType]);
    }

    //
    // Initialize local variables.
    //

    Rtl = Context->Rtl;
    Python = Context->Python;
    TraceContext = Context->TraceContext;

    Function = NULL;
    CallStackEntry = NULL;

    if (EventTraits.IsCall) {


        Context->Depth++;

        //
        // If we've been configured to track maximum reference counts, do that
        // now.
        //

        if (Context->Flags.TrackMaxRefCounts) {
            UpdateMaxRefCounts(Context);
        }

        //
        // Update our maximum depth, if applicable.
        //

        if (Context->Depth > Context->MaxDepth.QuadPart) {
            Context->MaxDepth.QuadPart = Context->Depth;
            if (Context->MaxDepth.HighPart) {
                __debugbreak();
            }
        }

        IsRecursiveCall = FALSE;

        //
        // As this is a call event, we need to create a new call stack entry.
        //

        //
        // Save the timestamp for this event.
        //

        TraceContextQueryPerformanceCounter(TraceContext, &Elapsed, &Timestamp);

        //
        // Allocate a call stack entry.
        //

        CallStackEntry = AllocatePythonCallStackEntry(Context, &Timestamp);

        if (!CallStackEntry) {
            return FALSE;
        }

        CallStackEntry->EnterTimestamp.QuadPart = Timestamp.QuadPart;
        InitializeListHead(&CallStackEntry->ListEntry);


    } else if (EventTraits.IsReturn) {

        Context->Depth--;

    }

    Success = TRUE;
    return Success;
}

// vim:set ts=8 sw=4 sts=4 tw=80 expandtab                                     :
