/*++

Copyright (c) 2016 Trent Nelson <trent@trent.me>

Module Name:

    TraceStoreBind.c

Abstract:

    This module implements functionality to bind a trace store to a trace
    context.  Binding behavior is slightly different between normal trace
    stores and metadata trace stores, as well as between tracing sessions
    and readonly ones.

    These routines are typically invoked via threadpool callbacks as part of
    initializing a trace context.

--*/

#include "stdafx.h"

_Use_decl_annotations_
BOOL
BindStore(
    PTRACE_CONTEXT TraceContext,
    PTRACE_STORE TraceStore
    )
/*++

Routine Description:

    This routine binds a trace store to a trace context.  It can be called
    against normal or metadata trace stores and in tracing or readonly mode.

    It is responsible for performing the common "binding" operations needed
    by all trace stores: creating memory maps, creating events, creating
    threadpool work items, creating file mappings and views, etc.

    If a bind complete callback has been provided (TraceStore->BindComplete),
    it will be called as the last step prior to returning if the trace store
    has been successfully bound.

Arguments:

    TraceContext - Supplies a pointer to a TRACE_CONTEXT structure for which
        the given TraceStore is to be bound.

    TraceStore - Supplies a pointer to a TRACE_STORE structure that will be
        bound to the given TraceContext.

Return Value:

    TRUE on success, FALSE on failure.

--*/
{
    BOOL Success;
    BOOL IsReadonly;
    BOOL IsMetadata;
    BOOL HasRelocations;
    ULONG NumberOfMaps = 0;
    TRACE_STORE_TRAITS Traits;
    TRACE_STORE_METADATA_ID MetadataId;
    HRESULT Result;
    PRTL Rtl;
    PBIND_COMPLETE BindComplete;
    PLARGE_INTEGER Requested;
    PTRACE_STORE_MEMORY_MAP FirstMemoryMap;
    PTRACE_STORE_ADDRESS AddressPointer;
    TRACE_STORE_ADDRESS Address;

    TraceStore->TraceContext = TraceContext;

    //
    // Initialize aliases.
    //

    IsReadonly = (BOOL)TraceContext->Flags.Readonly;
    IsMetadata = IsMetadataTraceStore(TraceStore);

    //
    // If we're readonly, dispatch to the custom bind routines.
    //

    if (IsReadonly) {
        if (IsMetadata) {
            return BindMetadataStoreReadonly(TraceContext, TraceStore);
        } else {
            return BindTraceStoreReadonly(TraceContext, TraceStore);
        }
    }

    //
    // Create memory maps, events and threadpool work items.
    //

    if (!CreateMemoryMapsForTraceStore(TraceStore,
                                       &FirstMemoryMap,
                                       &NumberOfMaps)) {
        return FALSE;
    }

    if (!CreateTraceStoreEvents(TraceStore)) {
        return FALSE;
    }

    if (!CreateTraceStoreThreadpoolWorkItems(TraceContext, TraceStore)) {
        return FALSE;
    }

    //
    // Initialize remaining aliases.
    //

    Traits = *TraceStore->pTraits;
    HasRelocations = TraceStoreHasRelocations(TraceStore);
    BindComplete = TraceStore->BindComplete;

    //
    // If we're metadata, jump straight to the preparation of the first map.
    //

    if (IsMetadata) {
        MetadataId = TraceStore->TraceStoreMetadataId;
        goto PrepareFirstMemoryMap;
    }

    //
    // Attempt to load the address record for this store.
    //

    Success = LoadNextTraceStoreAddress(TraceStore, &AddressPointer);
    if (!Success) {
        return FALSE;
    }

    //
    // The remaining logic deals with initializing an address structure for
    // first use.  This is normally handled as part of preparation for the
    // next memory map in the ConsumeNextTraceStoreMemoryMap() function, but
    // we need to do it here manually for the first map.
    //

    //
    // Take a local copy.
    //

    Rtl = TraceStore->Rtl;
    Result = Rtl->RtlCopyMappedMemory(&Address,
                                      AddressPointer,
                                      sizeof(Address));
    if (FAILED(Result)) {
        return FALSE;
    }

    //
    // Set the Requested timestamp.
    //

    Requested = &Address.Timestamp.Requested;
    TraceStoreQueryPerformanceCounter(TraceStore, Requested);

    //
    // Zero out everything else.
    //

    Address.Timestamp.Prepared.QuadPart = 0;
    Address.Timestamp.Consumed.QuadPart = 0;
    Address.Timestamp.Retired.QuadPart = 0;
    Address.Timestamp.Released.QuadPart = 0;

    Address.Elapsed.AwaitingPreparation.QuadPart = 0;
    Address.Elapsed.AwaitingConsumption.QuadPart = 0;
    Address.Elapsed.Active.QuadPart = 0;
    Address.Elapsed.AwaitingRelease.QuadPart = 0;

    //
    // Copy the address back.
    //

    Result = Rtl->RtlCopyMappedMemory(AddressPointer,
                                      &Address,
                                      sizeof(Address));
    if (FAILED(Result)) {
        return FALSE;
    }

    //
    // Update the memory map to point at the address struct.
    //

    FirstMemoryMap->pAddress = AddressPointer;

PrepareFirstMemoryMap:

    FirstMemoryMap->FileHandle = TraceStore->FileHandle;
    FirstMemoryMap->MappingSize.QuadPart = TraceStore->MappingSize.QuadPart;

    //
    // Prepare the first memory map, which will create the file mapping and
    // map a view of it, then consume it, which activates the the memory map
    // and allows it to be used by AllocateRecords().
    //

    Success = PrepareNextTraceStoreMemoryMap(TraceStore, FirstMemoryMap);
    if (!Success) {
        goto Error;
    }

    Success = ConsumeNextTraceStoreMemoryMap(TraceStore, FirstMemoryMap);
    if (!Success) {
        goto Error;
    }

    if (IsMetadata && MetadataId == TraceStoreMetadataMetadataInfoId) {

        //
        // MetadataInfo needs to be special-cased and have its BindComplete
        // routine called straight after memory map consumption, as it is
        // responsible for wiring up all the backing TRACE_STORE_INFO structs,
        // including the one for itself, which needs to happen before any of
        // the trace store's info fields (like end of file or allocation totals)
        // can be written to.
        //

        Success = BindComplete(TraceContext, TraceStore, FirstMemoryMap);
        if (!Success) {
            goto Error;
        }

        //
        // Clear the BindComplete pointer so we don't attempt to call it again
        // below.
        //

        BindComplete = NULL;
    }

    if (IsMetadata && IsSingleRecord(Traits)) {

        //
        // This will fake a single record allocation by manually setting the
        // trace store's end of file and totals to appropriate values.  (We
        // can't use TraceStore->AllocateRecords() here because that routine
        // doesn't support allocations against "single record" trace stores.)
        //

        InitializeMetadataFromRecordSize(TraceStore);
    }

    //
    // If we have a bind complete callback, call it now.
    //

    if (BindComplete) {
        Success = TraceStore->BindComplete(
            TraceContext,
            TraceStore,
            FirstMemoryMap
        );
        if (!Success) {
            goto Error;
        }
    }

    //
    // Finally, copy over the time from the trace context.
    //

    CopyTraceStoreTime(TraceContext, TraceStore);

    return TRUE;

Error:
    UnmapTraceStoreMemoryMap(FirstMemoryMap);
    return FALSE;
}

