/*++

Copyright (c) 2016 Trent Nelson <trent@trent.me>

Module Name:

    TraceStoreMetadata.c

Abstract:

    This module implements functionality specific to metadata trace stores,
    such as initialization and metadata ID mapping to other data structures.

--*/

#include "stdafx.h"

CONST ULONG TraceStoreMetadataRecordSizes[] = {
    sizeof(TRACE_STORE_METADATA_INFO),
    sizeof(TRACE_STORE_ALLOCATION),
    sizeof(TRACE_STORE_RELOC),
    sizeof(TRACE_STORE_ADDRESS),
    sizeof(TRACE_STORE_ADDRESS_RANGE),
    sizeof(TRACE_STORE_ALLOCATION_TIMESTAMP),
    sizeof(TRACE_STORE_ALLOCATION_TIMESTAMP_DELTA),
    sizeof(TRACE_STORE_SYNC),
    sizeof(TRACE_STORE_INFO)
};

CONST PCTRACE_STORE_TRAITS MetadataStoreTraits[] = {
    &MetadataInfoStoreTraits,
    &AllocationStoreTraits,
    &RelocationStoreTraits,
    &AddressStoreTraits,
    &AddressRangeStoreTraits,
    &AllocationTimestampStoreTraits,
    &AllocationTimestampDeltaStoreTraits,
    &SynchronizationStoreTraits,
    &InfoStoreTraits
};

CONST PBIND_COMPLETE TraceStoreMetadataBindCompletes[] = {
    MetadataInfoMetadataBindComplete,
    AllocationMetadataBindComplete,
    RelocationMetadataBindComplete,
    AddressMetadataBindComplete,
    AddressRangeMetadataBindComplete,
    AllocationTimestampMetadataBindComplete,
    AllocationTimestampDeltaMetadataBindComplete,
    SynchronizationMetadataBindComplete,
    InfoMetadataBindComplete
};

_Use_decl_annotations_
PCTRACE_STORE_TRAITS
TraceStoreMetadataIdToTraits(
    TRACE_STORE_METADATA_ID TraceStoreMetadataId
    )
{
    USHORT Index;

    Index = TraceStoreMetadataIdToArrayIndex(TraceStoreMetadataId);
    return MetadataStoreTraits[Index];
}

_Use_decl_annotations_
ULONG
TraceStoreMetadataIdToRecordSize(
    TRACE_STORE_METADATA_ID TraceStoreMetadataId
    )
{
    USHORT Index;

    Index = TraceStoreMetadataIdToArrayIndex(TraceStoreMetadataId);
    return TraceStoreMetadataRecordSizes[Index];
}

_Use_decl_annotations_
PBIND_COMPLETE
TraceStoreMetadataIdToBindComplete(
    TRACE_STORE_METADATA_ID TraceStoreMetadataId
    )
{
    USHORT Index;

    Index = TraceStoreMetadataIdToArrayIndex(TraceStoreMetadataId);
    return TraceStoreMetadataBindCompletes[Index];
}

_Use_decl_annotations_
PTRACE_STORE_INFO
TraceStoreMetadataIdToInfo(
    PTRACE_STORE TraceStore,
    TRACE_STORE_METADATA_ID TraceStoreMetadataId
    )
{
    USHORT Index;
    PVOID BaseAddress;
    PTRACE_STORE MetadataInfoStore;
    PTRACE_STORE_INFO Info;
    PTRACE_STORE_METADATA_INFO MetadataInfo;

    MetadataInfoStore = TraceStore->MetadataInfoStore;
    BaseAddress = MetadataInfoStore->MemoryMap->BaseAddress;
    MetadataInfo = (PTRACE_STORE_METADATA_INFO)BaseAddress;
    Index = TraceStoreMetadataIdToArrayIndex(TraceStoreMetadataId);

    Info = (((PTRACE_STORE_INFO)MetadataInfo) + Index);
    return Info;
}

