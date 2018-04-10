/*++

Copyright (c) 2016 Trent Nelson <trent@trent.me>

Module Name:

    HeapAllocator.h

Abstract:

    This is the main header file for the HeapAllocator component.  It defines
    structures and functions related to the implementation of the component.

--*/

#pragma once

#include "stdafx.h"

#ifdef __cplusplus
extern "C" {
#endif

//
// Default heap routines that use the kernel32 heap routines.
//

typedef
_Check_return_
_Success_(return != 0)
BOOL
(RTL_INITIALIZE_HEAP_ALLOCATOR)(
    _In_ PALLOCATOR Allocator
    );
typedef RTL_INITIALIZE_HEAP_ALLOCATOR *PRTL_INITIALIZE_HEAP_ALLOCATOR;

typedef
_Check_return_
_Success_(return != 0)
BOOL
(RTL_INITIALIZE_HEAP_ALLOCATOR_EX)(
    _In_ PALLOCATOR Allocator,
    _In_ DWORD HeapCreateOptions,
    _In_ SIZE_T InitialSize,
    _In_ SIZE_T MaximumSize
    );
typedef RTL_INITIALIZE_HEAP_ALLOCATOR_EX *PRTL_INITIALIZE_HEAP_ALLOCATOR_EX;

typedef
VOID
(RTL_DESTROY_HEAP_ALLOCATOR)(
    _In_opt_ PALLOCATOR Allocator
    );
typedef RTL_DESTROY_HEAP_ALLOCATOR *PRTL_DESTROY_HEAP_ALLOCATOR;
typedef RTL_DESTROY_HEAP_ALLOCATOR **PPRTL_DESTROY_HEAP_ALLOCATOR;

//
// Public symbols.
//

extern MALLOC RtlHeapAllocatorMalloc;
extern CALLOC RtlHeapAllocatorCalloc;
extern TRY_MALLOC RtlHeapAllocatorTryMalloc;
extern TRY_CALLOC RtlHeapAllocatorTryCalloc;
extern MALLOC_WITH_TIMESTAMP RtlHeapAllocatorMallocWithTimestamp;
extern CALLOC_WITH_TIMESTAMP RtlHeapAllocatorCallocWithTimestamp;
extern TRY_MALLOC_WITH_TIMESTAMP RtlHeapAllocatorTryMallocWithTimestamp;
extern TRY_CALLOC_WITH_TIMESTAMP RtlHeapAllocatorTryCallocWithTimestamp;
extern REALLOC RtlHeapAllocatorRealloc;
extern WRITE_BYTES RtlHeapAllocatorWriteBytes;
extern ALIGNED_MALLOC RtlHeapAllocatorAlignedMalloc;
extern TRY_ALIGNED_MALLOC RtlHeapAllocatorTryAlignedMalloc;
extern ALIGNED_MALLOC_WITH_TIMESTAMP RtlHeapAllocatorAlignedMallocWithTimestamp;
extern TRY_ALIGNED_MALLOC_WITH_TIMESTAMP RtlHeapAllocatorTryAlignedMallocWithTimestamp;
extern ALIGNED_OFFSET_MALLOC RtlHeapAllocatorAlignedOffsetMalloc;
extern TRY_ALIGNED_OFFSET_MALLOC RtlHeapAllocatorTryAlignedOffsetMalloc;
extern ALIGNED_OFFSET_MALLOC_WITH_TIMESTAMP RtlHeapAllocatorAlignedOffsetMallocWithTimestamp;
extern TRY_ALIGNED_OFFSET_MALLOC_WITH_TIMESTAMP RtlHeapAllocatorTryAlignedOffsetMallocWithTimestamp;
extern ALIGNED_CALLOC RtlHeapAllocatorAlignedCalloc;
extern TRY_ALIGNED_CALLOC RtlHeapAllocatorTryAlignedCalloc;
extern ALIGNED_CALLOC_WITH_TIMESTAMP RtlHeapAllocatorAlignedCallocWithTimestamp;
extern TRY_ALIGNED_CALLOC_WITH_TIMESTAMP RtlHeapAllocatorTryAlignedCallocWithTimestamp;
extern ALIGNED_OFFSET_CALLOC RtlHeapAllocatorAlignedOffsetCalloc;
extern TRY_ALIGNED_OFFSET_CALLOC RtlHeapAllocatorTryAlignedOffsetCalloc;
extern ALIGNED_OFFSET_CALLOC_WITH_TIMESTAMP RtlHeapAllocatorAlignedOffsetCallocWithTimestamp;
extern TRY_ALIGNED_OFFSET_CALLOC_WITH_TIMESTAMP RtlHeapAllocatorTryAlignedOffsetCallocWithTimestamp;
extern FREE RtlHeapAllocatorFree;
extern FREE_POINTER RtlHeapAllocatorFreePointer;
extern ALIGNED_FREE RtlHeapAllocatorAlignedFree;
extern ALIGNED_FREE_POINTER RtlHeapAllocatorAlignedFreePointer;
extern INITIALIZE_ALLOCATOR RtlHeapAllocatorInitialize;
extern DESTROY_ALLOCATOR RtlHeapAllocatorDestroy;
extern RTL_INITIALIZE_HEAP_ALLOCATOR RtlInitializeHeapAllocator;
extern RTL_INITIALIZE_HEAP_ALLOCATOR_EX RtlInitializeHeapAllocatorEx;
extern RTL_DESTROY_HEAP_ALLOCATOR RtlDestroyHeapAllocator;

//
// Inline functions.
//

FORCEINLINE
BOOLEAN
RtlInitializeHeapAllocatorExInline(
    _In_ PALLOCATOR Allocator,
    _In_ DWORD HeapCreateOptions,
    _In_ SIZE_T InitialSize,
    _In_ SIZE_T MaximumSize
    )
{
    HANDLE HeapHandle;

    if (!Allocator) {
        return FALSE;
    }

    HeapHandle = HeapCreate(HeapCreateOptions, InitialSize, MaximumSize);
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

FORCEINLINE
BOOLEAN
RtlInitializeHeapAllocatorInline(
    _In_ PALLOCATOR Allocator
    )
{
    return RtlInitializeHeapAllocatorExInline(Allocator, 0, 0, 0);
}

FORCEINLINE
BOOLEAN
RtlInitializeNonSerializedHeapAllocatorInline(
    _In_ PALLOCATOR Allocator
    )
{
    return RtlInitializeHeapAllocatorExInline(Allocator,
                                              HEAP_NO_SERIALIZE,
                                              0,
                                              0);
}

FORCEINLINE
VOID
RtlDestroyHeapAllocatorInline(
    _In_ PALLOCATOR Allocator
    )
{
    if (Allocator->HeapHandle) {
        HeapDestroy(Allocator->HeapHandle);
    }

    return;
}

#ifdef __cplusplus
} // extern "C"
#endif

// vim:set ts=8 sw=4 sts=4 tw=80 expandtab                                     :
