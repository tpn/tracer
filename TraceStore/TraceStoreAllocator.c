/*++

Copyright (c) 2016 Trent Nelson <trent@trent.me>

Module Name:

    TraceStoreAllocator.c

Abstract:

    This module implements an allocator interface for trace stores.  Functions
    are provided to initialize an allocator structure from a trace store, as
    well as the standard malloc, calloc, realloc and free functions.  Only the
    malloc and calloc functions have implementation bodies; realloc and free
    are just no-ops currently.  (Trace stores don't support the notion of
    realloc or free yet.)

--*/

#include "stdafx.h"

#define ContextToTraceStore(Context) (((PALLOCATOR)(Context))->TraceStore)

_Use_decl_annotations_
PVOID
TraceStoreAllocatorMalloc(
    PVOID Context,
    SIZE_T Size
    )
{
    PVOID Address;
    PTRACE_STORE TraceStore;
    PTRACE_CONTEXT TraceContext;
    ULONG_PTR NumberOfRecords = 1;

    TraceStore = ContextToTraceStore(Context);
    TraceContext = TraceStore->TraceContext;

    Address = TraceStore->AllocateRecords(TraceContext,
                                          TraceStore,
                                          NumberOfRecords,
                                          Size);

    return Address;
}

_Use_decl_annotations_
PVOID
TraceStoreAllocatorCalloc(
    PVOID Context,
    SIZE_T NumberOfElements,
    SIZE_T ElementSize
    )
{
    PVOID Address;
    PTRACE_STORE TraceStore;
    PTRACE_CONTEXT TraceContext;

    TraceStore = ContextToTraceStore(Context);
    TraceContext = TraceStore->TraceContext;

    if (ElementSize == 1) {
        __debugbreak();
    }

    Address = TraceStore->AllocateRecords(TraceContext,
                                          TraceStore,
                                          NumberOfElements,
                                          ElementSize);

    return Address;
}

_Use_decl_annotations_
PVOID
TraceStoreAllocatorTryMalloc(
    PVOID Context,
    SIZE_T Size
    )
{
    PVOID Address;
    PTRACE_STORE TraceStore;
    PTRACE_CONTEXT TraceContext;
    ULONG_PTR NumberOfRecords = 1;

    TraceStore = ContextToTraceStore(Context);
    TraceContext = TraceStore->TraceContext;

    Address = TraceStore->TryAllocateRecordsWithTimestamp(TraceContext,
                                                          TraceStore,
                                                          1,
                                                          Size,
                                                          NULL);
    return Address;
}

_Use_decl_annotations_
PVOID
TraceStoreAllocatorTryCalloc(
    PVOID Context,
    SIZE_T NumberOfElements,
    SIZE_T ElementSize
    )
{
    PVOID Address;
    PTRACE_STORE TraceStore;
    PTRACE_CONTEXT TraceContext;

    TraceStore = ContextToTraceStore(Context);
    TraceContext = TraceStore->TraceContext;

    if (ElementSize == 1) {
        __debugbreak();
    }

    Address = TraceStore->TryAllocateRecordsWithTimestamp(TraceContext,
                                                          TraceStore,
                                                          NumberOfElements,
                                                          ElementSize,
                                                          NULL);
    return Address;
}

_Use_decl_annotations_
PVOID
TraceStoreAllocatorMallocWithTimestamp(
    PVOID Context,
    SIZE_T Size,
    PLARGE_INTEGER TimestampPointer
    )
{
    PVOID Address;
    PTRACE_STORE TraceStore;
    PTRACE_CONTEXT TraceContext;
    ULONG_PTR NumberOfRecords = 1;

    TraceStore = ContextToTraceStore(Context);
    TraceContext = TraceStore->TraceContext;

    Address = TraceStore->AllocateRecordsWithTimestamp(TraceContext,
                                                       TraceStore,
                                                       1,
                                                       Size,
                                                       TimestampPointer);
    return Address;
}

