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
    sizeof(TRACE_STORE_BITMAP),
    sizeof(TRACE_STORE_INFO)
};

CONST PINITIALIZE_TRACE_STORE_METADATA TraceStoreMetadataInitializers[] = {
    InitializeMetadataInfoMetadata,     // MetadataInfo
    InitializeAllocationMetadata,       // Allocation
    InitializeRelocationMetadata,       // Relocation
    InitializeAddressMetadata,          // Address
    InitializeBitmapMetadata,           // Bitmap
    InitializeInfoMetadata              // Info
};

PTRACE_STORE_TRAITS MetadataStoreTraits[] = {
    &MetadataInfoStoreTraits,
    &AllocationStoreTraits,
    &RelocationStoreTraits,
    &AddressStoreTraits,
    &BitmapStoreTraits,
    &InfoStoreTraits
};

CONST PBIND_COMPLETE TraceStoreMetadataBindCompletes[] = {
    TraceStoreMetadataMetadataInfoBindComplete,     // MetadataInfo
    TraceStoreMetadataAllocationBindComplete,       // Allocation
    TraceStoreMetadataRelocationBindComplete,       // Relocation
    TraceStoreMetadataAddressBindComplete,          // Address
    TraceStoreMetadataBitmapBindComplete,           // Bitmap
    TraceStoreMetadataInfoBindComplete              // Info
};

_Use_decl_annotations_
PTRACE_STORE_TRAITS
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
PINITIALIZE_TRACE_STORE_METADATA
TraceStoreMetadataIdToInitializer(
    TRACE_STORE_METADATA_ID TraceStoreMetadataId
    )
{
    USHORT Index;

    Index = TraceStoreMetadataIdToArrayIndex(TraceStoreMetadataId);
    return TraceStoreMetadataInitializers[Index];
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
BOOL
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
        return FALSE;
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

    return TRUE;
}

_Use_decl_annotations_
BOOL
ZeroInitializeMetadata(
    PTRACE_STORE MetadataStore
    )
{
    PTRACE_STORE_EOF Eof;
    PTRACE_STORE_TOTALS Totals;

    if (MetadataStore->IsReadonly) {
        __debugbreak();
        return FALSE;
    }

    Eof = MetadataStore->Eof;
    Totals = MetadataStore->Totals;

    Eof->EndOfFile.QuadPart = 0;
    Totals->NumberOfAllocations.QuadPart = 0;
    Totals->AllocationSize.QuadPart = 0;

    return TRUE;
}

_Use_decl_annotations_
BOOL
InitializeMetadataInfoMetadata(
    PTRACE_STORE MetadataStore
    )
{

    if (!MetadataStore->NoPrefaulting) {
        __debugbreak();
        return FALSE;
    }
    if (!MetadataStore->NoTruncate) {
        __debugbreak();
        return FALSE;
    }

    return InitializeMetadataFromRecordSize(MetadataStore);
}

_Use_decl_annotations_
BOOL
InitializeAllocationMetadata(
    PTRACE_STORE MetadataStore
    )
{
    return ZeroInitializeMetadata(MetadataStore);
}

_Use_decl_annotations_
BOOL
InitializeRelocationMetadata(
    PTRACE_STORE MetadataStore
    )
{
    return ZeroInitializeMetadata(MetadataStore);
}

_Use_decl_annotations_
BOOL
InitializeAddressMetadata(
    PTRACE_STORE MetadataStore
    )
{
    return ZeroInitializeMetadata(MetadataStore);
}

_Use_decl_annotations_
BOOL
InitializeBitmapMetadata(
    PTRACE_STORE MetadataStore
    )
{
    return ZeroInitializeMetadata(MetadataStore);
}

_Use_decl_annotations_
BOOL
InitializeInfoMetadata(
    PTRACE_STORE MetadataStore
    )
{

    if (!MetadataStore->NoPrefaulting) {
        __debugbreak();
        return FALSE;
    }
    if (!MetadataStore->NoTruncate) {
        __debugbreak();
        return FALSE;
    }

    return InitializeMetadataFromRecordSize(MetadataStore);
}

_Use_decl_annotations_
BOOL
BindMetadataStore(
    PTRACE_CONTEXT TraceContext,
    PTRACE_STORE MetadataStore
    )
/*++

Routine Description:

    This routine binds a metadata trace store to a trace context.  It is called
    in parallel for all remaining metadata stores once :metadatainfo has been
    bound successfully.  Thus, the metadata trace stores will have addresses
    assigned for their backing TRACE_STORE_INFO elements like Eof, Time, Traits
    etc.

    This routine is responsible for creating events, memory maps and threadpool
    work items as appropriate.  The metadata store's traits are used to guide
    this process.

Arguments:

    TraceContext - Supplies a pointer to a TRACE_CONTEXT structure.

    MetadataStore - Supplies a pointer to a TRACE_STORE structure.

Return Value:

    TRUE on success, FALSE on failure.

--*/
{
    BOOL Readonly;
    TRACE_STORE_METADATA_ID MetadataId;
    PTRACE_STORE TraceStore;
    PTRACE_STORE ExpectedMetadataStore;
    PTRACE_STORE ActualMetadataStore;

    //
    // Invariant check: ensure we've been passed a metadata trace store, and
    // that it is not of type metadata info.
    //

    if (!IsMetadataTraceStore(MetadataStore)) {
        return FALSE;
    }

    MetadataId = MetadataStore->TraceStoreMetadataId;
    if (MetadataId == TraceStoreMetadataMetadataInfoId) {
        return FALSE;
    }

    //
    // Make sure the trace store linkage is correct.  The trace store we point
    // to should have its relevant MetadataStore pointing at us.
    //

    ExpectedMetadataStore = MetadataStore;
    TraceStore = MetadataStore->TraceStore;
    ActualMetadataStore = TraceStoreMetadataIdToStore(TraceStore, MetadataId);
    if (ExpectedMetadataStore != ActualMetadataStore) {
        return FALSE;
    }

    Readonly = (BOOL)TraceContext->Flags.Readonly;


    return FALSE;
}

// vim:set ts=8 sw=4 sts=4 tw=80 expandtab                                     :