_Use_decl_annotations_
PTRACE_STORE
TraceStoreMetadataIdToStore(
    PTRACE_STORE TraceStore,
    TRACE_STORE_METADATA_ID TraceStoreMetadataId
    )
{
    USHORT Index;
    PTRACE_STORE MetadataInfoStore;

    MetadataInfoStore = TraceStore->MetadataInfoStore;
    Index = TraceStoreMetadataIdToArrayIndex(TraceStoreMetadataId);
    return (MetadataInfoStore + Index);
}

_Use_decl_annotations_
VOID
InitializeMetadataFromRecordSize(
    PTRACE_STORE MetadataStore
    )
{
    ULONG RecordSize;
    TRACE_STORE_METADATA_ID MetadataId;
    PTRACE_STORE_EOF Eof;
    PTRACE_STORE_TOTALS Totals;

    if (MetadataStore->IsReadonly) {
        __debugbreak();
    }

    //
    // Set Eof to the record size and fake a similarly-sized allocation
    // in the totals struct.
    //

    MetadataId = MetadataStore->TraceStoreMetadataId;
    RecordSize = TraceStoreMetadataIdToRecordSize(MetadataId);

    Eof = MetadataStore->Eof;
    Totals = MetadataStore->Totals;

    Eof->EndOfFile.QuadPart = RecordSize;
    Totals->NumberOfAllocations.QuadPart = 1;
    Totals->AllocationSize.QuadPart = RecordSize;
}

_Use_decl_annotations_
BOOL
MetadataInfoMetadataBindComplete(
    PTRACE_CONTEXT TraceContext,
    PTRACE_STORE MetadataInfoStore,
    PTRACE_STORE_MEMORY_MAP FirstMemoryMap
    )
/*++

Routine Description:

    This is the bind complete callback routine for :MetadataInfo stores.  It
    is responsible for wiring up the TRACE_STORE_INFO structures for the other
    metadata stores.

Arguments:

    TraceContext - Supplies a pointer to a TRACE_CONTEXT structure.

    MetadataInfoStore - Supplies a pointer to the :metadatainfo TRACE_STORE.

    FirstMemoryMap - Supplies a pointer to a TRACE_STORE_MEMORY_MAP structure.

Return Value:

    TRUE on success, FALSE on failure.

--*/
{
    USHORT Index;
    PVOID BaseAddress;
    PTRACE_STORE_INFO Info;
    PTRACE_STORE TraceStore;
    PTRACE_STORE MetadataStore;
    PPTRACE_STORE MetadataStorePointer;
    PTRACE_STORE_METADATA_INFO MetadataInfo;

    BaseAddress = FirstMemoryMap->BaseAddress;
    TraceStore = MetadataInfoStore->TraceStore;
    MetadataInfo = (PTRACE_STORE_METADATA_INFO)BaseAddress;
    MetadataStorePointer = &TraceStore->MetadataInfoStore;

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

    return TRUE;
}

_Use_decl_annotations_
BOOL
RelocationMetadataBindComplete(
    PTRACE_CONTEXT TraceContext,
    PTRACE_STORE RelocationStore,
    PTRACE_STORE_MEMORY_MAP FirstMemoryMap
    )
/*++

Routine Description:

    This is the bind complete callback routine for :Relocation stores.
    It calls LoadTraceStoreRelocationInfo() if this is a readonly session,
    or SaveTraceStoreRelocationInfo() if this is a normal writable session.

Arguments:

    TraceContext - Supplies a pointer to a TRACE_CONTEXT structure.

    RelocationStore - Supplies a pointer to the :Relocation TRACE_STORE.

    FirstMemoryMap - Supplies a pointer to a TRACE_STORE_MEMORY_MAP structure.

Return Value:

    TRUE on success, FALSE on failure.

--*/
{
    BOOL Success;
    PTRACE_STORE TraceStore;

    TraceStore = RelocationStore->TraceStore;

    if (TraceStore->IsReadonly) {
        Success = LoadTraceStoreRelocationInfo(TraceStore);
    } else {
        Success = SaveTraceStoreRelocationInfo(TraceStore);
    }

    return Success;
}

