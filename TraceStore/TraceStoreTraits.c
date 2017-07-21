/*++

Copyright (c) 2016 Trent Nelson <trent@trent.me>

Module Name:

    TraceStoreTraits.c

Abstract:

    This module implements functionality related to trace store traits.

--*/

#include "stdafx.h"

_Use_decl_annotations_
BOOL
ValidateTraceStoreTraits(
    PTRACE_STORE TraceStore,
    TRACE_STORE_TRAITS Traits
    )
/*++

Routine Description:

    This routine validates the trace store traits invariants.

Arguments:

    TraceStore - Supplies a pointer to a TRACE_STORE structure for which the
        traits are to be validated.

    Traits - Supplies a TRAITS_STORE_TRAITS value to validate.

Return Value:

    TRUE on success, FALSE on failure.

--*/
{
    if (Traits.VaryingRecordSize) {
        if (!AssertTrue("MultipleRecords", Traits.MultipleRecords)) {
            return FALSE;
        }
    }

    if (!Traits.MultipleRecords) {
        if (!AssertFalse("StreamingWrite", Traits.StreamingWrite)) {
            return FALSE;
        }
        if (!AssertFalse("StreamingRead", Traits.StreamingRead)) {
            return FALSE;
        }
    }

    if (Traits.FrequentAllocations) {
        if (!AssertTrue("MultipleRecords", Traits.MultipleRecords)) {
            return FALSE;
        }
        // if (!AssertTrue("StreamingWrite", Traits.StreamingWrite)) {
        //    return FALSE;
        //}
    }

    if (Traits.LinkedStore) {
        if (!AssertTrue("BlockingAllocations", Traits.BlockingAllocations)) {
            return FALSE;
        }
    }

    if (Traits.CoalesceAllocations) {
        if (!AssertTrue("MultipleRecords", Traits.MultipleRecords)) {
            return FALSE;
        }
        if (!AssertFalse("TraceStore->IsMetadata", TraceStore->IsMetadata)) {
            return FALSE;
        }
    }

    if (Traits.ConcurrentAllocations) {
        if (!AssertTrue("MultipleRecords", Traits.MultipleRecords)) {
            return FALSE;
        }
        if (!AssertFalse("TraceStore->IsMetadata", TraceStore->IsMetadata)) {
            return FALSE;
        }
    }

    if (Traits.AllowPageSpill) {
        if (!AssertTrue("MultipleRecords", Traits.MultipleRecords)) {
            return FALSE;
        }
        if (!AssertFalse("RecordSizeIsAlwaysPowerOf2",
                         Traits.RecordSizeIsAlwaysPowerOf2)) {
            return FALSE;
        }
        if (!AssertFalse("TraceStore->IsMetadata", TraceStore->IsMetadata)) {
            return FALSE;
        }
    }

    if (Traits.PageAligned) {
        if (!AssertFalse("AllowPageSpill", Traits.AllowPageSpill)) {
            return FALSE;
        }
        if (!AssertTrue("MultipleRecords", Traits.MultipleRecords)) {
            return FALSE;
        }
        if (!AssertFalse("TraceStore->IsMetadata", TraceStore->IsMetadata)) {
            return FALSE;
        }
    }

    if (Traits.Periodic) {
        if (!AssertTrue("MultipleRecords", Traits.MultipleRecords)) {
            return FALSE;
        }
    }

    if (Traits.ConcurrentDataStructure) {
        if (!AssertFalse("TraceStore->IsMetadata", TraceStore->IsMetadata)) {
            return FALSE;
        }
    }

    if (Traits.NoAllocationAlignment) {
        if (!AssertTrue("AllowPageSpill", Traits.AllowPageSpill)) {
            return FALSE;
        }
        if (!AssertTrue("MultipleRecords", Traits.MultipleRecords)) {
            return FALSE;
        }
        if (!AssertFalse("RecordSizeIsAlwaysPowerOf2",
                         Traits.RecordSizeIsAlwaysPowerOf2)) {
            return FALSE;
        }
        if (!AssertFalse("TraceStore->IsMetadata", TraceStore->IsMetadata)) {
            return FALSE;
        }
    }

    return TRUE;
}

_Use_decl_annotations_
BOOL
InitializeTraceStoreTraits(
    PTRACE_STORE TraceStore
    )
