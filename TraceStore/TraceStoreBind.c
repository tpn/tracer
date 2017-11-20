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
    LARGE_INTEGER Timestamp;
    PLARGE_INTEGER Requested;
    PTRACE_STORE_MEMORY_MAP FirstMemoryMap;
    PTRACE_STORE_ADDRESS AddressPointer;
    TRACE_STORE_ADDRESS Address;

    //
    // Initialize aliases.
    //

    IsReadonly = (BOOL)TraceContext->Flags.Readonly;
    IsMetadata = IsMetadataTraceStore(TraceStore);

    if (IsMetadata) {

        TraceStore->TraceContext = TraceContext;

    } else {

        //
        // Invariant check: TraceStore->TraceContext should already match
        // TraceContext if we're not metadata.
        //

        if (TraceStore->TraceContext != TraceContext) {
            __debugbreak();
        }
    }

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
        __debugbreak();
        return FALSE;
    }

    if (!CreateTraceStoreEvents(TraceStore)) {
        __debugbreak();
        return FALSE;
    }

    if (!CreateTraceStoreThreadpoolWorkItems(TraceContext, TraceStore)) {
        __debugbreak();
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
        __debugbreak();
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
        __debugbreak();
        return FALSE;
    }

    //
    // Set the Requested timestamp.
    //

    Requested = &Address.Timestamp.Requested;
    TraceStoreQueryPerformanceCounter(TraceStore, Requested, &Timestamp);

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
        __debugbreak();
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
        __debugbreak();
        goto Error;
    }

    Success = ConsumeNextTraceStoreMemoryMap(TraceStore, FirstMemoryMap);
    if (!Success) {
        __debugbreak();
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
            __debugbreak();
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
            __debugbreak();
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

            //
            // N.B. This was getting hit when loading trace stores readonly that
            //      are still active.  Put an additional check in to only break
            //      if there has been at least one allocation.
            //

            if (TraceStore->Totals->NumberOfAllocations.QuadPart > 0) {
                __debugbreak();
                return FALSE;
            }
        }

        return TRUE;
    }

    MemoryMap->FileOffset.QuadPart = 0;
    MemoryMap->MappingSize.QuadPart = FileInfo.EndOfFile.QuadPart;

    //
    // Relocation stores get special-cased: we need to adjust a couple of
    // pointers once mapped (Reloc->Relocations and bitmap buffer pointers)
    // and it's vastly easier to just leverage copy-on-write here.
    //

    if (MetadataId == TraceStoreMetadataRelocationId) {
        CreateFileMappingProtectionFlags = PAGE_WRITECOPY;
        MapViewOfFileDesiredAccess = FILE_MAP_COPY;
    } else {
        CreateFileMappingProtectionFlags = PAGE_READONLY;
        MapViewOfFileDesiredAccess = FILE_MAP_READ;
    }

    //
    // N.B. There's a slight design flaw with this current logic.  As we don't
    //      provide a preferred base address when mapping readonly metadata
    //      stores, there's a slim chance we'll be assigned an address that will
    //      conflict with an address we want to request later when mapping other
    //      trace store address ranges.  I haven't witnessed this happen in the
    //      wild yet thanks to high-entropy ASLR, however.
    //
    //      This could be addressed as part of another enhancement that is
    //      being contemplated regarding a new trace store geared toward
    //      efficient capture of *all* trace store address ranges, such that
    //      free address ranges can be identified easily in situations like
    //      this.
    //

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
        MemoryMap->MappingSize.QuadPart,
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
    // metadata :Info store.
    //

    Traits = *TraceStore->Traits;

    //
    // Default to readonly mapping, unless we're non-streaming and we have
    // relocations.
    //

    TraceStore->CreateFileMappingProtectionFlags = PAGE_READONLY;
    TraceStore->MapViewOfFileDesiredAccess = FILE_MAP_READ;

    if (!IsStreamingRead(Traits) && TraceStore->HasRelocations) {
        TraceStore->CreateFileMappingProtectionFlags = PAGE_WRITECOPY;
        TraceStore->MapViewOfFileDesiredAccess = FILE_MAP_COPY;
    }

    //
    // (Ignore the "non-streaming" part of this name for now.  We're trialling
    //  it with normal streaming trace stores too.)
    //

    return BindNonStreamingReadonlyTraceStore(TraceContext, TraceStore);
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

    If applicable, it additionally creates a second mapping at a new base
    address that covers the entire range of the file, allowing the data to be
    accessed via a flat array from a single base address.

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
    USHORT NumaNode;
    ULONG Index;
    ULONG NumberOfMaps;
    DWORD ProcessId;
    DWORD ThreadId;
    HANDLE MappingHandle;
    HANDLE FlatMappingHandle;
    LARGE_INTEGER FileOffset;
    LONGLONG BytesRemaining;
    ULONGLONG EndOfFile;
    LARGE_INTEGER Timestamp;
    PLARGE_INTEGER Requested;
    FILE_STANDARD_INFO FileInfo;
    PALLOCATOR Allocator;
    PROCESSOR_NUMBER ProcessorNumber;
    PULARGE_INTEGER MappingSizes = NULL;
    PTRACE_STORE_ADDRESS Address;
    PTRACE_STORE_ADDRESS Addresses = NULL;
    PTRACE_STORE_MEMORY_MAP MemoryMap;
    PTRACE_STORE_MEMORY_MAP FlatMemoryMap;
    PTRACE_STORE_MEMORY_MAP FirstMemoryMap;
    PTRACE_STORE_MEMORY_MAP MemoryMaps = NULL;
    PTRACE_STORE_ADDRESS_RANGE AddressRange;
    PTRACE_STORE_ADDRESS_RANGE ReadonlyAddressRange;
    PTRACE_STORE_ADDRESS_RANGE AddressRanges = NULL;
    PTRACE_STORE_ADDRESS_RANGE ReadonlyAddressRanges = NULL;

    //
    // Check the file size; if it's empty, we don't need to do any binding,
    // so go straight to submitting a bind complete work item to the threadpool.
    //

    if (!GetTraceStoreFileInfo(TraceStore, &FileInfo)) {
        TraceStore->LastError = GetLastError();
        return FALSE;
    }

    if (FileInfo.EndOfFile.QuadPart == 0) {
        SubmitReadonlyNonStreamingBindComplete(TraceContext, TraceStore);
        return TRUE;
    }

    //
    // Load the number of allocations that were done.
    //

    TraceStore->NumberOfAllocations.QuadPart = (
        TraceStore->Totals->NumberOfAllocations.QuadPart
    );

    //
    // Sanity check the number of address ranges allocated wasn't over 32-bits.
    // (In practice, they'll be much, much less, but this is a good check for
    //  completely bogus data.)
    //

    if (TraceStore->NumberOfAddressRanges.HighPart != 0) {
        __debugbreak();
        return FALSE;
    }

    //
    // Initialize aliases.
    //

    AddressRanges = TraceStore->AddressRange;
    EndOfFile = (ULONGLONG)FileInfo.EndOfFile.QuadPart;

    //
    // Calculate the number of memory maps we need to create.  If the file
    // size is smaller than the mapping size, this is easy: we only need one
    // map.
    //
    // If the file size is larger than the mapping size, we mimic the original
    // memory map address ranges exactly.
    //

    if (AddressRanges->MappedSize.QuadPart > EndOfFile) {

        //
        // Mapping size is greater than than the end of the file, so we only
        // need one map.
        //

        NumberOfMaps = 1;

    } else {

        NumberOfMaps = TraceStore->NumberOfAddressRanges.LowPart;
    }

    //
    // Create the file mapping.
    //

    MappingHandle = CreateFileMappingNuma(
        TraceStore->FileHandle,
        NULL,
        TraceStore->CreateFileMappingProtectionFlags,
        FileInfo.EndOfFile.HighPart,
        FileInfo.EndOfFile.LowPart,
        NULL,
        TraceStore->NumaNode
    );

    if (MappingHandle == NULL || MappingHandle == INVALID_HANDLE_VALUE) {
        TraceStore->LastError = GetLastError();
        __debugbreak();
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
        __debugbreak();
        CloseHandle(MappingHandle);
        return FALSE;
    }

    //
    // Allocate space for a TRACE_STORE_ADDRESS structure for each memory map.
    // As we're readonly, we back these via the allocator versus the trace
    // store's :Address metadata store.
    //

    Allocator = TraceStore->pAllocator;
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

    //
    // Allocate space for an array of mapping sizes.
    //

    MappingSizes = (PULARGE_INTEGER)(
        Allocator->Calloc(
            Allocator->Context,
            NumberOfMaps,
            sizeof(ULARGE_INTEGER)
        )
    );

    if (!MappingSizes) {
        goto Error;
    }

    //
    // Copy the pointers back to the trace store.
    //

    TraceStore->ReadonlyMappingSizes = MappingSizes;
    TraceStore->ReadonlyMemoryMaps = FirstMemoryMap;
    TraceStore->ReadonlyAddressRanges = ReadonlyAddressRanges;
    TraceStore->ReadonlyPreferredAddressUnavailable = 0;

    //
    // Create events and threadpool work items.
    //

    if (!CreateTraceStoreEvents(TraceStore)) {
        __debugbreak();
        goto Error;
    }

    if (!CreateTraceStoreThreadpoolWorkItems(TraceContext, TraceStore)) {
        __debugbreak();
        goto Error;
    }

    //
    // If this is a single record trait, we already implicitly have a flat
    // mapping, so no further work needs to be done.
    //

    if (IsSingleRecord(*TraceStore->pTraits)) {
        goto PrepareMaps;
    }

    //
    // Create another mapping that covers the entire range of the file, using
    // the FlatMemoryMap and FlatAddress structures.  This will be submitted
    // for mapping once the trace context has finished loading all other trace
    // stores.
    //

    FlatMemoryMap = &TraceStore->FlatMemoryMap;

    //
    // Invariant check: make sure the SingleMemoryMap isn't being used for
    // anything else at the moment.
    //

    if (FlatMemoryMap->FileHandle || FlatMemoryMap->BaseAddress) {
        __debugbreak();
        goto Error;
    }

    //
    // Just inherit the normal mapping flags for now.  (We could experiment
    // with large pages here later.)
    //

    FlatMappingHandle = CreateFileMappingNuma(
        TraceStore->FileHandle,
        NULL,
        TraceStore->CreateFileMappingProtectionFlags,
        FileInfo.EndOfFile.HighPart,
        FileInfo.EndOfFile.LowPart,
        NULL,
        TraceStore->NumaNode
    );

    if (FlatMappingHandle == NULL ||
        FlatMappingHandle == INVALID_HANDLE_VALUE) {
        TraceStore->LastError = GetLastError();
        __debugbreak();
        goto Error;
    }

    TraceStore->FlatMappingHandle = FlatMappingHandle;

    FlatMemoryMap->FileHandle = TraceStore->FileHandle;
    FlatMemoryMap->MappingHandle = FlatMappingHandle;
    FlatMemoryMap->FileOffset.QuadPart = 0;
    FlatMemoryMap->PreferredBaseAddress = NULL;
    FlatMemoryMap->BaseAddress = NULL;
    FlatMemoryMap->MappingSize.QuadPart = FileInfo.EndOfFile.QuadPart;

    //
    // Point the pAddress element to the FlatAddress embedded structure.
    //

    FlatMemoryMap->pAddress = &TraceStore->FlatAddress;

    //
    // Submit the deferred flat memory map binding.
    //

    SubmitDeferredFlatMemoryMap(TraceContext, TraceStore);

    //
    // Continue with the preparation of the normal memory maps (that mirror
    // the mapping that was done originally by the process being traced).
    //

