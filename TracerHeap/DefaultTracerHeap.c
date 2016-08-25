
#include "stdafx.h"

_Use_decl_annotations_
PVOID
DefaultHeapMalloc(
    PVOID Context,
    SIZE_T Size
    )
{
    return HeapAlloc((HANDLE)Context, 0, Size);
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
        (HANDLE)Context,
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
    return HeapReAlloc((HANDLE)Context, 0, Buffer, NewSize);
}

_Use_decl_annotations_
VOID
DefaultHeapFree(
    PVOID Context,
    PVOID Buffer
    )
{
    HeapFree((HANDLE)Context, 0, Buffer);
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

    if (Allocator->Context) {
        HeapDestroy((HANDLE)Allocator->Context);
    }

    return;
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

    HeapHandle = HeapCreate(HEAP_GENERATE_EXCEPTIONS, 0, 0);
    if (!HeapHandle) {
        return FALSE;
    }

    Allocator->Context = HeapHandle;

    Allocator->Malloc = DefaultHeapMalloc;
    Allocator->Calloc = DefaultHeapCalloc;
    Allocator->Realloc = DefaultHeapRealloc;
    Allocator->Free = DefaultHeapFree;

    Allocator->Initialize = DefaultHeapInitializeAllocator;
    Allocator->Destroy = DefaultHeapDestroyAllocator;

    return TRUE;

}

// vim:set ts=8 sw=4 sts=4 tw=80 expandtab                                     :
