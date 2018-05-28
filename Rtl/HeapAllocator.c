/*++

Copyright (c) 2017 Trent Nelson <trent@trent.me>

Module Name:

    HeapAllocator.c

Abstract:

    This module implements an ALLOCATOR interface around the standard Win32
    heap functions (HeapCreate(), HeapAlloc() etc).

--*/

#include "stdafx.h"

#define ContextToHeapHandle(Context) (((PALLOCATOR)(Context))->HeapHandle)
#define PTR_SZ (sizeof(void *))

_Use_decl_annotations_
PVOID
RtlHeapAllocatorMalloc(
    PVOID Context,
    SIZE_T Size
    )
{
    return HeapAlloc(ContextToHeapHandle(Context), 0, Size);
}

_Use_decl_annotations_
PVOID
RtlHeapAllocatorCalloc(
    PVOID Context,
    SIZE_T NumberOfElements,
    SIZE_T ElementSize
    )
{
    SIZE_T Size = NumberOfElements * ElementSize;
    return HeapAlloc(
        ContextToHeapHandle(Context),
        HEAP_ZERO_MEMORY,
        Size
    );
}

_Use_decl_annotations_
PVOID
RtlHeapAllocatorRealloc(
    PVOID Context,
    PVOID Buffer,
    SIZE_T NewSize
    )
{
    return HeapReAlloc(ContextToHeapHandle(Context), 0, Buffer, NewSize);
}

_Use_decl_annotations_
VOID
RtlHeapAllocatorFree(
    PVOID Context,
    PVOID Buffer
    )
{
    HeapFree(ContextToHeapHandle(Context), 0, Buffer);
    return;
}

_Use_decl_annotations_
VOID
RtlHeapAllocatorFreePointer(
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

    RtlHeapAllocatorFree(Context, *BufferPointer);
    *BufferPointer = NULL;

    return;
}

_Use_decl_annotations_
VOID
RtlHeapAllocatorDestroy(
    PALLOCATOR Allocator
    )
{
    if (!Allocator) {
        return;
    }

    if (Allocator->HeapHandle) {
        HeapDestroy(Allocator->HeapHandle);
        Allocator->HeapHandle = NULL;
    }

    return;
}

_Use_decl_annotations_
PVOID
RtlHeapAllocatorTryMalloc(
    PVOID Context,
    SIZE_T Size
    )
{
    return RtlHeapAllocatorMalloc(Context, Size);
}

_Use_decl_annotations_
PVOID
RtlHeapAllocatorTryCalloc(
    PVOID Context,
    SIZE_T NumberOfElements,
    SIZE_T ElementSize
    )
{
    return RtlHeapAllocatorCalloc(Context, NumberOfElements, ElementSize);
}

_Use_decl_annotations_
PVOID
RtlHeapAllocatorMallocWithTimestamp(
    PVOID Context,
    SIZE_T Size,
    PLARGE_INTEGER TimestampPointer
    )
{
    UNREFERENCED_PARAMETER(TimestampPointer);
    return RtlHeapAllocatorMalloc(Context, Size);
}

_Use_decl_annotations_
PVOID
RtlHeapAllocatorCallocWithTimestamp(
    PVOID Context,
    SIZE_T NumberOfElements,
    SIZE_T ElementSize,
    PLARGE_INTEGER TimestampPointer
    )
{
    UNREFERENCED_PARAMETER(TimestampPointer);
    return RtlHeapAllocatorCalloc(Context, NumberOfElements, ElementSize);
}

_Use_decl_annotations_
PVOID
RtlHeapAllocatorTryMallocWithTimestamp(
    PVOID Context,
    SIZE_T Size,
    PLARGE_INTEGER TimestampPointer
    )
{
    UNREFERENCED_PARAMETER(TimestampPointer);
    return RtlHeapAllocatorMalloc(Context, Size);
}

_Use_decl_annotations_
PVOID
RtlHeapAllocatorTryCallocWithTimestamp(
    PVOID Context,
    SIZE_T NumberOfElements,
    SIZE_T ElementSize,
    PLARGE_INTEGER TimestampPointer
    )
{
    UNREFERENCED_PARAMETER(TimestampPointer);
    return RtlHeapAllocatorCalloc(Context, NumberOfElements, ElementSize);
}

