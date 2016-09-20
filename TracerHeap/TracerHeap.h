/*++

Copyright (c) 2016 Trent Nelson <trent@trent.me>

Module Name:

    StringTable.h

Abstract:

    This is the main header file for the StringTable component.  It defines
    structures and functions related to the implementation of the component.

    The main structures are the STRING_ARRAY structure, which is used by
    callers of this component to indicate the set of strings they'd like to
    add to the string table, and the STRING_TABLE structure, which is the main
    data structure used by this component.

    Functions are provided for creating, destroying and searching for whether
    or not there is a prefix string for a given search string present within a
    table.

    The design is optimized for relatively short strings (less than or equal to
    16 chars), and relatively few of them (less than or equal to 16).  These
    restrictive size constraints facilitate aggressive SIMD optimizations when
    searching for the strings within the table, with the goal to minimize the
    maximum possible latency incurred by the lookup mechanism.  The trade-off
    is usability and flexibility -- two things which can be better served by
    prefix trees if the pointer-chasing behavior of said data structures can
    be tolerated.

--*/

#pragma once

#ifdef _TRACER_HEAP_INTERNAL_BUILD

//
// This is an internal build of the StringTable component.
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

#pragma once

//
// Default heap routines that use the kernel32 heap routines.
//

TRACER_HEAP_API MALLOC DefaultHeapMalloc;
TRACER_HEAP_API CALLOC DefaultHeapCalloc;
TRACER_HEAP_API REALLOC DefaultHeapRealloc;
TRACER_HEAP_API FREE DefaultHeapFree;
TRACER_HEAP_API FREE_POINTER DefaultHeapFreePointer;
TRACER_HEAP_API INITIALIZE_ALLOCATOR DefaultHeapInitializeAllocator;
TRACER_HEAP_API DESTROY_ALLOCATOR DefaultHeapDestroyAllocator;

typedef
_Check_return_
_Success_(return != 0)
BOOL
(INITIALIZE_HEAP_ALLOCATOR)(
    _In_ PALLOCATOR Allocator
    );
typedef INITIALIZE_HEAP_ALLOCATOR *PINITIALIZE_HEAP_ALLOCATOR;
TRACER_HEAP_API INITIALIZE_HEAP_ALLOCATOR InitializeHeapAllocator;

typedef
VOID
(DESTROY_HEAP_ALLOCATOR)(
    _In_opt_ PALLOCATOR Allocator
    );
typedef DESTROY_HEAP_ALLOCATOR *PDESTROY_HEAP_ALLOCATOR;
typedef DESTROY_HEAP_ALLOCATOR **PPDESTROY_HEAP_ALLOCATOR;
TRACER_HEAP_API DESTROY_HEAP_ALLOCATOR DestroyHeapAllocator;

//
// Aligned routines.  These call out to the _aligned_* family of CRT functions.
//

TRACER_HEAP_API MALLOC AlignedHeapMalloc;
TRACER_HEAP_API CALLOC AlignedHeapCalloc;
TRACER_HEAP_API REALLOC AlignedHeapRealloc;
TRACER_HEAP_API FREE AlignedHeapFree;
TRACER_HEAP_API FREE_POINTER AlignedHeapFreePointer;
TRACER_HEAP_API INITIALIZE_ALLOCATOR AlignedHeapInitializeAllocator;
TRACER_HEAP_API DESTROY_ALLOCATOR AlignedHeapDestroyAllocator;

typedef
_Check_return_
_Success_(return != 0)
BOOL
(INITIALIZE_ALIGNED_ALLOCATOR)(
    _In_ PALLOCATOR Allocator
    );
typedef INITIALIZE_ALIGNED_ALLOCATOR *PINITIALIZE_ALIGNED_ALLOCATOR;
TRACER_HEAP_API INITIALIZE_ALIGNED_ALLOCATOR InitializeAlignedAllocator;

typedef
VOID
(DESTROY_ALIGNED_ALLOCATOR)(
    _In_opt_ PALLOCATOR Allocator
    );
typedef DESTROY_ALIGNED_ALLOCATOR *PDESTROY_ALIGNED_ALLOCATOR;
typedef DESTROY_ALIGNED_ALLOCATOR **PPDESTROY_ALIGNED_ALLOCATOR;
TRACER_HEAP_API DESTROY_ALIGNED_ALLOCATOR DestroyAlignedAllocator;


#ifdef __cplusplus
} // extern "C"
#endif

// vim:set ts=8 sw=4 sts=4 tw=80 expandtab                                     :
