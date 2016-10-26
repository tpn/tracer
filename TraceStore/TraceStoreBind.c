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

    if (!CreateMemoryMapsForTraceStore(TraceStore, &FirstMemoryMap)) {
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
    PBIND_COMPLETE BindComplete;
    FILE_STANDARD_INFO FileInfo;
    PTRACE_STORE_MEMORY_MAP MemoryMap;

    //
    // Sanity check we're readonly and we've been called with a metadata store.
    //

    if (!TraceStore->IsReadonly || !TraceStore->IsMetadata) {
        return FALSE;
    }

    IsMetadataInfo = (
        TraceStore->TraceStoreMetadataId == TraceStoreMetadataMetadataInfoId
    );

    MemoryMap = &TraceStore->SingleMemoryMap;
    MemoryMap->FileHandle = TraceStore->FileHandle;

    if (!GetTraceStoreMemoryMapFileInfo(MemoryMap, &FileInfo)) {
        TraceStore->LastError = GetLastError();
        return FALSE;
    }

    MemoryMap->FileOffset.QuadPart = 0;
    MemoryMap->MappingSize.QuadPart = FileInfo.EndOfFile.QuadPart;

    //
    // Make sure the mapping size is under 2GB.
    //

    if (MemoryMap->MappingSize.HighPart != 0) {
        return FALSE;
    }

    MemoryMap->MappingHandle = CreateFileMappingNuma(
        MemoryMap->FileHandle,
        NULL,
        TraceStore->CreateFileMappingProtectionFlags,
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
        TraceStore->MapViewOfFileDesiredAccess,
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

    Non-streaming implies that the entire trace store's contents is mapped into
    memory up-front.  If no other trace stores refer to us, we can simply map
    ourselves in 2GB chunks and be done.

    If other trace stores refer to us, then they'll need to perform relocations
    on any pointers embedded within their store that refer to us if either: a)
    we can't be mapped at the original address we had (the "preferred base
    address"), or b) we had an "preferred address unavailable" events occur when
    we were being written to (indicated by Stats->PreferredAddressUnavailable).

    We create 1 + Stats->PreferredAddressUnavailable maps.  The first map starts
    at byte 0 and extends to the point where we encountered our first preferred
    address unavailable event.  That gets carved off as memory map number one.
    We then carve off the next map starting at the first byte of the new range,
    and scan the :address metadata store looking for the next preferred address
    unavailable event.  This continues until we have maps filled in for all of
    the memory map ranges.

    We then loop through the maps and actually attempt to map the addresses at
    the preferred locations.  If we can't, we adjust the map such that the
    preferred address and the new base address are both captured.

Arguments:

    TraceContext - Supplies a pointer to a TRACE_CONTEXT structure for which
        the given TraceStore is to be bound.

    TraceStore - Supplies a pointer to a TRACE_STORE structure that will be
        bound to the given TraceContext.

Return Value:

    TRUE on success, FALSE on failure.

--*/
{
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
    return FALSE;
}

// vim:set ts=8 sw=4 sts=4 tw=80 expandtab                                     :
