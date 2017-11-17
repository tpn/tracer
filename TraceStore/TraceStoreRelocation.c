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
    PTRACE_STORE RelocationStore;
    PTRACE_STORE_RELOC pReloc;
    PTRACE_STORE_RELOC Reloc;
    PTRACE_STORE_FIELD_RELOC DestFieldReloc;
    PTRACE_STORE_FIELD_RELOC SourceFieldReloc;
    PTRACE_STORE_FIELD_RELOC FirstDestFieldReloc;
    PTRACE_STORE_FIELD_RELOC FirstSourceFieldReloc;
    PALLOCATE_RECORDS AllocateRecords;
    ULONG_PTR RecordSize;
    ULONG_PTR NumberOfRecords = 1;
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

    RelocationStore = TraceStore->RelocationStore;
    if (!RelocationStore) {
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

    RecordSize = sizeof(TRACE_STORE_RELOC);
    if (HasRelocations) {
        RecordSize += (
            pReloc->NumberOfRelocations *
            sizeof(TRACE_STORE_FIELD_RELOC)
        );
    }

    AllocateRecords = RelocationStore->AllocateRecords;

    BaseAddress = AllocateRecords(TraceStore->TraceContext,
                                  RelocationStore,
                                  NumberOfRecords,
                                  RecordSize);

    if (!BaseAddress) {
        __debugbreak();
        return FALSE;
    }

    Reloc = (PTRACE_STORE_RELOC)BaseAddress;

    TRY_MAPPED_MEMORY_OP {

        //
        // Initialize the constant fields that are filled out regardless of
        // whether or not we have any relocations or back references.
        //

        Reloc->SizeOfStruct = sizeof(*Reloc);

        Reloc->BitmapBufferSizeInQuadwords = (
            TRACE_STORE_BITMAP_SIZE_IN_QUADWORDS
        );

        Reloc->ForwardRefBitmap.SizeOfBitMap = MAX_TRACE_STORE_IDS;
        Reloc->ForwardRefBitmap.Buffer = (PULONG)(
            &Reloc->ForwardRefBitmapBuffer[0]
        );

        Reloc->BackRefBitmap.SizeOfBitMap = MAX_TRACE_STORE_IDS;
        Reloc->BackRefBitmap.Buffer = (PULONG)&Reloc->BackRefBitmapBuffer[0];

        //
        // If we have either, verify that the bitmap buffer size is as expected.
        //

        if (HasRelocations || HasRelocationBackRefs) {
            if (Reloc->BitmapBufferSizeInQuadwords !=
                pReloc->BitmapBufferSizeInQuadwords) {
                __debugbreak();
                return FALSE;
            }
        }

        //
        // Verify and copy bitmap buffers, set counters.
        //

        if (HasRelocations) {
            Reloc->NumberOfRelocations = pReloc->NumberOfRelocations;
            Reloc->Relocations = (PTRACE_STORE_FIELD_RELOC)(
                RtlOffsetToPointer(
                    BaseAddress,
                    sizeof(TRACE_STORE_RELOC)
                )
            );
            if (Reloc->ForwardRefBitmap.SizeOfBitMap !=
                pReloc->ForwardRefBitmap.SizeOfBitMap) {
                __debugbreak();
                return FALSE;
            }
            __movsq((PDWORD64)Reloc->ForwardRefBitmap.Buffer,
                    (PDWORD64)pReloc->ForwardRefBitmap.Buffer,
                    Reloc->BitmapBufferSizeInQuadwords);
        }

        if (HasRelocationBackRefs) {
            Reloc->NumberOfRelocationBackReferences = (
                pReloc->NumberOfRelocationBackReferences
            );
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
    TRACE_STORE_ID TraceStoreId;
    TRACE_STORE_ID TargetStoreId;
    TRACE_STORE_ID LastTargetStoreId;
    PRTL Rtl;
    PHANDLE Event;
    PHANDLE Events;
    PALLOCATOR Allocator;
    PTRACE_STORE RelocationStore;
    PPTRACE_STORE DependencyStore;
    PPTRACE_STORE DependencyStores;
    PTRACE_STORES TraceStores;
    PTRACE_CONTEXT TraceContext;
    PTRACE_STORE_RELOC Reloc;
    PRTL_BITMAP ForwardRefBitmap;

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

    RelocationStore = TraceStore->RelocationStore;

    if (!RelocationStore) {
        return FALSE;
    }

    //
    // Point TraceStore->Reloc at the base of the :Relocation trace store's
    // memory map.
    //

    Reloc = TraceStore->Reloc = (PTRACE_STORE_RELOC)(
        RelocationStore->MemoryMap->BaseAddress
    );

    if (!Reloc) {
        __debugbreak();
        return FALSE;
    }

    //
    // Initialize aliases.
    //

    Rtl = RelocationStore->Rtl;
    TraceContext = RelocationStore->TraceContext;
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

        //
        // Point the Relocations pointer at the end of our structure.
        //

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
        PreviousIndex = 0;
        NumberOfRelocationDependencies = 0;
        LastTargetStoreId = TraceStoreNullId;

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
            // If this is the null store ID, make a note that we have null
            // store relocations and continue the loop.  (A null store ID
            // indicates the field needs to be zeroed during relocation.)
            //

            if (TargetStoreId == TraceStoreNullId) {
                TraceStore->HasNullStoreRelocations = TRUE;
                continue;
            }

            //
            // Make a note of this ID, increment our counter, and continue the
            // loop.
            //

            LastTargetStoreId = TargetStoreId;
            NumberOfRelocationDependencies++;

        } while (1);

        if (NumberOfRelocationDependencies == 0) {

            //
            // We weren't dependent on any other stores.  Set the relevant
            // flag depending on whether or not we saw the null store.
            //

            if (TraceStore->HasNullStoreRelocations) {
                TraceStore->OnlyRelocationIsToNull = TRUE;
            } else {
                TraceStore->OnlyRelocationIsToSelf = TRUE;
            }

            return TRUE;
        }

        TraceStore->NumberOfRelocationDependencies = (
            NumberOfRelocationDependencies
        );

        if (NumberOfRelocationDependencies == 1) {

            //
            // If we only have a single dependency, the LastTargetStoreId will
            // have the trace store ID of the store we want.  Use this to look
            // up the store's wait handle.
            //

            TraceStore->RelocationCompleteWaitEvent = (
                TraceStoreIdToRelocationCompleteEvent(
                    TraceStores,
                    LastTargetStoreId
                )
            );

            //
            // Point the relocation dependency pointer at the target store.
            //

            TraceStore->RelocationDependencyStore = (
                TraceStoreIdToTraceStore(TraceStores, LastTargetStoreId)
            );

            return TRUE;
        }

        //
        // We are dependent upon more than one trace store, so we need to
        // create an array of relocation handles that can be waited on.  This
        // happens once we've finished loading all of our memory maps.  We use
        // this opportunity to create an array of trace store pointers, too.
        //

        TraceStore->HasMultipleRelocationWaits = TRUE;
        InterlockedIncrement(
            &TraceContext->NumberOfStoresWithMultipleRelocationDependencies
        );

        Allocator = TraceStore->pAllocator;
        Events = (PHANDLE)(
            Allocator->Calloc(
                Allocator->Context,
                NumberOfRelocationDependencies,

                //
                // Account for the size of the event we want to wait for.
                //

                sizeof(HANDLE) +

                //
                // Account for the size of a pointer to the trace store we're
                // dependent upon for relocations.
                //

                sizeof(PTRACE_STORE)
            )
        );

        if (!Events) {
            return FALSE;
        }

        TraceStore->RelocationCompleteWaitEvents = Events;

        //
        // Carve out the pointer to the array of dependent trace stores.
        //

        DependencyStores = (PPTRACE_STORE)(
            RtlOffsetToPointer(
                Events,
                sizeof(HANDLE) * NumberOfRelocationDependencies
            )
        );

        TraceStore->RelocationDependencyStores = DependencyStores;

        //
        // Reset the bitmap index variables used in the loop.
        //

        Index = 0;
        HintIndex = 0;
        PreviousIndex = 0;

        //
        // Point the Event and DependencyStore pointers at the base of their
        // respective arrays.
        //

        Event = Events;
        DependencyStore = DependencyStores;

        //
        // Walk the bitmap again, filling in the arrays of events and trace
        // store pointers.
        //

        do {

            Index = Rtl->RtlFindSetBits(ForwardRefBitmap, 1, HintIndex);
            if (Index <= PreviousIndex) {
                break;
            }

            PreviousIndex = Index;
            HintIndex = Index + 1;
            TargetStoreId = (TRACE_STORE_ID)Index;

            if (TargetStoreId == TraceStoreId ||
                TargetStoreId == TraceStoreNullId) {
                continue;
            }

            //
            // Save the event to the array and bump the pointer.
            //

            *Event++ = (
                TraceStoreIdToRelocationCompleteEvent(
                    TraceStores,
                    TargetStoreId
                )
            );

            //
            // Save the trace store to the array and bump the pointer.
            //

            *DependencyStore++ = (
                TraceStoreIdToTraceStore(
                    TraceStores,
                    TargetStoreId
                )
            );

        } while (1);
    }

    //
    // We're done, return success.
    //

    return TRUE;
}

