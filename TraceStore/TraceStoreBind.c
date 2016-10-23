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
    PVOID BaseAddress;
    PLARGE_INTEGER Requested;
    PTRACE_STORE_MEMORY_MAP FirstMemoryMap;
    PTRACE_STORE_ADDRESS AddressPointer;
    TRACE_STORE_ADDRESS Address;
    ULARGE_INTEGER RecordSize;
    ULARGE_INTEGER NumberOfRecords = { 1 };

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

    //
    // If we're metadata, go straight to preparation.
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

    if (!IsReadonly) {
        CopyTraceStoreTime(TraceContext, TraceStore);

        if (IsMetadata) {
            if (IsSingleRecord(Traits)) {

                //
                // Initialize single record metadata by doing a single
                // allocation that matches the size of the metadata record.
                // This ensures the Info->Eof (end of file) and Info->Totals
                // metadata is correct.
                //

                RecordSize.QuadPart = (
                    TraceStoreMetadataIdToRecordSize(MetadataId)
                );
                BaseAddress = TraceStore->AllocateRecords(
                    TraceContext,
                    TraceStore,
                    &RecordSize,
                    &NumberOfRecords
                );
                if (!BaseAddress) {
                    goto Error;
                }
            }
        }
    }

    //
    // If we have a bind complete callback, call it now.
    //

    if (TraceStore->BindComplete) {
        Success = TraceStore->BindComplete(
            TraceContext,
            TraceStore,
            FirstMemoryMap
        );
        if (!Success) {
            goto Error;
        }
    }

    return TRUE;

Error:
    UnmapTraceStoreMemoryMap(FirstMemoryMap);
    return FALSE;
}

// vim:set ts=8 sw=4 sts=4 tw=80 expandtab                                     :
