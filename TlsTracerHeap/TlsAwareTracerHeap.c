#include "stdafx.h"

#define ContextToDefaultHeapHandle(Context) \
    (((PALLOCATOR)(Context))->HeapHandle)

#define ContextToTlsHeapHandle(Context) \
    (((PALLOCATOR)TlsGetValue(((PALLOCATOR)(Context))->TlsIndex))->HeapHandle)

FORCEINLINE
HANDLE
ContextToHeapHandle(_In_ PVOID Context)
{
    PALLOCATOR Allocator;

    Allocator = (PALLOCATOR)Context;

    if (Allocator->Flags.IsTlsRedirectionEnabled) {
        if (Allocator->ThreadId != FastGetCurrentThreadId()) {
            return ContextToTlsHeapHandle(Allocator);
        } else {
            return Allocator->HeapHandle;
        }
    } else {
        return Allocator->HeapHandle;
    }
}

_Use_decl_annotations_
PVOID
TlsAwareMalloc(
    PVOID Context,
    SIZE_T Size
    )
{
    return HeapAlloc(ContextToHeapHandle(Context), 0, Size);
}

_Use_decl_annotations_
PVOID
TlsAwareCalloc(
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
TlsAwareRealloc(
    PVOID Context,
    PVOID Buffer,
    SIZE_T NewSize
    )
{
    return HeapReAlloc(ContextToHeapHandle(Context), 0, Buffer, NewSize);
}

_Use_decl_annotations_
VOID
TlsAwareFree(
    PVOID Context,
    PVOID Buffer
    )
{
    HeapFree(ContextToHeapHandle(Context), 0, Buffer);
    return;
}

_Use_decl_annotations_
VOID
TlsAwareFreePointer(
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

    TlsAwareFree(Context, *BufferPointer);
    *BufferPointer = NULL;

    return;
}

// vim:set ts=8 sw=4 sts=4 tw=80 expandtab                                     :
