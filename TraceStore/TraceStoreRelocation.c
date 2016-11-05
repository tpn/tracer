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

        Reloc->ForwardRefBitmap.SizeOfBitMap = MAX_TRACE_STORE_IDS;
        Reloc->ForwardRefBitmap.Buffer = (PULONG)(
            &Reloc->ForwardRefBitmapBuffer[0]
        );

        Reloc->BackRefBitmap.SizeOfBitMap = MAX_TRACE_STORE_IDS;
        Reloc->BackRefBitmap.Buffer = (PULONG)&Reloc->BackRefBitmapBuffer[0];

        if (HasRelocationBackRefs) {
            Reloc->NumberOfRelocationBackReferences = (
                pReloc->NumberOfRelocationBackReferences
            );
            if (Reloc->BitmapBufferSizeInQuadwords !=
                pReloc->BitmapBufferSizeInQuadwords) {
                __debugbreak();
                return FALSE;
            }
            if (Reloc->BackRefBitmap.SizeOfBitMap !=
                pReloc->BackRefBitmap.SizeOfBitMap) {
                __debugbreak();
                return FALSE;
            }
            __movsq((PDWORD64)Reloc->BackRefBitmap.Buffer,
                    (PDWORD64)pReloc->BackRefBitmap.Buffer,
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
    store from the :Relocation metadata store.  If a trace store has multiple
    relocations (excluding itself), an array will be allocated to hold
    references to all the relocation complete events of the referenced trace
    stores.  This will be waited on via WaitForMultipleObjects() once the trace
    store's memory maps have been loaded.

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
    ULONG NumberOfRelocationDependencies;
    ULONG Index;
    ULONG HintIndex;
    ULONG PreviousIndex;
    ULONG volatile *Outstanding;
    TRACE_STORE_ID TraceStoreId;
    TRACE_STORE_ID TargetStoreId;
    PRTL Rtl;
    PHANDLE Event;
    PHANDLE Events;
    PALLOCATOR Allocator;
    PTRACE_STORE TargetStore;
    PTRACE_STORES TraceStores;
    PTRACE_CONTEXT TraceContext;
    PTRACE_STORE_RELOC Reloc;
    PRTL_BITMAP ForwardRefBitmap;
    PRTL_BITMAP BackRefBitmap;

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

    //
    // Initialize aliases.
    //

    Rtl = TraceStore->Rtl;
    TraceContext = TraceStore->TraceContext;
    TraceStores = TraceContext->TraceStores;
    TraceStoreId = TraceStore->TraceStoreId;

    HasRelocations = (Reloc->NumberOfRelocations > 0);
    HasRelocationBackRefs = (Reloc->NumberOfRelocationBackReferences > 0);

    if (HasRelocationBackRefs) {
        TraceStore->IsRelocationTarget = TRUE;
    }

    //
    // Adjust the pointers embedded within the relocation structure.  We can
    // do this because we get special-cased with copy-on-write semantics
    // during readonly binding.
    //

    Reloc->ForwardRefBitmap.Buffer = (PULONG)&Reloc->ForwardRefBitmapBuffer[0];
    Reloc->BackRefBitmap.Buffer = (PULONG)&Reloc->BackRefBitmapBuffer[0];

    if (HasRelocations) {
        TraceStore->HasRelocations = TRUE;
        Reloc->Relocations = (PTRACE_STORE_FIELD_RELOC)(
            RtlOffsetToPointer(
                Reloc,
                sizeof(TRACE_STORE_RELOC)
            )
        );

        //
        // Clear counters before the loop.
        //

        Index = 0;
        HintIndex = 0;
        LastIndex = 0;
        PreviousIndex = 0;
        NumberOfRelocationDependencies = 0;

        //
        // Initialize our bitmap alias.
        //

        ForwardRefBitmap = &Reloc->ForwardRefBitmap;

        //
        // Walk the "forward reference" bitmap and count how many trace stores
        // we depend upon, excluding ourselves.
        //

        do {

            //
            // Extract the next bit from the bitmap.
            //

            Index = Rtl->RtlFindSetBits(ForwardRefBitmap, 1, HintIndex);

            //
            // Verify we got a sane index back.
            //

            if (Index == BITS_NOT_FOUND) {

                //
                // This should never happen.
                //

                __debugbreak();
                return FALSE;
            }

            if (Index <= PreviousIndex) {

                //
                // Our search has wrapped, so exit the loop.
                //

                break;
            }

            //
            // Update the previous index and hint index and resolve the trace
            // store ID.
            //

            PreviousIndex = Index;
            HintIndex = Index + 1;
            TargetStoreId = (TRACE_STORE_ID)Index;

            //
            // If the target store is us, make a note that we have self
            // relocation references and continue the loop.
            //

            if (TargetStoreId == TraceStoreId) {
                TraceStore->HasSelfRelocations = TRUE;
                continue;
            }

            //
            // Increment the number of dependencies counter and continue the
            // loop.
            //

            NumberOfRelocationDependencies++;

        } while (1);

        //
        // If we didn't see any other relocation dependencies, make a note in
        // the relevant trace store flag that the only relocations required are
        // dependent upon ourselves, and then return success.
        //

        if (NumberOfRelocationDependencies == 0) {
            TraceStore->OnlyRelocationIsToSelf = TRUE;
            return TRUE;
        }

        TraceStore->NumberOfRelocationDependencies = (
            NumberOfRelocationDependencies
        );

        if (NumberOfRelocationDependencies == 1) {
            HANDLE Handle;

            //
            // If we only have a single dependency, abuse two things:
            // a) the pointer-sized (read: HANDLE-sized) storage availabe at
            // TraceStore->RelocationCompleteWaitEvents, and b) the fact that
            // PreviousIndex will conveniently be castable to the trace store
            // ID of the single store we're dependent upon, allowing us to go
            // straight to resolution of the corresponding store's relocation
            // complete event.
            //

            TargetStoreId = (TRACE_STORE_ID)PreviousIndex;
            Handle = TraceStoreIdToRelocationCompleteEvent(TraceStores,
                                                           TargetStoreId);
            TraceStore->RelocationCompleteWaitEvents = (PHANDLE)Event;
            return TRUE;

        }

        //
        // We are dependent upon more than one trace store, so we need to
        // create an array of relocation handles that can be waited on.  This
        // happens once we've finished loading all of our memory maps.
        //

        TraceStore->HasMultipleRelocationWaits = TRUE;
        InterlockedIncrement(
            &TraceContext->NumberOfStoresWithMultipleRelocationDependencies
        );

        Events = (PHANDLE)(
            Allocator->Calloc(
                Allocator->Context,
                NumberOfRelocationDependencies,
                sizeof(HANDLE)
            )
        );

        if (!Events) {
            return FALSE;
        }

        TraceStore->RelocationCompleteWaitEvents = Events;

        //
        // Reset the bitmap index variables and walk the bitmap again,
        // filling in the event array with the relevant handle from the
        // array of "relocation complete" handles reserved in the trace
        // stores structure.
        //

        Index = 0;
        HintIndex = 0;
        PreviousIndex = 0;

        //
        // Point the Event pointer at the base of the allocated array.
        //

        Event = (PHANDLE)Events;

        do {

            Index = Rtl->RtlFindSetBits(ForwardRefBitmap, 1, HintIndex);
            if (Index <= PreviousIndex) {
                break;
            }

            PreviousIndex = Index;
            HintIndex = Index + 1;
            TargetStoreId = (TRACE_STORE_ID)Index;

            if (TargetStoreId == TraceStoreId) {
                continue;
            }

            *Event++ = TraceStoreIdToRelocationCompleteEvent(TargetStoreId);

        } while (1);
    }

    //
    // We're done, return success.
    //

    return TRUE;
}

// vim:set ts=8 sw=4 sts=4 tw=80 expandtab                                     :
