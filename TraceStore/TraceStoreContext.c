/*++

Copyright (c) 2016 Trent Nelson <trent@trent.me>

Module Name:

    TraceStoreContext.c

Abstract:

    This module implements trace context functionality.  Functions
    are provided for initializing a trace context record, as well as
    binding a trace store to a trace context.

--*/

#include "stdafx.h"

_Use_decl_annotations_
BOOL
InitializeTraceContext(
    PRTL Rtl,
    PALLOCATOR Allocator,
    PTRACE_CONTEXT TraceContext,
    PULONG SizeOfTraceContext,
    PTRACE_SESSION TraceSession,
    PTRACE_STORES TraceStores,
    PTP_CALLBACK_ENVIRON ThreadpoolCallbackEnvironment,
    PVOID UserData
    )
/*--

Routine Description:

    This routine initializes a TRACE_CONTEXT structure.  This involves setting
    relevant fields in the structure then binding the context to the trace
    stores.

Arguments:

    Rtl - Supplies a pointer to an RTL structure.

    Allocator - Supplies a pointer to an ALLOCATOR structure.

    TraceContext - Supplies a pointer to a TRACE_CONTEXT structure.

    SizeOfTraceContext - Supplies a pointer to a variable that contains the
        buffer size allocated for the TraceContext parameter.  The actual size
        of the structure will be written to this variable.

    TraceSession - Supplies a pointer to a TRACE_SESSION structure.

    TraceStores - Supplies a pointer to a TRACE_STORES structure to bind the
        trace context to.

    ThreadpoolCallbackEnvironment - Supplies a pointer to a threadpool callback
        environment to use for the trace context.  This threadpool will be used
        to submit various asynchronous thread pool memory map operations.

    UserData - Supplies an optional pointer to user data that can be used by
        a caller to track additional context information per TRACE_CONTEXT
        structure.

Return Value:

    TRUE on success, FALSE on failure.  The required buffer size for the
    TRACE_CONTEXT structure can be obtained by passing in a valid pointer
    for SizeOfTraceContext and NULL for the remaining parameters.

--*/
{
    USHORT Index;
    USHORT StoreIndex;

    //
    // Validate size parameters.
    //

    if (!TraceContext) {
        if (SizeOfTraceContext) {
            *SizeOfTraceContext = sizeof(*TraceContext);
        }
        return FALSE;
    }

    if (!SizeOfTraceContext) {
        return FALSE;
    }

    if (*SizeOfTraceContext < sizeof(*TraceContext)) {
        *SizeOfTraceContext = sizeof(*TraceContext);
        return FALSE;
    }

    //
    // Validate arguments.
    //

    if (!ARGUMENT_PRESENT(Rtl)) {
        return FALSE;
    }

    if (!ARGUMENT_PRESENT(Allocator)) {
        return FALSE;
    }

    if (!ARGUMENT_PRESENT(TraceSession)) {
        return FALSE;
    }

    if (!ARGUMENT_PRESENT(TraceStores)) {
        return FALSE;
    }

    if (!ARGUMENT_PRESENT(ThreadpoolCallbackEnvironment)) {
        return FALSE;
    }

    //
    // Make sure we haven't been called against a readonly trace store.
    //

    if (TraceStores->Flags.Readonly) {
        return FALSE;
    }

    SecureZeroMemory(TraceContext, *SizeOfTraceContext);

    TraceContext->TimerFunction = TraceStoreGetTimerFunction();
    if (!TraceContext->TimerFunction) {
        return FALSE;
    }

    TraceContext->SizeOfStruct = (USHORT)(*SizeOfTraceContext);
    TraceContext->TraceSession = TraceSession;
    TraceContext->TraceStores = TraceStores;
    TraceContext->SequenceId = 1;
    TraceContext->ThreadpoolCallbackEnvironment = ThreadpoolCallbackEnvironment;
    TraceContext->UserData = UserData;
    TraceContext->Allocator = Allocator;

    if (!InitializeTraceStoreTime(Rtl, &TraceContext->Time)) {
        return FALSE;
    }

    FOR_EACH_TRACE_STORE(TraceStores, Index, StoreIndex) {
        TRACE_STORE_DECLS();
        BIND_STORES();
    }

    return TRUE;
}

_Use_decl_annotations_
BOOL
BindTraceStoreToTraceContext(
    PTRACE_STORE TraceStore,
    PTRACE_CONTEXT TraceContext
    )
