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
    PTRACE_CONTEXT TraceContext,
    PULONG SizeOfTraceContext,
    PTRACE_SESSION TraceSession,
    PTRACE_STORES TraceStores,
    PTP_CALLBACK_ENVIRON ThreadpoolCallbackEnvironment,
    PVOID UserData
    )
/*--

Routine Description:

    This routine initializes an allocated TRACE_CONTEXT record.

Arguments:

    TBD.

Return Value:

    TRUE on success, FALSE on failure.

--*/
{
    USHORT Index;
    USHORT StoreIndex;

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

    if (!Rtl) {
        return FALSE;
    }

    if (!TraceSession) {
        return FALSE;
    }

    if (!TraceStores) {
        return FALSE;
    }

    if (!ThreadpoolCallbackEnvironment) {
        return FALSE;
    }

    TraceContext->TimerFunction = TraceStoreGetTimerFunction();
    if (!TraceContext->TimerFunction) {
        return FALSE;
    }

    TraceContext->Size = *SizeOfTraceContext;
    TraceContext->TraceSession = TraceSession;
    TraceContext->TraceStores = TraceStores;
    TraceContext->SequenceId = 1;
    TraceContext->ThreadpoolCallbackEnvironment = ThreadpoolCallbackEnvironment;
    TraceContext->UserData = UserData;

    TraceContext->HeapHandle = HeapCreate(0,
                                          InitialTraceContextHeapSize,
                                          MaximumTraceContextHeapSize);

    if (!TraceContext->HeapHandle) {
        return FALSE;
    }

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
    PTRACE_STORE_ADDRESS AddressPointer;
    PTRACE_STORE_MEMORY_MAP FirstMemoryMap;
    PTRACE_STORE_EOF Eof;
    PTRACE_STORE_TIME Time;
    PTRACE_STORE_STATS Stats;
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

    if (!TraceContext->HeapHandle) {
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

    Success = CreateMemoryMapsForTraceStore(
        TraceStore,
        TraceContext,
        InitialFreeMemoryMaps
    );

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

    //
    // If we're metadata, go straight to submission of the prepared memory map.
    //

    IsMetadata = IsMetadataTraceStore(TraceStore);

    if (IsMetadata) {
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
        // Metadata stores just use the TraceStore-backed, non-memory-mapped
        // TRACE_STORE_INFO struct.
        //

        TraceStore->pInfo = &TraceStore->Info;

    } else {

        PTRACE_STORE AllocationStore;
        PTRACE_STORE InfoStore;

        AllocationStore = TraceStore->AllocationStore;
        TraceStore->pAllocation = (
            (PTRACE_STORE_ALLOCATION)AllocationStore->MemoryMap->BaseAddress
        );

        InfoStore = TraceStore->InfoStore;
        TraceStore->pInfo = (
            (PTRACE_STORE_INFO)InfoStore->MemoryMap->BaseAddress
        );

    }

    Eof = TraceStore->pEof = &TraceStore->pInfo->Eof;
    Time = TraceStore->pTime = &TraceStore->pInfo->Time;
    Stats = TraceStore->pStats = &TraceStore->pInfo->Stats;

    if (!TraceStore->IsReadonly) {
        PTRACE_STORE_TIME SourceTime = &TraceContext->Time;

        //
        // Initialize Eof.
        //

        Eof->EndOfFile.QuadPart = 0;

        //
        // Copy time.
        //

        Rtl->RtlCopyMappedMemory(Time, SourceTime, sizeof(*Time));

        //
        // Zero out stats.
        //

        SecureZeroMemory(Stats, sizeof(*Stats));
    }

    return TRUE;
}

// vim:set ts=8 sw=4 sts=4 tw=80 expandtab                                     :
