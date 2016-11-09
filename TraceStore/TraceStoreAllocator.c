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
    ULARGE_INTEGER RecordSize;
    ULARGE_INTEGER NumberOfRecords = { 1 };

    TraceStore = ContextToTraceStore(Context);
    TraceContext = TraceStore->TraceContext;
    RecordSize.QuadPart = Size;

    Address = TraceStore->AllocateRecords(TraceContext,
                                          TraceStore,
                                          &RecordSize,
                                          &NumberOfRecords);

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
    ULARGE_INTEGER RecordSize;
    ULARGE_INTEGER NumberOfRecords;

    TraceStore = ContextToTraceStore(Context);
    TraceContext = TraceStore->TraceContext;
    RecordSize.QuadPart = ElementSize;
    NumberOfRecords.QuadPart = NumberOfElements;

    Address = TraceStore->AllocateRecords(TraceContext,
                                          TraceStore,
                                          &RecordSize,
                                          &NumberOfRecords);

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
    TraceStoreInitializeAllocator(Allocator);
    Allocator->TraceStore = TraceStore;

    return TRUE;
}

// vim:set ts=8 sw=4 sts=4 tw=80 expandtab                                     :
