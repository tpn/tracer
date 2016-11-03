/*++

Copyright (c) 2016 Trent Nelson <trent@trent.me>

Module Name:

    TraceStoreCallbacks.c

Abstract:

    This module contains all callbacks used by the trace store component.

--*/

#include "stdafx.h"

_Use_decl_annotations_
VOID
CALLBACK
PrefaultFutureTraceStorePageCallback(
    PTP_CALLBACK_INSTANCE Instance,
    PTRACE_STORE TraceStore,
    PTP_WORK Work
    )
/*++

Routine Description:

    This routine is the callback target for the prefault future trace store
    threadpool work.  It simply calls the PrefaultFutureTraceStorePage()
    inline routine (which forces a read of the memory address we want to
    prefault, which will result in a hard or soft fault if the page isn't
    resident).

Arguments:

    Instance - Not used.

    Context - Supplies a pointer to a TRACE_STORE struct.

    Work - Not used.

Return Value:

    None.

--*/
{
    //
    // Validate arguments.
    //

    if (!ARGUMENT_PRESENT(TraceStore)) {
        return;
    }

    PrefaultFutureTraceStorePage(TraceStore);
}

_Use_decl_annotations_
VOID
CALLBACK
BindMetadataInfoStoreCallback(
    PTP_CALLBACK_INSTANCE Instance,
    PTRACE_CONTEXT TraceContext,
    PTP_WORK Work
    )
/*++

Routine Description:

    This routine is the callback target for the bind metadata info threadpool
    work item of a trace context.  It pops a metadata info store of the trace
    context, calls BindStore(), and, if successful, submits threadpool work
    items for binding the rest of the metadata stores.

Arguments:

    Instance - Not used.

    TraceContext - Supplies a pointer to a TRACE_CONTEXT structure.

    Work - Not used.

Return Value:

    None.  If any operation fails, the metadata info trace store is pushed to
    the trace context's failure list, the failure count is incremented, and
    the failed event is set.

--*/
{
    BOOL Success;
    PTRACE_STORE TraceStore;
    PTRACE_STORE MetadataInfoStore;

    //
    // Validate arguments.
    //

    if (!ARGUMENT_PRESENT(TraceContext)) {
        return;
    }

    if (!PopBindMetadataInfoTraceStore(TraceContext, &MetadataInfoStore)) {
        return;
    }

    Success = BindStore(TraceContext, MetadataInfoStore);
    if (!Success) {
        goto Error;
    }

    //
    // The binding was successful, submit binding work items for the remaining
    // metadata stores to the threadpool.
    //

    TraceStore = MetadataInfoStore->TraceStore;

    //
    // Subtract one from NumberOfMetadataStores to account for the fact that
    // we just bound MetadataInfo.
    //

    InterlockedExchange(&TraceStore->MetadataBindsInProgress,
                        NumberOfMetadataStores - 1);

    SUBMIT_METADATA_BIND(Allocation);
    SUBMIT_METADATA_BIND(Relocation);
    SUBMIT_METADATA_BIND(Address);
    SUBMIT_METADATA_BIND(AddressRange);
    SUBMIT_METADATA_BIND(Bitmap);
    SUBMIT_METADATA_BIND(Info);

    return;

Error:
    PushFailedTraceStore(TraceContext, MetadataInfoStore);
    return;
}

_Use_decl_annotations_
VOID
CALLBACK
BindRemainingMetadataStoresCallback(
    PTP_CALLBACK_INSTANCE Instance,
    PTRACE_CONTEXT TraceContext,
    PTP_WORK Work
    )
