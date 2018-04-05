/*++

Copyright (c) 2016 Trent Nelson <trent@trent.me>

Module Name:

    TracerHeap.h

Abstract:

    This is the main header file for the TracerHeap component.  It defines
    structures and functions related to the implementation of the component.

--*/

#pragma once

#ifdef _TRACER_HEAP_INTERNAL_BUILD

//
// This is an internal build of the TracerHeap component.
//

#ifdef _TRACER_HEAP_DLL_BUILD

//
// This is the DLL build.
//

#define TRACER_HEAP_API __declspec(dllexport)
#define TRACER_HEAP_DATA extern __declspec(dllexport)

#else

//
// This is the static library build.
//

#define TRACER_HEAP_API
#define TRACER_HEAP_DATA

#endif

#include "stdafx.h"

#else

#ifdef _TRACER_HEAP_STATIC_LIB

//
// We're being included by another project as a static library.
//

#define TRACER_HEAP_API
#define TRACER_HEAP_DATA extern

#else

//
// We're being included by an external component that wants to use us as a DLL.
//

#define TRACER_HEAP_API __declspec(dllimport)
#define TRACER_HEAP_DATA extern __declspec(dllimport)

#endif

#include "../Rtl/Rtl.h"

#endif

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
(INITIALIZE_HEAP_ALLOCATOR)(
    _In_ PALLOCATOR Allocator
    );
typedef INITIALIZE_HEAP_ALLOCATOR *PINITIALIZE_HEAP_ALLOCATOR;

typedef
_Check_return_
_Success_(return != 0)
BOOL
(INITIALIZE_HEAP_ALLOCATOR_EX)(
    _In_ PALLOCATOR Allocator,
    _In_ DWORD HeapCreateOptions,
    _In_ SIZE_T InitialSize,
    _In_ SIZE_T MaximumSize
    );
typedef INITIALIZE_HEAP_ALLOCATOR_EX *PINITIALIZE_HEAP_ALLOCATOR_EX;

typedef
VOID
(DESTROY_HEAP_ALLOCATOR)(
    _In_opt_ PALLOCATOR Allocator
    );
typedef DESTROY_HEAP_ALLOCATOR *PDESTROY_HEAP_ALLOCATOR;
typedef DESTROY_HEAP_ALLOCATOR **PPDESTROY_HEAP_ALLOCATOR;

//
// Export symbols.
//

TRACER_HEAP_API MALLOC DefaultHeapMalloc;
TRACER_HEAP_API CALLOC DefaultHeapCalloc;
TRACER_HEAP_API TRY_MALLOC DefaultHeapTryMalloc;
TRACER_HEAP_API TRY_CALLOC DefaultHeapTryCalloc;
TRACER_HEAP_API MALLOC_WITH_TIMESTAMP DefaultHeapMallocWithTimestamp;
TRACER_HEAP_API CALLOC_WITH_TIMESTAMP DefaultHeapCallocWithTimestamp;
TRACER_HEAP_API TRY_MALLOC_WITH_TIMESTAMP DefaultHeapTryMallocWithTimestamp;
TRACER_HEAP_API TRY_CALLOC_WITH_TIMESTAMP DefaultHeapTryCallocWithTimestamp;
TRACER_HEAP_API REALLOC DefaultHeapRealloc;
TRACER_HEAP_API FREE DefaultHeapFree;
TRACER_HEAP_API FREE_POINTER DefaultHeapFreePointer;
TRACER_HEAP_API INITIALIZE_ALLOCATOR DefaultHeapInitializeAllocator;
TRACER_HEAP_API DESTROY_ALLOCATOR DefaultHeapDestroyAllocator;
TRACER_HEAP_API INITIALIZE_HEAP_ALLOCATOR InitializeHeapAllocator;
TRACER_HEAP_API INITIALIZE_HEAP_ALLOCATOR_EX InitializeHeapAllocatorEx;
TRACER_HEAP_API DESTROY_HEAP_ALLOCATOR DestroyHeapAllocator;

//
// Inline functions.
//

FORCEINLINE
BOOLEAN
InitializeHeapAllocatorExInline(
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

    //
    // Disable this for now whilst we experiment with an Rtl-based solution.
    //

    __debugbreak();

    /*
    InitializeAllocator(
        Allocator,
        Allocator,
        DefaultHeapMalloc,
        DefaultHeapCalloc,
        DefaultHeapRealloc,
        DefaultHeapFree,
        DefaultHeapFreePointer,
        DefaultHeapInitializeAllocator,
        DefaultHeapDestroyAllocator,
        DefaultHeapTryMalloc,
        DefaultHeapTryCalloc,
        DefaultHeapMallocWithTimestamp,
        DefaultHeapCallocWithTimestamp,
        DefaultHeapTryMallocWithTimestamp,
        DefaultHeapTryCallocWithTimestamp,
        HeapHandle
    );
    */

    return TRUE;
}

FORCEINLINE
BOOLEAN
InitializeHeapAllocatorInline(
    _In_ PALLOCATOR Allocator
    )
{
    return InitializeHeapAllocatorExInline(Allocator, 0, 0, 0);
}

FORCEINLINE
BOOLEAN
InitializeNonSerializedHeapAllocatorInline(
    _In_ PALLOCATOR Allocator
    )
{
    return InitializeHeapAllocatorExInline(Allocator, HEAP_NO_SERIALIZE, 0, 0);
}

FORCEINLINE
VOID
DestroyHeapAllocatorInline(
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
