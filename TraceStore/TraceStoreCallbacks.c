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
        __debugbreak();
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
    ULONG MetadataBindsSubmitted;

    //
    // Validate arguments.
    //

    if (!ARGUMENT_PRESENT(TraceContext)) {
        return;
    }

    if (!PopBindMetadataInfoTraceStore(TraceContext, &MetadataInfoStore)) {
        __debugbreak();
        return;
    }

    Success = BindStore(TraceContext, MetadataInfoStore);
    if (!Success) {
        __debugbreak();
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

    MetadataBindsSubmitted = 0;

    SUBMIT_METADATA_BIND(Allocation);
    SUBMIT_METADATA_BIND(Relocation);
    SUBMIT_METADATA_BIND(Address);
    SUBMIT_METADATA_BIND(AddressRange);
    SUBMIT_METADATA_BIND(AllocationTimestamp);
    SUBMIT_METADATA_BIND(AllocationTimestampDelta);
    SUBMIT_METADATA_BIND(Synchronization);
    SUBMIT_METADATA_BIND(Info);

    //
    // Invariant check that we've submitted the expected number of metadata
    // trace store binds.  We subtract 1 to account for the fact that we've
    // just bound the :MetadataInfo store.
    //

    if (MetadataBindsSubmitted != NumberOfMetadataStores-1) {
        __debugbreak();
    }

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
    for all metadata trace stores as soon as the :MetadataInfo store has
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
        __debugbreak();
        return;
    }

    Success = BindStore(TraceContext, MetadataStore);
    if (!Success) {
        __debugbreak();
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

    if (MetadataStore->IsReadonly) {
        MetadataStore->FlatMappingLoaded = TRUE;
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
    to be bound, threadpool work will be submitted for all pending flat memory
    map requests.

Arguments:

    Instance - Not used.

    TraceContext - Supplies a pointer to a TRACE_CONTEXT structure.

    Work - Not used.

Return Value:

    None.  If an error occurs, PushFailedTraceStore() is called.

--*/
{
    BOOL Success;
    ULONG Index;
    PTRACE_STORE TraceStore;

    //
    // Validate arguments.
    //

    if (!ARGUMENT_PRESENT(TraceContext)) {
        return;
    }

    if (!PopBindTraceStore(TraceContext, &TraceStore)) {
        __debugbreak();
        return;
    }

    Success = BindStore(TraceContext, TraceStore);
    if (!Success) {
        __debugbreak();
        goto Error;
    }

    //
    // The trace store was bound successfully.  If this was the last trace
    // store to be bound, process flat memory maps.
    //

    if (InterlockedDecrement(&TraceContext->BindsInProgress) == 0) {
        goto HandleFlatMemoryMaps;
    }

    return;

HandleFlatMemoryMaps:

    //
    // Set the loading complete event at this point, as all trace stores have
    // had their initial binding completed.
    //

    SetEvent(TraceContext->LoadingCompleteEvent);

    if (TraceContext->NumberOfFlatMemoryMaps == 0) {

        //
        // If there aren't any flat memory maps, we're finished.
        //

        return;
    }

    for (Index = 0; Index < TraceContext->NumberOfFlatMemoryMaps; Index++) {
        PTP_WORK Work = TraceContext->BindFlatMemoryMapWork.ThreadpoolWork;
        SubmitThreadpoolWork(Work);
    }

    return;

Error:
    PushFailedTraceStore(TraceContext, TraceStore);
    return;
}

_Use_decl_annotations_
VOID
CALLBACK
BindFlatMemoryMapCallback(
    PTP_CALLBACK_INSTANCE Instance,
    PTRACE_CONTEXT TraceContext,
    PTP_WORK Work
    )
/*++

Routine Description:

    This routine is the callback target for the process flat memory map
    threadpool work item of a trace context.  It is submitted when all normal
    memory maps for all trace stores have been loaded.

    It pops memory maps off the trace context's process flat memory map
    interlocked list, calls the BindFlatMemoryMap() method, and, if successful,
    decrements the trace context's count of in-progress flat memory map work
    items.  If this was the last item, the loading complete event is set.

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
    PTRACE_STORE_WORK PrepareIntervalsWork;
    PTRACE_STORE_MEMORY_MAP MemoryMap;

    //
    // Validate arguments.
    //

    if (!ARGUMENT_PRESENT(TraceContext)) {
        return;
    }

    if (!PopFlatMemoryMap(TraceContext, &MemoryMap)) {
        __debugbreak();
        return;
    }

    Success = BindFlatMemoryMap(TraceContext, MemoryMap);
    if (!Success) {
        __debugbreak();
        return;
    }

    //
    // Get the owning trace store and submit interval preparation work.
    //

    TraceStore = CONTAINING_RECORD(MemoryMap,
                                   TRACE_STORE,
                                   FlatMemoryMap);

    PrepareIntervalsWork = &TraceContext->PrepareIntervalsWork;
    PushIntervalPreparation(TraceContext, TraceStore);

    return;
}

_Use_decl_annotations_
VOID
CALLBACK
PrepareIntervalsCallback(
    PTP_CALLBACK_INSTANCE Instance,
    PTRACE_CONTEXT TraceContext,
    PTP_WORK Work
    )
/*++

Routine Description:

    This routine is the threadpool callback target for a trace store's prepare
    intervals routine.

Arguments:

    Instance - Unused.

    TraceContext - Supplies a pointer to a TRACE_CONTEXT structure.

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
        __debugbreak();
        return;
    }

    if (!PopIntervalPreparation(TraceContext, &TraceStore)) {
        __debugbreak();
        return;
    }

    Success = PrepareTraceStoreIntervals(TraceStore);
    if (!Success) {
        NOTHING;
    }

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
        __debugbreak();
        return;
    }

    if (!PopTraceStoreMemoryMap(&TraceStore->PrepareMemoryMaps, &MemoryMap)) {
        __debugbreak();
        return;
    }

    Success = PrepareNextTraceStoreMemoryMap(TraceStore, MemoryMap);
    if (Success) {
        PushTraceStoreMemoryMap(&TraceStore->NextMemoryMaps, MemoryMap);
        SetEvent(TraceStore->NextMemoryMapAvailableEvent);
    } else {
        __debugbreak();
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
    ULONG volatile *TraceStoreCounter;
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
        __debugbreak();
        return;
    }

    TraceContext = TraceStore->TraceContext;

    Success = PrepareReadonlyTraceStoreMemoryMap(TraceStore, MemoryMap);
    if (!Success) {
        __debugbreak();
        goto Error;
    }

    TraceStoreCounter = &TraceStore->PrepareReadonlyNonStreamingMapsInProgress;
    if (InterlockedDecrement(TraceStoreCounter) > 0) {
        return;
    }

    //
    // This was the last memory map to be prepared for the trace store.
    //

    Success = PrepareNonStreamingReadonlyTraceStoreMapsComplete(TraceContext,
                                                                TraceStore);
    if (Success) {
        return;
    }

    //
    // Intentional follow-on to Error.
    //

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
        __debugbreak();
        return;
    }

    Success = BindNonStreamingReadonlyTraceStoreComplete(TraceContext,
                                                         TraceStore);
    if (!Success) {
        __debugbreak();
        goto Error;
    }

    return;

Error:
    PushFailedTraceStore(TraceContext, TraceStore);
    return;
}

_Use_decl_annotations_
VOID
CALLBACK
NewModuleEntryCallback(
    PTP_CALLBACK_INSTANCE Instance,
    PTRACE_CONTEXT TraceContext,
    PTP_WORK Work
    )
/*++

Routine Description:

    This routine is the callback target for the NewModuleEntry threadpool work
    item of a trace context.  It is submitted when a TRACE_MODULE_TABLE_ENTRY
    structure is created.

Arguments:

    Instance - Not used.

    TraceContext - Supplies a pointer to a TRACE_CONTEXT structure.

    Work - Not used.

Return Value:

    None.  If an error occurs, PushFailedTraceStore() is called.

--*/
{
    BOOL Success;
    PTRACE_MODULE_TABLE_ENTRY ModuleTableEntry;

    //
    // Validate arguments.
    //

    if (!ARGUMENT_PRESENT(TraceContext)) {
        return;
    }

    //
    // Pop a new TRACE_MODULE_TABLE_ENTRY structure off the list.
    //

    if (!PopNewModuleTableEntry(TraceContext, &ModuleTableEntry)) {
        return;
    }

    EnterNewModuleTableCallback(TraceContext);
    TRY_MAPPED_MEMORY_OP {
        Success = ProcessNewModuleTableEntry(TraceContext, ModuleTableEntry);
    } CATCH_STATUS_IN_PAGE_ERROR {
        Success = FALSE;
    }
    LeaveNewModuleTableCallback(TraceContext);

    if (!Success) {
        __debugbreak();
        NOTHING;
    }

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
        __debugbreak();
        return;
    }

    if (!PopTraceStoreMemoryMap(&TraceStore->CloseMemoryMaps, &MemoryMap)) {
        __debugbreak();
        return;
    }

    //
    // Ignore the return value, we can't do anything.
    //

    CloseTraceStoreMemoryMap(TraceStore, MemoryMap);

    return;
}

_Use_decl_annotations_
VOID
CALLBACK
CleanupThreadpoolMembersCallback(
    PTP_CALLBACK_INSTANCE Instance,
    PTRACE_CONTEXT TraceContext,
    PTP_WORK Work
    )
/*++

Routine Description:

    This routine is the cancellation threadpool callback target for cleaning up
    all threadpool group members of the normal threadpool.  If a trace store
    encounters a fatal error, it will call PushFailedTraceStore(), which will
    trigger this callback in the cancellation threadpool.

    This routine calls CloseThreadpoolCleanupGroupMembers() against the main
    threadpool, then sets the LoadingComplete event.

Arguments:

    Instance - Unused.

    TraceContext - Supplies a pointer to a TRACE_CONTEXT structure.

    Work - Unused.

Return Value:

    None.

--*/
{
    BOOL CancelPendingCallbacks = TRUE;

    //
    // Validate arguments.
    //

    if (!ARGUMENT_PRESENT(TraceContext)) {
        return;
    }

    //
    // Close all items of the main threadpool.
    //

    CloseThreadpoolCleanupGroupMembers(TraceContext->ThreadpoolCleanupGroup,
                                       CancelPendingCallbacks,
                                       NULL);

    //
    // Set the loading complete event.
    //

    SetEvent(TraceContext->LoadingCompleteEvent);

    return;
}

//
// Disable this:
//
//      warning C26135 : Missing annotation
//        _Releases_lock_(TraceContext->WorkingSetChangesLock)
//        at function 'GetWorkingSetChangesTimerCallback'.
//

#pragma warning(push)
#pragma warning(disable: 26135)

_Use_decl_annotations_
VOID
CALLBACK
GetWorkingSetChangesTimerCallback(
    PTP_CALLBACK_INSTANCE Instance,
    PTRACE_CONTEXT TraceContext,
    PTP_WORK Work
    )
/*++

Routine Description:

    This routine is the threadpool timer callback target for the get working
    set info.

Arguments:

    Instance - Unused.

    TraceContext - Supplies a pointer to a TRACE_CONTEXT structure.

    Work - Unused.

Return Value:

    None.

--*/
{
    BOOL Success;

    //
    // Validate arguments.
    //

    if (!ARGUMENT_PRESENT(TraceContext)) {
        return;
    }

    //
    // If the SRWLock is already acquired, the existing thread callback is
    // still running.  Increment the contention counter and return.
    //

    if (!TryAcquireWorkingSetChangesLock(TraceContext)) {
        InterlockedIncrement(&TraceContext->WorkingSetTimerContention);
        return;
    }

    TRY_MAPPED_MEMORY_OP {
        Success = GetWorkingSetChanges(TraceContext);
    } CATCH_STATUS_IN_PAGE_ERROR_OR_ACCESS_VIOLATION {
        Success = FALSE;
    }

    if (!Success) {
        NOTHING;
    }

    //
    // Release the lock.
    //

    ReleaseWorkingSetChangesLock(TraceContext);

    return;
}

_Use_decl_annotations_
VOID
CALLBACK
CapturePerformanceMetricsTimerCallback(
    PTP_CALLBACK_INSTANCE Instance,
    PTRACE_CONTEXT TraceContext,
    PTP_WORK Work
    )
/*++

Routine Description:

    This routine is the threadpool timer callback target for the performance
    metrics timer.

Arguments:

    Instance - Unused.

    TraceContext - Supplies a pointer to a TRACE_CONTEXT structure.

    Work - Unused.

Return Value:

    None.

--*/
{
    BOOL Success;

    //
    // Validate arguments.
    //

    if (!ARGUMENT_PRESENT(TraceContext)) {
        return;
    }

    //
    // If the SRWLock is already acquired, the existing thread callback is
    // still running.  Increment the contention counter and return.
    //

    if (!TryAcquireCapturePerformanceMetricsLock(TraceContext)) {
        InterlockedIncrement(
            &TraceContext->CapturePerformanceMetricsTimerContention
        );
        return;
    }

    TRY_MAPPED_MEMORY_OP {
        Success = CapturePerformanceMetrics(TraceContext);
    } CATCH_STATUS_IN_PAGE_ERROR_OR_ACCESS_VIOLATION {
        Success = FALSE;
    }

    if (!Success) {
        NOTHING;
    }

    //
    // Release the lock.
    //

    ReleaseCapturePerformanceMetricsLock(TraceContext);

    return;
}

//
// Restore the disabled SAL warning.
//

#pragma warning(pop)

// vim:set ts=8 sw=4 sts=4 tw=80 expandtab                                     :
