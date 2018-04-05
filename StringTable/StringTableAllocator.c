/*++

Copyright (c) 2017 Trent Nelson <trent@trent.me>

Module Name:

    StringTableAllocator.c

Abstract:

    This module implements an ALLOCATOR interface specifically geared toward
    the alignment requirements of the StringTable component.

--*/

#include "stdafx.h"

#define ContextToHeapHandle(Context) \
    (((PALLOCATOR)(Context))->HeapHandle)

_Use_decl_annotations_
PVOID
StringTableMalloc(
    PVOID Context,
    SIZE_T Size
    )
{
    PCHAR Buffer;
    SIZE_T NewSize;

    NewSize = ALIGN_UP(Size, 512) << 1;

    Buffer = (PCHAR)HeapAlloc(
        ContextToHeapHandle(Context),
        HEAP_ZERO_MEMORY,
        NewSize
    );

    if (IsAligned512(Buffer)) {
        return Buffer;
    } else {
        Buffer = (PCHAR)ALIGN_UP(Buffer, 512);
        AssertAligned512(Buffer);
        return Buffer;
    }
}

_Use_decl_annotations_
PVOID
StringTableCalloc(
    PVOID Context,
    SIZE_T NumberOfElements,
    SIZE_T SizeOfElements
    )
{
    SIZE_T Size = NumberOfElements * SizeOfElements;

    return StringTableMalloc(Context, Size);
}

_Use_decl_annotations_
PVOID
StringTableRealloc(
    PVOID Context,
    PVOID Buffer,
    SIZE_T NewSize
    )
{
    __debugbreak();
    return NULL;
    //return HeapReAlloc(ContextToHeapHandle(Context), 0, Buffer, NewSize);
}

_Use_decl_annotations_
VOID
StringTableFree(
    PVOID Context,
    PVOID Buffer
    )
{
    HeapFree(ContextToHeapHandle(Context), 0, Buffer);
    return;
}

_Use_decl_annotations_
VOID
StringTableFreePointer(
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

    StringTableFree(Context, *BufferPointer);
    *BufferPointer = NULL;

    return;
}

_Use_decl_annotations_
VOID
StringTableDestroyAllocator(
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
BOOLEAN
StringTableInitializeAllocator(
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

    __debugbreak();
    return FALSE;

    /*
    InitializeAllocator(
        Allocator,
        Allocator,
        StringTableMalloc,
        StringTableCalloc,
        StringTableRealloc,
        StringTableFree,
        StringTableFreePointer,
        StringTableInitializeAllocator,
        StringTableDestroyAllocator,
        NULL,
        NULL,
        NULL,
        NULL,
        NULL,
        NULL,
        HeapHandle
    );
    */

    return TRUE;
}

//
// This is the public initializer.
//

_Use_decl_annotations_
BOOL
InitializeStringTableAllocator(
    PALLOCATOR Allocator
    )
{
    return StringTableInitializeAllocator(Allocator);
}

// vim:set ts=8 sw=4 sts=4 tw=80 expandtab                                     :