_Use_decl_annotations_
BOOL
ReadonlyNonStreamingTraceStoreCompleteRelocation(
    PTRACE_CONTEXT TraceContext,
    PTRACE_STORE TraceStore
    )
/*++

Routine Description:

    This routine is called when a non-streaming readonly trace store has
    completed its relocation step.  This is also called for trace stores
    that do not require any relocation.  This is because this routine will
    signal the relocation complete event for the given trace store, which
    other stores may also be waiting on.

Arguments:

    TraceContext - Supplies a pointer to a TRACE_CONTEXT structure.

    TraceStore - Supplies a pointer to a TRACE_STORE structure.

Return Value:

    TRUE on success, FALSE on failure.

--*/
{
    BOOL Success;
    HANDLE Event;
    PTRACE_STORES TraceStores;
    TRACE_STORE_ID TraceStoreId;

    //
    // Initialize aliases.
    //

    TraceStores = TraceContext->TraceStores;
    TraceStoreId = TraceStore->TraceStoreId;

    //
    // Get the relocation complete event handle for this trace store.
    //

    Event = (
        TraceStoreIdToRelocationCompleteEvent(
            TraceStores,
            TraceStoreId
        )
    );

    //
    // Signal it.  This will satisfy the waits of any other trace stores
    // that are dependent upon us.
    //

    if (!SetEvent(Event)) {
        return FALSE;
    }

    //
    // Complete the binding.
    //

    Success = (
        BindNonStreamingReadonlyTraceStoreComplete(
            TraceContext,
            TraceStore
        )
    );

    return Success;
}