_Use_decl_annotations_
PVOID
TraceStoreAllocatorCallocWithTimestamp(
    PVOID Context,
    SIZE_T NumberOfElements,
    SIZE_T ElementSize,
    PLARGE_INTEGER TimestampPointer
    )
{
    PVOID Address;
    PTRACE_STORE TraceStore;
    PTRACE_CONTEXT TraceContext;

    TraceStore = ContextToTraceStore(Context);
    TraceContext = TraceStore->TraceContext;

    if (ElementSize == 1) {
        if (NumberOfElements > 1) {
            __debugbreak();
        }
    }

    Address = TraceStore->AllocateRecordsWithTimestamp(TraceContext,
                                                       TraceStore,
                                                       NumberOfElements,
                                                       ElementSize,
                                                       TimestampPointer);
    return Address;
}

_Use_decl_annotations_
PVOID
TraceStoreAllocatorTryMallocWithTimestamp(
    PVOID Context,
    SIZE_T Size,
    PLARGE_INTEGER TimestampPointer
    )
{
    PVOID Address;
    PTRACE_STORE TraceStore;
    PTRACE_CONTEXT TraceContext;
    ULONG_PTR NumberOfRecords = 1;

    TraceStore = ContextToTraceStore(Context);
    TraceContext = TraceStore->TraceContext;

    Address = TraceStore->TryAllocateRecordsWithTimestamp(TraceContext,
                                                          TraceStore,
                                                          1,
                                                          Size,
                                                          TimestampPointer);
    return Address;
}

_Use_decl_annotations_
PVOID
TraceStoreAllocatorTryCallocWithTimestamp(
    PVOID Context,
    SIZE_T NumberOfElements,
    SIZE_T ElementSize,
    PLARGE_INTEGER TimestampPointer
    )
{
    PVOID Address;
    PTRACE_STORE TraceStore;
    PTRACE_CONTEXT TraceContext;

    TraceStore = ContextToTraceStore(Context);
    TraceContext = TraceStore->TraceContext;

    if (ElementSize == 1) {
        __debugbreak();
    }

    Address = TraceStore->TryAllocateRecordsWithTimestamp(TraceContext,
                                                          TraceStore,
                                                          NumberOfElements,
                                                          ElementSize,
                                                          TimestampPointer);
    return Address;
}

_Use_decl_annotations_
PVOID
TraceStoreAllocatorRealloc(
    PVOID Context,
    PVOID Buffer,
    SIZE_T NewSize
    )
{
    return NULL;
}

_Use_decl_annotations_
VOID
TraceStoreAllocatorFree(
    PVOID Context,
    PVOID Buffer
    )
{
    return;
}

_Use_decl_annotations_
VOID
TraceStoreAllocatorFreePointer(
    PVOID Context,
    PPVOID PointerToBuffer
    )
{
    *PointerToBuffer = NULL;
    return;
}

_Use_decl_annotations_
VOID
TraceStoreDestroyAllocator(
    PALLOCATOR Allocator
    )
{
    return;
}

//
// Aligned variants.
//

_Use_decl_annotations_
BOOLEAN
TraceStoreAllocatorWriteBytes(
    PVOID Context,
    const VOID *Source,
    SIZE_T SizeInBytes
    )
{
    PVOID Dest;

    __debugbreak();

    return FALSE;

    Dest = TraceStoreAllocatorMalloc(Context, SizeInBytes);
    if (!Dest) {
        return FALSE;
    }

    CopyMemory(Dest, Source, SizeInBytes);

    return TRUE;
}

//
// Define helper function for determining the alignment of the next address for
// a trace store's active memory map.
//

BOOL
TraceStoreCheckAlignment(
    PTRACE_STORE TraceStore,
    ULONGLONG Alignment,
    ULONGLONG Offset
    )
{
    PVOID NextAddress;
    ULONGLONG NextAddressAlignment;
    volatile TRACE_STORE_MEMORY_MAP *MemoryMap;

    MemoryMap = (volatile TRACE_STORE_MEMORY_MAP *)TraceStore->MemoryMap;

    //
    // The MemoryMap may be NULL if this is the first allocation and
    // the trace store hasn't completed initialization yet.  Just return
    // TRUE for now; it'll bomb out later if the alignment request can't
    // be satisfied (highly unlikely for the first allocation as it'll be
    // page-size aligned at the very least).
    //

    if (!MemoryMap || !MemoryMap->NextAddress) {
        return TRUE;
    }

    NextAddress = RtlOffsetToPointer(MemoryMap->NextAddress, Offset);
    NextAddressAlignment = GetAddressAlignment(NextAddress);

    return (Alignment < NextAddressAlignment);
}

