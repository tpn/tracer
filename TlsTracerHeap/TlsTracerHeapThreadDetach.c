#include "stdafx.h"

_Use_decl_annotations_
BOOL
TlsTracerHeapThreadDetach(
    HMODULE Module,
    ULONG   Reason,
    LPVOID  Reserved
    )
/*--

Routine Description:

    Creates a new thread-local ALLOCATOR and associates it with the TlsIndex
    slot obtained when the DLL was loaded.

Arguments:

    Module - Unused.

    Reason - Unused.

    Reserved - Unused.

Return Value:

    TRUE on success, FALSE on failure.

--*/
{
    PALLOCATOR GlobalAllocator;
    PALLOCATOR TlsAllocator;

    UNREFERENCED_PARAMETER(Module);
    UNREFERENCED_PARAMETER(Reason);
    UNREFERENCED_PARAMETER(Reserved);

    //
    // Ensure TracerConfig is set.
    //

    if (!TracerConfig) {
        return FALSE;
    }

    GlobalAllocator = TracerConfig->Allocator;

    if (!GlobalAllocator) {
        return FALSE;
    }

    TlsAllocator = (PALLOCATOR)TlsGetValue(TlsIndex);
    if (!TlsAllocator && GetLastError() != ERROR_SUCCESS) {
        return FALSE;
    }

    TlsHeapDestroyAllocator(TlsAllocator);

    OriginalFree(GlobalAllocator->Context, TlsAllocator);

    //
    // If this was the last thread attached to the allocator, disable TLS
    // awareness and restore the old functions.
    //

    if (InterlockedDecrement(&GlobalAllocator->NumberOfThreads) == 0) {

        GlobalAllocator->Malloc = OriginalMalloc;
        GlobalAllocator->Calloc = OriginalCalloc;
        GlobalAllocator->Realloc = OriginalRealloc;
        GlobalAllocator->Free = OriginalFree;
        GlobalAllocator->FreePointer = OriginalFreePointer;

        GlobalAllocator->Flags.IsTlsAware = FALSE;
        GlobalAllocator->Flags.IsTlsRedirectionEnabled = FALSE;
    }

    return TRUE;

}

// vim:set ts=8 sw=4 sts=4 tw=80 expandtab                                     :