PrepareMaps:

    //
    // N.B. Code must not 'goto Error' after this point as memory maps will
    //      potentially be in play.
    //

    //
    // Initialize remaining loop constants.
    //

    NumaNode = 0;
    FileOffset.QuadPart = 0;
    ThreadId = FastGetCurrentThreadId();
    ProcessId = FastGetCurrentProcessId();
    MemoryMaps = FirstMemoryMap;
    GetCurrentProcessorNumberEx(&ProcessorNumber);
    GetNumaProcessorNodeEx(&ProcessorNumber, &NumaNode);
    BytesRemaining = FileInfo.EndOfFile.QuadPart;

    //
    // If we only have one map, its size will be the end of the file.
    //

    if (NumberOfMaps == 1) {
        MappingSizes[0].QuadPart = BytesRemaining;
    } else {

        //
        // Otherwise, loop through the address ranges and fill in the mapping
        // sizes.
        //

        for (Index = 0; Index < NumberOfMaps; Index++) {

            AddressRange = &AddressRanges[Index];
            BytesRemaining -= AddressRange->MappedSize.QuadPart;

            //
            // If bytes remaining is less than zero, use the end of file minus
            // the current file offset as the final mapping size, then break out
            // of the loop.
            //

            if (BytesRemaining < 0) {
                MappingSizes[Index].QuadPart = EndOfFile - FileOffset.QuadPart;
                break;
            }

            //
            // Otherwise, use the original address range's mapped size as this
            // mapping size, and update the file offset.
            //

            MappingSizes[Index].QuadPart = AddressRange->MappedSize.QuadPart;
            FileOffset.QuadPart += AddressRange->MappedSize.QuadPart;
        }

        //
        // If the loop index is less than the number of maps, decrement our
        // number of maps count.  This isn't that unusual; a trace store may
        // have prepared memory maps ahead of time, encountered a preferred
        // address unavailable event, created a new address range, then never
        // actually ended up using it.
        //

        if (Index < (NumberOfMaps - 1)) {
            NumberOfMaps = Index + 1;
        }
    }

    //
    // Reflect the number of maps back to the trace store's number of readonly
    // address ranges field.
    //

    TraceStore->NumberOfReadonlyAddressRanges.QuadPart = NumberOfMaps;

    //
    // Reset the file offset and bytes remaining counter.
    //

    FileOffset.QuadPart = 0;
    BytesRemaining = FileInfo.EndOfFile.QuadPart;

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
        ReadonlyAddressRange = &ReadonlyAddressRanges[Index];

        //
        // Fill in the rest of the memory map's details.
        //

        MemoryMap->FileHandle = TraceStore->FileHandle;
        MemoryMap->MappingHandle = MappingHandle;
        MemoryMap->FileOffset.QuadPart = FileOffset.QuadPart;
        MemoryMap->PreferredBaseAddress = AddressRange->PreferredBaseAddress;
        MemoryMap->BaseAddress = AddressRange->PreferredBaseAddress;
        MemoryMap->MappingSize.QuadPart = MappingSizes[Index].QuadPart;

        //
        // Update the file offset.
        //

        FileOffset.QuadPart += MemoryMap->MappingSize.QuadPart;
        BytesRemaining -= (LONGLONG)MemoryMap->MappingSize.QuadPart;

        //
        // Fill in address details.
        //

        Requested = &Address->Timestamp.Requested;
        TraceStoreQueryPerformanceCounter(TraceStore, Requested, &Timestamp);
        Address->RequestingThreadId = ThreadId;
        Address->ProcessId = ProcessId;
        Address->RequestingProcessor = ProcessorNumber;
        Address->RequestingNumaNode = (UCHAR)NumaNode;
        Address->MappedSequenceId = (
            InterlockedIncrement(&TraceStore->MappedSequenceId)
        );

        //
        // Point the corresponding readonly address range record at this
        // original address range.  The rest of the readonly address range
        // details are filled in via PrepareReadonlyTraceStoreMemoryMap().
        //

        ReadonlyAddressRange->OriginalAddressRange = AddressRange;

        //
        // Wire up the address to the memory map.
        //

        MemoryMap->pAddress = Address;

        //
        // Submit the threadpool prepare.
        //

        SubmitPrepareReadonlyNonStreamingMap(TraceContext,
                                             TraceStore,
                                             MemoryMap);

    }

    //
    // Invariant checks: bytes remaining should be 0, and file offset should
    // match the end of file.
    //

    if (BytesRemaining != 0) {
        __debugbreak();
        return FALSE;
    }

    if (FileOffset.QuadPart != FileInfo.EndOfFile.QuadPart) {
        __debugbreak();
        return FALSE;
    }

    Success = TRUE;


    goto End;

