/*++

Copyright (c) 2016 Trent Nelson <trent@trent.me>

Module Name:

    TraceStoreRelocation.c

Abstract:

    Trace stores such as those used for function tables and function table
    entries persist C structures with embedded pointers to disk.  In order
    for a future reader of a trace store to use such a structure, support must
    be provided for relocating pointers if the original base address used for
    a mapping is unavailable.  This module implements functionality to support
    this, referred to generally as "relocation".

    Functions are provided for saving relocation information to a trace store's
    relocation metadata store, creating an address relocation table from a
    trace store's address and allocation metadata stores, and relocating a
    record using a relocation table.

    N.B.: trace store relocation is analogous to the relocation process the
          loader has to undertake if a DLLs preferred base address is not
          available.

--*/

#include "stdafx.h"

_Use_decl_annotations_
BOOL
SaveTraceStoreRelocationInfo(
    PTRACE_STORE TraceStore
    )
/*++

Routine Description:

    This routine saves the relocation information associated with a TRACE_STORE
    into the :Relocation metadata store.

Arguments:

    TraceStore - Supplies a pointer to a TRACE_STORE structure.

Return Value:

    TRUE on success, FALSE on failure.

--*/
{
    BOOL HasRelocations;
    BOOL HasRelocationBackRefs;
    USHORT Index;
    PTRACE_STORE_RELOC pReloc;
    PTRACE_STORE_RELOC Reloc;
    PTRACE_STORE_FIELD_RELOC DestFieldReloc;
    PTRACE_STORE_FIELD_RELOC SourceFieldReloc;
    PTRACE_STORE_FIELD_RELOC FirstDestFieldReloc;
    PTRACE_STORE_FIELD_RELOC FirstSourceFieldReloc;
    ULARGE_INTEGER AllocationSize;
    ULARGE_INTEGER NumberOfRecords = { 1 };
    PVOID BaseAddress;

    //
    // Validate arguments.
    //

    if (!ARGUMENT_PRESENT(TraceStore)) {
        return FALSE;
    }

    //
    // Make sure we're not readonly, the relocation info is non-NULL, and the
    // relocation metadata store has been initialized.
    //

    if (TraceStore->IsReadonly) {
        __debugbreak();
        return FALSE;
    }

    if (!TraceStore->RelocationStore) {
        __debugbreak();
        return FALSE;
    }

    pReloc = TraceStore->pReloc;

    if (!pReloc) {
        HasRelocations = FALSE;
        HasRelocationBackRefs = FALSE;
    } else {
        HasRelocations = (pReloc->NumberOfRelocations > 0);
        HasRelocationBackRefs = (pReloc->NumberOfRelocationBackReferences > 0);
    }

    //
    // Calculate the total size required for the TRACE_STORES_RELOC structure,
    // plus a copy of the trailing TRACE_STORE_FIELD_RELOC array if we have
    // relocations.
    //

    AllocationSize.QuadPart = sizeof(TRACE_STORE_RELOC);
    if (HasRelocations) {
        AllocationSize.QuadPart += (
            pReloc->NumberOfRelocations *
            sizeof(TRACE_STORE_FIELD_RELOC)
        );
    }

    BaseAddress = TraceStore->RelocationStore->AllocateRecords(
        TraceStore->TraceContext,
        TraceStore->RelocationStore,
        &AllocationSize,
        &NumberOfRecords
    );

    if (!BaseAddress) {
        __debugbreak();
        return FALSE;
    }

    Reloc = (PTRACE_STORE_RELOC)BaseAddress;

    TRY_MAPPED_MEMORY_OP {

        //
        // Copy the initial relocation information over.
        //

        Reloc->SizeOfStruct = sizeof(*Reloc);

        if (HasRelocations) {
            Reloc->NumberOfRelocations = pReloc->NumberOfRelocations;
            Reloc->Relocations = (PTRACE_STORE_FIELD_RELOC)(
                RtlOffsetToPointer(
                    BaseAddress,
                    sizeof(TRACE_STORE_RELOC)
                )
            );
        }

        Reloc->BitmapBufferSizeInQuadwords = (
            TRACE_STORE_BITMAP_SIZE_IN_QUADWORDS
        );

        Reloc->Bitmap.SizeOfBitMap = MAX_TRACE_STORE_IDS;
        Reloc->Bitmap.Buffer = (PULONG)&Reloc->BitmapBuffer[0];

        if (HasRelocationBackRefs) {
            Reloc->NumberOfRelocationBackReferences = (
                pReloc->NumberOfRelocationBackReferences
            );
            if (Reloc->BitmapBufferSizeInQuadwords !=
                pReloc->BitmapBufferSizeInQuadwords) {
                __debugbreak();
                return FALSE;
            }
            if (Reloc->Bitmap.SizeOfBitMap != pReloc->Bitmap.SizeOfBitMap) {
                __debugbreak();
                return FALSE;
            }
            __movsq((PDWORD64)Reloc->Bitmap.Buffer,
                    (PDWORD64)pReloc->Bitmap.Buffer,
                    Reloc->BitmapBufferSizeInQuadwords);
        }

        if (!HasRelocations) {
            goto End;
        }

        FirstDestFieldReloc = Reloc->Relocations;
        FirstSourceFieldReloc = pReloc->Relocations;

        //
        // Copy the individual field relocations over.
        //

        for (Index = 0; Index < Reloc->NumberOfRelocations; Index++) {
            DestFieldReloc = FirstDestFieldReloc + Index;
            SourceFieldReloc = FirstSourceFieldReloc + Index;

            DestFieldReloc->Offset = SourceFieldReloc->Offset;
            DestFieldReloc->TraceStoreId = SourceFieldReloc->TraceStoreId;
        }

    } CATCH_STATUS_IN_PAGE_ERROR {
        return FALSE;
    }

End:
    TraceStore->Reloc = Reloc;

    return TRUE;
}