_Use_decl_annotations_
BOOLEAN
RtlHeapAllocatorWriteBytes(
    PVOID Context,
    const VOID *Source,
    SIZE_T SizeInBytes
    )
{
    PVOID Dest;

    Dest = RtlHeapAllocatorMalloc(Context, SizeInBytes);
    if (!Dest) {
        return FALSE;
    }

    CopyMemory(Dest, Source, SizeInBytes);

    return TRUE;
}


//
// Define aligned offset malloc-oriented functions.
//

_Use_decl_annotations_
PVOID
RtlHeapAllocatorAlignedOffsetMalloc(
    PVOID Context,
    SIZE_T Size,
    SIZE_T Alignment,
    SIZE_T Offset
    )
{
    ULONG_PTR Buffer;
    ULONG_PTR Padding;
    ULONG_PTR ReturnAddress;
    ULONG_PTR AddressPointer;
    SIZE_T Overhead;
    SIZE_T AllocSize;
    SIZE_T Align;

    Align = (Alignment > PTR_SZ ? Alignment : PTR_SZ) - 1;

    Padding = (0 - Offset) & (PTR_SZ - 1);
    Overhead = (PTR_SZ + Padding + Align);
    AllocSize = Overhead + Size;

    Buffer = (ULONG_PTR)RtlHeapAllocatorMalloc(Context, AllocSize);
    if (!Buffer) {
        return NULL;
    }

    ReturnAddress = ((Buffer + Overhead + Offset) & ~Align) - Offset;
    AddressPointer = (ReturnAddress - Padding) - sizeof(ULONG_PTR);
    ((ULONG_PTR *)(ReturnAddress - Padding))[-1] = Buffer;

    return (PVOID)ReturnAddress;
}

_Use_decl_annotations_
PVOID
RtlHeapAllocatorTryAlignedOffsetMalloc(
    PVOID Context,
    SIZE_T Size,
    SIZE_T Alignment,
    SIZE_T Offset
    )
{
    return RtlHeapAllocatorAlignedOffsetMalloc(Context,
                                               Size,
                                               Alignment,
                                               Offset);
}

_Use_decl_annotations_
PVOID
RtlHeapAllocatorAlignedOffsetMallocWithTimestamp(
    PVOID Context,
    SIZE_T Size,
    SIZE_T Alignment,
    SIZE_T Offset,
    PLARGE_INTEGER TimestampPointer
    )
{
    UNREFERENCED_PARAMETER(TimestampPointer);
    return RtlHeapAllocatorAlignedOffsetMalloc(Context,
                                               Size,
                                               Alignment,
                                               Offset);
}

_Use_decl_annotations_
PVOID
RtlHeapAllocatorTryAlignedOffsetMallocWithTimestamp(
    PVOID Context,
    SIZE_T Size,
    SIZE_T Alignment,
    SIZE_T Offset,
    PLARGE_INTEGER TimestampPointer
    )
{
    UNREFERENCED_PARAMETER(TimestampPointer);
    return RtlHeapAllocatorAlignedOffsetMalloc(Context,
                                               Size,
                                               Alignment,
                                               Offset);
}

//
// Define aligned malloc-oriented functions.
//

_Use_decl_annotations_
PVOID
RtlHeapAllocatorAlignedMalloc(
    PVOID Context,
    SIZE_T Size,
    SIZE_T Alignment
    )
{
    return RtlHeapAllocatorAlignedOffsetMalloc(Context, Size, Alignment, 0);
}

_Use_decl_annotations_
PVOID
RtlHeapAllocatorAlignedMallocWithTimestamp(
    PVOID Context,
    SIZE_T Size,
    SIZE_T Alignment,
    PLARGE_INTEGER TimestampPointer
    )
{
    UNREFERENCED_PARAMETER(TimestampPointer);
    return RtlHeapAllocatorAlignedOffsetMalloc(Context, Size, Alignment, 0);
}

_Use_decl_annotations_
PVOID
RtlHeapAllocatorTryAlignedMalloc(
    PVOID Context,
    SIZE_T Size,
    SIZE_T Alignment
    )
{
    return RtlHeapAllocatorAlignedOffsetMalloc(Context, Size, Alignment, 0);
}

