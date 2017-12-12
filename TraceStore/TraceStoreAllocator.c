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
        TraceStoreAllocatorTryMalloc,
        TraceStoreAllocatorTryCalloc,
        TraceStoreAllocatorMallocWithTimestamp,
        TraceStoreAllocatorCallocWithTimestamp,
        TraceStoreAllocatorTryMallocWithTimestamp,
        TraceStoreAllocatorTryCallocWithTimestamp,
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
