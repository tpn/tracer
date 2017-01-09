#include "stdafx.h"

#define ContextToHeapHandle(Context) \
    (((PALLOCATOR)TlsGetValue(((PALLOCATOR)(Context))->TlsIndex))->HeapHandle)

_Use_decl_annotations_
PVOID
TlsHeapMalloc(
    PVOID Context,
    SIZE_T Size
    )
{
    return HeapAlloc(ContextToHeapHandle(Context), 0, Size);
}

_Use_decl_annotations_
PVOID
TlsHeapCalloc(
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
TlsHeapRealloc(
    PVOID Context,
    PVOID Buffer,
    SIZE_T NewSize
    )
{
    return HeapReAlloc(ContextToHeapHandle(Context), 0, Buffer, NewSize);
}

_Use_decl_annotations_
VOID
TlsHeapFree(
    PVOID Context,
    PVOID Buffer
    )
{
    HeapFree(ContextToHeapHandle(Context), 0, Buffer);
    return;
}

_Use_decl_annotations_
VOID
TlsHeapFreePointer(
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

    TlsHeapFree(Context, *BufferPointer);
    *BufferPointer = NULL;

    return;
}

_Use_decl_annotations_
VOID
TlsHeapDestroyAllocator(
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
TlsHeapInitializeAllocator(
    PALLOCATOR Allocator
    )
{
    BOOL Success;
    ULONG HeapFlags;
    HANDLE TlsHeapHandle;

    //
    // Validate arguments.
    //

    if (!Allocator) {
        return FALSE;
    }

    if (!TracerConfig) {
        return FALSE;
    }

    if (TlsIndex == TLS_OUT_OF_INDEXES) {
        return FALSE;
    }

    //
    // Create a new thread-local heap.
    //

    HeapFlags = HEAP_GENERATE_EXCEPTIONS | HEAP_NO_SERIALIZE;

    TlsHeapHandle = HeapCreate(HeapFlags, 0, 0);

    if (!TlsHeapHandle) {
        return FALSE;
    }

    InitializeTlsAllocator(
        Allocator,
        Allocator,
        TlsHeapMalloc,
        TlsHeapCalloc,
        TlsHeapRealloc,
        TlsHeapFree,
        TlsHeapFreePointer,
        TlsHeapInitializeAllocator,
        TlsHeapDestroyAllocator,
        NULL,
        NULL,
        NULL,
        NULL,
        NULL,
        NULL,
        TlsHeapHandle,
        TracerConfig->Allocator,
        TlsIndex
    );

    //
    // Finally, set the Tls value for our slot to the newly-created allocator.
    //

    Success = TlsSetValue(TlsIndex, Allocator);
    if (!Success) {
        return FALSE;
    }


    return TRUE;
}

// vim:set ts=8 sw=4 sts=4 tw=80 expandtab                                     :
