/*++

Copyright (c) 2016 Trent Nelson <trent@trent.me>

Module Name:

    PyTraceEvent2.c

Abstract:

    This module implements the tracing callback PyTraceEvent2, which uses the
    the PYTHON_TRACE_EVENT2 structure.

--*/

#include "stdafx.h"

#ifdef __cplusplus
extern "C" {
#endif

_Use_decl_annotations_
PPYTHON_TRACE_EVENT2
AllocatePythonTraceEvent2(
    PTRACE_STORE TraceStore,
    PLARGE_INTEGER Timestamp
    )
{
    ULARGE_INTEGER NumberOfRecords = { 1 };
    ULARGE_INTEGER RecordSize = { sizeof(PYTHON_TRACE_EVENT2) };

    return (PPYTHON_TRACE_EVENT2)(
        TraceStore->AllocateRecordsWithTimestamp(
            TraceStore->TraceContext,
            TraceStore,
            &RecordSize,
            &NumberOfRecords,
            Timestamp
        )
    );
}

_Check_return_
_Success_(return != 0)
PPYTHON_EVENT_TRAITS_EX
AllocatePythonEventTraitsEx(
    _In_ PTRACE_STORE TraceStore
    )
{
    ULARGE_INTEGER NumberOfRecords = { 1 };
    ULARGE_INTEGER RecordSize = { sizeof(PYTHON_EVENT_TRAITS_EX) };

    return (PPYTHON_EVENT_TRAITS_EX)(
        TraceStore->AllocateRecordsWithTimestamp(
            TraceStore->TraceContext,
            TraceStore,
            &RecordSize,
            &NumberOfRecords,
            NULL
        )
    );
}

_Use_decl_annotations_
BOOL
PyTraceEvent2(
    PPYTHON_TRACE_CONTEXT   Context,
    PPYFRAMEOBJECT          FrameObject,
    LONG                    EventType,
    PPYOBJECT               ArgObject
    )
{
    BOOL Success;

    PYTHON_TRACE_CONTEXT_FLAGS Flags;
    PYTHON_EVENT_TRAITS EventTraits;
    PYTHON_EVENT_TRAITS_EX EventTraitsEx;
    PPYTHON_EVENT_TRAITS_EX EventTraitsExPointer;

    PRTL Rtl;
    PPYTHON Python;
    PTRACE_CONTEXT TraceContext;
    PTRACE_STORES TraceStores;
    PTRACE_STORE EventStore;
    PTRACE_STORE EventTraitsExStore;
    PPYTHON_TRACE_EVENT2 Event;
    PPYTHON_FUNCTION Function = NULL;
    LARGE_INTEGER Elapsed;
    LARGE_INTEGER Timestamp;

    if (!InitializePythonTraceEvent(Context, EventType, &EventTraits)) {
        return TRUE;
    }

    Flags = Context->Flags;

    //
    // Attempt to register the frame and get the underlying function object.
    //

    Rtl = Context->Rtl;
    Python = Context->Python;

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
        return 0;
    }

    if (!Function->PathEntry.IsValid) {

        //
        // The function's path entry should always be valid if RegisterFrame()
        // succeeded.
        //

        __debugbreak();
        return FALSE;
    }

    if (!Function->PathEntry.FullName.Length) {
        __debugbreak();
    }

    if (Function->PathEntry.FullName.Buffer[0] == '\0') {
        __debugbreak();
    }

    if (Context->Depth > Function->MaxCallStackDepth) {
        Function->MaxCallStackDepth = (ULONG)Context->Depth;
    }

    //
    // We obtained the PYTHON_FUNCTION for this frame, check to see if it's
    // of interest to this tracing session.
    //

    if (!IsFunctionOfInterest(Rtl, Context, Function)) {

        //
        // Function isn't of interest (i.e. doesn't reside in a module we're
        // tracing), so return.
        //

        return TRUE;
    }

    //
    // The function resides in a module (or submodule) we're tracing, continue.
    //


    //
    // Load the events trace store.
    //

    TraceContext = Context->TraceContext;
    TraceStores = TraceContext->TraceStores;
    EventStore = &TraceStores->Stores[TraceStoreEventIndex];
    EventTraitsExStore = &TraceStores->Stores[TraceStoreEventTraitsExIndex];

    //
    // Save the timestamp for this event.
    //

    TraceContextQueryPerformanceCounter(TraceContext, &Elapsed, &Timestamp);

    //
    // Allocate an event.
    //

    Event = AllocatePythonTraceEvent2(EventStore, &Timestamp);

    if (!Event) {
        return FALSE;
    }

    //
    // Allocate a traits event.
    //

    EventTraitsExPointer = AllocatePythonEventTraitsEx(EventTraitsExStore);

    if (!EventTraitsExPointer) {
        return FALSE;
    }

    //
    // Fill out the event and traits.
    //

    Event->Function = Function;

    EventTraitsEx.AsLong = (ULONG)EventTraits.AsByte;
    EventTraitsEx.LineNumberOrCallStackDepth = (ULONG)(
        EventTraits.IsLine ?
            Python->PyFrame_GetLineNumber(FrameObject) :
            Context->Depth
    );

    EventTraitsExPointer->AsLong = EventTraitsEx.AsLong;

    //InitializeListHead(&Event->ListEntry);
    //AppendTailList(&Function->ListEntry, &Event->ListEntry);

    return TRUE;
}

#ifdef __cplusplus
}; // extern "C"
#endif

// vim:set ts=8 sw=4 sts=4 tw=80 expandtab                                     :
