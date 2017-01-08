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
        that will be used to initialize the newly allocated address range.

Return Value:

    TRUE on success, FALSE on failure.

--*/
{
    PVOID BaseAddress;
    PTRACE_STORE AddressRangeStore;
    PTRACE_STORE_ADDRESS_RANGE NewAddressRange;
    PTRACE_CONTEXT TraceContext;
    LARGE_INTEGER Timestamp;
    ULONG_PTR NumberOfRecords = 1;
    ULONG_PTR RecordSize = sizeof(TRACE_STORE_ADDRESS_RANGE);

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
        NumberOfRecords,
        RecordSize
    );

    if (!BaseAddress) {
        return FALSE;
    }

    NewAddressRange = (PTRACE_STORE_ADDRESS_RANGE)BaseAddress;

    //
    // Query the performance counter.  This becomes our ValidFrom time, and,
    // if there was a previous address range, its ValidTo time.
    //

    QueryPerformanceCounter(&Timestamp);
    AddressRange->Timestamp.ValidFrom.QuadPart = Timestamp.QuadPart;

    TRY_MAPPED_MEMORY_OP {

        //
        // Copy the caller's address range structure over.
        //

        __movsq((PDWORD64)NewAddressRange,
                (PDWORD64)AddressRange,
                sizeof(*NewAddressRange) >> 3);

        //
        // If there's an existing address range set, update its ValidTo
        // timestamp.
        //

        if (TraceStore->AddressRange) {
            TraceStore->AddressRange->Timestamp.ValidTo.QuadPart = (
                Timestamp.QuadPart
            );
        }

        //
        // Update the trace store's address range pointer.
        //

        TraceStore->AddressRange = NewAddressRange;

    } CATCH_STATUS_IN_PAGE_ERROR {

        //
        // We'll leak the address range we just allocated here, but a copy
        // failure is indicative of much bigger issues (drive full, network
        // map disappearing) than leaking ~32 bytes, so we don't attempt to
        // roll back the allocation.
        //

        return FALSE;
    }


    return TRUE;
}

_Use_decl_annotations_
BOOL
RegisterNewReadonlyTraceStoreAddressRange(
    PTRACE_STORE TraceStore,
    PTRACE_STORE_ADDRESS_RANGE AddressRange,
    PTRACE_STORE_MEMORY_MAP MemoryMap
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

    MemoryMap - Supplies a pointer to the memory map associated with this
        address range.

Return Value:

    TRUE on success, FALSE on failure.

--*/
{
    PTRACE_STORE_ADDRESS_RANGE OriginalAddressRange;
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

    NewAddressRange = TraceStoreReadonlyAddressRangeFromMemoryMap(TraceStore,
                                                                  MemoryMap);

    if (!NewAddressRange) {
        return FALSE;
    }

    //
    // Save a copy of the original address range.
    //

    OriginalAddressRange = NewAddressRange->OriginalAddressRange;

    if (!CopyTraceStoreAddressRange(NewAddressRange, AddressRange)) {
        return FALSE;
    }

    //
    // Copy the original valid from and valid to timestamps back.
    //

    NewAddressRange->Timestamp.ValidFrom.QuadPart = (
        OriginalAddressRange->Timestamp.ValidFrom.QuadPart
    );

    NewAddressRange->Timestamp.ValidTo.QuadPart = (
        OriginalAddressRange->Timestamp.ValidTo.QuadPart
    );

    return TRUE;
}

// vim:set ts=8 sw=4 sts=4 tw=80 expandtab                                     :
