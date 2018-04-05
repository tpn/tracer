/*++

Copyright (c) 2016 Trent Nelson <trent@trent.me>

Module Name:

    HeapAllocator.c

Abstract:

    This module implements the routine CreateAndInitializeDefaultHeapAllocator,
    which ensures dynamic clients (such as Python via ctypes) have a means for
    obtaining an allocator that can be passed to InitializeTracerConfig()
    without needing to also load the TracerHeap.dll module first.

    It also implements GetOrCreateGlobalAllocator, which will return a global
    allocator.

--*/

#include "stdafx.h"

PALLOCATOR GlobalAllocator = NULL;

_Use_decl_annotations_
BOOLEAN
CreateAndInitializeDefaultHeapAllocator(
    PPALLOCATOR AllocatorPointer
    )
/*++

Routine Description:

    This routine creates a new heap via HeapCreate(), allocates sufficient
    space for an ALLOCATOR structure, then initializes the structure via the
    default heap allocation methods (e.g. DefaultHeapMalloc, etc) from the
    TracerHeap static library.

Arguments:

    AllocatorPointer - Supplies a pointer to a variable that will receive
        the address of the newly-created and initialized ALLOCATOR structure
        if the routine was successful, NULL otherwise.

Return Value:

    TRUE on success, FALSE on failure.

--*/
{
    HANDLE HeapHandle;
    PALLOCATOR Allocator;

    //
    // Validate arguments.
    //

    if (!ARGUMENT_PRESENT(AllocatorPointer)) {
        return FALSE;
    }

    //
    // Clear the caller's pointer up-front.
    //

    *AllocatorPointer = NULL;

    //
    // Create a new heap.
    //

    HeapHandle = HeapCreate(HEAP_GENERATE_EXCEPTIONS, 0, 0);
    if (!HeapHandle) {
        return FALSE;
    }

    //
    // Allocate sufficient space for an allocator.
    //

    Allocator = HeapAlloc(HeapHandle, HEAP_ZERO_MEMORY, sizeof(ALLOCATOR));

    if (!Allocator) {
        return FALSE;
    }

    //
    // Initialize the structure with the default heap allocation methods.
    //

    InitializeAllocator(
        Allocator,
        Allocator,
        DefaultHeapMalloc,
        DefaultHeapCalloc,
        DefaultHeapRealloc,
        DefaultHeapFree,
        DefaultHeapFreePointer,
        DefaultHeapInitializeAllocator,
        DefaultHeapDestroyAllocator,
        NULL,
        NULL,
        NULL,
        NULL,
        NULL,
        NULL,
        NULL,
        NULL,
        NULL,
        NULL,
        NULL,
        NULL,
        NULL,
        NULL,
        NULL,
        NULL,
        NULL,
        NULL,
        NULL,
        NULL,
        NULL,
        NULL,
        NULL,
        NULL,
        NULL,
        HeapHandle
    );

    //
    // Update the caller's pointer and return success.
    //

    *AllocatorPointer = Allocator;

    return TRUE;
}

_Use_decl_annotations_
BOOLEAN
GetOrCreateGlobalAllocator(
    PPALLOCATOR AllocatorPointer
    )
/*++

Routine Description:

    Upon first invocation, this routine will create and initialize a new heap
    allocator structure via CreateAndInitializeDefaultHeapAllocator() and then
    return the value to the caller.

    Subsequent invocations will return the same structure.

    No synchronization is provided by this routine, so the caller must ensure
    multiple threads aren't attempting to call this method simultaneously.

Arguments:

    AllocatorPointer - Supplies a pointer to a variable that will receive
        the address of the global ALLOCATOR structure, or NULL if an error
        has occurred.

Return Value:

    TRUE on success, FALSE on failure.

--*/
{

    //
    // Validate arguments.
    //

    if (!ARGUMENT_PRESENT(AllocatorPointer)) {
        return FALSE;
    }


    //
    // Create and initialize the global allocator if needed.
    //

    if (!GlobalAllocator) {

        if (CreateAndInitializeDefaultHeapAllocator(&GlobalAllocator)) {

            //
            // If the call succeeded, disable the destroy allocator function
            // pointer to prevent callers from destroying the global allocator.
            //

            GlobalAllocator->Destroy = NULL;
        }
    }

    //
    // Update the caller's pointer and return TRUE if the allocator is non-NULL.
    //

    *AllocatorPointer = GlobalAllocator;

    return (GlobalAllocator != NULL);
}

// vim:set ts=8 sw=4 sts=4 tw=80 expandtab                                     :