/*--

Routine Description:

    This routine binds a TraceContext to a TraceStore.  This is required before
    a trace store can be used (i.e. have AllocateRecords called).

Arguments:

    TraceStore - Supplies a pointer to a TRACE_STORE structure.

    TraceContext - Supplies a pointer to a TRACE_CONTEXT structure.

Return Value:

    TRUE if successful, FALSE on failure.

--*/
{
    BOOL Success;
    BOOL IsMetadata;
    DWORD WaitResult;
    HRESULT Result;
    PRTL Rtl;
    TRACE_STORE_METADATA_ID MetadataId;
    PTRACE_STORE_ADDRESS AddressPointer;
    PTRACE_STORE_MEMORY_MAP FirstMemoryMap;
    PTRACE_STORE_EOF Eof;
    PTRACE_STORE_TIME Time;
    PTRACE_STORE_STATS Stats;
    PTRACE_STORE_TOTALS Totals;
    PTRACE_STORE_INFO Info;
    PTRACE_STORE_RELOC Reloc;
    PTRACE_STORE_ALLOCATION Allocation;
    PTRACE_STORE_BITMAP Bitmap;
    PTRACE_STORE_TIME SourceTime;
    PLARGE_INTEGER Elapsed;
    TRACE_STORE_ADDRESS Address;

    //
    // Verify arguments.
    //

    if (!ARGUMENT_PRESENT(TraceStore)) {
        return FALSE;
    }

    if (!ARGUMENT_PRESENT(TraceContext)) {
        return FALSE;
    }

    if (!TraceContext->Allocator) {
        return FALSE;
    }

    Rtl = TraceStore->Rtl;

    //
    // Initialize all of our singly-linked list heads.
    //

    InitializeSListHead(&TraceStore->CloseMemoryMaps);
    InitializeSListHead(&TraceStore->PrepareMemoryMaps);
    InitializeSListHead(&TraceStore->FreeMemoryMaps);
    InitializeSListHead(&TraceStore->NextMemoryMaps);
    InitializeSListHead(&TraceStore->PrefaultMemoryMaps);

    //
    // Create the initial set of memory map records.
    //

    Success = CreateMemoryMapsForTraceStore(TraceStore);

    if (!Success) {
        return FALSE;
    }

    Success = PopFreeTraceStoreMemoryMap(TraceStore, &FirstMemoryMap);

    if (!Success) {
        return FALSE;
    }

    TraceStore->NextMemoryMapAvailableEvent = (
        CreateEvent(
            NULL,
            FALSE,
            FALSE,
            NULL
        )
    );

    if (!TraceStore->NextMemoryMapAvailableEvent) {
        return FALSE;
    }

    TraceStore->AllMemoryMapsAreFreeEvent = (
        CreateEvent(
            NULL,
            FALSE,
            FALSE,
            NULL
        )
    );

    if (!TraceStore->AllMemoryMapsAreFreeEvent) {
        return FALSE;
    }

    TraceStore->PrepareNextMemoryMapWork = CreateThreadpoolWork(
        &PrepareNextTraceStoreMemoryMapCallback,
        TraceStore,
        TraceContext->ThreadpoolCallbackEnvironment
    );

    if (!TraceStore->PrepareNextMemoryMapWork) {
        return FALSE;
    }

    TraceStore->PrefaultFuturePageWork = CreateThreadpoolWork(
        &PrefaultFutureTraceStorePageCallback,
        TraceStore,
        TraceContext->ThreadpoolCallbackEnvironment
    );

    if (!TraceStore->PrefaultFuturePageWork) {
        return FALSE;
    }

    TraceStore->CloseMemoryMapWork = CreateThreadpoolWork(
        &ReleasePrevTraceStoreMemoryMapCallback,
        TraceStore,
        TraceContext->ThreadpoolCallbackEnvironment
    );

    if (!TraceStore->CloseMemoryMapWork) {
        return FALSE;
    }

    TraceStore->TraceContext = TraceContext;

    FirstMemoryMap->FileHandle = TraceStore->FileHandle;
    FirstMemoryMap->MappingSize.QuadPart = TraceStore->MappingSize.QuadPart;

    IsMetadata = IsMetadataTraceStore(TraceStore);

    if (IsMetadata) {

        MetadataId = TraceStore->TraceStoreMetadataId;

        //
        // If we're metadata, go straight to submission of the prepared
        // memory map.
        //

        goto SubmitFirstMemoryMap;
    }

    //
    // Attempt to load the TRACE_STORE_ADDRESS record for the memory map.
    //

    Success = LoadNextTraceStoreAddress(TraceStore, &AddressPointer);

    if (!Success) {

        //
        // If we couldn't load the address, just go straight to submission of
        // the memory map.
        //

        goto SubmitFirstMemoryMap;
    }

    //
    // The remaining logic deals with initializing an address structure for
    // first use.  This is normally dealt with by ConsumeNextTraceStoreMemory-
    // Map() as part of preparation of the next memory map, but we need to do it
    // manually here for the first map.
    //

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

        goto SubmitFirstMemoryMap;
    }

    //
    // Set the Requested timestamp.
    //

    Elapsed = &Address.Timestamp.Requested;
    TraceStoreQueryPerformanceCounter(TraceStore, Elapsed);

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
    }

