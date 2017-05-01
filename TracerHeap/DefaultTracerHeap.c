/*++

Copyright (c) 2016 Trent Nelson <trent@trent.me>

Module Name:

    DefaultTracerHeap.c

Abstract:

    This module implements an ALLOCATOR interface around the standard Win32
    heap functions (HeapCreate(), HeapAlloc() etc).

--*/

#include "stdafx.h"

#define ContextToHeapHandle(Context) \
    (((PALLOCATOR)(Context))->HeapHandle)

_Use_decl_annotations_
PVOID
DefaultHeapMalloc(
    PVOID Context,
    SIZE_T Size
    )
{
    return HeapAlloc(
        ContextToHeapHandle(Context),
        HEAP_ZERO_MEMORY,
        Size
    );
}

_Use_decl_annotations_
PVOID
DefaultHeapCalloc(
    PVOID Context,
    SIZE_T NumberOfElements,
    SIZE_T SizeOfElements
    )
{
    SIZE_T Size = NumberOfElements * SizeOfElements;
    return HeapAlloc(
        ContextToHeapHandle(Context),
        HEAP_ZERO_MEMORY,
        Size
    );
}

_Use_decl_annotations_
PVOID
DefaultHeapRealloc(
    PVOID Context,
    PVOID Buffer,
    SIZE_T NewSize
    )
{
    return HeapReAlloc(ContextToHeapHandle(Context), 0, Buffer, NewSize);
}

_Use_decl_annotations_
VOID
DefaultHeapFree(
    PVOID Context,
    PVOID Buffer
    )
{
    HeapFree(ContextToHeapHandle(Context), 0, Buffer);
    return;
}

_Use_decl_annotations_
VOID
DefaultHeapFreePointer(
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

    DefaultHeapFree(Context, *BufferPointer);
    *BufferPointer = NULL;

    return;
}

_Use_decl_annotations_
VOID
DefaultHeapDestroyAllocator(
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
DefaultHeapTryMalloc(
    PVOID Context,
    SIZE_T Size
    )
{
    return DefaultHeapMalloc(Context, Size);
}

_Use_decl_annotations_
PVOID
DefaultHeapTryCalloc(
    PVOID Context,
    SIZE_T NumberOfElements,
    SIZE_T ElementSize
    )
{
    return DefaultHeapCalloc(Context, NumberOfElements, ElementSize);
}

_Use_decl_annotations_
PVOID
DefaultHeapMallocWithTimestamp(
    PVOID Context,
    SIZE_T Size,
    PLARGE_INTEGER TimestampPointer
    )
{
    UNREFERENCED_PARAMETER(TimestampPointer);
    return DefaultHeapMalloc(Context, Size);
}

_Use_decl_annotations_
PVOID
DefaultHeapCallocWithTimestamp(
    PVOID Context,
    SIZE_T NumberOfElements,
    SIZE_T ElementSize,
    PLARGE_INTEGER TimestampPointer
    )
{
    UNREFERENCED_PARAMETER(TimestampPointer);
    return DefaultHeapCalloc(Context, NumberOfElements, ElementSize);
}

_Use_decl_annotations_
PVOID
DefaultHeapTryMallocWithTimestamp(
    PVOID Context,
    SIZE_T Size,
    PLARGE_INTEGER TimestampPointer
    )
{
    UNREFERENCED_PARAMETER(TimestampPointer);
    return DefaultHeapMalloc(Context, Size);
}

_Use_decl_annotations_
PVOID
DefaultHeapTryCallocWithTimestamp(
    PVOID Context,
    SIZE_T NumberOfElements,
    SIZE_T ElementSize,
    PLARGE_INTEGER TimestampPointer
    )
{
    UNREFERENCED_PARAMETER(TimestampPointer);
    return DefaultHeapCalloc(Context, NumberOfElements, ElementSize);
}

_Use_decl_annotations_
BOOLEAN
DefaultHeapInitializeAllocator(
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
        DefaultHeapMalloc,
        DefaultHeapCalloc,
        DefaultHeapRealloc,
        DefaultHeapFree,
        DefaultHeapFreePointer,
        DefaultHeapInitializeAllocator,
        DefaultHeapDestroyAllocator,
        DefaultHeapTryMalloc,
        DefaultHeapTryCalloc,
        DefaultHeapMallocWithTimestamp,
        DefaultHeapCallocWithTimestamp,
        DefaultHeapTryMallocWithTimestamp,
        DefaultHeapTryCallocWithTimestamp,
        HeapHandle
    );

    return TRUE;
}

_Use_decl_annotations_
BOOL
InitializeHeapAllocatorEx(
    PALLOCATOR Allocator,
    DWORD HeapCreateOptions,
    SIZE_T InitialSize,
    SIZE_T MaximumSize
    )
{
    return InitializeHeapAllocatorExInline(Allocator,
                                           HeapCreateOptions,
                                           InitialSize,
                                           MaximumSize);
}

// vim:set ts=8 sw=4 sts=4 tw=80 expandtab                                     :
