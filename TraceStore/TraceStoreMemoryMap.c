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
VOID
CALLBACK
PrepareNextTraceStoreMemoryMapCallback(
    PTP_CALLBACK_INSTANCE Instance,
    PVOID Context,
    PTP_WORK Work
    )
/*--

Routine Description:

    This routine is the threadpool callback target for a TraceStore's
    PrepareNextMemoryMapWork function.  It simply calls the
    PrepareNextTraceStoreMemoryMap function.

Arguments:

    Instance - Unused.

    Context - Supplies a pointer to a PTRACE_STORE.

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

    Stats = TraceStore->pStats;

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
        // This shouldn't occur if all our memory map machinery isn't working
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

    Result = Rtl->RtlCopyMappedMemory(
        AddressPointer,
        &Address,
        sizeof(Address)
    );

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

        PVOID BaseAddress = MemoryMap->BaseAddress;

        //
        // Prefault the first two pages.  The AllocateRecords function will
        // take care of prefaulting subsequent pages.
        //

        PrefaultPage(BaseAddress);
        PrefaultNextPage(BaseAddress);
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

VOID
CloseMemoryMap(_In_ PTRACE_STORE_MEMORY_MAP MemoryMap)
{
    if (!MemoryMap) {
        return;
    }

    if (MemoryMap->BaseAddress) {
        FlushViewOfFile(MemoryMap->BaseAddress, 0);
        UnmapViewOfFile(MemoryMap->BaseAddress);
        MemoryMap->BaseAddress = NULL;
    }

    if (MemoryMap->MappingHandle) {
        CloseHandle(MemoryMap->MappingHandle);
        MemoryMap->MappingHandle = NULL;
    }
}

VOID
SubmitCloseMemoryMapThreadpoolWork(
    _In_ PTRACE_STORE TraceStore,
    _Inout_ PPTRACE_STORE_MEMORY_MAP MemoryMap
    )
{
    if (!ARGUMENT_PRESENT(TraceStore)) {
        return;
    }

    if (!ARGUMENT_PRESENT(MemoryMap) || !*MemoryMap) {
        return;
    }

    PushTraceStoreMemoryMap(&TraceStore->CloseMemoryMaps, *MemoryMap);
    SubmitThreadpoolWork(TraceStore->CloseMemoryMapWork);
    *MemoryMap = NULL;
}

BOOL
ConsumeNextTraceStoreMemoryMap(
    _Inout_ PTRACE_STORE TraceStore
    )
{
    BOOL Success;
    BOOL IsMetadata;
    HRESULT Result;
    PRTL Rtl;
    PTRACE_STORE_MEMORY_MAP PrevPrevMemoryMap;
    PTRACE_STORE_MEMORY_MAP MemoryMap;
    PTRACE_STORE_MEMORY_MAP PrepareMemoryMap;
    TRACE_STORE_ADDRESS Address;
    PTRACE_STORE_ADDRESS AddressPointer;
    LARGE_INTEGER RequestedTimestamp;
    LARGE_INTEGER Elapsed;
    PTRACE_STORE_STATS Stats;
    TRACE_STORE_STATS DummyStats = { 0 };

    Rtl = TraceStore->Rtl;

    //
    // We need to switch out the current memory map for a new one in order
    // to satisfy this allocation.  This involves: a) retiring the current
    // memory map (sending it to the "prev" list), b) popping an available
    // one off the ready list, and c) preparing a "next memory map" based
    // off the ready one for submission to the threadpool.
    //
    // Note: we want our allocator to be as low-latency as possible, so
    // dropped trace records are preferable to indeterminate waits for
    // resources to be available.
    //


    //
    // We may not have a stats struct available yet if this is the first
    // call to ConsumeNextTraceStoreMemoryMap().  If that's the case, just
    // point the pointer at a dummy one.  This simplifies the rest of the
    // code in the function.
    //

    Stats = TraceStore->pStats;

    if (!Stats) {
        Stats = &DummyStats;
    }

    PrevPrevMemoryMap = TraceStore->PrevMemoryMap;

    IsMetadata = IsMetadataTraceStore(TraceStore);

    //
    // Retire the previous previous memory map if it exists.
    //

    if (!PrevPrevMemoryMap) {
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

    Result = Rtl->RtlCopyMappedMemory(&Address,
                                      AddressPointer,
                                      sizeof(Address));

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

    Result = Rtl->RtlCopyMappedMemory(AddressPointer,
                                      &Address,
                                      sizeof(Address));

    if (FAILED(Result)) {

        PrevPrevMemoryMap->pAddress = NULL;
    }

RetireOldMemoryMap:

    PushTraceStoreMemoryMap(
        &TraceStore->CloseMemoryMaps,
        PrevPrevMemoryMap
    );

    SubmitThreadpoolWork(TraceStore->CloseMemoryMapWork);

    //
    // Pop a memory map descriptor off the free list to use for the
    // PrepareMemoryMap.
    //

StartPreparation:

    Success = PopFreeTraceStoreMemoryMap(TraceStore,
                                         &PrepareMemoryMap);

    if (!Success) {

        Success = CreateMemoryMapsForTraceStore(TraceStore,
                                                TraceStore->TraceContext,
                                                0);

        if (Success) {

            Success = PopFreeTraceStoreMemoryMap(TraceStore,
                                                 &PrepareMemoryMap);
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

    Result = Rtl->RtlCopyMappedMemory(&Address,
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
    // Take a local copy of the timestamp.  We'll use this for both the "next"
    // memory map's Consumed timestamp and the "prepare" memory map's Requested
    // timestamp.
    //

    TraceStoreQueryPerformanceCounter(TraceStore, &Elapsed);

    //
    // Save the timestamp before we start fiddling with it.
    //

    RequestedTimestamp.QuadPart = Elapsed.QuadPart;

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

    Rtl->RtlCopyMappedMemory(MemoryMap->pAddress,
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

    Result = Rtl->RtlCopyMappedMemory(
        &Address,
        AddressPointer,
        sizeof(Address)
    );

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

    Result = Rtl->RtlCopyMappedMemory(AddressPointer,
                                      &Address,
                                      sizeof(Address));

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
    PushTraceStoreMemoryMap(&TraceStore->PrepareMemoryMaps, PrepareMemoryMap);
    SubmitThreadpoolWork(TraceStore->PrepareNextMemoryMapWork);

    return TRUE;
}


// vim:set ts=8 sw=4 sts=4 tw=80 expandtab                                     :