_Use_decl_annotations_
BOOL
BindMetadataStoreReadonly(
    PTRACE_CONTEXT TraceContext,
    PTRACE_STORE TraceStore
    )
/*++

Routine Description:

    This routine binds a readonly metadata trace store to a trace context.
    This algorithm differs from normal "writable" trace session binding in
    that metadata stores are mapped in their entirety up-front via a single
    memory map operation.

Arguments:

    TraceContext - Supplies a pointer to a TRACE_CONTEXT structure for which
        the given TraceStore is to be bound.

    TraceStore - Supplies a pointer to a TRACE_STORE structure that will be
        bound to the given TraceContext.

Return Value:

    TRUE on success, FALSE on failure.

--*/
{
    BOOL Success;
    BOOL IsMetadataInfo;
    DWORD CreateFileMappingProtectionFlags;
    DWORD MapViewOfFileDesiredAccess;
    TRACE_STORE_METADATA_ID MetadataId;
    PBIND_COMPLETE BindComplete;
    FILE_STANDARD_INFO FileInfo;
    PTRACE_STORE_MEMORY_MAP MemoryMap;

    //
    // Sanity check we're readonly and we've been called with a metadata store.
    //

    if (!TraceStore->IsReadonly || !TraceStore->IsMetadata) {
        return FALSE;
    }

    MetadataId = TraceStore->TraceStoreMetadataId;
    IsMetadataInfo = (MetadataId == TraceStoreMetadataMetadataInfoId);

    //
    // Wire up the embedded memory map such that TraceStore->MemoryMap always
    // points to a valid structure.
    //

    MemoryMap = &TraceStore->SingleMemoryMap;
    MemoryMap->FileHandle = TraceStore->FileHandle;
    TraceStore->MemoryMap = MemoryMap;

    if (!GetTraceStoreMemoryMapFileInfo(MemoryMap, &FileInfo)) {
        TraceStore->LastError = GetLastError();
        return FALSE;
    }

    //
    // If nothing was written to the metadata, there's no more to do.
    //

    if (FileInfo.EndOfFile.QuadPart == 0) {
        if (MetadataId == TraceStoreMetadataMetadataInfoId ||
            MetadataId == TraceStoreMetadataInfoId) {

            //
            // MetadataInfo and Info should always have a value.
            //
            __debugbreak();
            return FALSE;
        }

        return TRUE;
    }

    MemoryMap->FileOffset.QuadPart = 0;
    MemoryMap->MappingSize.QuadPart = FileInfo.EndOfFile.QuadPart;

    //
    // Make sure the mapping size is under 2GB.
    //

    if (MemoryMap->MappingSize.HighPart != 0) {
        return FALSE;
    }

    //
    // Relocation stores get special-cased: we need to adjust a couple of
    // pointers once mapped (Reloc->Relocations and Reloc->Bitmap.Buffer)
    // and it's vastly easier to just leverage copy-on-write here than any
    // other approach.
    //

    if (MetadataId == TraceStoreMetadataRelocationId) {
        CreateFileMappingProtectionFlags = PAGE_WRITECOPY;
        MapViewOfFileDesiredAccess = FILE_MAP_COPY;
    } else {
        CreateFileMappingProtectionFlags = PAGE_READONLY;
        MapViewOfFileDesiredAccess = FILE_MAP_COPY;
    }

    MemoryMap->MappingHandle = CreateFileMappingNuma(
        MemoryMap->FileHandle,
        NULL,
        CreateFileMappingProtectionFlags,
        MemoryMap->MappingSize.HighPart,
        MemoryMap->MappingSize.LowPart,
        NULL,
        TraceStore->NumaNode
    );

    if (MemoryMap->MappingHandle == NULL ||
        MemoryMap->MappingHandle == INVALID_HANDLE_VALUE) {
        TraceStore->LastError = GetLastError();
        return FALSE;
    }

    MemoryMap->BaseAddress = MapViewOfFileExNuma(
        MemoryMap->MappingHandle,
        MapViewOfFileDesiredAccess,
        MemoryMap->FileOffset.HighPart,
        MemoryMap->FileOffset.LowPart,
        MemoryMap->MappingSize.LowPart,
        0,
        TraceStore->NumaNode
    );

    if (!MemoryMap->BaseAddress) {
        TraceStore->LastError = GetLastError();
        goto Error;
    }

    BindComplete = TraceStore->BindComplete;
    if (BindComplete) {
        Success = BindComplete(TraceContext, TraceStore, MemoryMap);
        if (!Success) {
            goto Error;
        }
    }

    return TRUE;

Error:
    UnmapTraceStoreMemoryMap(MemoryMap);
    return FALSE;
}

