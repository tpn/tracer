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

#ifdef __cplusplus
} // extern "C"
#endif

// vim:set ts=8 sw=4 sts=4 tw=80 expandtab                                     :