/*++

Routine Description:

    This routine initializes trace store traits for a trace store.  If the
    store is readonly, the traits will be read from the TRACE_STORE_TRAITS
    structure stored in the info metadata store via the LoadTraceStoreTraits()
    routine.  If the trace store is not readonly, the traits will be obtained
    from the TraceStore->InitialTraits field if this is a non-metadata store,
    otherwise they will be obtained from the TraceStoreMetadataIdToTraits array.

Arguments:

    TraceStore - Supplies a pointer to a TRACE_STORE structure.

Return Value:

    TRUE on success, FALSE on failure.

--*/
{
    TRACE_STORE_TRAITS Traits;
    TRACE_STORE_TRAITS InitialTraits;
    TRACE_STORE_METADATA_ID MetadataId;
    PCTRACE_STORE_TRAITS pTraits;

    //
    // Validate arguments.
    //

    if (!ARGUMENT_PRESENT(TraceStore)) {
        return FALSE;
    }

    InitialTraits = TraceStore->InitialTraits;

    //
    // Load the traits from the relevant location.
    //

    if (TraceStore->IsMetadata) {
        MetadataId = TraceStore->TraceStoreMetadataId;
        pTraits = TraceStoreMetadataIdToTraits(MetadataId);
    } else {
        USHORT Index = TraceStoreIdToArrayIndex(TraceStore->TraceStoreId);
        pTraits = (PTRACE_STORE_TRAITS)&TraceStoreTraits[Index];
    }

    if (!pTraits) {
        return FALSE;
    }

    //
    // Dereference the pointer and take a local copy of the traits, which is
    // nicer to work with.  (Also, force any traps related to dodgy pointers
    // sooner rather than later.)
    //

    Traits = *pTraits;

    //
    // Validate the trait invariants.
    //

    if (!ValidateTraceStoreTraits(TraceStore, Traits)) {
        return FALSE;
    }

    //
    // Update the trace store's pTraits pointer.
    //

    TraceStore->pTraits = pTraits;

    //
    // Make sure the no retire flag matches the streaming trait.
    //

    if (TraceStore->IsReadonly) {
        if (!IsStreamingRead(Traits)) {
            TraceStore->NoRetire = TRUE;
        }
    } else {
        if (!IsStreamingWrite(Traits)) {
            TraceStore->NoRetire = TRUE;
        }
    }

    if (IsSingleRecord(Traits)) {

        //
        // Single record stores don't participate in pre-faulting.
        //

        TraceStore->NoPrefaulting = TRUE;
        TraceStore->NoTruncate = TRUE;
    }

    return TRUE;
}

_Use_decl_annotations_
BOOL
SaveTraceStoreTraits(
    PTRACE_STORE TraceStore
    )
/*++

Routine Description:

    This routine saves the traits of a trace store into the TRACE_STORE_TRAITS
    structure of the metadata :Info store.  The trace store must not be set to
    readonly.  It is called by the :Info metadata store's bind complete routine.

Arguments:

    TraceStore - Supplies a pointer to a TRACE_STORE structure that is not
        readonly.

Return Value:

    TRUE on success, FALSE on failure.  Failure will be because the trace
    store is marked readonly or the TraceStore was invalid.

--*/
{
    USHORT Index;
    PTRACE_STORE_TRAITS pTraits;

    //
    // Validate arguments.
    //

    if (!ARGUMENT_PRESENT(TraceStore)) {
        return FALSE;
    }

    if (TraceStore->IsReadonly) {
        return FALSE;
    }

    //
    // The TraceStore->Traits and pTraits fields should be set for us by this
    // point.  Verify that now.
    //

    if (!TraceStore->pTraits) {
        return FALSE;
    }

    if (!TraceStore->Traits) {
        return FALSE;
    }

    //
    // Get the array index for this trace store, then load the traits from
    // the static array.  Compare these traits to the ones we're saving and
    // break if they differ.
    //

    Index = TraceStoreIdToArrayIndex(TraceStore->TraceStoreId);
    pTraits = (PTRACE_STORE_TRAITS)&TraceStoreTraits[Index];

    if (*((PULONG)pTraits) != *((PULONG)TraceStore->pTraits)) {
        __debugbreak();
        return FALSE;
    }

    //
    // Save the traits.  We can just write to the underlying pointer; we don't
    // need to go through InfoStore->AllocateRecords() as the backing structure
    // is "single instance" and allocated up-front.
    //

    *TraceStore->Traits = *TraceStore->pTraits;

    return TRUE;
}

_Use_decl_annotations_
BOOL
LoadTraceStoreTraits(
    PTRACE_STORE TraceStore
    )
/*++

Routine Description:

    This routine loads trace store traits from a trace store's :Info metadata.
    It is called by the :Info metadata store's bind complete routine, and is
    only used for readonly stores.

Arguments:

    TraceStore - Supplies a pointer to a TRACE_STORE structure that is marked
        as readonly.

Return Value:

    TRUE on success, FALSE on failure.  Failure will be because the trace
    store is not marked as readonly or the TraceStore was invalid.

--*/
{
    USHORT Index;
    PTRACE_STORE_TRAITS pTraits;

    //
    // Validate arguments.
    //

    if (!ARGUMENT_PRESENT(TraceStore)) {
        return FALSE;
    }

    if (!TraceStore->IsReadonly) {
        return FALSE;
    }

    //
    // TraceStore->Traits should be non-NULL here.
    //

    if (!TraceStore->Traits) {
        __debugbreak();
        return FALSE;
    }

    //
    // Get the array index for this trace store, then load the traits from
    // the static array.  Compare these traits to the ones we're loading and
    // break if they differ.  (This test will eventually go away as we want
    // the reader to be independent.)
    //

    Index = TraceStoreIdToArrayIndex(TraceStore->TraceStoreId);
    pTraits = (PTRACE_STORE_TRAITS)&TraceStoreTraits[Index];

    if (*((PULONG)pTraits) != *((PULONG)TraceStore->Traits)) {
        __debugbreak();
        return FALSE;
    }

    //
    // Load the traits.  In this case, loading simply means pointing pTraits
    // at TraceStore->Traits.
    //

    TraceStore->pTraits = TraceStore->Traits;

    return TRUE;
}


// vim:set ts=8 sw=4 sts=4 tw=80 expandtab                                     :
