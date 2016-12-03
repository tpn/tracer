/*++

Copyright (c) 2016 Trent Nelson <trent@trent.me>

Module Name:

    TlsTracerHeapThreadDetach.c

Abstract:

    This module implements the detach routine that is called when the process
    receives a DLL_THREAD_DETACH indication from _DllMainCRTStartup().

--*/

#include "stdafx.h"

_Use_decl_annotations_
BOOL
TlsTracerHeapThreadDetach(
    HMODULE Module,
    ULONG   Reason,
    LPVOID  Reserved
    )
/*++

Routine Description:

    Destroys the thread-local ALLOCATOR structure that was initialized when
    the DLL was loaded.  Additionally, if this was the last thread attached
    to the allocator, disable TLS awareness and restore the global allocator.

Arguments:

    Module - Unused.

    Reason - Unused.

    Reserved - Used to determine if the process is terminating.

Return Value:

    TRUE on success, FALSE on failure.

--*/
{
    BOOL IsProcessTerminating;
    PALLOCATOR GlobalAllocator;
    PALLOCATOR TlsAllocator;

    UNREFERENCED_PARAMETER(Module);
    UNREFERENCED_PARAMETER(Reason);

    //
    // Ensure TracerConfig is set.
    //

    if (!TracerConfig) {
        return FALSE;
    }

    IsProcessTerminating = (Reserved != NULL);

    if (IsProcessTerminating) {
        return TRUE;
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

    //
    // Disable the following line for now as it seems to do nothing but cause
    // issues.

    //OriginalFree(GlobalAllocator->Context, TlsAllocator);

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
