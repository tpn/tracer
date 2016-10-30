/*++

Copyright (c) 2016 Trent Nelson <trent@trent.me>

Module Name:

    TraceStoreAddressRange.c

Abstract:

    This module implements functionality related to the trace store address
    range structure.

--*/

#include "stdafx.h"

_Use_decl_annotations_
BOOL
RegisterNewTraceStoreAddressRange(
    PTRACE_STORE TraceStore,
    PTRACE_STORE_ADDRESS_RANGE AddressRange
    )
/*++

Routine Description:

    This routine is called during trace store memory map preparation if the
    preferred base address is unavailable.  It allocates a new address range
    structure from the :AddressRange metadata store, copies the details in
    the address range details into it, and then updates the trace store's
    AddressRange pointer to point at it.

Arguments:

    TraceStore - Supplies a pointer to a TRACE_STORE structure for which the
        next address range is to be loaded.  This must be a normal trace store
        (e.g. non-metadata) that is not readonly.

    AddressRange - Supplies a pointer to a TRACE_STORE_ADDRESS_RANGE structure
        that will be used to initialized the newly allocated address range.

Return Value:

    TRUE on success, FALSE on failure.

--*/
{
    PVOID BaseAddress;
    PTRACE_STORE AddressRangeStore;
    PTRACE_STORE_ADDRESS_RANGE NewAddressRange;
    PTRACE_CONTEXT TraceContext;
    ULARGE_INTEGER RecordSize = { 1 };
    ULARGE_INTEGER AllocationSize = { sizeof(TRACE_STORE_ADDRESS_RANGE) };

    //
    // Validate arguments.
    //

    if (TraceStore->IsMetadata) {
        return FALSE;
    }

    if (TraceStore->IsReadonly) {
        return FALSE;
    }

    //
    // Initialize aliases.
    //

    TraceContext = TraceStore->TraceContext;
    AddressRangeStore = TraceStore->AddressRangeStore;

    //
    // Attempt to allocate a new address range structure.
    //

    BaseAddress = AddressRangeStore->AllocateRecords(
        TraceContext,
        AddressRangeStore,
        &AllocationSize,
        &RecordSize
    );

    if (!BaseAddress) {
        return FALSE;
    }

    NewAddressRange = (PTRACE_STORE_ADDRESS_RANGE)BaseAddress;

    //
    // Copy the caller's address range structure over.
    //

    if (!CopyTraceStoreAddressRange(NewAddressRange, AddressRange)) {

        //
        // We'll leak the address range we just allocated here, but a copy
        // failure is indicative of much bigger issues (drive full, network
        // map disappearing) than leaking ~32 bytes, so we don't attempt to
        // roll back the allocation.
        //

        return FALSE;
    }

    //
    // Update the trace store's address range pointer and return success.
    //

    TraceStore->AddressRange = NewAddressRange;

    return TRUE;
}

// vim:set ts=8 sw=4 sts=4 tw=80 expandtab                                     :