/*++

Routine Description:

    This routine is the callback target for the bind remaining metadata
    threadpool work item of a trace context.  It is submitted in parallel
    for all metadata trace stores as soon as the :metadatainfo store has
    been bound.  It pops a metadata store off the trace context, calls the
    bind method, and, if successful, decrements the main trace store's count
    of in-progress metadata bindings.  If this was the last metadata binding,
    a bind of the main trace store is submitted to the threadpool.

Arguments:

    Instance - Not used.

    TraceContext - Supplies a pointer to a TRACE_CONTEXT structure.

    Work - Not used.

Return Value:

    None.  If an error occurs, PushFailedTraceStore() is called.

--*/
{
    BOOL Success;
    PTRACE_STORE TraceStore;
    PTRACE_STORE MetadataStore;

    //
    // Validate arguments.
    //

    if (!ARGUMENT_PRESENT(TraceContext)) {
        return;
    }

    if (!PopBindRemainingMetadataTraceStore(TraceContext, &MetadataStore)) {
        return;
    }

    Success = BindStore(TraceContext, MetadataStore);
    if (!Success) {
        goto Error;
    }

    //
    // The metadata was bound successfully.  If this was the last metadata
    // store being bound, submit a threadpool work item to bind the main trace
    // store now that all the metadata stores are available.
    //

    TraceStore = MetadataStore->TraceStore;
    if (InterlockedDecrement(&TraceStore->MetadataBindsInProgress) == 0) {
        SubmitBindTraceStoreWork(TraceContext, TraceStore);
    }

    return;

Error:
    PushFailedTraceStore(TraceContext, MetadataStore);
    return;
}

_Use_decl_annotations_
VOID
CALLBACK
BindTraceStoreCallback(
    PTP_CALLBACK_INSTANCE Instance,
    PTRACE_CONTEXT TraceContext,
    PTP_WORK Work
    )
/*++

Routine Description:

    This routine is the callback target for the bind trace store threadpool
    work item of a trace context.  It is submitted when all metadata stores
    for a trace store have been bound successfully.  It pops the trace store
    off the trace context's bind trace store interlocked list, calls the
    BindStore() method, and, if successful, decrements the trace context's
    count of in-progress trace store binds.  If this was the last trace store
    to be bound, the trace context's loading complete event is set.

Arguments:

    Instance - Not used.

    TraceContext - Supplies a pointer to a TRACE_CONTEXT structure.

    Work - Not used.

Return Value:

    None.  If an error occurs, PushFailedTraceStore() is called.

--*/
{
    BOOL Success;
    PTRACE_STORE TraceStore;

    //
    // Validate arguments.
    //

    if (!ARGUMENT_PRESENT(TraceContext)) {
        return;
    }

    if (!PopBindTraceStore(TraceContext, &TraceStore)) {
        return;
    }

    Success = BindStore(TraceContext, TraceStore);
    if (!Success) {
        goto Error;
    }

    //
    // The trace store was bound successfully.  If this was the last trace
    // store to be bound, set the trace context's loading complete event.
    //

    if (InterlockedDecrement(&TraceContext->BindsInProgress) == 0) {
        SetEvent(TraceContext->LoadingCompleteEvent);
    }

    return;

Error:
    PushFailedTraceStore(TraceContext, TraceStore);
    return;
}

_Use_decl_annotations_
VOID
CALLBACK
PrepareNextTraceStoreMemoryMapCallback(
    PTP_CALLBACK_INSTANCE Instance,
    PTRACE_STORE TraceStore,
    PTP_WORK Work
    )
/*++

Routine Description:

    This routine is the threadpool callback target for a trace store's prepare
    next memory map routine.

Arguments:

    Instance - Unused.

    TraceStore - Supplies a pointer to a TRACE_STORE structure.

    Work - Unused.

Return Value:

    None.

--*/
{
    BOOL Success;
    PTRACE_STORE_MEMORY_MAP MemoryMap;

    //
    // Validate arguments.
    //

    if (!ARGUMENT_PRESENT(TraceStore)) {
        return;
    }

    if (!PopTraceStoreMemoryMap(&TraceStore->PrepareMemoryMaps, &MemoryMap)) {
        return;
    }

    Success = PrepareNextTraceStoreMemoryMap(TraceStore, MemoryMap);
    if (Success) {
        PushTraceStoreMemoryMap(&TraceStore->NextMemoryMaps, MemoryMap);
        SetEvent(TraceStore->NextMemoryMapAvailableEvent);
    } else {
        UnmapTraceStoreMemoryMap(MemoryMap);
        ReturnFreeTraceStoreMemoryMap(TraceStore, MemoryMap);
    }
    return;
}

