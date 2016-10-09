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
    into the :relocation metadata store.

Arguments:

    TraceStore - Supplies a pointer to a TRACE_STORE structure.

Return Value:

    TRUE on success, FALSE on failure.

--*/
{
    USHORT Index;
    PRTL Rtl;
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
    // Make sure we're not readonly, that the trace store has relocation info,
    // the relocation info is non-NULL, and the relocation metadata store has
    // been initialized.
    //

    if (TraceStore->IsReadonly) {
        return FALSE;
    }

    if (!TraceStore->HasRelocations) {
        return FALSE;
    }

    if (!TraceStore->RelocationStore) {
        return FALSE;
    }

    pReloc = TraceStore->pReloc;
    if (!pReloc || pReloc->NumberOfRelocations == 0) {
        return FALSE;
    }

    //
    // Calculate the total size required for the TRACE_STORES_RELOC structure,
    // plus a copy of the trailing TRACE_STORE_FIELD_RELOC array.
    //

    AllocationSize.QuadPart = (
        sizeof(TRACE_STORE_RELOC) + (
            pReloc->NumberOfRelocations *
            sizeof(TRACE_STORE_FIELD_RELOC)
        )
    );

    Rtl = TraceStore->Rtl;

    BaseAddress = TraceStore->RelocationStore->AllocateRecords(
        TraceStore->TraceContext,
        TraceStore->RelocationStore,
        &AllocationSize,
        &NumberOfRecords
    );

    if (!BaseAddress) {
        return FALSE;
    }

    Reloc = (PTRACE_STORE_RELOC)BaseAddress;

    TRY_MAPPED_MEMORY_OP {

        //
        // Copy the initial relocation information over.
        //

        Reloc->SizeOfStruct = pReloc->SizeOfStruct;
        Reloc->NumberOfRelocations = pReloc->NumberOfRelocations;
        Reloc->Unused1 = pReloc->Unused1;
        Reloc->Relocations = (PTRACE_STORE_FIELD_RELOC)(
            RtlOffsetToPointer(
                BaseAddress,
                sizeof(TRACE_STORE_RELOC)
            )
        );

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


    return TRUE;
}

// vim:set ts=8 sw=4 sts=4 tw=80 expandtab                                     :
