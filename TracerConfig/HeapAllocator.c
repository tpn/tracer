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

PALLOCATOR GlobalAllocator;

_Use_decl_annotations_
BOOLEAN
CreateAndInitializeDefaultHeapAllocator(
    _Out_ PPALLOCATOR AllocatorPointer
    )
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
        HeapHandle
    );

    //
    // Update the caller's pointer and return success.
    //

    *AllocatorPointer = Allocator;

    return TRUE;
}

// vim:set ts=8 sw=4 sts=4 tw=80 expandtab                                     :