_Use_decl_annotations_
BOOL
AllocationMetadataBindComplete(
    PTRACE_CONTEXT TraceContext,
    PTRACE_STORE AllocationStore,
    PTRACE_STORE_MEMORY_MAP FirstMemoryMap
    )
/*++

Routine Description:

    This is the bind complete callback routine for :Allocation metadata.
    It initializes the TraceStore->Allocation pointer.

Arguments:

    TraceContext - Supplies a pointer to a TRACE_CONTEXT structure.

    AllocationStore - Supplies a pointer to the :allocation TRACE_STORE.

    FirstMemoryMap - Supplies a pointer to a TRACE_STORE_MEMORY_MAP structure.

Return Value:

    TRUE on success, FALSE on failure.

--*/
{
    PTRACE_STORE TraceStore;

    TraceStore = AllocationStore->TraceStore;
    TraceStore->Allocation = (PTRACE_STORE_ALLOCATION)(
        TraceStore->AllocationStore->MemoryMap->BaseAddress
    );

    return TRUE;
}

_Use_decl_annotations_
BOOL
AddressMetadataBindComplete(
    PTRACE_CONTEXT TraceContext,
    PTRACE_STORE AddressStore,
    PTRACE_STORE_MEMORY_MAP FirstMemoryMap
    )
/*++

Routine Description:

    This is the bind complete callback routine for :Address metadata.  When
    readonly, it initializes the TraceStore->Address pointer to the base
    address of the first memory map.

Arguments:

    TraceContext - Supplies a pointer to a TRACE_CONTEXT structure.

    AddressStore - Supplies a pointer to the :Address TRACE_STORE.

    FirstMemoryMap - Supplies a pointer to a TRACE_STORE_MEMORY_MAP structure.

Return Value:

    TRUE on success, FALSE on failure.

--*/
{
    PTRACE_STORE TraceStore;

    TraceStore = AddressStore->TraceStore;

    if (!TraceStore->IsReadonly) {
        return TRUE;
    }

    //
    // Point TraceStore->Address at the base of the address store's memory map.
    //

    TraceStore->Address = (PTRACE_STORE_ADDRESS)(
        TraceStore->AddressStore->MemoryMap->BaseAddress
    );

    //
    // Load the number of addresses that were allocated.
    //

    TraceStore->NumberOfAddresses.QuadPart = (
        AddressStore->Totals->NumberOfAllocations.QuadPart
    );

    return TRUE;
}

_Use_decl_annotations_
BOOL
AllocationTimestampMetadataBindComplete(
    PTRACE_CONTEXT TraceContext,
    PTRACE_STORE AllocationTimestampStore,
    PTRACE_STORE_MEMORY_MAP FirstMemoryMap
    )
/*++

Routine Description:

    When readonly, this routine initializes TraceStore->AllocationTimestamp
    to point at the base address of the :AllocationTimestamp metadata store.

Arguments:

    TraceContext - Supplies a pointer to a TRACE_CONTEXT structure.

    AllocationTimestampStore - Supplies a pointer to the allocation timestamp
        metadata store.

    FirstMemoryMap - Supplies a pointer to a TRACE_STORE_MEMORY_MAP structure.

Return Value:

    TRUE on success, FALSE on failure.

--*/
{
    PTRACE_STORE TraceStore;

    TraceStore = AllocationTimestampStore->TraceStore;

    if (!TraceStore->IsReadonly) {
        return TRUE;
    }

    //
    // Point TraceStore->AllocationTimestamp at the base of the allocation
    // timestamp store's memory map.
    //

    TraceStore->AllocationTimestamp = (PTRACE_STORE_ALLOCATION_TIMESTAMP)(
        TraceStore->AllocationTimestampStore->MemoryMap->BaseAddress
    );

    return TRUE;
}

_Use_decl_annotations_
BOOL
AllocationTimestampDeltaMetadataBindComplete(
    PTRACE_CONTEXT TraceContext,
    PTRACE_STORE AllocationTimestampDeltaStore,
    PTRACE_STORE_MEMORY_MAP FirstMemoryMap
    )