_Use_decl_annotations_
VOID
CALLBACK
PrepareReadonlyTraceStoreMemoryMapCallback(
    PTP_CALLBACK_INSTANCE Instance,
    PTRACE_STORE TraceStore,
    PTP_WORK Work
    )
/*++

Routine Description:

    This routine is the threadpool callback target for a trace store's prepare
    next memory map routine.

Arguments:

    Instance - Unused.

    TraceStore - Supplies a pointer to a TRACE_STORE structure.

    Work - Unused.

Return Value:

    None.

--*/
{
    BOOL Success;
    PTRACE_STORE_MEMORY_MAP MemoryMap;
    PTRACE_CONTEXT TraceContext;

    //
    // Validate arguments.
    //

    if (!ARGUMENT_PRESENT(TraceStore)) {
        return;
    }

    Success = PopTraceStoreMemoryMap(
        &TraceStore->PrepareReadonlyMemoryMaps,
        &MemoryMap
    );

    if (!Success) {
        return;
    }

    TraceContext = TraceStore->TraceContext;

    Success = PrepareReadonlyTraceStoreMemoryMap(TraceStore, MemoryMap);
    if (!Success) {
        goto Error;
    }

    if (InterlockedDecrement(
        &TraceStore->PrepareReadonlyNonStreamingMapsInProgress)) {
        return;
    }

    if (!InterlockedDecrement(
        &TraceContext->PrepareReadonlyNonStreamingMapsInProgress)) {
        SetEvent(TraceContext->LoadingCompleteEvent);
    }

    return;

Error:
    PushFailedTraceStore(TraceContext, TraceStore);
    return;
}

_Use_decl_annotations_
VOID
CALLBACK
ReadonlyNonStreamingBindCompleteCallback(
    PTP_CALLBACK_INSTANCE Instance,
    PTRACE_CONTEXT TraceContext,
    PTP_WORK Work
    )
/*++

Routine Description:

    This routine is the threadpool callback target for a trace store's prepare
    next memory map routine.

Arguments:

    Instance - Unused.

    TraceStore - Supplies a pointer to a TRACE_STORE structure.

    Work - Unused.

Return Value:

    None.

--*/
{
    BOOL Success;
    PTRACE_STORE TraceStore;

    //
    // Validate arguments.
    //

    if (!ARGUMENT_PRESENT(TraceContext)) {
        return;
    }

    Success = PopReadonlyNonStreamingBindComplete(TraceContext, &TraceStore);
    if (!Success) {
        return;
    }

    Success = BindNonStreamingReadonlyTraceStoreComplete(TraceContext,
                                                         TraceStore);
    if (!Success) {
        goto Error;
    }

    /*
    if (InterlockedDecrement(
        &TraceStore->PrepareReadonlyNonStreamingMapsInProgress)) {
        return;
    }

    if (!InterlockedDecrement(
        &TraceContext->PrepareReadonlyNonStreamingMapsInProgress)) {
        SetEvent(TraceContext->LoadingCompleteEvent);
    }
    */

    return;

Error:
    PushFailedTraceStore(TraceContext, TraceStore);
    return;
}


_Use_decl_annotations_
VOID
CALLBACK
CloseTraceStoreMemoryMapCallback(
    PTP_CALLBACK_INSTANCE Instance,
    PTRACE_STORE TraceStore,
    PTP_WORK Work
    )
/*++

Routine Description:

    This routine is the threadpool callback target for a trace store's release
    memory map routine.

Arguments:

    Instance - Unused.

    TraceStore - Supplies a pointer to a TRACE_STORE structure.

    Work - Unused.

Return Value:

    None.

--*/
{
    PTRACE_STORE_MEMORY_MAP MemoryMap;

    //
    // Validate arguments.
    //

    if (!ARGUMENT_PRESENT(TraceStore)) {
        return;
    }

    if (!PopTraceStoreMemoryMap(&TraceStore->CloseMemoryMaps, &MemoryMap)) {
        return;
    }

    //
    // Ignore the return value, we can't do anything.
    //

    CloseTraceStoreMemoryMap(TraceStore, MemoryMap);

    return;
}

// vim:set ts=8 sw=4 sts=4 tw=80 expandtab                                     :