_Use_decl_annotations_
PVOID
RtlHeapAllocatorTryAlignedMallocWithTimestamp(
    PVOID Context,
    SIZE_T Size,
    SIZE_T Alignment,
    PLARGE_INTEGER TimestampPointer
    )
{
    UNREFERENCED_PARAMETER(TimestampPointer);
    return RtlHeapAllocatorAlignedOffsetMalloc(Context, Size, Alignment, 0);
}

//
// Define aligned calloc-oriented functions.
//

_Use_decl_annotations_
PVOID
RtlHeapAllocatorAlignedCalloc(
    PVOID Context,
    SIZE_T NumberOfElements,
    SIZE_T ElementSize,
    SIZE_T Alignment
    )
{
    SIZE_T Size = NumberOfElements * ElementSize;

    return RtlHeapAllocatorAlignedOffsetMalloc(Context, Size, Alignment, 0);
}

_Use_decl_annotations_
PVOID
RtlHeapAllocatorTryAlignedCalloc(
    PVOID Context,
    SIZE_T NumberOfElements,
    SIZE_T ElementSize,
    SIZE_T Alignment
    )
{
    SIZE_T Size = NumberOfElements * ElementSize;

    return RtlHeapAllocatorAlignedOffsetMalloc(Context, Size, Alignment, 0);
}

_Use_decl_annotations_
PVOID
RtlHeapAllocatorAlignedCallocWithTimestamp(
    PVOID Context,
    SIZE_T NumberOfElements,
    SIZE_T ElementSize,
    SIZE_T Alignment,
    PLARGE_INTEGER TimestampPointer
    )
{
    UNREFERENCED_PARAMETER(TimestampPointer);
    SIZE_T Size = NumberOfElements * ElementSize;

    return RtlHeapAllocatorAlignedOffsetMalloc(Context, Size, Alignment, 0);
}

_Use_decl_annotations_
PVOID
RtlHeapAllocatorTryAlignedCallocWithTimestamp(
    PVOID Context,
    SIZE_T NumberOfElements,
    SIZE_T ElementSize,
    SIZE_T Alignment,
    PLARGE_INTEGER TimestampPointer
    )
{
    UNREFERENCED_PARAMETER(TimestampPointer);
    SIZE_T Size = NumberOfElements * ElementSize;

    return RtlHeapAllocatorAlignedOffsetMalloc(Context, Size, Alignment, 0);
}

//
// Define aligned offset calloc-oriented functions.
//

_Use_decl_annotations_
PVOID
RtlHeapAllocatorAlignedOffsetCalloc(
    PVOID Context,
    SIZE_T NumberOfElements,
    SIZE_T ElementSize,
    SIZE_T Alignment,
    SIZE_T Offset
    )
{
    SIZE_T Size = NumberOfElements * ElementSize;

    return RtlHeapAllocatorAlignedOffsetMalloc(Context,
                                               Size,
                                               Alignment,
                                               Offset);
}


_Use_decl_annotations_
PVOID
RtlHeapAllocatorTryAlignedOffsetCalloc(
    PVOID Context,
    SIZE_T NumberOfElements,
    SIZE_T ElementSize,
    SIZE_T Alignment,
    SIZE_T Offset
    )
{
    SIZE_T Size = NumberOfElements * ElementSize;

    return RtlHeapAllocatorAlignedOffsetMalloc(Context,
                                               Size,
                                               Alignment,
                                               Offset);
}

_Use_decl_annotations_
PVOID
RtlHeapAllocatorAlignedOffsetCallocWithTimestamp(
    PVOID Context,
    SIZE_T NumberOfElements,
    SIZE_T ElementSize,
    SIZE_T Alignment,
    SIZE_T Offset,
    PLARGE_INTEGER TimestampPointer
    )
{
    UNREFERENCED_PARAMETER(TimestampPointer);
    SIZE_T Size = NumberOfElements * ElementSize;

    return RtlHeapAllocatorAlignedOffsetMalloc(Context,
                                               Size,
                                               Alignment,
                                               Offset);
}

