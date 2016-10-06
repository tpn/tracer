/*++

Copyright (c) 2016 Trent Nelson <trent@trent.me>

Module Name:

    TraceStoreMemoryMap.c

Abstract:

    This module implements functionality related to trace store memory maps.
    These are the workhorse of the trace store functionality.  Functions are
    provided for preparing, consuming, flushing, retiring, closing, and
    releasing memory maps.

--*/

#include "stdafx.h"

_Use_decl_annotations_
BOOL
CreateMemoryMapsForTraceStore(
    PTRACE_STORE TraceStore,
    PTRACE_CONTEXT TraceContext,
    ULONG NumberOfItems
    )
/*--

Routine Description:

    This routine creates memory maps for a given trace store.

Arguments:

    TraceStore - Supplies a pointer to a TRACE_STORE struct that the memory
        maps are created for.

    TraceContext - Supplies a pointer to the TRACE_CONTEXT struct associated
        with the trace store.

    NumberOfItems - Supplies the number of memory maps to create.  If this
        value is zero, the default value will be used.

Return Value:

    TRUE on success, FALSE on failure.

--*/
{
    PTRACE_STORE_MEMORY_MAP MemoryMaps;
    SIZE_T AllocationSize;
    ULONG Index;

    if (!NumberOfItems) {
        NumberOfItems = InitialFreeMemoryMaps;
    }

    AllocationSize = sizeof(TRACE_STORE_MEMORY_MAP) * NumberOfItems;

    MemoryMaps = HeapAlloc(TraceContext->HeapHandle,
                           HEAP_ZERO_MEMORY,
                           AllocationSize);

    if (!MemoryMaps) {
        return FALSE;
    }

    //
    // Carve the chunk of memory allocated above into list items, then push
    // them onto the free list.
    //

    for (Index = 0; Index < NumberOfItems; Index++) {
        PTRACE_STORE_MEMORY_MAP MemoryMap = &MemoryMaps[Index];

        //
        // Ensure the ListItem is aligned on the MEMORY_ALLOCATION_ALIGNMENT.
        // This is a requirement for the underlying SLIST_ENTRY.
        //
        if (((ULONG_PTR)MemoryMap & (MEMORY_ALLOCATION_ALIGNMENT - 1)) != 0) {
            __debugbreak();
        }

        PushTraceStoreMemoryMap(&TraceStore->FreeMemoryMaps, MemoryMap);
    }

    return TRUE;
}


_Use_decl_annotations_
VOID
CALLBACK
PrepareNextTraceStoreMemoryMapCallback(
    PTP_CALLBACK_INSTANCE Instance,
    PVOID                 Context,
    PTP_WORK              Work
    )
/*--

Routine Description:

    This routine is the threadpool callback target for a TraceStore's
    PrepareNextMemoryMapWork function.  It simply calls the
    PrepareNextTraceStoreMemoryMap function.

Arguments:

    Instance - Unused.

    Context - Supplies a pointer to a TRACE_STORE structure.

    Work - Unused.

Return Value:

    None.

--*/
{
    PrepareNextTraceStoreMemoryMap((PTRACE_STORE)Context);
}

_Use_decl_annotations_
BOOL
PrepareNextTraceStoreMemoryMap(
    PTRACE_STORE TraceStore
    )