//
// Define aligned offset malloc-oriented functions.
//

_Use_decl_annotations_
PVOID
TraceStoreAllocatorAlignedOffsetMallocWithTimestamp(
    PVOID Context,
    SIZE_T Size,
    SIZE_T Alignment,
    SIZE_T Offset,
    PLARGE_INTEGER TimestampPointer
    )
{
    PVOID Address;
    PTRACE_STORE TraceStore;
    PTRACE_CONTEXT TraceContext;
    ULONG_PTR NumberOfRecords = 1;

    TraceStore = ContextToTraceStore(Context);
    TraceContext = TraceStore->TraceContext;

    if (!TraceStoreCheckAlignment(TraceStore, Alignment, Offset)) {
        return NULL;
    }

    Address = TraceStore->AllocateRecordsWithTimestamp(TraceContext,
                                                       TraceStore,
                                                       NumberOfRecords,
                                                       Size,
                                                       TimestampPointer);

    return Address;
}

_Use_decl_annotations_
PVOID
TraceStoreAllocatorTryAlignedOffsetMallocWithTimestamp(
    PVOID Context,
    SIZE_T Size,
    SIZE_T Alignment,
    SIZE_T Offset,
    PLARGE_INTEGER TimestampPointer
    )
{
    PVOID Address;
    PTRACE_STORE TraceStore;
    PTRACE_CONTEXT TraceContext;
    ULONG_PTR NumberOfRecords = 1;

    TraceStore = ContextToTraceStore(Context);
    TraceContext = TraceStore->TraceContext;

    if (!TraceStoreCheckAlignment(TraceStore, Alignment, Offset)) {
        return NULL;
    }

    Address = TraceStore->TryAllocateRecordsWithTimestamp(TraceContext,
                                                          TraceStore,
                                                          NumberOfRecords,
                                                          Size,
                                                          TimestampPointer);

    return Address;
}


_Use_decl_annotations_
PVOID
TraceStoreAllocatorAlignedOffsetMalloc(
    PVOID Context,
    SIZE_T Size,
    SIZE_T Alignment,
    SIZE_T Offset
    )
{
    return TraceStoreAllocatorAlignedOffsetMallocWithTimestamp(
        Context,
        Size,
        Alignment,
        Offset,
        NULL
    );
}


_Use_decl_annotations_
PVOID
TraceStoreAllocatorTryAlignedOffsetMalloc(
    PVOID Context,
    SIZE_T Size,
    SIZE_T Alignment,
    SIZE_T Offset
    )
{
    return TraceStoreAllocatorTryAlignedOffsetMallocWithTimestamp(
        Context,
        Size,
        Alignment,
        Offset,
        NULL
    );
}


//
// Define aligned malloc-oriented functions.
//

_Use_decl_annotations_
PVOID
TraceStoreAllocatorAlignedMalloc(
    PVOID Context,
    SIZE_T Size,
    SIZE_T Alignment
    )
{
    return TraceStoreAllocatorAlignedOffsetMallocWithTimestamp(
        Context,
        Size,
        Alignment,
        0,
        NULL
    );
}

_Use_decl_annotations_
PVOID
TraceStoreAllocatorAlignedMallocWithTimestamp(
    PVOID Context,
    SIZE_T Size,
    SIZE_T Alignment,
    PLARGE_INTEGER TimestampPointer
    )
{
    return TraceStoreAllocatorAlignedOffsetMallocWithTimestamp(
        Context,
        Size,
        Alignment,
        0,
        TimestampPointer
    );
}

_Use_decl_annotations_
PVOID
TraceStoreAllocatorTryAlignedMalloc(
    PVOID Context,
    SIZE_T Size,
    SIZE_T Alignment
    )
{
    return TraceStoreAllocatorTryAlignedOffsetMallocWithTimestamp(
        Context,
        Size,
        Alignment,
        0,
        NULL
    );
}

_Use_decl_annotations_
PVOID
TraceStoreAllocatorTryAlignedMallocWithTimestamp(
    PVOID Context,
    SIZE_T Size,
    SIZE_T Alignment,
    PLARGE_INTEGER TimestampPointer
    )
{
    return TraceStoreAllocatorTryAlignedOffsetMallocWithTimestamp(
        Context,
        Size,
        Alignment,
        0,
        TimestampPointer
    );
}

