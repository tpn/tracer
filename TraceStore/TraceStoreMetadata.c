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
    sizeof(TRACE_STORE_METADATA_ID),
    sizeof(TRACE_STORE_ALLOCATION),
    sizeof(TRACE_STORE_RELOC),
    sizeof(TRACE_STORE_ADDRESS),
    sizeof(TRACE_STORE_BITMAP),
    sizeof(TRACE_STORE_INFO)
};

CONST PINITIALIZE_TRACE_STORE_METADATA TraceStoreMetadataInitializers[] = {
    InitializeMetadataInfoMetadata,     // MetadataInfo
    NULL,                               // Allocation
    InitializeRelocationMetadata,       // Relocation
    NULL,                               // Address
    InitializeBitmapMetadata,           // Bitmap
    InitializeInfoMetadata              // Info
};

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

    Info = (PTRACE_STORE_INFO)(MetadataInfo + Index);
    return Info;
}

_Use_decl_annotations_
BOOL
InitializeMetadataInfoMetadata(
    _In_ PTRACE_STORE MetadataStore
    )
{
    MetadataStore->NoPrefaulting = TRUE;
    return TRUE;
}

_Use_decl_annotations_
BOOL
InitializeRelocationMetadata(
    _In_ PTRACE_STORE MetadataStore
    )
{
    return TRUE;
}

_Use_decl_annotations_
BOOL
InitializeBitmapMetadata(
    _In_ PTRACE_STORE MetadataStore
    )
{
    return TRUE;
}

_Use_decl_annotations_
BOOL
InitializeInfoMetadata(
    _In_ PTRACE_STORE MetadataStore
    )
{
    MetadataStore->NoPrefaulting = TRUE;
    return TRUE;
}

// vim:set ts=8 sw=4 sts=4 tw=80 expandtab                                     :