/*--

Routine Description:

    This routine is responsible for preparing the next memory map for a trace
    store given the current state of the underlying file (in terms of where
    the current file pointer is).  It is also called to initialize the first
    memory map used by a trace store.

    The routine will query the current file offset

Arguments:

    TraceStore - Supplies a pointer to a TRACE_STORE struct for which a new
        memory map is to be prepared.

Return Value:

    TRUE on success, FALSE on failure.  Failure can be caused by invalid
    arugments, or failure of the FileTimeToLocalFileTime() function or
    Rtl->RtlTimeToSecondsSince1970() function.

--*/
{
    USHORT NumaNode;
    BOOL Success;
    BOOL IsMetadata;
    BOOL HaveAddress;
    PRTL Rtl;
    HRESULT Result;
    PVOID PreferredBaseAddress;
    PVOID OriginalPreferredBaseAddress;
    TRACE_STORE_ADDRESS Address;
    PTRACE_STORE_ADDRESS AddressPointer;
    FILE_STANDARD_INFO FileInfo;
    PTRACE_STORE_MEMORY_MAP MemoryMap;
    LARGE_INTEGER CurrentFileOffset;
    LARGE_INTEGER NewFileOffset;
    LARGE_INTEGER DistanceToMove;
    LARGE_INTEGER Elapsed;
    PTRACE_STORE_STATS Stats;
    TRACE_STORE_STATS DummyStats = { 0 };

    if (!ARGUMENT_PRESENT(TraceStore)) {
        return FALSE;
    }

    Rtl = TraceStore->Rtl;

    //
    // We may not have a stats struct available yet if this is the first
    // call to PrepareNextTraceStoreMemoryMap().  If that's the case, just
    // point the pointer at a dummy one.  This simplifies the rest of the
    // code in the function.
    //

    Stats = TraceStore->Stats;

    if (!Stats) {
        Stats = &DummyStats;
    }

    if (!Rtl) {
        return FALSE;
    }

    if (!PopTraceStoreMemoryMap(&TraceStore->PrepareMemoryMaps, &MemoryMap)) {
        return FALSE;
    }

    if (!MemoryMap->FileHandle) {
        goto Error;
    }

    if (!GetTraceStoreMemoryMapFileInfo(MemoryMap, &FileInfo)) {
        goto Error;
    }

    IsMetadata = IsMetadataTraceStore(TraceStore);

    //
    // Get the current file offset.
    //

    DistanceToMove.QuadPart = 0;

    Success = SetFilePointerEx(TraceStore->FileHandle,
                               DistanceToMove,
                               &CurrentFileOffset,
                               FILE_CURRENT);

    if (!Success) {
        goto Error;
    }

    if (CurrentFileOffset.QuadPart != MemoryMap->FileOffset.QuadPart) {

        //
        // This shouldn't occur if all our memory map machinery is working
        // correctly.
        //

        __debugbreak();
    }

    //
    // Determine the distance we need to move the file pointer.
    //

    DistanceToMove.QuadPart = (
        MemoryMap->FileOffset.QuadPart +
        MemoryMap->MappingSize.QuadPart
    );

    //
    // Adjust the file pointer from the current position.
    //

    Success = SetFilePointerEx(MemoryMap->FileHandle,
                               DistanceToMove,
                               &NewFileOffset,
                               FILE_BEGIN);

    if (!Success) {
        goto Error;
    }

    //
    // If the new file offset is past the end of the file, extend it.
    //

    if (FileInfo.EndOfFile.QuadPart < NewFileOffset.QuadPart) {

        if (TraceStore->IsReadonly) {

            //
            // Something has gone wrong if we're extending a readonly store.
            //

            __debugbreak();

            goto Error;
        }

        if (!SetEndOfFile(MemoryMap->FileHandle)) {

            goto Error;
        }
    }

    //
    // Create a new file mapping for this memory map's slice of data.
    //

    MemoryMap->MappingHandle = CreateFileMapping(
        MemoryMap->FileHandle,
        NULL,
        TraceStore->CreateFileMappingProtectionFlags,
        NewFileOffset.HighPart,
        NewFileOffset.LowPart,
        NULL
    );

    if (MemoryMap->MappingHandle == NULL ||
        MemoryMap->MappingHandle == INVALID_HANDLE_VALUE) {
        DWORD LastError = GetLastError();
        goto Error;
    }

    OriginalPreferredBaseAddress = NULL;

    //
    // MemoryMap->BaseAddress will be the next contiguous address for this
    // mapping.  It is set by ConsumeNextTraceStoreMemoryMap().  We use it
    // as our preferred base address if we can.
    //

    PreferredBaseAddress = MemoryMap->BaseAddress;

    AddressPointer = MemoryMap->pAddress;

    if (IsMetadata || (AddressPointer == NULL)) {

        //
        // We don't attempt to re-use addresses if we're metadata, and we can't
        // attempt re-use if there is no backing address struct.
        //

        HaveAddress = FALSE;

        goto TryMapMemory;
    }

    HaveAddress = TRUE;

    //
    // Take a local copy of the address.
    //

    Result = Rtl->RtlCopyMappedMemory(&Address,
                                      AddressPointer,
                                      sizeof(Address));

    if (FAILED(Result)) {

        //
        // Disable the address and go straight to preparation.
        //

        HaveAddress = FALSE;
        AddressPointer = NULL;
        MemoryMap->pAddress = NULL;
        goto TryMapMemory;

    }

TryMapMemory:

    MemoryMap->BaseAddress = MapViewOfFileEx(
        MemoryMap->MappingHandle,
        TraceStore->MapViewOfFileDesiredAccess,
        MemoryMap->FileOffset.HighPart,
        MemoryMap->FileOffset.LowPart,
        MemoryMap->MappingSize.LowPart,
        PreferredBaseAddress
    );

    if (!MemoryMap->BaseAddress) {

        if (PreferredBaseAddress) {

            //
            // Make a note of the original preferred base address, clear it,
            // then attempt the mapping again.
            //

            OriginalPreferredBaseAddress = PreferredBaseAddress;
            PreferredBaseAddress = NULL;
            Stats->PreferredAddressUnavailable++;
            goto TryMapMemory;
        }

        goto Error;

    }

    if (IsMetadata) {
        goto Finalize;
    }

    if (!PreferredBaseAddress && OriginalPreferredBaseAddress) {

        //
        // The mapping succeeded, but not at our original preferred address.
        // When we implement relocation support, we'll need to do that here.
        //

        //
        // We copy the original preferred base address back so that it can be
        // picked up in the section below where it is saved to the address
        // struct.
        //

        PreferredBaseAddress = OriginalPreferredBaseAddress;

    }

    if (!HaveAddress) {
        goto Finalize;
    }

    //
    // Record all of the mapping information in our address record.
    //

    Address.PreferredBaseAddress = PreferredBaseAddress;
    Address.BaseAddress = MemoryMap->BaseAddress;
    Address.FileOffset.QuadPart = MemoryMap->FileOffset.QuadPart;
    Address.MappedSize.QuadPart = MemoryMap->MappingSize.QuadPart;

    //
    // Fill in the thread and processor information.
    //

    Address.FulfillingThreadId = FastGetCurrentThreadId();
    GetCurrentProcessorNumberEx(&Address.FulfillingProcessor);
    Success = GetNumaProcessorNodeEx(&Address.FulfillingProcessor, &NumaNode);
    Address.FulfillingNumaNode = (Success ? (UCHAR)NumaNode : 0);

    //
    // Take a local copy of the timestamp.
    //

    TraceStoreQueryPerformanceCounter(TraceStore, &Elapsed);

    //
    // Copy it to the Prepared timestamp.
    //

    Address.Timestamp.Prepared.QuadPart = Elapsed.QuadPart;

    //
    // Calculate the elapsed time spent awaiting preparation.
    //

    Elapsed.QuadPart -= Address.Timestamp.Requested.QuadPart;
    Address.Elapsed.AwaitingPreparation.QuadPart = Elapsed.QuadPart;

    //
    // Finally, copy the updated record back to the memory-mapped backing
    // store.
    //

    Result = Rtl->RtlCopyMappedMemory(AddressPointer,
                                      &Address,
                                      sizeof(Address));

    if (FAILED(Result)) {

        //
        // Disable the address struct.
        //

        MemoryMap->pAddress = NULL;

    } else if (MemoryMap->pAddress != AddressPointer) {

        //
        // Invariant check: this should never get hit.
        //

        __debugbreak();

    }

    //
    // Intentional follow-on.
    //

Finalize:

    //
    // Initialize the next address to the base address.
    //

    MemoryMap->NextAddress = MemoryMap->BaseAddress;

    if (!TraceStore->NoPrefaulting) {

        //
        // Make sure we don't prefault a page past the end of the file.
        // This can happen with the smaller metadata stores that only memory
        // map their exact structure size.
        //

        ULONG_PTR BaseAddress = (ULONG_PTR)MemoryMap->BaseAddress;
        ULONG_PTR PrefaultPage = BaseAddress + (PAGE_SIZE << 1);
        ULONG_PTR EndPage = BaseAddress + (ULONG_PTR)NewFileOffset.QuadPart;

        if (PrefaultPage < EndPage) {

            //
            // Prefault the first two pages.  The AllocateRecords function will
            // take care of prefaulting subsequent pages.
            //

            if (!Rtl->PrefaultPages((PVOID)BaseAddress, 2)) {
                goto Error;
            }
        }
    }

    Success = TRUE;
    PushTraceStoreMemoryMap(&TraceStore->NextMemoryMaps, MemoryMap);
    SetEvent(TraceStore->NextMemoryMapAvailableEvent);

    goto End;

Error:
    Success = FALSE;

    if (MemoryMap->MappingHandle) {
        CloseHandle(MemoryMap->MappingHandle);
        MemoryMap->MappingHandle = NULL;
    }

    ReturnFreeTraceStoreMemoryMap(TraceStore, MemoryMap);

End:
    return Success;
}

