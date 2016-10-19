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
ValidateTraceStoreTraitsInvariants(
    TRACE_STORE_TRAITS Traits
    )
/*++

Routine Description:

    This routine validates the trace store traits invariants.

Arguments:

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
    from the TraceStoreTraits[] array.

    N.B.: The description above is how the routine *should* work.  How it
          currently works is less sophisticated: it always obtains the traits
          from the TraceStoreTraits[] array and updates TraceStore->pTraits
          accordingly.

          The split between initialization and binding to a context needs to
          be refactored before the logic can above can be implemented cleanly.

Arguments:

    TraceStore - Supplies a pointer to a TRACE_STORE structure.

Return Value:

    TRUE on success, FALSE on failure.

--*/
{
    TRACE_STORE_TRAITS Traits;
    PTRACE_STORE_TRAITS pTraits;

    //
    // Validate arguments.
    //

    if (!ARGUMENT_PRESENT(TraceStore)) {
        return FALSE;
    }

    //
    // Load the traits from the relevant location.
    //

    if (TraceStore->IsMetadata) {
        TRACE_STORE_METADATA_ID MetadataId = TraceStore->TraceStoreMetadataId;
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

    if (!ValidateTraceStoreTraitsInvariants(Traits)) {
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
        if (!Traits.StreamingRead) {
            TraceStore->NoRetire = TRUE;
        }
    } else {
        if (!Traits.StreamingWrite) {
            TraceStore->NoRetire = TRUE;
        }
    }

    if (!Traits.MultipleRecords) {

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
    structure of the metadata Info store.  The trace store must not be set to
    readonly.

Arguments:

    TraceStore - Supplies a pointer to a TRACE_STORE structure that is not
        readonly.

Return Value:

    TRUE on success, FALSE on failure.  Failure will be because the trace
    store is marked readonly or the TraceStore was invalid.

--*/
{
    USHORT Index;

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
    // Get the array index for this trace store, then load the traits from
    // the static array.
    //

    Index = TraceStoreIdToArrayIndex(TraceStore->TraceStoreId);
    TraceStore->pTraits = (PTRACE_STORE_TRAITS)&TraceStoreTraits[Index];

    return TRUE;
}

_Use_decl_annotations_
BOOL
LoadTraceStoreTraits(
    PTRACE_STORE TraceStore
    )
/*++

Routine Description:

    This routine loads trace store traits from a trace store's :info metadata.

Arguments:

    TraceStore - Supplies a pointer to a TRACE_STORE structure that is marked
        as readonly.

Return Value:

    TRUE on success, FALSE on failure.  Failure will be because the trace
    store is not marked as readonly or the TraceStore was invalid.

--*/
{
    USHORT Index;

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
    // Compatibility shim: if the size of the TRACE_STORE_INFO structure is
    // 128, it will not contain the TRACE_STORE_TRAITS field.  In this case,
    // just load the traits from the TraceStoreTraits array.
    //

    if (sizeof(TRACE_STORE_INFO) == 128) {
        Index = TraceStoreIdToArrayIndex(TraceStore->TraceStoreId);
        TraceStore->pTraits = (PTRACE_STORE_TRAITS)&TraceStoreTraits[Index];
        *TraceStore->Traits = *TraceStore->pTraits;
    }

    //
    // Get the array index for this trace store, then load the traits from
    // the static array.
    //

    return TRUE;
}


// vim:set ts=8 sw=4 sts=4 tw=80 expandtab                                     :
