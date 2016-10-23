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
{
/*++

Routine Description:

    This routine binds a trace store to a trace context.  It can be called
    against normal or metadata trace stores and in tracing or readonly mode.

    It is responsible for performing the common "binding" operations needed
    by all trace stores: creating memory maps, creating events, creating
    threadpool work items, creating file mappings and views, etc.

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
    USHORT Index;
    USHORT NumberOfMetadataStores;
    TRACE_STORE_METADATA_ID MetadataId;
    PVOID PreferredBaseAddress;
    PVOID OriginalPreferredBaseAddress;
    PVOID BaseAddress;
    PLARGE_INTEGER Requested;
    PTRACE_STORE_INFO Info;
    PTRACE_STORE TraceStore;
    PTRACE_STORE MetadataStore;
    PTRACE_STORE_MEMORY_MAP MemoryMap;
    PTRACE_STORE_MEMORY_MAP FirstMemoryMap;
    PTRACE_STORE_ADDRESS AddressPointer;
    TRACE_STORE_ADDRESS Address;

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

    IsReadonly = (BOOL)TraceContext->Flags.Readonly;
    IsMetadata = IsMetadataTraceStore(TraceStore);
    HasRelocations = TraceStoreHasRelocations(TraceStore);

    //
    // If we're metadata, go straight to preparation.
    //

    if (IsMetadata) {
        goto PrepareFirstMemoryMap;
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

    Success = PrepareNextTraceStoreMemoryMap(TraceStore, FirstMemoryMap);
    if (!Success) {
        UnmapTraceStoreMemoryMap(FirstMemoryMap);
        return FALSE;
    }

    Success = ConsumeNextTraceStoreMemoryMap(TraceStore, FirstMemoryMap);
    if (!Success) {
        UnmapTraceStoreMemoryMap(FirstMemoryMap);
        return FALSE;
    }

}

_Use_decl_annotations_
BOOL
BindTraceStore(
    PTRACE_CONTEXT TraceContext,
    PTRACE_STORE TraceStore
    )
/*++

Routine Description:

    This routine binds a trace store to a trace context.

Arguments:

    TraceContext - Supplies a pointer to a TRACE_CONTEXT structure.

    TraceStore - Supplies a pointer to a TRACE_STORE structure.

Return Value:

    TRUE on success, FALSE on failure.

--*/
{
    return FALSE;
}

_Use_decl_annotations_
BOOL
BindTraceStore(
    PTRACE_CONTEXT TraceContext,
    PTRACE_STORE TraceStore
    )
/*++

Routine Description:

    This routine binds a trace store to a trace context.

Arguments:

    TraceContext - Supplies a pointer to a TRACE_CONTEXT structure.

    TraceStore - Supplies a pointer to a TRACE_STORE structure.

Return Value:

    TRUE on success, FALSE on failure.

--*/
{
    return FALSE;
}

_Use_decl_annotations_
BOOL
BindMetadataInfoStore(
    PTRACE_CONTEXT TraceContext,
    PTRACE_STORE MetadataInfoStore
    )
/*++

Routine Description:

    This routine binds a metadata info metadata trace store to a trace context.
    This metadata store acts as a backing for the other metadata store's static
    TRACE_STORE_INFO structures, and thus, is called first when a main trace
    store is being bound.

Arguments:

    TraceContext - Supplies a pointer to a TRACE_CONTEXT structure to bind the
        metadata info metadata store to.

    MetadataInfoStore - Supplies a pointer to a metadata info trace store.

Return Value:

    TRUE on success, FALSE on failure.

--*/
{
    BOOL IsReadonly;
    BOOL Success;
    USHORT Index;
    USHORT NumberOfMetadataStores;
    TRACE_STORE_METADATA_ID MetadataId;
    PVOID BaseAddress;
    PTRACE_STORE_INFO Info;
    PTRACE_STORE TraceStore;
    PTRACE_STORE MetadataStore;
    PPTRACE_STORE MetadataStorePointer;
    PTRACE_STORE_MEMORY_MAP MemoryMap;
    PTRACE_STORE_METADATA_INFO MetadataInfo;
    ULARGE_INTEGER NumberOfRecords = { 1 };
    ULARGE_INTEGER RecordSize = { sizeof(TRACE_STORE_METADATA_INFO) };

    //
    // Ensure we've been passed a metadata info metadata trace store.
    //

    if (!IsMetadataTraceStore(MetadataInfoStore)) {
        return FALSE;
    }

    MetadataId = MetadataInfoStore->TraceStoreMetadataId;
    if (MetadataId != TraceStoreMetadataMetadataInfoId) {
        return FALSE;
    }

    //
    // Make sure the trace store linkage is correct.  The trace store we point
    // to should have its MetadataInfoStore field point to us.
    //

    TraceStore = MetadataInfoStore->TraceStore;
    if (TraceStore->MetadataInfoStore != MetadataInfoStore) {
        return FALSE;
    }

    Success = BindMetadataStore(TraceContext, MetadataInfoStore);
    if (!Success) {
        return FALSE;
    }

    //
    // Allocate space for a TRACE_STORE_METADATA_INFO structure.  Enumerate
    // through the metadata stores (including this metadata info store) and
    // wire up each store's TRACE_STORE_INFO-related pointers to be backed by
    // their relevant offset in the TRACE_STORE_METADATA_INFO structure.
    //

    BaseAddress = MetadataInfoStore->AllocateRecords(
        TraceContext,
        TraceStore,
        &RecordSize,
        &NumberOfRecords
    );

    if (!BaseAddress) {
        return FALSE;
    }

    MetadataInfo = (PTRACE_STORE_METADATA_INFO)BaseAddress;
    MetadataStorePointer = &TraceStore->MetadataInfoStore;

    //
    // Subtract one to account for the normal trace store.
    //

    NumberOfMetadataStores = (USHORT)(
        TraceContext->TraceStores->ElementsPerTraceStore - 1
    );

    for (Index = 0; Index < NumberOfMetadataStores; Index++) {
        Info = (((PTRACE_STORE_INFO)MetadataInfo) + Index);

        //
        // N.B.: We abuse the fact that the trace store's metadata store
        //       pointers are laid out consecutively (and contiguously) in
        //       the same order as implied by their TraceStoreMetadataStoreId.
        //       That allows us to use *MetadataStorePointer++ below.
        //

        MetadataStore = *MetadataStorePointer++;
        MetadataStore->Info = Info;
        MetadataStore->Eof = &Info->Eof;
        MetadataStore->Time = &Info->Time;
        MetadataStore->Stats = &Info->Stats;
        MetadataStore->Totals = &Info->Totals;
        MetadataStore->Traits = &Info->Traits;
    }

    //
    //

    //
    // If we're not readonly, initialize end of file and time.
    //

    IsReadonly = (BOOL)TraceContext->Flags.Readonly;

    if (!IsReadonly) {
        Info = (PTRACE_STORE_INFO)MetadataInfo;
        MemoryMap->
        Info->Eof.EndOfFile.QuadPart = MemoryMap->MappingSize.QuadPart;
        __movsb((PBYTE)&Info->Time,
                (PBYTE)&TraceContext->Time,
                sizeof(Info->Time));
    }

    Success = TRUE;
    goto End;

Error:
    Success = FALSE;

    if (MemoryMap->MappingHandle) {
        CloseHandle(MemoryMap->MappingHandle);
        MemoryMap->MappingHandle = NULL;
    }

End:
    return Success;
}

// vim:set ts=8 sw=4 sts=4 tw=80 expandtab                                     :