/*++

Routine Description:

    When readonly, this routine initializes TraceStore->AllocationTimestampDelta
    to point at the base address of the :AllocationTimestampDelta metadata
    store's base address.

Arguments:

    TraceContext - Supplies a pointer to a TRACE_CONTEXT structure.

    AllocationTimestampDeltaStore - Supplies a pointer to the allocation
        timestamp delta metadata store.

    FirstMemoryMap - Supplies a pointer to a TRACE_STORE_MEMORY_MAP structure.

Return Value:

    TRUE on success, FALSE on failure.

--*/
{
    PTRACE_STORE TraceStore;

    TraceStore = AllocationTimestampDeltaStore->TraceStore;

    if (!TraceStore->IsReadonly) {
        return TRUE;
    }

    //
    // Point TraceStore->AllocationTimestampDelta at the base of the allocation
    // timestamp delta store's memory map.
    //

    TraceStore->AllocationTimestampDelta = (
        (PTRACE_STORE_ALLOCATION_TIMESTAMP_DELTA)(
            TraceStore->AllocationTimestampDeltaStore->MemoryMap->BaseAddress
        )
    );

    return TRUE;
}

_Use_decl_annotations_
BOOL
AddressRangeMetadataBindComplete(
    PTRACE_CONTEXT TraceContext,
    PTRACE_STORE AddressRangeStore,
    PTRACE_STORE_MEMORY_MAP FirstMemoryMap
    )
/*++

Routine Description:

    This is the bind complete callback routine for :AddressRange metadata.
    In normal tracing mode, this routine has no effect.  In readonly mode,
    it is responsible for wiring up the TraceStore->AddressRange pointer
    to the base memory map address of the :AddressRange metadata store.

Arguments:

    TraceContext - Supplies a pointer to a TRACE_CONTEXT structure.

    AddressRangeStore - Supplies a pointer to the :AddressRange TRACE_STORE.

    FirstMemoryMap - Supplies a pointer to a TRACE_STORE_MEMORY_MAP structure.

Return Value:

    TRUE on success, FALSE on failure.

--*/
{
    PTRACE_STORE TraceStore;

    //
    // Initialize trace store alias.
    //

    TraceStore = AddressRangeStore->TraceStore;

    //
    // If we're writable (non-readonly), the initialization of the first
    // address range structure occurs when PrepareNextTraceStoreMemoryMap()
    // calls RegisterNewTraceStoreAddressRange(), so we don't have to do
    // anything here in this bind complete method.
    //

    if (!TraceStore->IsReadonly) {
        return TRUE;
    }

    //
    // Wire up the TraceStore->AddressRange pointer to the base address of the
    // address range store's memory map.
    //

    TraceStore->AddressRange = (PTRACE_STORE_ADDRESS_RANGE)(
        TraceStore->AddressRangeStore->MemoryMap->BaseAddress
    );

    //
    // Load the number of address ranges that were originally allocated by the
    // trace store.
    //

    TraceStore->NumberOfAddressRanges.QuadPart = (
        TraceStore->AddressRangeStore->Totals->NumberOfAllocations.QuadPart
    );

    return TRUE;
}

_Use_decl_annotations_
BOOL
SynchronizationMetadataBindComplete(
    PTRACE_CONTEXT TraceContext,
    PTRACE_STORE SynchronizationStore,
    PTRACE_STORE_MEMORY_MAP FirstMemoryMap
    )