_Use_decl_annotations_
BOOL
LoadTraceStoreRelocationInfo(
    PTRACE_STORE TraceStore
    )
/*++

Routine Description:

    This routine loads any relocation information associated with the trace
    store from the :Relocation metadata store.  It is called in the final
    stages of binding a trace store to a trace context.

Arguments:

    TraceStore - Supplies a pointer to a TRACE_STORE structure.

Return Value:

    TRUE on success, FALSE on failure.  TRUE doesn't necessarily imply that
    relocation information was loaded; check TraceStore->HasRelocations for
    this information.

--*/
{
    BOOL HasRelocations;
    BOOL HasRelocationBackRefs;
    PTRACE_STORE_RELOC Reloc;

    //
    // Validate arguments.
    //

    if (!ARGUMENT_PRESENT(TraceStore)) {
        return FALSE;
    }

    //
    // Make sure we're not a metadata store.
    //

    if (TraceStore->IsMetadata) {
        return FALSE;
    }

    //
    // Make sure we're readonly and that a relocation store has been configured.
    //

    if (!TraceStore->IsReadonly) {
        return FALSE;
    }

    if (!TraceStore->RelocationStore) {
        return FALSE;
    }

    //
    // Point TraceStore->Reloc at the base of the :Relocation trace store's
    // memory map.
    //

    Reloc = TraceStore->Reloc = (PTRACE_STORE_RELOC)(
        TraceStore->RelocationStore->MemoryMap->BaseAddress
    );

    if (!Reloc) {
        __debugbreak();
        return FALSE;
    }

    HasRelocations = (Reloc->NumberOfRelocations > 0);
    HasRelocationBackRefs = (Reloc->NumberOfRelocationBackReferences > 0);

    //
    // Adjust the two pointers embedded within the relocation structure.  We
    // can do this because we get special-cased with copy-on-write semantics
    // during readonly binding.
    //

    if (HasRelocations) {
        TraceStore->HasRelocations = TRUE;
        Reloc->Relocations = (PTRACE_STORE_FIELD_RELOC)(
            RtlOffsetToPointer(
                Reloc,
                sizeof(TRACE_STORE_RELOC)
            )
        );
    }

    Reloc->Bitmap.Buffer = (PULONG)&Reloc->BitmapBuffer[0];
    if (HasRelocationBackRefs) {
        TraceStore->IsRelocationTarget = TRUE;
    }

    return TRUE;
}

// vim:set ts=8 sw=4 sts=4 tw=80 expandtab                                     :