_Use_decl_annotations_
BOOL
BindTraceStoreReadonly(
    PTRACE_CONTEXT TraceContext,
    PTRACE_STORE TraceStore
    )
/*++

Routine Description:

    This routine binds a readonly trace store to a trace context.

Arguments:

    TraceContext - Supplies a pointer to a TRACE_CONTEXT structure for which
        the given TraceStore is to be bound.

    TraceStore - Supplies a pointer to a TRACE_STORE structure that will be
        bound to the given TraceContext.

Return Value:

    TRUE on success, FALSE on failure.

--*/
{
    TRACE_STORE_TRAITS Traits;

    //
    // Sanity check we're readonly and we're not a metadata store.
    //

    if (!TraceStore->IsReadonly || TraceStore->IsMetadata) {
        return FALSE;
    }

    //
    // Load the trace store's traits, which will have been saved in the store's
    // metadata :info store.
    //

    Traits = *TraceStore->Traits;

    //
    // Dispatch to the relevant handler.
    //

    if (!IsStreamingRead(Traits)) {
        return BindNonStreamingReadonlyTraceStore(TraceContext, TraceStore);
    } else {
        return BindStreamingReadonlyTraceStore(TraceContext, TraceStore);
    }

}

_Use_decl_annotations_
BOOL
BindNonStreamingReadonlyTraceStore(
    PTRACE_CONTEXT TraceContext,
    PTRACE_STORE TraceStore
    )