_Use_decl_annotations_
BOOL
ReleasePrevTraceStoreMemoryMap(
    PTRACE_STORE TraceStore
    )
/*++

Routine Description:

    This routine closes a trace store memory map.  This flushes the view of
    the file at the memory map's base address, then unmaps it and clears the
    base address pointer, then closes the corresponding memory map handle
    and clears that pointer.

    This routine is forgiving: it can be called with a NULL pointer, a NULL
    BaseAddress pointer, or a NULL MappingHandle, simplifying error handling
    logic.

    BaseAddress and MappingHandle will be cleared after the flushing/unmapping
    and handle closing has been done, respectively.

Arguments:

    MemoryMap - Supplies a pointer to a TRACE_STORE_MEMORY_MAP structure.

Return Value:

    None.

--*/
{
    PRTL Rtl;
    HRESULT Result;
    PTRACE_STORE_MEMORY_MAP MemoryMap;
    TRACE_STORE_ADDRESS Address;
    LARGE_INTEGER PreviousTimestamp;
    LARGE_INTEGER Elapsed;
    PLARGE_INTEGER ElapsedPointer;

    if (!PopTraceStoreMemoryMap(&TraceStore->CloseMemoryMaps, &MemoryMap)) {
        return FALSE;
    }

    UnmapTraceStoreMemoryMap(MemoryMap);

    if (!MemoryMap->pAddress) {
        goto Finish;
    }

    Rtl = TraceStore->Rtl;

    //
    // Take a local copy of the address record, update timestamps and
    // calculate elapsed time, then save the local record back to the
    // backing TRACE_STORE_ADDRESS struct.
    //

    Result = Rtl->RtlCopyMappedMemory(
        &Address,
        MemoryMap->pAddress,
        sizeof(Address)
    );

    if (FAILED(Result)) {

        //
        // Ignore and continue.
        //

        goto Finish;
    }

    //
    // Get a local copy of the elapsed start time.
    //

    TraceStoreQueryPerformanceCounter(TraceStore, &Elapsed);

    //
    // Copy it to the Released timestamp.
    //

    Address.Timestamp.Released.QuadPart = Elapsed.QuadPart;

    //
    // Determine what state this memory map was in at the time of being closed.
    // For a memory map that has progressed through the normal lifecycle, it'll
    // typically be in 'AwaitingRelease' at this point.  However, we could be
    // getting called against an active memory map or prepared memory map if
    // we're getting released as a result of closing the trace store.
    //

    if (Address.Timestamp.Retired.QuadPart != 0) {

        //
        // Normal memory map awaiting retirement.  Elapsed.AwaitingRelease
        // will receive our elapsed time.
        //

        PreviousTimestamp.QuadPart = Address.Timestamp.Retired.QuadPart;
        ElapsedPointer = &Address.Elapsed.AwaitingRelease;

    } else if (Address.Timestamp.Consumed.QuadPart != 0) {

        //
        // An active memory map.  Elapsed.Active will receive our elapsed time.
        //

        PreviousTimestamp.QuadPart = Address.Timestamp.Consumed.QuadPart;
        ElapsedPointer = &Address.Elapsed.Active;

    } else if (Address.Timestamp.Prepared.QuadPart != 0) {

        //
        // A prepared memory map awaiting consumption.
        // Elapsed.AwaitingConsumption will receive our elapsed time.
        //

        PreviousTimestamp.QuadPart = Address.Timestamp.Prepared.QuadPart;
        ElapsedPointer = &Address.Elapsed.AwaitingConsumption;

    } else {

        //
        // A memory map that wasn't even prepared.  Highly unlikely.
        //

        PreviousTimestamp.QuadPart = Address.Timestamp.Requested.QuadPart;
        ElapsedPointer = &Address.Elapsed.AwaitingPreparation;
    }

    //
    // Calculate the elapsed time.
    //

    Elapsed.QuadPart -= PreviousTimestamp.QuadPart;

    //
    // Update the target elapsed time.
    //

    ElapsedPointer->QuadPart = Elapsed.QuadPart;

    //
    // Copy the local record back to the backing store and ignore the
    // return value.
    //

    Rtl->RtlCopyMappedMemory(MemoryMap->pAddress,
                             &Address,
                             sizeof(Address));

Finish:
    ReturnFreeTraceStoreMemoryMap(TraceStore, MemoryMap);
    return TRUE;
}

