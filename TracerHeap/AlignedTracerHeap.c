/*++

Copyright (c) 2016 Trent Nelson <trent@trent.me>

Module Name:

    AlignedTracerHeap.c

Abstract:

    This module implements an ALLOCATOR interface for performing aligned
    memory allocations.

--*/

#include "stdafx.h"

FORCEINLINE
SIZE_T
Align(_In_ SIZE_T Size)
{
    SIZE_T NumberOfBitsSet = __popcnt64(Size);
    SIZE_T LeadingZeros;
    SIZE_T PowerOfTwo;
    SIZE_T NextPowerOfTwo;
    SIZE_T NewSize;

    if (NumberOfBitsSet == 1) {
        return Size;
    }

    LeadingZeros = _lzcnt_u64(Size);
    PowerOfTwo = 63 - LeadingZeros;
    NextPowerOfTwo = PowerOfTwo + 1;
    NewSize = 1ULL << NextPowerOfTwo;

    return NewSize;
}

_Use_decl_annotations_
PVOID
AlignedHeapMalloc(
    PVOID Context,
    SIZE_T Size
    )
{
    return _aligned_malloc(Size, Align(Size));
}

_Use_decl_annotations_
PVOID
AlignedHeapCalloc(
    PVOID Context,
    SIZE_T NumberOfElements,
    SIZE_T SizeOfElements
    )
{
    PVOID Buffer;
    SIZE_T Size = NumberOfElements * SizeOfElements;
    SIZE_T AlignedSize = Align(Size);
    Buffer = _aligned_malloc(Size, AlignedSize);
    if (Buffer) {
        SecureZeroMemory(Buffer, AlignedSize);
    }
    return Buffer;
}

_Use_decl_annotations_
PVOID
AlignedHeapRealloc(
    PVOID Context,
    PVOID Buffer,
    SIZE_T NewSize
    )
{
    return _aligned_realloc(Buffer, NewSize, Align(NewSize));
}

_Use_decl_annotations_
VOID
AlignedHeapFree(
    PVOID Context,
    PVOID Buffer
    )
{
    _aligned_free(Buffer);
}

_Use_decl_annotations_
VOID
AlignedHeapFreePointer(
    PVOID Context,
    PPVOID BufferPointer
    )
{
    if (!ARGUMENT_PRESENT(BufferPointer)) {
        return;
    }

    if (!ARGUMENT_PRESENT(*BufferPointer)) {
        return;
    }

    AlignedHeapFree(Context, *BufferPointer);
    *BufferPointer = NULL;

    return;
}

_Use_decl_annotations_
VOID
AlignedHeapDestroyAllocator(
    PALLOCATOR Allocator
    )
{
    return;
}

_Use_decl_annotations_
BOOLEAN
AlignedHeapInitializeAllocator(
    PALLOCATOR Allocator
    )
{
    if (!Allocator) {
        return FALSE;
    }

    InitializeAllocator(
        Allocator,
        Allocator,
        AlignedHeapMalloc,
        AlignedHeapCalloc,
        AlignedHeapRealloc,
        AlignedHeapFree,
        AlignedHeapFreePointer,
        AlignedHeapInitializeAllocator,
        AlignedHeapDestroyAllocator,
        NULL
    );

    return TRUE;
}

_Use_decl_annotations_
BOOL
InitializeAlignedAllocator(
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
DestroyAlignedAllocator(
    PALLOCATOR Allocator
    )
/*++

Routine Description:

    Destroys an ALLOCATOR structure that was initialized by the routine
    InitializeAlignedAllocator().

Arguments:

    Allocator - Supplies a pointer to an ALLOCATOR structure to be destroyed.
        If this value is NULL, the routine returns immediately.

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