/*++

Routine Description:

    This routine binds a non-streaming readonly trace store to a trace context.

    It uses the trace store's address range metadata information to identify
    how many memory maps to create and which addresses to map them at.

Arguments:

    TraceContext - Supplies a pointer to a TRACE_CONTEXT structure for which
        the given TraceStore is to be bound.

    TraceStore - Supplies a pointer to a TRACE_STORE structure that will be
        bound to the given TraceContext.

Return Value:

    TRUE on success, FALSE on failure.

--*/
{
    BOOL Success;
    ULONG Index;
    ULONG NumberOfMaps;
    USHORT NumaNode;
    HANDLE MappingHandle;
    DWORD ProcessId;
    DWORD ThreadId;
    LARGE_INTEGER FileOffset;
    ULARGE_INTEGER NumberOfAddressRanges;
    PLARGE_INTEGER Requested;
    FILE_STANDARD_INFO FileInfo;
    PALLOCATOR Allocator;
    PROCESSOR_NUMBER ProcessorNumber;
    PTRACE_STORE_ADDRESS Address;
    PTRACE_STORE_ADDRESS Addresses = NULL;
    PTRACE_STORE_MEMORY_MAP MemoryMap;
    PTRACE_STORE_MEMORY_MAP FirstMemoryMap;
    PTRACE_STORE_MEMORY_MAP MemoryMaps = NULL;
    PTRACE_STORE_ADDRESS_RANGE AddressRange;
    PTRACE_STORE_ADDRESS_RANGE AddressRanges = NULL;
    PTRACE_STORE_ADDRESS_RANGE ReadonlyAddressRanges = NULL;

    //
    // Check the file size; if it's empty, we don't need to do anything else.
    //

    if (!GetTraceStoreFileInfo(TraceStore, &FileInfo)) {
        TraceStore->LastError = GetLastError();
        return FALSE;
    }

    if (FileInfo.EndOfFile.QuadPart == 0) {
        return TRUE;
    }

    //
    // Identify how many maps we need to create by looking at how many address
    // range records there were.  A map is created for each range.
    //

    NumberOfAddressRanges.QuadPart = (
        NumberOfTraceStoreAddressRanges(TraceStore)
    );

    //
    // (These should probably be elsewhere.)
    //

    TraceStore->Address = (PTRACE_STORE_ADDRESS)(
        TraceStore->AddressStore->MemoryMap->BaseAddress
    );

    TraceStore->NumberOfAllocations.QuadPart = (
        NumberOfTraceStoreAllocations(TraceStore)
    );

    TraceStore->NumberOfAddressRanges.QuadPart = (
        NumberOfTraceStoreAddressRanges(TraceStore)
    );

    //
    // Sanity check the total number isn't over 32-bits.
    //

    if (NumberOfAddressRanges.HighPart != 0) {
        return FALSE;
    }

    NumberOfMaps = NumberOfAddressRanges.LowPart;

    //
    // Create a file mapping for the entire file's contents up front.  Use
    // PAGE_WRITECOPY in order to get copy-on-write semantics, which will be
    // needed if we need to alter any of the pages for relocation purposes.
    //

    TraceStore->MapViewOfFileDesiredAccess = FILE_MAP_COPY;
    MappingHandle = CreateFileMappingNuma(
        TraceStore->FileHandle,
        NULL,
        PAGE_WRITECOPY,
        FileInfo.EndOfFile.HighPart,
        FileInfo.EndOfFile.LowPart,
        NULL,
        TraceStore->NumaNode
    );

    if (MappingHandle == NULL || MappingHandle == INVALID_HANDLE_VALUE) {
        TraceStore->LastError = GetLastError();
        return FALSE;
    }

    TraceStore->MappingHandle = MappingHandle;

    //
    // Create the memory maps.
    //

    Success = CreateMemoryMapsForReadonlyTraceStore(TraceStore,
                                                    &FirstMemoryMap,
                                                    &NumberOfMaps);

    if (!Success) {
        CloseHandle(MappingHandle);
        return FALSE;
    }

    //
    // Allocate space for a TRACE_STORE_ADDRESS structure for each memory map.
    // As we're readonly, we back these via the allocator versus the trace
    // store's :Address metadata store.
    //

    Allocator = TraceStore->Allocator;
    Addresses = (PTRACE_STORE_ADDRESS)(
        Allocator->Calloc(
            Allocator->Context,
            NumberOfMaps,
            sizeof(TRACE_STORE_ADDRESS)
        )
    );

    if (!Addresses) {
        goto Error;
    }

    TraceStore->ReadonlyAddresses = Addresses;

    //
    // Allocate space for another TRACE_STORE_ADDRESS_RANGE for every map for
    // the readonly store to use.  The RegisterNewTraceStoreAddressRange()
    // routine will use these records.
    //

    ReadonlyAddressRanges = (PTRACE_STORE_ADDRESS_RANGE)(
        Allocator->Calloc(
            Allocator->Context,
            NumberOfMaps,
            sizeof(TRACE_STORE_ADDRESS_RANGE)
        )
    );

    if (!ReadonlyAddressRanges) {
        goto Error;
    }

    TraceStore->ReadonlyAddressRanges = ReadonlyAddressRanges;
    TraceStore->ReadonlyAddressRangesConsumed = 0;

    //
    // Create events and threadpool work items.
    //

    if (!CreateTraceStoreEvents(TraceStore)) {
        goto Error;
    }

    if (!CreateTraceStoreThreadpoolWorkItems(TraceContext, TraceStore)) {
        goto Error;
    }

    //
    // N.B. Code must not 'goto Error' after this point as memory maps will
    //      potentially be in play.
    //

    //
    // Increment the context's prepare counter.
    //

    InterlockedIncrement(&TraceContext->PrepareReadonlyMapsInProgress);

    //
    // Initialize remaining loop constants.
    //

    NumaNode = 0;
    FileOffset.QuadPart = 0;
    ThreadId = FastGetCurrentThreadId();
    ProcessId = FastGetCurrentProcessId();
    MemoryMaps = FirstMemoryMap;
    AddressRanges = TraceStore->AddressRange;
    GetCurrentProcessorNumberEx(&ProcessorNumber);
    GetNumaProcessorNodeEx(&ProcessorNumber, &NumaNode);

    //
    // Enumerate the address range records, fill in a memory map and address
    // for each one, then submit a threadpool work item to prepare it.
    //

    for (Index = 0; Index < NumberOfMaps; Index++) {

        //
        // Initialize aliases.
        //

        Address = &Addresses[Index];
        MemoryMap = &MemoryMaps[Index];
        AddressRange = &AddressRanges[Index];

        //
        // Fill in the rest of the memory map's details.
        //

        MemoryMap->FileHandle = TraceStore->FileHandle;
        MemoryMap->MappingHandle = MappingHandle;
        MemoryMap->FileOffset.QuadPart = FileOffset.QuadPart;
        MemoryMap->MappingSize.QuadPart = AddressRange->MappedSize.QuadPart;
        MemoryMap->PreferredBaseAddress = AddressRange->PreferredBaseAddress;
        MemoryMap->BaseAddress = AddressRange->PreferredBaseAddress;

        //
        // XXX todo: verify EndAddresses match.
        //

        //
        // Update the file offset.
        //

        FileOffset.QuadPart += MemoryMap->MappingSize.QuadPart;

        //
        // Fill in address details.
        //

        Requested = &Address->Timestamp.Requested;
        TraceStoreQueryPerformanceCounter(TraceStore, Requested);
        Address->RequestingThreadId = ThreadId;
        Address->ProcessId = ProcessId;
        Address->RequestingProcessor = ProcessorNumber;
        Address->RequestingNumaNode = (UCHAR)NumaNode;
        Address->MappedSequenceId = (
            InterlockedIncrement(&TraceStore->MappedSequenceId)
        );

        //
        // Wire up the address to the memory map.
        //

        MemoryMap->pAddress = Address;

        //
        // Submit the threadpool prepare.
        //

        SubmitPrepareReadonlyMap(TraceContext, TraceStore, MemoryMap);

    }

    return TRUE;

Error:

    if (TraceStore->MappingHandle) {
        CloseHandle(TraceStore->MappingHandle);
        TraceStore->MappingHandle = NULL;
    }

    return FALSE;
}

_Use_decl_annotations_
BOOL
BindStreamingReadonlyTraceStore(
    PTRACE_CONTEXT TraceContext,
    PTRACE_STORE TraceStore
    )
/*++

Routine Description:

    This routine binds a streaming readonly trace store to a trace context.

Arguments:

    TraceContext - Supplies a pointer to a TRACE_CONTEXT structure for which
        the given TraceStore is to be bound.

    TraceStore - Supplies a pointer to a TRACE_STORE structure that will be
        bound to the given TraceContext.

Return Value:

    TRUE on success, FALSE on failure.

--*/
{

    //
    // XXX todo: not yet implemented.
    //

    return TRUE;
}

// vim:set ts=8 sw=4 sts=4 tw=80 expandtab                                     :
