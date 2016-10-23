/*++

Copyright (c) 2016 Trent Nelson <trent@trent.me>

Module Name:

    TraceStoreAllocation.c

Abstract:

    This module implements trace store allocation functionality.  Functions
    are provided for allocating records from a trace store, as well as
    recording allocations in the relevant allocation metadata structure.

--*/

#include "stdafx.h"

_Use_decl_annotations_
PVOID
TraceStoreAllocateRecords(
    PTRACE_CONTEXT  TraceContext,
    PTRACE_STORE    TraceStore,
    PULARGE_INTEGER RecordSize,
    PULARGE_INTEGER NumberOfRecords
    )
/*++

Routine Description:

    This routine allocates records from a trace store.  It can be considered
    the TraceStore-equivalent of a calloc()-type interface, in that the record
    size and number of records are specified rather than the total size.

    The memory to satisfy the total size is sourced from the trace store's
    active memory map.  If sufficient space is available, allocation is simply
    a matter of adjusting the relevant pointers and returning.

    If the required size can't be satisfied by the remaining memory in the
    map, this routine consumes the "next" memory map that has been prepared
    asynchronously in a threadpool if one is available.  If the prepared map
    is not yet available, the allocation fails and NULL is returned.

    N.B. This routine should be called indirectly through the TraceStore's
         AllocationRoutine function pointer.

    The total size that can be allocated is limited to the maximum size of the
    memory map.  The allocator is geared more toward lots of small allocations
    versus larger ones.

Arguments:

    TraceContext - Supplies a pointer to a TRACE_CONTEXT structure.

    TraceStore - Supplies a pointer to a TRACE_STORE structure that the memory
        is to be allocated from.

    RecordSize - Supplies a pointer to the address of a ULARGE_INTEGER that
        contains the size of the record to allocate.

    NumberOfRecords - Supplies a pointer to the address of a ULARGE_INTEGER
        that contains the number of records to allocate.  The total size is
        derived by multiplying RecordSize with NumberOfRecords.

Return Value:

    A pointer to the base memory address satisfying the total requested size
    if the memory could be obtained successfully, NULL otherwise.

--*/
{
    BOOL Success;
    PTRACE_STORE_MEMORY_MAP PrevMemoryMap;
    PTRACE_STORE_MEMORY_MAP MemoryMap;
    ULONG_PTR AllocationSize;
    PVOID ReturnAddress = NULL;
    PVOID NextAddress;
    PVOID EndAddress;
    PVOID PrevPage;
    PVOID NextPage;
    PVOID PageAfterNextPage;

    //
    // Validate arguments.
    //

    if (!ARGUMENT_PRESENT(TraceStore)) {
        return NULL;
    }

    MemoryMap = TraceStore->MemoryMap;

    if (!MemoryMap) {
        return NULL;
    }

    AllocationSize = (
        (ULONG_PTR)(
            RecordSize->QuadPart *
            NumberOfRecords->QuadPart
        )
    );

    AllocationSize = ALIGN_UP(AllocationSize, sizeof(ULONG_PTR));

    if (AllocationSize > (ULONG_PTR)MemoryMap->MappingSize.QuadPart) {
        return NULL;
    }

    NextAddress = (
        (PVOID)RtlOffsetToPointer(
            MemoryMap->NextAddress,
            AllocationSize
        )
    );

    EndAddress = (
        (PVOID)RtlOffsetToPointer(
            MemoryMap->BaseAddress,
            MemoryMap->MappingSize.LowPart
        )
    );

    PrevPage = (PVOID)ALIGN_DOWN(MemoryMap->NextAddress, PAGE_SIZE);
    NextPage = (PVOID)ALIGN_DOWN(NextAddress, PAGE_SIZE);
    PageAfterNextPage = (PVOID)((ULONG_PTR)NextPage + PAGE_SIZE);

    if (NextAddress > EndAddress) {

        ULONG_PTR PrevMemoryMapAllocSize;
        ULONG_PTR NextMemoryMapAllocSize;

        PrevMemoryMap = MemoryMap;

        PrevMemoryMapAllocSize = (
            (ULONG_PTR)EndAddress -
            (ULONG_PTR)PrevMemoryMap->NextAddress
        );

        NextMemoryMapAllocSize = (
            (ULONG_PTR)NextAddress -
            (ULONG_PTR)EndAddress
        );

        if (!ConsumeNextTraceStoreMemoryMap(TraceStore, NULL)) {
            return NULL;
        }

        MemoryMap = TraceStore->MemoryMap;

        if (PrevMemoryMapAllocSize == 0) {

            //
            // No spill necessary.
            //

            ReturnAddress = MemoryMap->BaseAddress;

            MemoryMap->NextAddress = (
                (PVOID)RtlOffsetToPointer(
                    MemoryMap->BaseAddress,
                    AllocationSize
                )
            );

        } else {

            //
            // The requested allocation will spill over into the next memory
            // map.  This is fine as long as the next memory map is mapped at a
            // contiguous address.
            //

            if (MemoryMap->BaseAddress != EndAddress) {

                //
                // Ugh, non-contiguous mapping.
                //

                if (!TraceStore->IsReadonly &&
                    !IsMetadataTraceStore(TraceStore) &&
                    !HasVaryingRecordSizes(TraceStore)) {

                    __debugbreak();
                }

                ReturnAddress = MemoryMap->BaseAddress;

                MemoryMap->NextAddress = (
                    (PVOID)RtlOffsetToPointer(
                        MemoryMap->BaseAddress,
                        AllocationSize
                    )
                );

            } else {

                //
                // The mapping is contiguous.
                //


                //
                // Our return address will be the value of the previous memory
                // map's next address.
                //

                ReturnAddress = PrevMemoryMap->NextAddress;

                //
                // Update the previous address fields.
                //

                PrevMemoryMap->PrevAddress = PrevMemoryMap->NextAddress;
                TraceStore->PrevAddress = PrevMemoryMap->NextAddress;
                MemoryMap->PrevAddress = PrevMemoryMap->NextAddress;

                //
                // Adjust the new memory map's NextAddress to account for the
                // bytes we had to spill over.
                //

                MemoryMap->NextAddress = (
                    (PVOID)RtlOffsetToPointer(
                        MemoryMap->BaseAddress,
                        NextMemoryMapAllocSize
                    )
                );

                if (MemoryMap->NextAddress != NextAddress) {
                    __debugbreak();
                }
            }

        }

    } else {

        if (TraceStore->NoPrefaulting) {
            goto UpdateAddresses;
        }

        //
        // If this allocation crosses a page boundary, we prefault the page
        // after the next page in a separate thread (as long as it's still
        // within our allocated range).  (We do this in a separate thread
        // as a page fault may put the thread into an alertable wait (i.e.
        // suspends it) until the underlying I/O completes if the request
        // couldn't be served by the cache manager.  That would adversely
        // affect the latency of our hot-path tracing code where
        // allocations are done quite frequently.)
        //

        if (PrevPage != NextPage) {

            //
            // Allocation crosses a page boundary.
            //

            if (PageAfterNextPage < EndAddress) {

                //
                // The page after the next page is still within our mapped
                // chunk of memory, so submit an asynchronous prefault to the
                // threadpool.
                //

                PTRACE_STORE_MEMORY_MAP PrefaultMemoryMap;

                Success = PopFreeTraceStoreMemoryMap(
                    TraceStore,
                    &PrefaultMemoryMap
                );

                if (!Success) {
                    goto UpdateAddresses;
                }

                //
                // Prefault the page after the next page after this
                // address.  That is, prefault the page that is two
                // pages away from whatever page NextAddress is in.
                //

                PrefaultMemoryMap->NextAddress = PageAfterNextPage;

                PushTraceStoreMemoryMap(
                    &TraceStore->PrefaultMemoryMaps,
                    PrefaultMemoryMap
                );

                SubmitThreadpoolWork(TraceStore->PrefaultFuturePageWork);
            }
        }

        //
        // Update the relevant memory map addresses.
        //

UpdateAddresses:
        ReturnAddress           = MemoryMap->NextAddress;
        MemoryMap->PrevAddress  = MemoryMap->NextAddress;
        TraceStore->PrevAddress = MemoryMap->NextAddress;

        MemoryMap->NextAddress = NextAddress;
    }

    if (TraceStore->IsReadonly) {
        goto End;
    }

    if (TraceStore->IsMetadata) {
        goto UpdateTotals;
    }

    //
    // Record the allocation.
    //

    Success = RecordTraceStoreAllocation(
        TraceStore,
        RecordSize,
        NumberOfRecords
    );

    if (!Success) {

        //
        // It's not clear what the best course of action to take is if we were
        // unable to record the allocation.  In normal operating conditions,
        // this would only happen if we failed to extend the underlying metadata
        // stores, which would be due to STATUS_IN_PAGE exceptions being caught,
        // which will be triggered by no more space being left on the device or
        // a network drive being lost.  If this happened on a metadata store,
        // it will inevitably happen soon after on a normal store, in which
        // case this routine would end up returning a NULL pointer.
        //
        // We can't return NULL here as we've already adjusted all the address
        // mappings as if the allocation was satisfied.  And we don't currently
        // have any infrastructure in place for rolling back such allocations,
        // a feat which is further complicated by the fact we may have crossed
        // page and mapping boundaries and kicked off asynchronous page prep
        // work.
        //
        // So, for now, we do nothing if we couldn't record the allocation.
        // (To keep SAL happy, we keep the test of Success though.)
        //

        NOTHING;
    }

UpdateTotals:
    TraceStore->Totals->NumberOfAllocations.QuadPart += 1;
    TraceStore->Totals->AllocationSize.QuadPart += AllocationSize;
    TraceStore->Eof->EndOfFile.QuadPart += AllocationSize;

End:
    if (MemoryMap->NextAddress == EndAddress) {

        //
        // This memory map has been filled entirely; attempt to consume the
        // next one.  Ignore the return code; we can't do much at this point
        // if it fails.
        //

        Success = ConsumeNextTraceStoreMemoryMap(TraceStore, NULL);

        if (!Success) {
            NOTHING;
        }
    }

    return ReturnAddress;
}