_Use_decl_annotations_
PVOID
RtlHeapAllocatorTryAlignedOffsetCallocWithTimestamp(
    PVOID Context,
    SIZE_T NumberOfElements,
    SIZE_T ElementSize,
    SIZE_T Alignment,
    SIZE_T Offset,
    PLARGE_INTEGER TimestampPointer
    )
{
    UNREFERENCED_PARAMETER(TimestampPointer);
    SIZE_T Size = NumberOfElements * ElementSize;

    return RtlHeapAllocatorAlignedOffsetMalloc(Context,
                                               Size,
                                               Alignment,
                                               Offset);
}

//
// Define aligned free functions.
//

_Use_decl_annotations_
VOID
RtlHeapAllocatorAlignedFree(
    PVOID Context,
    PVOID Buffer
    )
{
    ULONG_PTR Address;

    if (!Buffer) {
        return;
    }

    Address = (ULONG_PTR)Buffer;

    Address = (Address & ~(PTR_SZ - 1)) - PTR_SZ;
    Address = *((ULONG_PTR *)Address);

    ASSERT(Address != 0);

    RtlHeapAllocatorFree(Context, (PVOID)Address);
    return;
}

_Use_decl_annotations_
VOID
RtlHeapAllocatorAlignedFreePointer(
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

    RtlHeapAllocatorAlignedFree(Context, *BufferPointer);
    *BufferPointer = NULL;

    return;
}

//
// Define initialization functions.
//

_Use_decl_annotations_
BOOLEAN
RtlHeapAllocatorInitialize(
    PALLOCATOR Allocator
    )
{
    HANDLE HeapHandle;

    if (!Allocator) {
        return FALSE;
    }

    HeapHandle = HeapCreate(0, 0, 0);
    if (!HeapHandle) {
        return FALSE;
    }

    InitializeAllocator(
        Allocator,
        Allocator,
        RtlHeapAllocatorMalloc,
        RtlHeapAllocatorCalloc,
        RtlHeapAllocatorRealloc,
        RtlHeapAllocatorFree,
        RtlHeapAllocatorFreePointer,
        RtlHeapAllocatorInitialize,
        RtlHeapAllocatorDestroy,
        RtlHeapAllocatorWriteBytes,
        RtlHeapAllocatorTryMalloc,
        RtlHeapAllocatorTryCalloc,
        RtlHeapAllocatorMallocWithTimestamp,
        RtlHeapAllocatorCallocWithTimestamp,
        RtlHeapAllocatorTryMallocWithTimestamp,
        RtlHeapAllocatorTryCallocWithTimestamp,
        RtlHeapAllocatorAlignedMalloc,
        RtlHeapAllocatorTryAlignedMalloc,
        RtlHeapAllocatorAlignedMallocWithTimestamp,
        RtlHeapAllocatorTryAlignedMallocWithTimestamp,
        RtlHeapAllocatorAlignedOffsetMalloc,
        RtlHeapAllocatorTryAlignedOffsetMalloc,
        RtlHeapAllocatorAlignedOffsetMallocWithTimestamp,
        RtlHeapAllocatorTryAlignedOffsetMallocWithTimestamp,
        RtlHeapAllocatorAlignedCalloc,
        RtlHeapAllocatorTryAlignedCalloc,
        RtlHeapAllocatorAlignedCallocWithTimestamp,
        RtlHeapAllocatorTryAlignedCallocWithTimestamp,
        RtlHeapAllocatorAlignedOffsetCalloc,
        RtlHeapAllocatorTryAlignedOffsetCalloc,
        RtlHeapAllocatorAlignedOffsetCallocWithTimestamp,
        RtlHeapAllocatorTryAlignedOffsetCallocWithTimestamp,
        RtlHeapAllocatorAlignedFree,
        RtlHeapAllocatorAlignedFreePointer,
        HeapHandle
    );

    return TRUE;
}

_Use_decl_annotations_
BOOL
RtlHeapAllocatorInitializeEx(
    PALLOCATOR Allocator,
    DWORD HeapCreateOptions,
    SIZE_T InitialSize,
    SIZE_T MaximumSize
    )
{
    return RtlInitializeHeapAllocatorExInline(Allocator,
                                              HeapCreateOptions,
                                              InitialSize,
                                              MaximumSize);
}

// vim:set ts=8 sw=4 sts=4 tw=80 expandtab                                     :