_Use_decl_annotations_
VOID
CALLBACK
ReleasePrevTraceStoreMemoryMapCallback(
    PTP_CALLBACK_INSTANCE   Instance,
    PVOID                   Context,
    PTP_WORK                Work
    )
{
    //
    // Ensure Context is non-NULL.
    //
    if (!Context) {
        return;
    }

    ReleasePrevTraceStoreMemoryMap((PTRACE_STORE)Context);
}


_Use_decl_annotations_
VOID
RundownTraceStoreMemoryMap(
    PTRACE_STORE_MEMORY_MAP MemoryMap
    )
/*++

Routine Description:

    This routine runs down a trace store memory map.  This flushes the view of
    the file at the memory map's base address, then unmaps it and clears the
    base address pointer, then closes the corresponding memory map handle
    and clears that pointer.

    This routine is forgiving: it can be called with a NULL pointer, a NULL
    BaseAddress pointer, or a NULL MappingHandle, simplifying error handling
    logic.

    BaseAddress and MappingHandle will be cleared after the flushing/unmapping
    and handle closing has been done, respectively.

Arguments:

    MemoryMap - Supplies a pointer to a TRACE_STORE_MEMORY_MAP structure.

Return Value:

    None.

--*/
{
    //
    // Validate arguments.
    //

    if (!MemoryMap) {
        return;
    }

    if (MemoryMap->BaseAddress) {

        //
        // Flush the view, unmap it, then clear the base address pointer.
        //

        FlushViewOfFile(MemoryMap->BaseAddress, 0);
        UnmapViewOfFile(MemoryMap->BaseAddress);
        MemoryMap->BaseAddress = NULL;
    }

    if (MemoryMap->MappingHandle) {

        //
        // Close the memory mapping handle and clear the pointer.
        //

        CloseHandle(MemoryMap->MappingHandle);
        MemoryMap->MappingHandle = NULL;
    }
}

