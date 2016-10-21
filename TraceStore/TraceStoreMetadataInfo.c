/*++

Copyright (c) 2016 Trent Nelson <trent@trent.me>

Module Name:

    TraceStoreMetadataInfo.c

Abstract:

    This module implements functionality related to the metadata info metadata
    trace store, which uses the TRACE_STORE_METADATA_INFO structure, and serves
    as the backing store for the TRACE_STORE_INFO structures used by the other
    metadata trace stores.

--*/

#include "stdafx.h"

_Use_decl_annotations_
BOOL
BindMetadataInfo(
    PTRACE_CONTEXT TraceContext,
    PTRACE_STORE MetadataInfoStore
    )
/*--

Routine Description:

    This routine binds a metadata info trace store to a trace context.

Arguments:

    TraceContext - Supplies a pointer to a TRACE_CONTEXT structure to bind the
        metadata info metadata store to.

    MetadataInfoStore - Supplies a pointer to a TRACE_STORE for a metadata info
        metadata store.

Return Value:

    TRUE on success, FALSE on failure.

--*/
{
    BOOL Readonly;
    USHORT Index;
    USHORT NumberOfMetadataStores;
    PVOID BaseAddress;
    TRACE_STORE_INFO Info;
    TRACE_STORE_METADATA_ID MetadataId;
    PTRACE_STORE TraceStore;
    PTRACE_STORE MetadataStore;
    PPTRACE_STORE MetadataStorePointer;
    PTRACE_STORE_MEMORY_MAP MemoryMap;
    PTRACE_STORE_METADATA_INFO MetadataInfo;

    //
    // Ensure we've been passed a metadata info metadata trace store.
    //

    if (!IsMetadataTraceStore(MetadataStore)) {
        return FALSE;
    }

    MetadataId = MetadataStore->TraceStoreMetadataId;
    if (MetadataId != TraceStoreMetadataMetadataInfoId) {
        return FALSE;
    }

    TraceStore = MetadataStore->TraceStore;

    if (TraceStore->MetadataInfoStore != MetadataStore) {
        return FALSE;
    }

    Readonly = (BOOL)TraceContext->Flags.Readonly;

    //
    // Subtract one to account for the normal trace store.
    //

    NumberOfMetadataStores = (USHORT)TraceStores->ElementsPerTraceStore - 1;


    //
    // MetadataInfo only uses a single memory map.
    //

    MemoryMap = &TraceStore->SingleMemoryMap;
    TraceStore->MemoryMap = MemoryMap;

    MemoryMap->FileHandle = TraceStore->FileHandle;
    MemoryMap->FileOffset.QuadPart = 0;
    MemoryMap->MappingSize.QuadPart = sizeof(TRACE_STORE_METADATA_INFO);

    //
    // Create the file mapping.
    //

    MemoryMap->MappingHandle = CreateFileMappingNuma(
        MemoryMap->FileHandle,
        NULL,
        TraceStore->CreateFileMappingProtectionFlags,
        0,
        0,
        NULL,
        TraceStore->NumaNode
    );

    if (MemoryMap->MappingHandle == NULL ||
        MemoryMap->MappingHandle == INVALID_HANDLE_VALUE) {
        DWORD LastError = GetLastError();
        return FALSE;
    }

    //
    // Map the view.
    //

    MemoryMap->BaseAddress = MapViewOfFileExNuma(
        MemoryMap->MappingHandle,
        TraceStore->MapViewOfFileDesiredAccess,
        MemoryMap->FileOffset.HighPart,
        MemoryMap->FileOffset.LowPart,
        MemoryMap->MappingSize.LowPart,
        NULL,
        TraceStore->NumaNode
    );

    if (!MemoryMap->BaseAddress) {
        goto Error;
    }

    BaseAddress = MemoryMap->NextAddress = MemoryMap->BaseAddress;

    //
    // We successfully mapped the view.  The memory map's base address now
    // points to an area of sufficient size to hold a TRACE_STORE_METADATA_INFO
    // structure, which is essentially an array of TRACE_STORE_INFO structures.
    // Enumerate through the metadata stores (including this one) and wire up
    // the relevant pointers.
    //

    MetadataInfo = (PTRACE_STORE_METADATA_INFO)BaseAddress;
    MetadataInfoStore = TraceStore->MetadataInfoStore;
    MetadataStorePointer = &TraceStore->MetadataInfoStore;

    for (Index = 0; Index < NumberOfMetadataStores; Index++) {
        Info = (((PTRACE_STORE_INFO)MetadataInfo) + Index);
        MetadataStore = (*MetadataStorePointer)++;
        MetadataStore->Info = Info;
        MetadataStore->Eof = &Info->Eof;
        MetadataStore->Time = &Info->Time;
        MetadataStore->Stats = &Info->Stats;
        MetadataStore->Totals = &Info->Totals;
    }

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
