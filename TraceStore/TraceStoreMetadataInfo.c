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

    This routine binds a metadata info metadata trace store to a trace context.

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

    Readonly = (BOOL)TraceContext->Flags.Readonly;

    //
    // Subtract one to account for the normal trace store.
    //

    NumberOfMetadataStores = (USHORT)(
        TraceContext->TraceStores->ElementsPerTraceStore - 1
    );

    //
    // Initialize the memory map.  As metadata info stores only have a
    // single allocation, we don't need to create multiple memory maps.
    //

    MemoryMap = &TraceStore->SingleMemoryMap;
    TraceStore->MemoryMap = MemoryMap;
    MemoryMap->FileHandle = TraceStore->FileHandle;
    MemoryMap->FileOffset.QuadPart = 0;
    MemoryMap->MappingSize.QuadPart = sizeof(TRACE_STORE_METADATA_INFO);

    //
    // If we're not readonly, extend the file to the expected size.
    //

    if (!Readonly) {

        Success = SetFilePointerEx(
            MemoryMap->FileHandle,
            MemoryMap->MappingSize,
            NULL,
            FILE_BEGIN
        );

        if (!Success) {
            TraceStore->LastError = GetLastError();
            return FALSE;
        }

        if (!SetEndOfFile(MemoryMap->FileHandle)) {
            TraceStore->LastError = GetLastError();
            return FALSE;
        }
    }

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
        TraceStore->LastError = GetLastError();
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
        TraceStore->LastError = GetLastError();
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
    MetadataStorePointer = &TraceStore->MetadataInfoStore;

    for (Index = 0; Index < NumberOfMetadataStores; Index++) {
        Info = (((PTRACE_STORE_INFO)MetadataInfo) + Index);

        //
        // N.B.: We abuse the fact that the trace store's metadata store
        //       pointers are laid out consecutively (and contiguously) in
        //       the same order as implied by their TraceStoreMetadataStoreId.
        //       That allows us to use (*MetadataStorePointer)++ below.  (The
        //       alernate approach would be to have a method that returns the
        //       field offset within the TRACE_STORE structure for a given
        //       metadata store ID.)
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
    // If we're not readonly, initialize end of file and time.
    //

    if (!Readonly) {
        Info = (PTRACE_STORE_INFO)MetadataInfo;
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
