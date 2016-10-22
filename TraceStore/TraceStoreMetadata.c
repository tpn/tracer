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

    This routine binds a metadata trace store to a trace context.

Arguments:

    TraceContext - Supplies a pointer to a TRACE_CONTEXT structure.

    MetadataStore - Supplies a pointer to a TRACE_STORE structure.

Return Value:

    TRUE on success, FALSE on failure.

--*/
{
    return FALSE;
}

// vim:set ts=8 sw=4 sts=4 tw=80 expandtab                                     :
