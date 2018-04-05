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
// Export symbols.
//

RTL_API MALLOC RtlHeapAllocatorMalloc;
RTL_API CALLOC RtlHeapAllocatorCalloc;
RTL_API TRY_MALLOC RtlHeapAllocatorTryMalloc;
RTL_API TRY_CALLOC RtlHeapAllocatorTryCalloc;
RTL_API MALLOC_WITH_TIMESTAMP RtlHeapAllocatorMallocWithTimestamp;
RTL_API CALLOC_WITH_TIMESTAMP RtlHeapAllocatorCallocWithTimestamp;
RTL_API TRY_MALLOC_WITH_TIMESTAMP RtlHeapAllocatorTryMallocWithTimestamp;
RTL_API TRY_CALLOC_WITH_TIMESTAMP RtlHeapAllocatorTryCallocWithTimestamp;
RTL_API REALLOC RtlHeapAllocatorRealloc;
RTL_API WRITE_BYTES RtlHeapAllocatorWriteBytes;
RTL_API ALIGNED_MALLOC RtlHeapAllocatorAlignedMalloc;
RTL_API TRY_ALIGNED_MALLOC RtlHeapAllocatorTryAlignedMalloc;
RTL_API ALIGNED_MALLOC_WITH_TIMESTAMP RtlHeapAllocatorAlignedMallocWithTimestamp;
RTL_API TRY_ALIGNED_MALLOC_WITH_TIMESTAMP RtlHeapAllocatorTryAlignedMallocWithTimestamp;
RTL_API ALIGNED_OFFSET_MALLOC RtlHeapAllocatorAlignedOffsetMalloc;
RTL_API TRY_ALIGNED_OFFSET_MALLOC RtlHeapAllocatorTryAlignedOffsetMalloc;
RTL_API ALIGNED_OFFSET_MALLOC_WITH_TIMESTAMP RtlHeapAllocatorAlignedOffsetMallocWithTimestamp;
RTL_API TRY_ALIGNED_OFFSET_MALLOC_WITH_TIMESTAMP RtlHeapAllocatorTryAlignedOffsetMallocWithTimestamp;
RTL_API ALIGNED_CALLOC RtlHeapAllocatorAlignedCalloc;
RTL_API TRY_ALIGNED_CALLOC RtlHeapAllocatorTryAlignedCalloc;
RTL_API ALIGNED_CALLOC_WITH_TIMESTAMP RtlHeapAllocatorAlignedCallocWithTimestamp;
RTL_API TRY_ALIGNED_CALLOC_WITH_TIMESTAMP RtlHeapAllocatorTryAlignedCallocWithTimestamp;
RTL_API ALIGNED_OFFSET_CALLOC RtlHeapAllocatorAlignedOffsetCalloc;
RTL_API TRY_ALIGNED_OFFSET_CALLOC RtlHeapAllocatorTryAlignedOffsetCalloc;
RTL_API ALIGNED_OFFSET_CALLOC_WITH_TIMESTAMP RtlHeapAllocatorAlignedOffsetCallocWithTimestamp;
RTL_API TRY_ALIGNED_OFFSET_CALLOC_WITH_TIMESTAMP RtlHeapAllocatorTryAlignedOffsetCallocWithTimestamp;
RTL_API FREE RtlHeapAllocatorFree;
RTL_API FREE_POINTER RtlHeapAllocatorFreePointer;
RTL_API ALIGNED_FREE RtlHeapAllocatorAlignedFree;
RTL_API ALIGNED_FREE_POINTER RtlHeapAllocatorAlignedFreePointer;
RTL_API INITIALIZE_ALLOCATOR RtlHeapAllocatorInitialize;
RTL_API DESTROY_ALLOCATOR RtlHeapAllocatorDestroy;
RTL_API RTL_INITIALIZE_HEAP_ALLOCATOR RtlInitializeHeapAllocator;
RTL_API RTL_INITIALIZE_HEAP_ALLOCATOR_EX RtlInitializeHeapAllocatorEx;
RTL_API RTL_DESTROY_HEAP_ALLOCATOR RtlDestroyHeapAllocator;

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