//
// Define aligned offset calloc-oriented functions.
//

_Use_decl_annotations_
PVOID
TraceStoreAllocatorAlignedOffsetCallocWithTimestamp(
    PVOID Context,
    SIZE_T NumberOfElements,
    SIZE_T ElementSize,
    SIZE_T Alignment,
    SIZE_T Offset,
    PLARGE_INTEGER TimestampPointer
    )
{
    PVOID Address;
    PTRACE_STORE TraceStore;
    PTRACE_CONTEXT TraceContext;

    TraceStore = ContextToTraceStore(Context);
    TraceContext = TraceStore->TraceContext;

    if (!TraceStoreCheckAlignment(TraceStore, Alignment, Offset)) {
        return NULL;
    }

    Address = TraceStore->AllocateRecordsWithTimestamp(TraceContext,
                                                       TraceStore,
                                                       NumberOfElements,
                                                       ElementSize,
                                                       TimestampPointer);

    return Address;
}

_Use_decl_annotations_
PVOID
TraceStoreAllocatorTryAlignedOffsetCallocWithTimestamp(
    PVOID Context,
    SIZE_T NumberOfElements,
    SIZE_T ElementSize,
    SIZE_T Alignment,
    SIZE_T Offset,
    PLARGE_INTEGER TimestampPointer
    )
{
    PVOID Address;
    PTRACE_STORE TraceStore;
    PTRACE_CONTEXT TraceContext;

    TraceStore = ContextToTraceStore(Context);
    TraceContext = TraceStore->TraceContext;

    if (!TraceStoreCheckAlignment(TraceStore, Alignment, Offset)) {
        return NULL;
    }

    Address = TraceStore->TryAllocateRecordsWithTimestamp(TraceContext,
                                                          TraceStore,
                                                          NumberOfElements,
                                                          ElementSize,
                                                          TimestampPointer);

    return Address;
}

_Use_decl_annotations_
PVOID
TraceStoreAllocatorAlignedOffsetCalloc(
    PVOID Context,
    SIZE_T NumberOfElements,
    SIZE_T ElementSize,
    SIZE_T Alignment,
    SIZE_T Offset
    )
{
    return TraceStoreAllocatorAlignedOffsetCallocWithTimestamp(
        Context,
        NumberOfElements,
        ElementSize,
        Alignment,
        Offset,
        NULL
    );
}


_Use_decl_annotations_
PVOID
TraceStoreAllocatorTryAlignedOffsetCalloc(
    PVOID Context,
    SIZE_T NumberOfElements,
    SIZE_T ElementSize,
    SIZE_T Alignment,
    SIZE_T Offset
    )
{
    return TraceStoreAllocatorTryAlignedOffsetCallocWithTimestamp(
        Context,
        NumberOfElements,
        ElementSize,
        Alignment,
        Offset,
        NULL
    );
}

//
// Define aligned calloc-oriented functions.
//

_Use_decl_annotations_
PVOID
TraceStoreAllocatorAlignedCallocWithTimestamp(
    PVOID Context,
    SIZE_T NumberOfElements,
    SIZE_T ElementSize,
    SIZE_T Alignment,
    PLARGE_INTEGER TimestampPointer
    )
{
    return TraceStoreAllocatorAlignedOffsetCallocWithTimestamp(
        Context,
        NumberOfElements,
        ElementSize,
        Alignment,
        0,
        TimestampPointer
    );
}

_Use_decl_annotations_
PVOID
TraceStoreAllocatorTryAlignedCalloc(
    PVOID Context,
    SIZE_T NumberOfElements,
    SIZE_T ElementSize,
    SIZE_T Alignment
    )
{
    return TraceStoreAllocatorTryAlignedOffsetCallocWithTimestamp(
        Context,
        NumberOfElements,
        ElementSize,
        Alignment,
        0,
        NULL
    );
}

_Use_decl_annotations_
PVOID
TraceStoreAllocatorAlignedCalloc(
    PVOID Context,
    SIZE_T NumberOfElements,
    SIZE_T ElementSize,
    SIZE_T Alignment
    )
{
    return TraceStoreAllocatorAlignedOffsetCallocWithTimestamp(
        Context,
        NumberOfElements,
        ElementSize,
        Alignment,
        0,
        NULL
    );
}