SubmitFirstMemoryMap:
    PushTraceStoreMemoryMap(&TraceStore->PrepareMemoryMaps, FirstMemoryMap);
    SubmitThreadpoolWork(TraceStore->PrepareNextMemoryMapWork);

    WaitResult = WaitForSingleObject(TraceStore->NextMemoryMapAvailableEvent,
                                     INFINITE);

    if (WaitResult != WAIT_OBJECT_0) {
        return FALSE;
    }

    Success = ConsumeNextTraceStoreMemoryMap(TraceStore);

    if (!Success) {
        return FALSE;
    }

    if (IsMetadata) {

        //
        // Metadata stores only have Info filled out, and it's backed by the
        // :metadatainfo store.
        //

        Info = TraceStoreMetadataIdToInfo(TraceStore, MetadataId);
        Allocation = NULL;
        Bitmap = NULL;
        Reloc = NULL;

    } else {

        Allocation = (PTRACE_STORE_ALLOCATION)(
            TraceStore->AllocationStore->MemoryMap->BaseAddress
        );

        Info = (PTRACE_STORE_INFO)(
            TraceStore->InfoStore->MemoryMap->BaseAddress
        );

        Bitmap = (PTRACE_STORE_BITMAP)(
            TraceStore->BitmapStore->MemoryMap->BaseAddress
        );

        Reloc = (PTRACE_STORE_RELOC)(
            TraceStore->RelocationStore->MemoryMap->BaseAddress
        );
    }

    TraceStore->Allocation = Allocation;
    TraceStore->Bitmap = Bitmap;
    TraceStore->Reloc = Reloc;
    TraceStore->Info = Info;

    Eof = TraceStore->Eof = &Info->Eof;
    Time = TraceStore->Time = &Info->Time;
    Stats = TraceStore->Stats = &Info->Stats;
    Totals = TraceStore->Totals = &Info->Totals;

    if (IsMetadata) {
        PINITIALIZE_TRACE_STORE_METADATA Initializer;

        //
        // Call the metadata's custom initializer.
        //

        Initializer = TraceStoreMetadataIdToInitializer(MetadataId);

        Success = Initializer(TraceStore);
        if (!Success) {
            return FALSE;
        }
    }

    if (TraceStore->IsReadonly) {

        //
        // TraceStore is readonly, try load relocation information.
        //

        if (!LoadTraceStoreRelocationInfo(TraceStore)) {
            return FALSE;
        }

    } else {

        //
        // TraceStore is not readonly.
        //

        //
        // Copy time.
        //

        SourceTime = &TraceContext->Time;
        Rtl->RtlCopyMappedMemory(Time, SourceTime, sizeof(*Time));

        //
        // Zero out stats.
        //

        SecureZeroMemory(Stats, sizeof(*Stats));

        if (!IsMetadata) {

            //
            // Initialize eof and zero totals as long as we're not metadata.
            //

            Eof->EndOfFile.QuadPart = 0;
            SecureZeroMemory(Totals, sizeof(*Totals));

            //
            // Initialize relocation information if present.
            //

            if (TraceStore->HasRelocations) {
                if (!SaveTraceStoreRelocationInfo(TraceStore)) {
                    return FALSE;
                }
            }
        }
    }

    return TRUE;
}

// vim:set ts=8 sw=4 sts=4 tw=80 expandtab                                     :
