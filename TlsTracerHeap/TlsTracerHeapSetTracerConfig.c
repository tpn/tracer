#include "stdafx.h"

_Use_decl_annotations_
BOOL
TlsTracerHeapSetTracerConfig(
    PTRACER_CONFIG TracerConfigPointer
    )
/*--

Routine Description:

    Sets the global TracerConfig variable to TracerConfigPointer, and sets the
    TracerConfig->Allocator->TlsIndex to the global TlsIndex allocated via the
    TlsAlloc() call that was performed when the DLL was loaded.

Arguments:

    TracerConfigPointer - Supplies a pointer to an initialized TRACER_CONFIG
        structure (with an allocator set).

Return Value:

    TRUE on success, FALSE on failure.

--*/
{
    PALLOCATOR Allocator;

    //
    // Validate arguments.
    //

    if (!ARGUMENT_PRESENT(TracerConfigPointer)) {
        return FALSE;
    }

    if (!ARGUMENT_PRESENT(TracerConfigPointer->Allocator)) {
        return FALSE;
    }

    //
    // Ensure TlsIndex is set to something sane.  (We shouldn't be able to even
    // get to this point if the TlsAlloc() done in DLL_PROCESS_ATTACH failed as
    // it would have prevented the DLL from loading.)
    //

    if (TlsIndex == TLS_OUT_OF_INDEXES) {
        return FALSE;
    }

    //
    // Set the global TracerConfig and the allocator's TlsIndex.  When new
    // threads attach, TlsHeapThreadAttach() will be called, which will create
    // a new ALLOCATOR structure (from the global heap), and then a new thread
    // local heap (with HEAP_NO_SERIALIZE).
    //

    TracerConfig = TracerConfigPointer;

    Allocator = TracerConfig->Allocator;
    Allocator->TlsIndex = TlsIndex;

    //
    // Take a copy of all the allocator's original routines.  We override these
    // with TLS-aware versions once the first thread is attached, and restore
    // them once the last thread detaches.
    //


    OriginalMalloc = Allocator->Malloc;
    OriginalCalloc = Allocator->Calloc;
    OriginalRealloc = Allocator->Realloc;
    OriginalFree = Allocator->Free;
    OriginalFreePointer = Allocator->FreePointer;

    //
    // We're done, return success.
    //

    return TRUE;
}

// vim:set ts=8 sw=4 sts=4 tw=80 expandtab                                     :
