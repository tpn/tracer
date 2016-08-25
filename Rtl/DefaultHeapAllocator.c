#include "stdafx.h"
#include "Memory.h"
#include "DefaultHeapAllocator.h"

_Use_decl_annotations_
void * __restrict
DefaultHeapMalloc(
    PVOID Context,
    SIZE_T Size
    )
{
    return HeapAlloc((HANDLE)Context, 0, Size);
}

_Use_decl_annotations_
void * __restrict
DefaultHeapCalloc(
    PVOID Context,
    SIZE_T NumberOfElements,
    SIZE_T SizeOfElements
    )
{
    return HeapAlloc(
        (HANDLE)Context,
        HEAP_ZERO_MEMORY,
        NumberOfElements * SizeOfElements
    );
}

_Use_decl_annotations_
VOID * __restrict
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
        HANDLE Handle = (HANDLE)Allocator->Context;
        if (Handle != GetProcessHeap()) {
            HeapDestroy(Handle);
        }
    }

    SecureZeroMemory(Allocator, sizeof(*Allocator));

    return;
}

_Use_decl_annotations_
BOOLEAN
DefaultHeapInitializeAllocator(
    PALLOCATOR Allocator
    )
{

    if (!Allocator) {
        return FALSE;
    }

    if (!Allocator->Context) {
        Allocator->Context = (HANDLE)GetProcessHeap();
    }

    Allocator->Malloc = DefaultHeapMalloc;
    Allocator->Calloc = DefaultHeapCalloc;
    Allocator->Realloc = DefaultHeapRealloc;
    Allocator->Free = DefaultHeapFree;

    Allocator->Initialize = DefaultHeapInitializeAllocator;
    Allocator->Destroy = DefaultHeapDestroyAllocator;

    return TRUE;

}

// vim:set ts=8 sw=4 sts=4 tw=80 expandtab                                     :