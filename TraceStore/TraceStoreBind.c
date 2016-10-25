/*++

Copyright (c) 2016 Trent Nelson <trent@trent.me>

Module Name:

    TraceStoreBind.c

Abstract:

    This module implements functionality to bind a trace store to a trace
    context.  Binding behavior is slightly different between normal trace
    stores and metadata trace stores, as well as between tracing sessions
    and readonly ones.

    Functions are provided to bind common trace store (BindStore()), bind
    metadata info trace stores (BindMetadataInfoStore()), bind the remaining
    metadata stores (BindRemainingMetadataStores()), and, finally, binding
    the actual trace store (BindTraceStore()).

    These routines are called via threadpool callbacks.

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
    // Initialize aliases.
    //

    Traits = *TraceStore->pTraits;
    IsReadonly = (BOOL)TraceContext->Flags.Readonly;
    IsMetadata = IsMetadataTraceStore(TraceStore);
    HasRelocations = TraceStoreHasRelocations(TraceStore);
    BindComplete = TraceStore->BindComplete;

    //
    // Check to see if we're metadata and dispatch to our custom binder if
    // we're readonly, otherwise, jump straight to preparation of the first
    // memory map.
    //

    if (IsMetadata) {
        if (IsReadonly) {
            return BindMetadataStoreReadonly(TraceContext, TraceStore);
        }
        MetadataId = TraceStore->TraceStoreMetadataId;
        goto PrepareFirstMemoryMap;
    }

    if (IsReadonly) {
        __debugbreak();
    }

    //
    // Attempt to load the address record for this store.
    //

    Success = LoadNextTraceStoreAddress(TraceStore, &AddressPointer);
    if (!Success) {
        if (IsReadonly && HasRelocations) {

            //
            // Not being able to read the :address metadata store if we're
            // readonly and have relocations is an unrecoverable error.
            //

            return FALSE;
        }

        //
        // (Should we continue if we can't load the first :address?)
        //

        __debugbreak();
        goto PrepareFirstMemoryMap;
    }

    if (IsReadonly) {
        goto PrepareFirstMemoryMap;
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

        //
        // Ignore and go straight to submission.
        //

        __debugbreak();
        goto PrepareFirstMemoryMap;
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
    if (SUCCEEDED(Result)) {

        //
        // Update the memory map to point at the address struct.
        //

        FirstMemoryMap->pAddress = AddressPointer;

    } else {

        //
        // XXX: failure?
        //

        __debugbreak();
    }

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

    if (!IsReadonly && IsMetadata && IsSingleRecord(Traits)) {

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

    if (!IsReadonly) {
        CopyTraceStoreTime(TraceContext, TraceStore);
    }

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

// vim:set ts=8 sw=4 sts=4 tw=80 expandtab                                     :
