/*++

Copyright (c) 2016 Trent Nelson <trent@trent.me>

Module Name:

    StringTableAllocator.c

Abstract:

    This module implements the functionality related to string table allocators.
    It provides routines for initializing and destroying ALLOCATOR structures
    geared specifically toward the strict alignment requirements of the
    STRING_TABLE and STRING_ARRAY structures.

    This module is currently a wrapper around the functionality exposed by
    ../TracerHeap/AlignedTracerHeap.c.

--*/

#include "stdafx.h"

#include "../TracerHeap/AlignedTracerHeap.c"

_Use_decl_annotations_
BOOL
InitializeStringTableAllocator(
    PALLOCATOR Allocator
    )
/*++

Routine Description:

    Initializes an ALLOCATOR structure such that it can be used for string
    table allocations.

Arguments:

    Allocator - Supplies a pointer to an ALLOCATOR structure to be initialized.

Return Value:

    TRUE if the allocator was successfully initialized, FALSE on failure.

--*/
{
    //
    // Validate arguments.
    //

    if (!ARGUMENT_PRESENT(Allocator)) {
        return FALSE;
    }

    return AlignedHeapInitializeAllocator(Allocator);
}

_Use_decl_annotations_
VOID
DestroyStringTableAllocator(
    PALLOCATOR Allocator
    )
/*++

Routine Description:

    Destroys an ALLOCATOR previously initialized by the
    InitializeStringTableAllocator() routine.

Arguments:

    Allocator - Supplies a pointer to an ALLOCATOR structure to be destroyed.

Return Value:

    None.

--*/
{
    //
    // Validate arguments.
    //

    if (!ARGUMENT_PRESENT(Allocator)) {
        return;
    }

    AlignedHeapDestroyAllocator(Allocator);

}

// vim:set ts=8 sw=4 sts=4 tw=80 expandtab                                     :
