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

PTRACE_STORE_TRAITS MetadataStoreTraits[] = {
    &MetadataInfoStoreTraits,
    &AllocationStoreTraits,
    &RelocationStoreTraits,
    &AddressStoreTraits,
    &BitmapStoreTraits,
    &InfoStoreTraits
};

CONST PBIND_COMPLETE TraceStoreMetadataBindCompletes[] = {
    MetadataInfoMetadataBindComplete,       // MetadataInfo
    NULL,                                   // Allocation
    RelocationMetadataBindComplete,         // Relocation
    NULL,                                   // Address
    NULL,                                   // Bitmap
    NULL                                    // Info
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
MetadataInfoMetadataBindComplete(
    PTRACE_CONTEXT TraceContext,
    PTRACE_STORE MetadataInfoStore,
    PTRACE_STORE_MEMORY_MAP FirstMemoryMap
    )
/*++

Routine Description:

    This is the bind complete callback routine for :metadatainfo stores.  It
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
    USHORT NumberOfMetadataStores;
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

    This is the bind complete callback routine for :relocation stores.
    It calls LoadTraceStoreRelocationInfo() if this is a readonly session,
    or SaveTraceStoreRelocationInfo() if this is a normal writable session.

Arguments:

    TraceContext - Supplies a pointer to a TRACE_CONTEXT structure.

    RelocationStore - Supplies a pointer to the :relocation TRACE_STORE.

    FirstMemoryMap - Supplies a pointer to a TRACE_STORE_MEMORY_MAP structure.

Return Value:

    TRUE on success, FALSE on failure.

--*/
{
    PTRACE_STORE TraceStore;

    TraceStore = RelocationStore->TraceStore;
    if (TraceStore->IsReadonly) {
        return LoadTraceStoreRelocationInfo(TraceStore);
    } else {
        return SaveTraceStoreRelocationInfo(TraceStore);
    }
}

// vim:set ts=8 sw=4 sts=4 tw=80 expandtab                                     :