_Use_decl_annotations_
BOOL
ReadonlyNonStreamingTraceStoreReadyForRelocation(
    PTRACE_CONTEXT TraceContext,
    PTRACE_STORE TraceStore
    )
/*++

Routine Description:

    This routine is called when a non-streaming readonly trace store receives
    notification that all of its dependent trace stores have completed their
    relocation.  It is responsible for initiating the relocation of the trace
    store.

Arguments:

    TraceContext - Supplies a pointer to a TRACE_CONTEXT structure.

    TraceStore - Supplies a pointer to a TRACE_STORE structure.

Return Value:

    TRUE on success, FALSE on failure.

--*/
{
    BOOL Success;


    //
    // This has started triggering and I'm not sure if it's correct either.
    //

    if (!TraceStore->OnlyRelocationIsToNull) {
        __debugbreak();
    }

    //
    // N.B. This has also started triggering.
    //

    if (TraceStore->NumberOfRelocationsRequired != 0) {
        __debugbreak();
    }

    //
    // For now, just complete the relocation.
    //

    Success = (
        ReadonlyNonStreamingTraceStoreCompleteRelocation(
            TraceContext,
            TraceStore
        )
    );

    return Success;
}

// vim:set ts=8 sw=4 sts=4 tw=80 expandtab                                     :