_Use_decl_annotations_
PVOID
TraceStoreAllocatorTryAlignedCallocWithTimestamp(
    PVOID Context,
    SIZE_T NumberOfElements,
    SIZE_T ElementSize,
    SIZE_T Alignment,
    PLARGE_INTEGER TimestampPointer
    )
{
    return TraceStoreAllocatorTryAlignedOffsetCallocWithTimestamp(
        Context,
        NumberOfElements,
        ElementSize,
        Alignment,
        0,
        TimestampPointer
    );
}

//
// Define aligned free functions.
//

_Use_decl_annotations_
VOID
TraceStoreAllocatorAlignedFree(
    PVOID Context,
    PVOID Buffer
    )
{
    return;
}

_Use_decl_annotations_
VOID
TraceStoreAllocatorAlignedFreePointer(
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

    TraceStoreAllocatorAlignedFree(Context, *BufferPointer);
    *BufferPointer = NULL;

    return;
}



//
// Initializer.
//

_Use_decl_annotations_
BOOLEAN
TraceStoreInitializeAllocator(
    PALLOCATOR Allocator
    )
{

    InitializeAllocator(
        Allocator,
        Allocator,
        TraceStoreAllocatorMalloc,
        TraceStoreAllocatorCalloc,
        TraceStoreAllocatorRealloc,
        TraceStoreAllocatorFree,
        TraceStoreAllocatorFreePointer,
        TraceStoreInitializeAllocator,
        TraceStoreDestroyAllocator,
        TraceStoreAllocatorWriteBytes,
        TraceStoreAllocatorTryMalloc,
        TraceStoreAllocatorTryCalloc,
        TraceStoreAllocatorMallocWithTimestamp,
        TraceStoreAllocatorCallocWithTimestamp,
        TraceStoreAllocatorTryMallocWithTimestamp,
        TraceStoreAllocatorTryCallocWithTimestamp,
        TraceStoreAllocatorAlignedMalloc,
        TraceStoreAllocatorTryAlignedMalloc,
        TraceStoreAllocatorAlignedMallocWithTimestamp,
        TraceStoreAllocatorTryAlignedMallocWithTimestamp,
        TraceStoreAllocatorAlignedOffsetMalloc,
        TraceStoreAllocatorTryAlignedOffsetMalloc,
        TraceStoreAllocatorAlignedOffsetMallocWithTimestamp,
        TraceStoreAllocatorTryAlignedOffsetMallocWithTimestamp,
        TraceStoreAllocatorAlignedCalloc,
        TraceStoreAllocatorTryAlignedCalloc,
        TraceStoreAllocatorAlignedCallocWithTimestamp,
        TraceStoreAllocatorTryAlignedCallocWithTimestamp,
        TraceStoreAllocatorAlignedOffsetCalloc,
        TraceStoreAllocatorTryAlignedOffsetCalloc,
        TraceStoreAllocatorAlignedOffsetCallocWithTimestamp,
        TraceStoreAllocatorTryAlignedOffsetCallocWithTimestamp,
        TraceStoreAllocatorAlignedFree,
        TraceStoreAllocatorAlignedFreePointer,
        NULL
    );

    return TRUE;
}

_Use_decl_annotations_
BOOL
InitializeAllocatorFromTraceStore(
    PTRACE_STORE TraceStore,
    PALLOCATOR Allocator
    )
{
    if (TraceStore->Excluded) {
        __debugbreak();
        return FALSE;
    }

    if (!TraceStoreInitializeAllocator(Allocator)) {
        return FALSE;
    }

    Allocator->TraceStore = TraceStore;

    return TRUE;
}

_Use_decl_annotations_
BOOL
InitializeTraceStoreAllocator(
    PTRACE_STORE TraceStore
    )
{
    PALLOCATOR Allocator;

    if (TraceStore->Excluded) {
        __debugbreak();
        return FALSE;
    }

    Allocator = &TraceStore->Allocator;

    if (!TraceStoreInitializeAllocator(Allocator)) {
        return FALSE;
    }

    Allocator->TraceStore = TraceStore;

    return TRUE;
}


// vim:set ts=8 sw=4 sts=4 tw=80 expandtab                                     :