_Use_decl_annotations_
BOOL
UnmapTraceStoreMemoryMap(
    PTRACE_STORE_MEMORY_MAP MemoryMap
    )
/*++

Routine Description:

    This routine unmaps a trace store memory map's view and closes the memory
    mapping handle.

Arguments:

    MemoryMap - Supplies a pointer to a TRACE_STORE_MEMORY_MAP structure.

Return Value:

    None.

--*/
{
    if (!MemoryMap) {
        return FALSE;
    }

    if (MemoryMap->BaseAddress) {
        UnmapViewOfFile(MemoryMap->BaseAddress);
        MemoryMap->BaseAddress = NULL;
    }

    if (MemoryMap->MappingHandle) {
        CloseHandle(MemoryMap->MappingHandle);
        MemoryMap->MappingHandle = NULL;
    }

    return TRUE;
}


_Use_decl_annotations_
VOID
SubmitCloseMemoryMapThreadpoolWork(
    PTRACE_STORE TraceStore,
    PPTRACE_STORE_MEMORY_MAP MemoryMap
    )
/*++

Routine Description:

    This routine submits a memory map to a trace store's close memory map
    thread pool work routine.  It is used to asynchronously close trace store
    memory maps.

Arguments:

    TraceStore - Supplies a pointer to a TRACE_STORE structure.

    MemoryMap - Supplies a pointer to an address that contains a pointer to the
        trace store memory map to close.  The pointer will be cleared as the
        last step of this routine.

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

    if (!ARGUMENT_PRESENT(MemoryMap) || !*MemoryMap) {
        return;
    }

    //
    // Push the referenced memory map onto the trace store's close memory map
    // list and submit a close memory map threadpool work item, then clear the
    // memory map pointer.
    //

    PushTraceStoreMemoryMap(&TraceStore->CloseMemoryMaps, *MemoryMap);
    SubmitThreadpoolWork(TraceStore->CloseMemoryMapWork);
    *MemoryMap = NULL;
}

_Use_decl_annotations_
BOOL
ConsumeNextTraceStoreMemoryMap(
    PTRACE_STORE TraceStore
    )
/*++

Routine Description:

    This routine consumes the next trace store memory map for a trace store.
    It is called via two paths: a) once, by BindTraceStoreToTraceContext(),
    when a trace store's first memory map needs to be activated, and b) by
    a trace store's AllocateRecords() function, when the current memory map
    has been exhausted and it needs the next one in order to satisfy the memory
    allocation.

    Consuming the next memory map involves: a) retiring the current memory map
    (sending it to the "prev" list), b) popping the next memory map off the
    "ready" list, and b) preparing a "next memory map" based off the ready one
    for submission to the "prepare next memory map" threadpool.

    A central design tenet of the tracing machinery is that it should be as
    low-latency as possible, with a dropped trace record being preferable to
    a thread stalled waiting for backing memory maps to be available.  Thus,
    if this routine cannot immediately satisfy the steps required to perform
    its duty without blocking, it will immediately return FALSE, indicating
    that the next memory map is not yet ready.

Arguments:

    TraceStore - Supplies a pointer to a TRACE_STORE structure from which to
        consume the next memory map.

Return Value:

    TRUE if the next memory map was successfully "consumed", FALSE otherwise.

--*/
{
    BOOL Success;
    BOOL IsMetadata;
    HRESULT Result;
    PRTL Rtl;
    PTRACE_CONTEXT TraceContext;
    PTRACE_STORE_MEMORY_MAP PrevPrevMemoryMap;
    PTRACE_STORE_MEMORY_MAP MemoryMap;
    PTRACE_STORE_MEMORY_MAP PrepareMemoryMap;
    TRACE_STORE_ADDRESS Address;
    PTRACE_STORE_ADDRESS AddressPointer;
    LARGE_INTEGER RequestedTimestamp;
    LARGE_INTEGER Elapsed;
    PTRACE_STORE_STATS Stats;
    TRACE_STORE_STATS DummyStats = { 0 };
    PRTL_COPY_MAPPED_MEMORY RtlCopyMappedMemory;

    //
    // Validate arguments.
    //

    if (!TraceStore) {
        return FALSE;
    }

    //
    // Initialize aliases.
    //

    Rtl = TraceStore->Rtl;
    RtlCopyMappedMemory = Rtl->RtlCopyMappedMemory;
    TraceContext = TraceStore->TraceContext;

    //
    // We may not have a stats struct available yet if this is the first
    // call to ConsumeNextTraceStoreMemoryMap().  If that's the case, just
    // point the pointer at a dummy one.  This simplifies the rest of the
    // code in the function.
    //

    Stats = TraceStore->Stats;

    if (!Stats) {
        Stats = &DummyStats;
    }

    //
    // The previous memory map becomes the previous-previous memory map.
    //

    PrevPrevMemoryMap = TraceStore->PrevMemoryMap;

    IsMetadata = IsMetadataTraceStore(TraceStore);

    //
    // Retire the previous previous memory map if it exists.
    //

    if (!PrevPrevMemoryMap) {

        //
        // No previous previous memory map needs to be retired, so we can jump
        // straight into preparation.
        //

        goto StartPreparation;
    }

    TraceStore->PrevMemoryMap = NULL;

    if (TraceStore->NoRetire) {
        goto StartPreparation;
    }

    //
    // We need to retire this memory map.  If we're metadata or there's no
    // underlying address record for this memory map, we can go straight to the
    // retire logic.  Otherwise, we need to update the various timestamps.
    //

    AddressPointer = PrevPrevMemoryMap->pAddress;

    if (IsMetadata || (AddressPointer == NULL)) {
        goto RetireOldMemoryMap;
    }

    //
    // Take a local copy of the address record.
    //

    Result = RtlCopyMappedMemory(&Address, AddressPointer, sizeof(Address));

    if (FAILED(Result)) {

        PrevPrevMemoryMap->pAddress = NULL;
        goto RetireOldMemoryMap;
    }

    //
    // Take a local timestamp.
    //

    TraceStoreQueryPerformanceCounter(TraceStore, &Elapsed);

    //
    // Copy to the Retired timestamp.
    //

    Address.Timestamp.Retired.QuadPart = Elapsed.QuadPart;

    //
    // Calculate the memory map's active elapsed time.
    //

    Elapsed.QuadPart -= Address.Timestamp.Consumed.QuadPart;

    Address.Elapsed.Active.QuadPart = Elapsed.QuadPart;

    //
    // Copy back to the memory mapped backing store.
    //

    Result = RtlCopyMappedMemory(AddressPointer, &Address, sizeof(Address));

    if (FAILED(Result)) {

        PrevPrevMemoryMap->pAddress = NULL;
    }

RetireOldMemoryMap:

    PushTraceStoreMemoryMap(
        &TraceStore->CloseMemoryMaps,
        PrevPrevMemoryMap
    );

    SubmitThreadpoolWork(TraceStore->CloseMemoryMapWork);

StartPreparation:

    //
    // Pop a memory map descriptor off the free list to use for the
    // PrepareMemoryMap.  If there are no free memory maps, attempt to create
    // a new set.  If this fails, increment the dropped record and exhausted
    // free memory maps counter and return FALSE indicating that we failed to
    // prepare the next memory map.
    //

    Success = PopFreeTraceStoreMemoryMap(TraceStore, &PrepareMemoryMap);

    if (!Success) {

        Success = CreateMemoryMapsForTraceStore(TraceStore, TraceContext, 0);

        if (Success) {

            //
            // Attempt to obtain a free memory map again.
            //

            Success = PopFreeTraceStoreMemoryMap(TraceStore, &PrepareMemoryMap);
        }

        if (!Success) {
            Stats->DroppedRecords++;
            Stats->ExhaustedFreeMemoryMaps++;
            return FALSE;
        }
    }

    if (!PopTraceStoreMemoryMap(&TraceStore->NextMemoryMaps, &MemoryMap)) {

        //
        // Our allocations are outpacing the next memory map preparation
        // being done asynchronously in the threadpool, so drop the record.
        //

        Stats->AllocationsOutpacingNextMemoryMapPreparation++;
        Stats->DroppedRecords++;

        //
        // Return the PrepareMemoryMap back to the free list.
        //

        ReturnFreeTraceStoreMemoryMap(TraceStore, PrepareMemoryMap);
        return FALSE;
    }

    //
    // We've now got the two things we need: a free memory map to fill in with
    // the details of the next memory map to prepare (PrepareMemoryMap), and
    // the ready memory map (MemoryMap) that contains an active mapping ready
    // for use.
    //

    if (TraceStore->MemoryMap) {
        TraceStore->PrevAddress = TraceStore->MemoryMap->PrevAddress;
        MemoryMap->PrevAddress = TraceStore->MemoryMap->PrevAddress;
        TraceStore->PrevMemoryMap = TraceStore->MemoryMap;
    }

    TraceStore->MemoryMap = MemoryMap;

    //
    // Take a local copy of the timestamp.  We'll use this for both the "next"
    // memory map's Consumed timestamp and the "prepare" memory map's Requested
    // timestamp.
    //

    TraceStoreQueryPerformanceCounter(TraceStore, &Elapsed);

    //
    // Save the timestamp before we start fiddling with it.
    //

    RequestedTimestamp.QuadPart = Elapsed.QuadPart;

    if (IsMetadata) {

        //
        // Skip the TRACE_STORE_ADDRESS logic for metadata stores.
        //

        goto PrepareMemoryMap;
    }

    if (!MemoryMap->pAddress) {

        //
        // There was a STATUS_IN_PAGE_ERROR preventing an address record for
        // this memory map.
        //

        goto PrepareMemoryMap;
    }

    //
    // Take a local copy of the address record, update timestamps and
    // calculate elapsed time, then save the local record back to the
    // backing TRACE_STORE_ADDRESS struct.
    //

    Result = RtlCopyMappedMemory(&Address,
                                 MemoryMap->pAddress,
                                 sizeof(Address));

    if (FAILED(Result)) {

        //
        // Ignore and continue.
        //

        MemoryMap->pAddress = NULL;
        goto PrepareMemoryMap;
    }

    //
    // Update the Consumed timestamp.
    //

    Address.Timestamp.Consumed.QuadPart = Elapsed.QuadPart;

    //
    // Calculate elapsed time between Prepared and Consumed.
    //

    Elapsed.QuadPart -= Address.Timestamp.Prepared.QuadPart;

    //
    // Save as the AwaitingConsumption timestamp.
    //

    Address.Elapsed.AwaitingConsumption.QuadPart = Elapsed.QuadPart;

    //
    // Copy the local record back to the backing store and ignore the
    // return value.
    //

    RtlCopyMappedMemory(MemoryMap->pAddress,
                        &Address,
                        sizeof(Address));

PrepareMemoryMap:

    //
    // Prepare the next memory map with the relevant offset details based
    // on the new memory map and submit it to the threadpool.
    //

    PrepareMemoryMap->FileHandle = MemoryMap->FileHandle;
    PrepareMemoryMap->MappingSize.QuadPart = MemoryMap->MappingSize.QuadPart;

    PrepareMemoryMap->FileOffset.QuadPart = (
        MemoryMap->FileOffset.QuadPart +
        MemoryMap->MappingSize.QuadPart
    );

    //
    // Set the base address to the next contiguous address for this mapping.
    // PrepareNextTraceStoreMemoryMap() will attempt to honor this if it can.
    //

    PrepareMemoryMap->BaseAddress = (
        (PVOID)RtlOffsetToPointer(
            MemoryMap->BaseAddress,
            MemoryMap->MappingSize.LowPart
        )
    );

    if (IsMetadata) {
        goto SubmitPreparedMemoryMap;
    }

    //
    // Attempt to load the next address record and fill in the relevant details.
    // This will become the prepared memory map's address record if everything
    // goes successfully.
    //

    Success = LoadNextTraceStoreAddress(TraceStore, &AddressPointer);

    if (!Success) {

        //
        // Ignore and go straight to submission.
        //

        goto SubmitPreparedMemoryMap;
    }

    //
    // Take a local copy.
    //

    Result = RtlCopyMappedMemory(&Address, AddressPointer, sizeof(Address));

    if (FAILED(Result)) {

        //
        // Ignore and go straight to submission.
        //

        goto SubmitPreparedMemoryMap;
    }

    //
    // Copy the timestamp we took earlier to the Requested timestamp.
    //

    Address.Timestamp.Requested.QuadPart = RequestedTimestamp.QuadPart;

    //
    // Copy the local record back to the backing store.
    //

    Result = RtlCopyMappedMemory(AddressPointer, &Address, sizeof(Address));

    if (SUCCEEDED(Result)) {

        //
        // Update the memory map to point at the address struct.
        //

        PrepareMemoryMap->pAddress = AddressPointer;

    } else if (PrepareMemoryMap->pAddress != NULL) {

        //
        // Invariant check: this should never get hit.
        //

        __debugbreak();
    }

SubmitPreparedMemoryMap:

    //
    // Push the prepare memory map to the relevant list and submit a threadpool
    // work item.
    //

    PushTraceStoreMemoryMap(&TraceStore->PrepareMemoryMaps, PrepareMemoryMap);
    SubmitThreadpoolWork(TraceStore->PrepareNextMemoryMapWork);

    return TRUE;
}


// vim:set ts=8 sw=4 sts=4 tw=80 expandtab                                     :