/*++

Routine Description:

    This is the bind complete callback routine for :Synchronization metadata.
    It is responsible for wiring up the TraceStore->Sync pointer to the base
    memory map address of the :Synchronization metadata store.  It then
    initializes applicable critical sections associated with the
    TRACE_STORE_SYNC structure.

Arguments:

    TraceContext - Supplies a pointer to a TRACE_CONTEXT structure.

    SynchronizationStore - Supplies a pointer to the synchronization trace
        store.

    FirstMemoryMap - Supplies a pointer to a TRACE_STORE_MEMORY_MAP structure.

Return Value:

    TRUE on success, FALSE on failure.

--*/
{
    BOOL Success;
    DWORD SpinCount;
    TRACE_STORE_TRAITS Traits;
    PTRACE_STORE TraceStore;
    PTRACE_STORE_SYNC Sync;
    PTRACER_CONFIG TracerConfig;
    PTRACER_RUNTIME_PARAMETERS RuntimeParameters;

    //
    // Initialize trace store alias.
    //

    TraceStore = SynchronizationStore->TraceStore;

    //
    // Wire up the TraceStore->Sync pointer to the base address of the trace
    // store.
    //

    Sync = TraceStore->Sync = (PTRACE_STORE_SYNC)(
        TraceStore->SynchronizationStore->MemoryMap->BaseAddress
    );

    //
    // If we're readonly or the trace store has already been initialized, we're
    // done.
    //

    if (TraceStore->IsReadonly || Sync->Flags.Initialized) {
        return TRUE;
    }

    Traits = *TraceStore->pTraits;

    //
    // Initialize the applicable critical sections.
    //

    TracerConfig = TraceContext->TracerConfig;
    RuntimeParameters = &TracerConfig->RuntimeParameters;
    SpinCount = (
        RuntimeParameters->ConcurrentAllocationsCriticalSectionSpinCount
    );

    if (IsConcurrentDataStructure(Traits)) {
        InitializeSRWLock(&Sync->SRWLock);
    }

    if (HasConcurrentAllocations(Traits)) {

        Success = InitializeCriticalSectionAndSpinCount(
            &Sync->AllocationCriticalSection,
            SpinCount
        );

        if (Success) {
            Sync->Flags.AllocationCriticalSection = TRUE;
        }
    }

    if (IsPeriodic(Traits)) {
        Success = InitializeCriticalSectionAndSpinCount(
            &Sync->CallbackCriticalSection,
            SpinCount
        );

        if (Success) {
            Sync->Flags.CallbackCriticalSection = TRUE;
        }
    }

    if (Success) {
        Sync->Flags.Initialized = TRUE;
    }

    return Success;
}

_Use_decl_annotations_
BOOL
InfoMetadataBindComplete(
    PTRACE_CONTEXT TraceContext,
    PTRACE_STORE InfoStore,
    PTRACE_STORE_MEMORY_MAP FirstMemoryMap
    )
/*++

Routine Description:

    This is the bind complete callback routine for :Info stores.

    It calls LoadTraceStoreTraits() if this is a readonly session, otherwise,
    it calls SaveTraceStoreTraits() if this is a normal writable session.

Arguments:

    TraceContext - Supplies a pointer to a TRACE_CONTEXT structure.

    InfoStore - Supplies a pointer to the :info metadata trace store.

    FirstMemoryMap - Supplies a pointer to a TRACE_STORE_MEMORY_MAP structure.

Return Value:

    TRUE on success, FALSE on failure.

--*/
{
    BOOL Success;
    PTRACE_STORE TraceStore;

    TraceStore = InfoStore->TraceStore;

    //
    // Wire up the convenience pointers.
    //

    TraceStore->Info = (PTRACE_STORE_INFO)(
        TraceStore->InfoStore->MemoryMap->BaseAddress
    );

    TraceStore->Eof = &TraceStore->Info->Eof;
    TraceStore->Time = &TraceStore->Info->Time;
    TraceStore->Stats = &TraceStore->Info->Stats;
    TraceStore->Totals = &TraceStore->Info->Totals;
    TraceStore->Traits = &TraceStore->Info->Traits;

    //
    // Load traits if we're readonly, save traits if not.
    //

    if (TraceStore->IsReadonly) {
        Success = LoadTraceStoreTraits(TraceStore);
    } else {
        Success = SaveTraceStoreTraits(TraceStore);
    }

    return Success;
}

// vim:set ts=8 sw=4 sts=4 tw=80 expandtab                                     :