Error:

    Success = FALSE;

    if (TraceStore->MappingHandle) {
        CloseHandle(TraceStore->MappingHandle);
        TraceStore->MappingHandle = NULL;
    }

    //
    // Intentional follow-on to End.
    //

End:

    return Success;
}

_Use_decl_annotations_
BOOL
PrepareNonStreamingReadonlyTraceStoreMapsComplete(
    PTRACE_CONTEXT TraceContext,
    PTRACE_STORE TraceStore
    )
/*++

Routine Description:

    This routine is called when a non-streaming readonly trace store has
    completed memory mapping its contents.  If the trace store requires
    no relocation, it sets its relocation complete event and decrements the
    trace context's binds in progress counter.

    If the trace store has relocations, it will wait on the relocation complete
    events of dependent trace stores.  Once satisfied, it will submit threadpool
    relocation work items for each of its memory maps.

Arguments:

    TraceContext - Supplies a pointer to a TRACE_CONTEXT structure.

    TraceStore - Supplies a pointer to a TRACE_STORE structure.

Return Value:

    TRUE on success, FALSE on failure.

--*/
{
    BOOL Success;
    ULONG Index;
    ULONG WaitResult;
    ULONG NumberOfWaits;
    ULONG NumberOfRelocationsRequired;
    TRACE_STORE_ID TraceStoreId;
    PRTL Rtl;
    HANDLE Event;
    PHANDLE Events;
    PTRACE_STORE TargetStore;
    PTRACE_STORES TraceStores;
    PTRACE_STORE_RELOC Reloc;
    PPTRACE_STORE DependencyStore;

    //
    // Initialize aliases.
    //

    Rtl = TraceStore->Rtl;
    Reloc = TraceStore->Reloc;
    TraceStores = TraceContext->TraceStores;
    TraceStoreId = TraceStore->TraceStoreId;

    if (!TraceStore->HasRelocations) {

        //
        // If the trace store does not have any relocations, we still "complete"
        // the relocation process, as this signals our relocation complete event
        // and satisfies the wait of other trace stores dependent upon us before
        // they can start their relocation.
        //

        goto CompleteRelocation;
    }

    if (TraceStore->OnlyRelocationIsToNull) {

        //
        // A fast-path for trace stores that only have null relocations.  These
        // stores require various field offsets to be zeroed, but don't actually
        // need to do any relocation with regards to other stores.
        //

        goto ReadyForRelocation;

    } else if (TraceStore->OnlyRelocationIsToSelf) {

        //
        // Invariant check: HasNullStoreRelocations should be false here.
        //

        if (TraceStore->HasNullStoreRelocations) {
            __debugbreak();
            return FALSE;
        }

        if (TraceStore->ReadonlyPreferredAddressUnavailable == 0) {

            //
            // If the only relocations we had were self-referencing, and we
            // didn't have any preferred address unavailable events, we don't
            // need to do any relocation.
            //

            goto CompleteRelocation;
        }

        //
        // We have self-references, and we had one or more preferred address
        // unavailable events, so we'll have to relocate.
        //

        TraceStore->RequiresSelfRelocation = TRUE;
        TraceStore->NumberOfRelocationsRequired = 1;
        goto ReadyForRelocation;
    }

    //
    // Invariant check: number of relocation dependencies should be greater
    // than or equal to one.
    //

    if (TraceStore->NumberOfRelocationDependencies == 0) {
        __debugbreak();
        return FALSE;
    }

    //
    // If we reach this point, we have relocations.  What we do next depends
    // on whether we're dependent upon multiple stores or just one.
    //

    if (TraceStore->HasMultipleRelocationWaits) {
        BOOL WaitAll = TRUE;

        //
        // We depend on more than one store, so we need to wait on multiple
        // event handles.  These were prepared for us in advance by the
        // LoadTraceStoreRelocationInfo() routine.
        //

        Events = TraceStore->RelocationCompleteWaitEvents;
        NumberOfWaits = TraceStore->NumberOfRelocationDependencies;

        //
        // Sanity check that we're waiting on a valid number of events.
        //

        if (NumberOfWaits > MAXIMUM_WAIT_OBJECTS) {
            __debugbreak();
            return FALSE;
        }

        //
        // Wait for all of the other stores to complete their relocation.
        //

        WaitResult = WaitForMultipleObjects(NumberOfWaits,
                                            Events,
                                            WaitAll,
                                            INFINITE);

        //
        // Verify all waits were satisfied.
        //

        if (WaitResult != WAIT_OBJECT_0) {
            __debugbreak();
            return FALSE;
        }

        //
        // Initialize variables before the loop.
        //

        NumberOfRelocationsRequired = 0;
        DependencyStore = TraceStore->RelocationDependencyStores;

        //
        // Enumerate each dependent store and check to see if it actually
        // needed to be relocated.  If it did, increment our counter.  If
        // not, clear that pointer in the dependency array.
        //

        for (Index = 0; Index < NumberOfWaits; Index++) {

            if ((*DependencyStore)->ReadonlyPreferredAddressUnavailable > 0) {

                //
                // Increment our counter and advance the store pointer.
                //

                NumberOfRelocationsRequired++;
                DependencyStore++;

            } else {

                //
                // Clear this store's pointer in the relocation array.
                //

                *DependencyStore++ = NULL;
            }
        }

        if (NumberOfRelocationsRequired == 0) {

            //
            // None of the other trace stores we're dependent upon needed
            // relocation.  Check to see if we have any self references and
            // whether or not we had any preferred address unavailable events.
            //

            if (TraceStore->HasSelfRelocations) {
                if (TraceStore->ReadonlyPreferredAddressUnavailable > 0) {
                    TraceStore->RequiresSelfRelocation = TRUE;
                    NumberOfRelocationsRequired++;
                }
            }
        }

        TraceStore->NumberOfRelocationsRequired = NumberOfRelocationsRequired;

        if (NumberOfRelocationsRequired == 0) {

            //
            // If we still have no relocations required, we can complete
            // early at this point.
            //

            goto CompleteRelocation;
        }

        //
        // If we get here, we need to perform relocations.
        //

        goto ReadyForRelocation;
    }

    //
    // Invariant check: number of relocation dependencies should be 1 here.
    //

    if (TraceStore->NumberOfRelocationDependencies != 1) {
        __debugbreak();
        return FALSE;
    }

    //
    // Wait for the dependent trace store to complete its relocation.
    //

    Event = TraceStore->RelocationCompleteWaitEvent;

    WaitResult = WaitForSingleObject(Event, INFINITE);

    if (WaitResult != WAIT_OBJECT_0) {
        return FALSE;
    }

    TargetStore = TraceStore->RelocationDependencyStore;
    if (TargetStore->ReadonlyPreferredAddressUnavailable > 0) {
        goto ReadyForRelocation;
    }

    //
    // Intentional follow-on to CompleteRelocation.
    //

CompleteRelocation:

    if (TraceStore->HasNullStoreRelocations) {
        goto ReadyForRelocation;
    }

    Success = ReadonlyNonStreamingTraceStoreCompleteRelocation(
        TraceContext,
        TraceStore
    );

    goto End;

ReadyForRelocation:

    //
    // Invariant check: number of relocations required should be greater than
    // or equal to one, or has null relocations should be set.
    //

    if (TraceStore->NumberOfRelocationsRequired == 0) {
        if (!TraceStore->HasNullStoreRelocations) {

            //
            // (This is currently getting hit for TraceStorePathTableEntry,
            //  but I'm not sure why.  I'm not sure if this invariant test
            //  is event correct.  Disable for now.)
            //

            __debugbreak();
            //return FALSE;
        }
    }

    Success = ReadonlyNonStreamingTraceStoreReadyForRelocation(
        TraceContext,
        TraceStore
    );

    //
    // Intentional follow-on to End.
    //

End:

    return Success;
}