_Use_decl_annotations_
BOOL
RecordTraceStoreAllocation(
    PTRACE_STORE     TraceStore,
    PULARGE_INTEGER  RecordSize,
    PULARGE_INTEGER  NumberOfRecords
    )
/*++

Routine Description:

    This routine records a trace store allocation in a trace store's allocation
    metadata trace store.

Arguments:

    TraceStore - Supplies a pointer to a TRACE_STORE structure that the
        allocation is to be recorded against.

    RecordSize - Supplies a pointer to the address of a ULARGE_INTEGER that
        contains the size of the records that were allocated.

    NumberOfRecords - Supplies a pointer to the address of a ULARGE_INTEGER
        that contains the number of records that were allocated.

Return Value:

    TRUE if the allocation could be recorded successfully, FALSE otherwise.

--*/
{
    PTRACE_STORE_ALLOCATION Allocation;

    if (TraceStore->IsReadonly || TraceStore->IsMetadata) {

        //
        // This should never happen.
        //

        __debugbreak();
    }

    Allocation = TraceStore->Allocation;

    if (Allocation->NumberOfRecords.QuadPart == 0 ||
        Allocation->RecordSize.QuadPart != RecordSize->QuadPart) {

        PVOID Address;
        ULARGE_INTEGER AllocationRecordSize = { sizeof(*Allocation) };
        ULARGE_INTEGER NumberOfAllocationRecords = { 1 };

        //
        // Allocate a new metadata record.
        //

        Address = TraceStore->AllocationStore->AllocateRecords(
            TraceStore->TraceContext,
            TraceStore->AllocationStore,
            &AllocationRecordSize,
            &NumberOfAllocationRecords
        );

        if (!Address) {
            return FALSE;
        }

        Allocation = (PTRACE_STORE_ALLOCATION)Address;
        Allocation->RecordSize.QuadPart = RecordSize->QuadPart;
        Allocation->NumberOfRecords.QuadPart = 0;

        TraceStore->Allocation = Allocation;
    }

    //
    // Update the record count.
    //

    Allocation->NumberOfRecords.QuadPart += NumberOfRecords->QuadPart;
    return TRUE;
}


// vim:set ts=8 sw=4 sts=4 tw=80 expandtab                                     :
