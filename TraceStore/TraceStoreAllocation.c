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
TraceStoreAllocateRecordsWithTimestamp(
    PTRACE_CONTEXT  TraceContext,
    PTRACE_STORE    TraceStore,
    ULONG_PTR       NumberOfRecords,
    ULONG_PTR       RecordSize,
    PLARGE_INTEGER  TimestampPointer
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

    NumberOfRecords - Supplies the number of records to allocate.  The total
        size is derived by multiplying RecordSize with NumberOfRecords.

    RecordSize - Supplies the size of the record to allocate.

    TimestampPointer - Optionally supplies a pointer to a timestamp value to
        associate with the allocation.  When non-NULL, the 64-bit integer
        pointed to by the variable will be written to the allocation timestamp
        metadata store.  This should be NULL if the trace store is a metadata
        store, or if the trace store's traits indicates that it is a linked
        store (implying that allocation timestamp information will be provided
        elsewhere).

Return Value:

    A pointer to the base memory address satisfying the total requested size
    if the memory could be obtained successfully, NULL otherwise.

--*/
{
    PVOID Address;

    //
    // Increment the active allocators count, call the underlying allocator's
    // implementation method from within the confines of a SEH block that will
    // suppress STATUS_IN_PAGE errors, decrement the counter and return the
    // address.
    //

    InterlockedIncrement(&TraceStore->ActiveAllocators);

    TRY_MAPPED_MEMORY_OP {

        Address = TraceStoreAllocateRecordsWithTimestampImpl(
            TraceContext,
            TraceStore,
            NumberOfRecords,
            RecordSize,
            TimestampPointer
        );

    } CATCH_STATUS_IN_PAGE_ERROR {
        Address = NULL;
    }

    InterlockedDecrement(&TraceStore->ActiveAllocators);

    return Address;
}

//
// Helper macro for initializing a timestamp.  If allocation timestamps have
// been disabled, zero the timestamp.  If no timestamp has been provided and the
// trace store is not marked as linked, capture a timestamp.  Otherwise, use the
// caller's timestamp.
//

#define INIT_TIMESTAMP(Timestamp)                        \
    if (TraceStore->NoAllocationTimestamps) {            \
        Timestamp.QuadPart = 0;                          \
    } else if (!ARGUMENT_PRESENT(TimestampPointer)) {    \
        if (!IsLinkedStore(Traits)) {                    \
            QueryPerformanceCounter(&Timestamp);         \
        } else {                                         \
            Timestamp.QuadPart = 0;                      \
        }                                                \
    } else {                                             \
        Timestamp.QuadPart = TimestampPointer->QuadPart; \
    }


_Use_decl_annotations_
PVOID
TraceStoreAllocateRecordsWithTimestampImpl(
    PTRACE_CONTEXT  TraceContext,
    PTRACE_STORE    TraceStore,
    ULONG_PTR       NumberOfRecords,
    ULONG_PTR       RecordSize,
    PLARGE_INTEGER  TimestampPointer
    )
/*++

Routine Description:

    This is the main implementation body of the trace store allocation record.
    See TraceStoreAllocateRecordsWithTimestamp() for method documentation.

--*/
{
    BOOL Success;
    BOOL CheckPageSpill = FALSE;
    PTRACE_STORE_MEMORY_MAP PrevMemoryMap;
    PTRACE_STORE_MEMORY_MAP MemoryMap;
    ULONG_PTR AllocationSize;
    ULONG_PTR WastedBytes = 0;
    ULONG_PTR OriginalAllocationSize;
    PVOID ReturnAddress = NULL;
    PVOID NextAddress;
    PVOID EndAddress;
    PVOID ThisPage;
    PVOID NextPage;
    PVOID PageAfterNextPage;
    LARGE_INTEGER Timestamp;
    TRACE_STORE_TRAITS Traits;

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

    OriginalAllocationSize = AllocationSize = RecordSize * NumberOfRecords;

    if (RecordSize == 1 && NumberOfRecords > 1) {
        __debugbreak();
    }

    if (TraceStore->TraceStoreIndex == TraceStoreStringTableIndex) {
        if (!(RecordSize == 512 && NumberOfRecords == 1)) {
            __debugbreak();
        }
    }

    //
    // If the record size for the trace store isn't fixed, and the no alignment
    // trait hasn't been set, align the allocation size up to the size of a
    // pointer.
    //

    Traits = *TraceStore->pTraits;
    if (!IsFixedRecordSize(Traits) && !NoAllocationAlignment(Traits)) {
        AllocationSize = ALIGN_UP(AllocationSize, sizeof(ULONG_PTR));
    }

    if (AllocationSize > (ULONG_PTR)MemoryMap->MappingSize.QuadPart) {
        return NULL;
    }

    //
    // Ensure the traits don't indicate page alignment; that should be handled
    // by the page allocator.
    //

    if (!AssertFalse("WantsPageAlignment", WantsPageAlignment(Traits))) {
        return NULL;
    }

    CheckPageSpill = (
        HasVaryingRecords(Traits) &&
        HasMultipleRecords(Traits) &&
        !IsRecordSizeAlwaysPowerOf2(Traits) &&
        PreventPageSpill(Traits)
    );

    INIT_TIMESTAMP(Timestamp);

CalculateAddresses:
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

    ThisPage = (PVOID)ALIGN_DOWN(MemoryMap->NextAddress, PAGE_SIZE);
    NextPage = (PVOID)ALIGN_DOWN(NextAddress, PAGE_SIZE);
    PageAfterNextPage = (PVOID)((ULONG_PTR)NextPage + PAGE_SIZE);

    if (CheckPageSpill && ThisPage != NextPage) {

        //
        // The allocation will spill over a page boundary and the caller has
        // requested we prevent this, so, calculate the amount of space
        // remaining in the current page and adjust the allocation size
        // accordingly.
        //

        WastedBytes = (
            ((ULONG_PTR)NextPage) -
            ((ULONG_PTR)MemoryMap->NextAddress)
        );
        if (AllocationSize < WastedBytes) {
            __debugbreak();
        } else if (AllocationSize == WastedBytes) {
            WastedBytes = 0;
        } else {
            OriginalAllocationSize = AllocationSize;
            AllocationSize += WastedBytes;
        }
        CheckPageSpill = FALSE;
        goto CalculateAddresses;
    }

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
            __debugbreak();
            return NULL;
        }

        MemoryMap = TraceStore->MemoryMap;

        if (PrevMemoryMapAllocSize == 0) {

            //
            // No memory map spill necessary.
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
            // map.  If we're non-contiguous, our return address has to be
            // based on the new memory map's base address (potentially losing
            // the remaining bytes on the existing one).
            //

            if (MemoryMap->BaseAddress != EndAddress) {

                //
                // Non-contiguous mapping.
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

        if (ThisPage != NextPage) {

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

    if (WastedBytes) {

        //
        // Extra bytes were added to the allocation size in order to prevent
        // a page spill (by forward-padding the allocation out to the next
        // page boundary).  Adjust the pointers by this amount now.
        //

        ReturnAddress = RtlOffsetToPointer(ReturnAddress, WastedBytes);
        MemoryMap->PrevAddress  = ReturnAddress;
        TraceStore->PrevAddress = ReturnAddress;
    }

    if (TraceStore->IsMetadata) {
        goto UpdateTotals;
    }

    //
    // Record the allocation.
    //

    Success = RecordTraceStoreAllocation(TraceStore,
                                         NumberOfRecords,
                                         RecordSize,
                                         WastedBytes,
                                         Timestamp);

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
    if (WastedBytes) {
        TraceStore->Totals->NumberOfAllocations.QuadPart += 1;
    }
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
PVOID
TraceStoreAllocateRecords(
    PTRACE_CONTEXT  TraceContext,
    PTRACE_STORE    TraceStore,
    ULONG_PTR       NumberOfRecords,
    ULONG_PTR       RecordSize
    )
/*++

Routine Description:

    This routine allocates records from a trace store.  It is equivalent to
    calling TraceStoreAllocateRecordsWithTimestamp() with a NULL timestamp.

Arguments:

    TraceContext - Supplies a pointer to a TRACE_CONTEXT structure.

    TraceStore - Supplies a pointer to a TRACE_STORE structure that the memory
        is to be allocated from.

    NumberOfRecords - Supplies the number of records to allocate.

    RecordSize - Supplies the size of the record to allocate.

Return Value:

    A pointer to the base memory address satisfying the total requested size
    if the memory could be obtained successfully, NULL otherwise.

--*/
{
    return TraceStore->AllocateRecordsWithTimestamp(TraceContext,
                                                    TraceStore,
                                                    NumberOfRecords,
                                                    RecordSize,
                                                    NULL);
}

_Use_decl_annotations_
PVOID
TraceStoreAllocatePageAlignedRecordsWithTimestampImpl(
    PTRACE_CONTEXT  TraceContext,
    PTRACE_STORE    TraceStore,
    ULONG_PTR       NumberOfRecords,
    ULONG_PTR       RecordSize,
    PLARGE_INTEGER  TimestampPointer
    )
/*++

Routine Description:

    This routine allocates records from a trace store in multiples of the page
    size (4K).  The allocated memory is guaranteed to begin on a new page, and,
    if the allocation consumes multiple pages, is also guaranteed that the final
    page will belong to the allocation exclusively, even if the total allocation
    size does not consume the entire contents of the final page.  That is, the
    remaining bytes at the end of the final page will not be allocated to any
    other subsequent allocation.  The unused trailing bytes will always be zero.

Arguments:

    TraceContext - Supplies a pointer to a TRACE_CONTEXT structure.

    TraceStore - Supplies a pointer to a TRACE_STORE structure that the memory
        is to be allocated from.

    NumberOfRecords - Supplies the number of records to allocate.

    RecordSize - Supplies the size of the record to allocate.

Return Value:

    A pointer to the base memory address satisfying the total requested size
    if the memory could be obtained successfully, NULL otherwise.

--*/
{
    BOOL Success;
    PTRACE_STORE_MEMORY_MAP PrevMemoryMap;
    PTRACE_STORE_MEMORY_MAP MemoryMap;
    ULONG_PTR RequestedSize;
    ULONG_PTR AllocationSize;
    ULONG_PTR WastedBytes = 0;
    ULONG_PTR NumberOfPages;
    PVOID ReturnAddress = NULL;
    PVOID NextAddress;
    PVOID EndAddress;
    PVOID ThisPage;
    PVOID NextPage;
    PVOID EndPage;
    LARGE_INTEGER Timestamp;
    TRACE_STORE_TRAITS Traits;

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

    //
    // Load the traits.
    //

    Traits = *TraceStore->pTraits;

    INIT_TIMESTAMP(Timestamp);

#ifdef _DEBUG

    //
    // Ensure the traits indicate page alignment.
    //

    if (!AssertTrue("WantsPageAlignment", WantsPageAlignment(Traits))) {
        return NULL;
    }

    //
    // Ensure we're not metadata.
    //

    if (!AssertFalse("TraceStore->IsMetadata", TraceStore->IsMetadata)) {
        return NULL;
    }

#endif

    //
    // Calculate the requested size, and then round up to the nearest page
    // multiple.
    //

    RequestedSize = NumberOfRecords * RecordSize;
    AllocationSize = ROUND_TO_PAGES(RequestedSize);

    if (!AssertTrue("AllocationSize <= MemoryMap->MappingSize",
                    (AllocationSize <=
                     (ULONG_PTR)MemoryMap->MappingSize.QuadPart))) {
        return NULL;
    }

    //
    // Resolve page addresses.
    //

    ThisPage = PAGE_ALIGN(MemoryMap->NextAddress);
    EndPage = PAGE_ALIGN(((ULONG_PTR)MemoryMap->NextAddress) + RequestedSize);
    NumberOfPages = RequestedSize >> PAGE_SHIFT;

    //
    // Ensure our next ("current") address is page aligned.
    //

    if (!AssertTrue("MemoryMap->NextAddress is not page aligned",
                    MemoryMap->NextAddress == ThisPage)) {
        return NULL;
    }

    //
    // Calculate the next address for the memory map if we fulfill this
    // allocation, as well as the memory map's end address.
    //

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

    //
    // Ensure the next address is page aligned, and that it doesn't share the
    // same page as our final page.
    //

    NextPage = PAGE_ALIGN(NextAddress);

    if (!AssertTrue("NextAddress is not page aligned",
                    NextAddress == NextPage)) {
        return NULL;
    }

    if (!AssertTrue("EndPage <= NextPage",
                    ((ULONG_PTR)EndPage) <= ((ULONG_PTR)NextPage))) {
        return NULL;
    }

    //
    // If the next address exceeds the limits of our current memory map, we
    // can't satisfy this allocation within the remaining space.  Thus, we
    // need to retire the active map and consume a new one.
    //

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
            __debugbreak();
            return NULL;
        }

        MemoryMap = TraceStore->MemoryMap;

        if (PrevMemoryMapAllocSize == 0) {

            //
            // No memory map spill necessary.
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
            // map.  If we're non-contiguous, our return address has to be
            // based on the new memory map's base address (potentially losing
            // the remaining bytes on the existing one).
            //

            if (MemoryMap->BaseAddress != EndAddress) {

                //
                // Non-contiguous mapping.

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

        //
        // The allocation can be satisfied by the existing memory map.  Update
        // addresses accordingly.  (We don't currently prefault page aligned
        // allocations.)
        //

        ReturnAddress           = MemoryMap->NextAddress;
        MemoryMap->PrevAddress  = MemoryMap->NextAddress;
        TraceStore->PrevAddress = MemoryMap->NextAddress;

        MemoryMap->NextAddress = NextAddress;
    }

    if (TraceStore->IsReadonly) {
        goto End;
    }

    //
    // Record the allocation.
    //

    Success = RecordTraceStoreAllocation(TraceStore,
                                         NumberOfRecords,
                                         RecordSize,
                                         0,
                                         Timestamp);

    if (!Success) {

        //
        // See comment in TraceStoreAllocateRecordsWithTimestampImpl() regarding
        // the NOTHING below.
        //

        NOTHING;
    }

    //
    // Update totals and end of file.
    //

    TraceStore->Totals->NumberOfAllocations.QuadPart += 1;
    TraceStore->Totals->AllocationSize.QuadPart += AllocationSize;
    TraceStore->Eof->EndOfFile.QuadPart += AllocationSize;

    //
    // Consume the next memory map if applicable.
    //

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
PVOID
SuspendedTraceStoreAllocateRecordsWithTimestamp(
    PTRACE_CONTEXT  TraceContext,
    PTRACE_STORE    TraceStore,
    ULONG_PTR       NumberOfRecords,
    ULONG_PTR       RecordSize,
    PLARGE_INTEGER  TimestampPointer
    )
/*++

Routine Description:

    This routine is a stand-in for the normal AllocateRecordsWithTimestamp()
    method used to satisfy trace store allocations.

Arguments:

    TraceContext - Supplies a pointer to a TRACE_CONTEXT structure.

    TraceStore - Supplies a pointer to a TRACE_STORE structure that the memory
        is to be allocated from.

    NumberOfRecords - Supplies the number of records to allocate.

    RecordSize - Supplies the size of the record to allocate.

    TimestampPointer - Optionally supplies a pointer to a timestamp value to
        associate with the allocation.  When non-NULL, the 64-bit integer
        pointed to by the variable will be written to the allocation timestamp
        metadata store.  This should be NULL if the trace store is a metadata
        store, or if the trace store's traits indicates that it is a linked
        store (implying that allocation timestamp information will be provided
        elsewhere).

Return Value:

    A pointer to the base memory address satisfying the total requested size
    if the memory could be obtained successfully, NULL otherwise.

--*/
{
    ULONG WaitResult;
    HANDLE Event;
    ULONG ElapsedMicroseconds;
    LARGE_INTEGER BeforeTimestamp;
    LARGE_INTEGER BeforeElapsed;
    LARGE_INTEGER AfterTimestamp;
    LARGE_INTEGER AfterElapsed;
    PALLOCATE_RECORDS_WITH_TIMESTAMP AllocateWithTimestamp;

    //
    // Take a timestamp snapshot before waiting on the event.
    //

    TraceStoreQueryPerformanceCounter(TraceStore,
                                      &BeforeElapsed,
                                      &BeforeTimestamp);

    //
    // Wait on the resume allocations event.  This will be set when the trace
    // store has completed binding, or when whatever requested for the allocator
    // to be suspended has completed its work and signaled that allocations may
    // resume.
    //

    Event = TraceStore->ResumeAllocationsEvent;
    WaitResult = WaitForSingleObject(Event, INFINITE);

    if (WaitResult != WAIT_OBJECT_0) {

        //
        // Wait wasn't successful, abort the allocation.
        //

        return NULL;
    }

    //
    // Increment the suspended allocations counter.
    //

    InterlockedIncrement(&TraceStore->Stats->SuspendedAllocations);

    //
    // Take another timestamp snapshot and calculate elapsed microseconds.
    //

    TraceStoreQueryPerformanceCounter(TraceStore,
                                      &AfterElapsed,
                                      &AfterTimestamp);

    ElapsedMicroseconds = (ULONG)(
        AfterElapsed.QuadPart -
        BeforeElapsed.QuadPart
    );

    //
    // Update the count of elapsed microseconds we've spent suspended.
    //

    InterlockedAdd(&TraceStore->Stats->ElapsedSuspensionTimeInMicroseconds,
                   ElapsedMicroseconds);


    //
    // Wait was successful, allocations can resume.  Forward the request to the
    // original allocator.
    //

    AllocateWithTimestamp = TraceStore->AllocateRecordsWithTimestampImpl1;
    TraceContext = TraceStore->TraceContext;

    return AllocateWithTimestamp(TraceContext,
                                 TraceStore,
                                 NumberOfRecords,
                                 RecordSize,
                                 TimestampPointer);
}

_Use_decl_annotations_
PVOID
ConcurrentTraceStoreAllocateRecordsWithTimestamp(
    PTRACE_CONTEXT  TraceContext,
    PTRACE_STORE    TraceStore,
    ULONG_PTR       NumberOfRecords,
    ULONG_PTR       RecordSize,
    PLARGE_INTEGER  TimestampPointer
    )
/*++

Routine Description:

    This routine serializes trace store allocations in a multithreaded
    environment by acquiring the trace store's critical section before
    dispatching the allocation request.  It is enabled automatically if
    the trace store was configured with the concurrent allocations trait
    set.

Arguments:

    TraceContext - Supplies a pointer to a TRACE_CONTEXT structure.

    TraceStore - Supplies a pointer to a TRACE_STORE structure that the memory
        is to be allocated from.

    NumberOfRecords - Supplies the number of records to allocate.

    RecordSize - Supplies the size of the record to allocate.

    TimestampPointer - Optionally supplies a pointer to a timestamp value to
        associate with the allocation.  When non-NULL, the 64-bit integer
        pointed to by the variable will be written to the allocation timestamp
        metadata store.  This should be NULL if the trace store is a metadata
        store, or if the trace store's traits indicates that it is a linked
        store (implying that allocation timestamp information will be provided
        elsewhere).

Return Value:

    A pointer to the base memory address satisfying the total requested size
    if the memory could be obtained successfully, NULL otherwise.

--*/
{
    PVOID Address;
    PALLOCATE_RECORDS_WITH_TIMESTAMP AllocateWithTimestamp;

    AllocateWithTimestamp = TraceStore->AllocateRecordsWithTimestampImpl2;
    EnterCriticalSection(&TraceStore->Sync->AllocationCriticalSection);
    Address = AllocateWithTimestamp(TraceContext,
                                    TraceStore,
                                    NumberOfRecords,
                                    RecordSize,
                                    TimestampPointer);
    LeaveCriticalSection(&TraceStore->Sync->AllocationCriticalSection);
    return Address;
}

_Use_decl_annotations_
PVOID
TraceStoreTryAllocateRecordsWithTimestamp(
    PTRACE_CONTEXT  TraceContext,
    PTRACE_STORE    TraceStore,
    ULONG_PTR       NumberOfRecords,
    ULONG_PTR       RecordSize,
    PLARGE_INTEGER  TimestampPointer
    )
/*++

Routine Description:

    This routine attempts to acquire the trace store's critical section lock
    before dispatching the allocation request.  If the lock is contended, or
    allocations have been suspended, this routine returns immediately with a
    NULL pointer.

    N.B. There is no way to distinguish between a contended lock, suspended
         allocations and a failed allocation attempt due to some underlying
         error.  Callers should structure their code such that the number of
         times TryAllocateRecordsWithTimestamp() is called is limited to a
         certain number, and after which point, a normal call is made to the
         AllocateRecordsWithTimestamp() routine.  If NULL is still returned
         by that call, there has been an underlying error with the trace store.

Arguments:

    TraceContext - Supplies a pointer to a TRACE_CONTEXT structure.

    TraceStore - Supplies a pointer to a TRACE_STORE structure that the memory
        is to be allocated from.

    NumberOfRecords - Supplies the number of records to allocate.

    RecordSize - Supplies the size of the record to allocate.

    TimestampPointer - Optionally supplies a pointer to a timestamp value to
        associate with the allocation.  When non-NULL, the 64-bit integer
        pointed to by the variable will be written to the allocation timestamp
        metadata store.  This should be NULL if the trace store is a metadata
        store, or if the trace store's traits indicates that it is a linked
        store (implying that allocation timestamp information will be provided
        elsewhere).

Return Value:

    A pointer to the base memory address satisfying the total requested size
    if the memory could be obtained successfully, NULL otherwise.

--*/
{
    PVOID Address = NULL;
    HANDLE Event;
    ULONG WaitResult;
    PALLOCATE_RECORDS_WITH_TIMESTAMP AllocateWithTimestamp;
    PCRITICAL_SECTION CriticalSection;

    //
    // Immediately increment the active allocator count before we see if the
    // resume allocations event is signaled.
    //

    InterlockedIncrement(&TraceStore->ActiveAllocators);

    //
    // Ensure allocations aren't currently suspended.
    //

    Event = TraceStore->ResumeAllocationsEvent;
    WaitResult = WaitForSingleObject(Event, 0);

    if (WaitResult != WAIT_OBJECT_0) {

        //
        // The wait wasn't successful or allocations are currently suspended.
        //

        goto End;
    }

    //
    // Allocations aren't suspended.  Attempt to acquire the critical section.
    //

    CriticalSection = &TraceStore->Sync->AllocationCriticalSection;
    if (!TryEnterCriticalSection(CriticalSection)) {
        goto End;
    }

    //
    // Continue with allocation.  We now own the critical section.
    //

    AllocateWithTimestamp = TraceStore->AllocateRecordsWithTimestampImpl2;
    Address = AllocateWithTimestamp(TraceContext,
                                    TraceStore,
                                    NumberOfRecords,
                                    RecordSize,
                                    TimestampPointer);

    //
    // Leave the critical section and decrement the active allocator count, then
    // return the address to the caller.
    //

    LeaveCriticalSection(CriticalSection);

End:
    InterlockedDecrement(&TraceStore->ActiveAllocators);

    return Address;
}

_Use_decl_annotations_
PVOID
TraceStoreTryAllocateRecords(
    PTRACE_CONTEXT  TraceContext,
    PTRACE_STORE    TraceStore,
    ULONG_PTR       NumberOfRecords,
    ULONG_PTR       RecordSize
    )
/*++

Routine Description:

    This routine is equivalent to calling TryAllocateRecordsWithTimestamp()
    with a NULL timestamp parameter.

Arguments:

    TraceContext - Supplies a pointer to a TRACE_CONTEXT structure.

    TraceStore - Supplies a pointer to a TRACE_STORE structure that the memory
        is to be allocated from.

    NumberOfRecords - Supplies the number of records to allocate.

    RecordSize - Supplies the size of the record to allocate.

Return Value:

    A pointer to the base memory address satisfying the total requested size
    if the memory could be obtained successfully, NULL otherwise.

--*/
{
    return TraceStore->TryAllocateRecordsWithTimestamp(TraceContext,
                                                       TraceStore,
                                                       NumberOfRecords,
                                                       RecordSize,
                                                       NULL);
}

_Use_decl_annotations_
BOOL
RecordTraceStoreAllocation(
    PTRACE_STORE     TraceStore,
    ULONG_PTR        NumberOfRecords,
    ULONG_PTR        RecordSize,
    ULONG_PTR        WastedBytes,
    LARGE_INTEGER    Timestamp
    )
/*++

Routine Description:

    This routine records a trace store allocation in a trace store's allocation
    metadata trace store.  If the trace store's traits indicate coalesced
    allocations and the previous allocation is of the same record size as this
    allocation, the existing number of records count will be incremented
    accordingly (instead of writing a new allocation record).

Arguments:

    TraceStore - Supplies a pointer to a TRACE_STORE structure that the
        allocation is to be recorded against.

    NumberOfRecords - Supplies the number of records to allocate.

    RecordSize - Supplies the size of the record to allocate.

    WastedBytes - Optionally supplies the number of extra bytes the caller's
        allocation request had to be padded by in order to prevent a page
        spill.  If present, an extra allocation record will be written with
        the highest bit of the NumberOfRecords field set to 1.

    Timestamp - Supplies the value of the performance counter that will be
        saved to the allocation timestamp store.

Return Value:

    TRUE if the allocation could be recorded successfully, FALSE otherwise.

--*/
{
    BOOL RecordNewRecord;
    PVOID Address;
    TRACE_STORE_TRAITS Traits;
    PTRACE_STORE_ALLOCATION Allocation;

    if (TraceStore->IsReadonly || TraceStore->IsMetadata) {

        //
        // This should never happen.
        //

        __debugbreak();
    }

    //
    // Sanity check inputs.
    //

    if ((LONGLONG)NumberOfRecords <= 0) {
        __debugbreak();
    }

    if ((LONGLONG)RecordSize <= 0) {
        __debugbreak();
    }

    //
    // Record the allocation timestamp if applicable.
    //

    if (!RecordTraceStoreAllocationTimestamp(TraceStore, Timestamp)) {
        return FALSE;
    }

    Traits = *TraceStore->pTraits;
    Allocation = TraceStore->Allocation;

    if (WastedBytes) {

        if (Allocation->NumberOfRecords.QuadPart == 0) {

            //
            // WastedBytes should only happen because of a page spill; page
            // spills shouldn't happen if we haven't allocated any records yet.
            // Assert this invariant now.
            //

            __debugbreak();
            return FALSE;
        }

        //
        // Record a new dummy allocation record for the extra bytes.
        //

        Address = TraceStore->AllocationStore->AllocateRecordsWithTimestamp(
            TraceStore->TraceContext,
            TraceStore->AllocationStore,
            1,
            sizeof(*Allocation),
            NULL
        );

        if (!Address) {
            return FALSE;
        }

        Allocation = (PTRACE_STORE_ALLOCATION)Address;
        Allocation->RecordSize.QuadPart = WastedBytes;

        //
        // Use -1 to indicate this was a dummy allocation.
        //

        Allocation->NumberOfRecords.SignedQuadPart = -1;

        //
        // Record the padding in the trace store's stats.
        //

        TraceStore->Stats->WastedBytes += WastedBytes;
        TraceStore->Stats->PaddedAllocations += 1;

        //
        // Force a new record to be recorded.
        //

        RecordNewRecord = TRUE;

    } else {

        //
        // We record a new allocation record if:
        //
        //  A) The trace store has explicitly disabled coalesced allocations, or
        //  B) This is the first allocation (number of records will be 0), or
        //  C) The previous record size doesn't match the current record size.
        //

        RecordNewRecord = (
            !WantsCoalescedAllocations(Traits) || (
                Allocation->NumberOfRecords.QuadPart == 0 ||
                Allocation->RecordSize.QuadPart != RecordSize
            )
        );

    }

    if (RecordNewRecord) {

        //
        // Allocate a new allocation record.
        //

        Address = TraceStore->AllocationStore->AllocateRecordsWithTimestamp(
            TraceStore->TraceContext,
            TraceStore->AllocationStore,
            1,
            sizeof(*Allocation),
            NULL
        );

        if (!Address) {
            return FALSE;
        }

        //
        // Initialize the allocation record with the new details.
        //

        Allocation = (PTRACE_STORE_ALLOCATION)Address;
        Allocation->RecordSize.QuadPart = RecordSize;
        Allocation->NumberOfRecords.QuadPart = 0;

        //
        // Point the trace store's allocation pointer at this new allocation.
        //

        TraceStore->Allocation = Allocation;
    }

    //
    // Update the record count.
    //

    Allocation->NumberOfRecords.QuadPart += NumberOfRecords;

    //
    // Update the trace store's total record count and total record size.
    //

    TraceStore->Totals->NumberOfRecords.QuadPart += NumberOfRecords;
    TraceStore->Totals->RecordSize.QuadPart += (
        NumberOfRecords * RecordSize
    );

    //
    // Return success to the caller.
    //

    return TRUE;
}


// vim:set ts=8 sw=4 sts=4 tw=80 expandtab                                     :
