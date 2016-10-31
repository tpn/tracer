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
        (e.g. non-metadata).

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

        //
        // Redirect to the readonly version.
        //

        return RegisterNewReadonlyTraceStoreAddressRange(TraceStore,
                                                         AddressRange);
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

_Use_decl_annotations_
BOOL
RegisterNewReadonlyTraceStoreAddressRange(
    PTRACE_STORE TraceStore,
    PTRACE_STORE_ADDRESS_RANGE AddressRange
    )
/*++

Routine Description:

    This routine is called during readonly trace store memory map preparation
    if the preferred base address is unavailable.

Arguments:

    TraceStore - Supplies a pointer to a TRACE_STORE structure for which the
        next address range is to be loaded.  This must be a normal trace store
        (e.g. non-metadata) that is readonly.

    AddressRange - Supplies a pointer to a TRACE_STORE_ADDRESS_RANGE structure
        that will be used to initialized the newly allocated address range.

Return Value:

    TRUE on success, FALSE on failure.

--*/
{
    ULONGLONG Count;
    PTRACE_STORE_ADDRESS_RANGE NewAddressRange;

    //
    // Validate arguments.
    //

    if (TraceStore->IsMetadata) {
        return FALSE;
    }

    if (!TraceStore->IsReadonly) {
        return FALSE;
    }

    Count = InterlockedIncrement64(&TraceStore->ReadonlyAddressRangesConsumed);
    if (Count > TraceStore->NumberOfAddressRanges.QuadPart) {
        __debugbreak();
        return FALSE;
    }

    NewAddressRange = &TraceStore->ReadonlyAddressRanges[Count-1];
    if (!CopyTraceStoreAddressRange(NewAddressRange, AddressRange)) {
        InterlockedDecrement64(&TraceStore->ReadonlyAddressRangesConsumed);
        return FALSE;
    }

    TraceStore->AddressRange = NewAddressRange;

    return TRUE;
}

// vim:set ts=8 sw=4 sts=4 tw=80 expandtab                                     :
