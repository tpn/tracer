/*++

Copyright (c) 2016 Trent Nelson <trent@trent.me>

Module Name:

    TraceStoreCallbacks.c

Abstract:

    This module contains all callbacks used by the trace store component.
    Callbacks are the targets of threadpool work submissions.

--*/

#include "stdafx.h"

_Use_decl_annotations_
VOID
CALLBACK
PrefaultFutureTraceStorePageCallback(
    PTP_CALLBACK_INSTANCE Instance,
    PVOID Context,
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
    // Ensure Context has a value.
    //

    if (!Context) {
        return;
    }

    PrefaultFutureTraceStorePage((PTRACE_STORE)Context);
}


_Use_decl_annotations_
VOID
CALLBACK
FinalizeFirstTraceStoreMemoryMapCallback(
    PTP_CALLBACK_INSTANCE Instance,
    PVOID Context,
    PTP_WORK Work
    )
/*++

Routine Description:

    This routine is the callback target for the finalize first trace store
    memory map threadpool work.

Arguments:

    Instance - Not used.

    Context - Supplies a pointer to a TRACE_STORE struct.

    Work - Not used.

Return Value:

    None.

--*/
{
    BOOL Success;
    PTRACE_STORE TraceStore;

    //
    // Ensure Context has a value.
    //

    if (!Context) {
        return;
    }

    TraceStore = (PTRACE_STORE)Context;

    Success = FinalizeFirstTraceStoreMemoryMap(TraceStore);

    if (!Success) {

        //
        // XXX TODO: set some sort of a flag/event indicating
        // failure.
        //
        __debugbreak();
    }
}

_Use_decl_annotations_
VOID
CALLBACK
BindMetadataInfoCallback(
    PTP_CALLBACK_INSTANCE Instance,
    PTRACE_CONTEXT TraceContext,
    PTP_WORK Work
    )
/*++

Routine Description:

    This routine is the callback target for the bind metadata info threadpool
    work item of a trace context.  It pops a metadata info store of the trace
    context, calls the BindMetadataInfo() method, and, if successful, submits
    threadpool work items for binding the rest of the metadata stores.

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
    PTRACE_STORE MetadataInfoStore;

    //
    // Validate arguments.
    //

    if (!ARGUMENT_PRESENT(TraceContext)) {
        return;
    }

    if (!PopBindMetadataInfoTraceStore(TraceContext, &MetadataInfoStore)) {
        goto Error;
    }

    Success = BindMetadataInfo(TraceContext, MetadataInfoStore);
    if (!Success) {
        goto Error;
    }

    //
    // xxx todo: submit binds for the remaining metadata info stores.
    //

    return;

Error:
    PushFailedTraceStore(TraceContext, MetadataInfoStore);
    return;
}

_Use_decl_annotations_
VOID
CALLBACK
LoadMetadataInfoCallback(
    PTP_CALLBACK_INSTANCE Instance,
    PVOID Context,
    PTP_WORK Work
    )
/*++

Routine Description:

    This routine is the callback target for the finalize first trace store
    memory map threadpool work.

Arguments:

    Instance - Not used.

    Context - Supplies a pointer to a TRACE_CONTEXT structure.

    Work - Not used.

Return Value:

    None.

--*/
{


}

_Use_decl_annotations_
VOID
CALLBACK
LoadRemainingMetadataCallback(
    PTP_CALLBACK_INSTANCE Instance,
    PVOID Context,
    PTP_WORK Work
    )
/*++

Routine Description:

    This routine is the callback target for the finalize first trace store
    memory map threadpool work.

Arguments:

    Instance - Not used.

    Context - Supplies a pointer to a TRACE_CONTEXT structure.

    Work - Not used.

Return Value:

    None.

--*/
{


}

_Use_decl_annotations_
VOID
CALLBACK
LoadTraceStoreCallback(
    PTP_CALLBACK_INSTANCE Instance,
    PVOID Context,
    PTP_WORK Work
    )
/*++

Routine Description:

    This routine is the callback target for the finalize first trace store
    memory map threadpool work.

Arguments:

    Instance - Not used.

    Context - Supplies a pointer to a TRACE_CONTEXT structure.

    Work - Not used.

Return Value:

    None.

--*/
{


}

// vim:set ts=8 sw=4 sts=4 tw=80 expandtab                                     :