_Use_decl_annotations_
BOOL
BindFlatMemoryMap(
    PTRACE_CONTEXT TraceContext,
    PTRACE_STORE_MEMORY_MAP FlatMemoryMap
    )
/*++

Routine Description:

    This routine binds a flat readonly trace store memory map to a trace
    context.  This simply involves mapping the entire file for the trace store
    via a single MapViewOfFileExNuma() call.  This routine is always executed
    after all other mappings (which try and maintain preferred addresses) are
    complete -- otherwise, we may get assigned an address range that conflicts
    with a range used by the trace store initially.

Arguments:

    TraceContext - Supplies a pointer to a TRACE_CONTEXT structure for which
        the given FlatMemoryMap is to be bound.

    FlatMemoryMap - Supplies a pointer to a TRACE_STORE's embedded FlatMemoryMap
        structure that is used to capture details about the flat mapping.

Return Value:

    TRUE on success, FALSE on failure.

--*/
{
    USHORT NumaNode;
    ULONG ProcessId;
    ULONG ThreadId;
    PLARGE_INTEGER Requested;
    LARGE_INTEGER FileOffset;
    LARGE_INTEGER Timestamp;
    LARGE_INTEGER Elapsed;
    PTRACE_STORE TraceStore;
    PTRACE_STORE_ADDRESS FlatAddress;
    PTRACE_STORE_ADDRESS_RANGE FlatAddressRange;
    PROCESSOR_NUMBER ProcessorNumber;

    //
    // Resolve the trace store from the flat memory map and verify the invariant
    // that HasFlatMapping is set to TRUE.
    //

    TraceStore = CONTAINING_RECORD(FlatMemoryMap,
                                   TRACE_STORE,
                                   FlatMemoryMap);

    if (!TraceStore->HasFlatMapping) {
        __debugbreak();
    }

    //
    // Initialize local variables.
    //

    NumaNode = 0;
    FileOffset.QuadPart = 0;
    ThreadId = FastGetCurrentThreadId();
    ProcessId = FastGetCurrentProcessId();
    GetCurrentProcessorNumberEx(&ProcessorNumber);
    GetNumaProcessorNodeEx(&ProcessorNumber, &NumaNode);

    //
    // Fill in address details.
    //

    FlatAddress = &TraceStore->FlatAddress;
    Requested = &FlatAddress->Timestamp.Requested;
    TraceStoreQueryPerformanceCounter(TraceStore, Requested, &Timestamp);
    FlatAddress->RequestingThreadId = ThreadId;
    FlatAddress->ProcessId = ProcessId;
    FlatAddress->RequestingProcessor = ProcessorNumber;
    FlatAddress->RequestingNumaNode = (UCHAR)NumaNode;
    FlatAddress->MappedSequenceId = 0;

    //
    // Fill in address range details.
    //

    FlatAddressRange = &TraceStore->FlatAddressRange;
    FlatAddressRange->PreferredBaseAddress = NULL;
    FlatAddressRange->MappedSize.QuadPart = FlatMemoryMap->MappingSize.QuadPart;

    //
    // Attempt to map the memory.
    //

    FlatMemoryMap->BaseAddress = MapViewOfFileExNuma(
        FlatMemoryMap->MappingHandle,
        TraceStore->MapViewOfFileDesiredAccess,
        FlatMemoryMap->FileOffset.HighPart,
        FlatMemoryMap->FileOffset.LowPart,
        FlatMemoryMap->MappingSize.QuadPart,
        NULL,
        TraceStore->NumaNode
    );

    if (!FlatMemoryMap->BaseAddress) {
        TraceStore->LastError = GetLastError();
        __debugbreak();
        return FALSE;
    }

    //
    // The mapping was successful.  Finalize details.
    //

    //
    // Take a local copy of the timestamp.
    //

    TraceStoreQueryPerformanceCounter(TraceStore, &Elapsed, &Timestamp);

    //
    // Copy it to the Prepared timestamp.
    //

    FlatAddress->Timestamp.Prepared.QuadPart = Elapsed.QuadPart;

    //
    // Calculate the elapsed time spent awaiting preparation.
    //

    Elapsed.QuadPart -= FlatAddress->Timestamp.Requested.QuadPart;
    FlatAddress->Elapsed.AwaitingPreparation.QuadPart = Elapsed.QuadPart;

    //
    // "Consume" the memory map now, too.  This is simple in the case of flat
    // memory maps as preparation and consumption aren't two different things;
    // so, just copy the preparation details where applicable.
    //

    FlatAddress->Timestamp.Consumed.QuadPart = Elapsed.QuadPart;
    FlatAddress->Elapsed.AwaitingConsumption.QuadPart = 0;

    //
    // Fill in final FlatAddress details.
    //

    FlatAddress->BaseAddress = FlatMemoryMap->BaseAddress;
    FlatAddress->FulfillingThreadId = ThreadId;
    FlatAddress->FulfillingNumaNode = (UCHAR)NumaNode;

    //
    // Fill in final FlatAddressRange details.
    //

    FlatAddressRange->ActualBaseAddress = (PVOID)(
        RtlOffsetToPointer(
            FlatMemoryMap->BaseAddress,
            FlatMemoryMap->MappingSize.QuadPart
        )
    );

    FlatAddressRange->ActualBaseAddress = FlatMemoryMap->BaseAddress;
    FlatAddressRange->BitCounts.Actual = (
        GetTraceStoreAddressBitCounts(
            FlatMemoryMap->BaseAddress
        )
    );

    //
    // Toggle the flag indicating our flat memory map has loaded.  This allows
    // downstream code to skip the WaitForSingleObject() call on the loading
    // complete event (in the trace context).
    //

    TraceStore->FlatMappingLoaded = TRUE;

    //
    // We're done, return success;
    //

    return TRUE;
}

_Use_decl_annotations_
BOOL
BindNonStreamingReadonlyTraceStoreComplete(
    PTRACE_CONTEXT TraceContext,
    PTRACE_STORE TraceStore
    )
/*++

Routine Description:

    This routine is called when a non-streaming readonly trace store has
    completed its binding.  That is, all memory maps for the address ranges
    that were used during the trace session have been created and mapped and
    any relocation required has been performed.

Arguments:

    TraceContext - Supplies a pointer to a TRACE_CONTEXT structure.

    TraceStore - Supplies a pointer to a TRACE_STORE structure.

Return Value:

    TRUE on success, FALSE on failure.

--*/
{
    return TRUE;
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
