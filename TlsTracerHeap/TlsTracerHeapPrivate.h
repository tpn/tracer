#pragma once

#include "TlsTracerHeap.h"

//
// Static storage for globals.
//

static ULONG TlsIndex = TLS_OUT_OF_INDEXES;
static PTRACER_CONFIG TracerConfig = NULL;

//
// Copies of the old allocation routines that will be restored when the last
// thread detaches.
//

static PMALLOC OldMalloc = NULL;
static PCALLOC OldCalloc = NULL;
static PREALLOC OldRealloc = NULL;
static PFREE OldFree = NULL;
static PFREE_POINTER OldFreePointer = NULL;

//
// Typedefs for private functions.
//

typedef
_Success_(return != 0)
BOOL
(TLS_TRACER_HEAP_DLL_MAIN)(
    _In_    HMODULE     Module,
    _In_    DWORD       Reason,
    _In_    LPVOID      Reserved
    );
typedef TLS_TRACER_HEAP_DLL_MAIN *PTLS_TRACER_HEAP_DLL_MAIN;

typedef TLS_TRACER_HEAP_DLL_MAIN TLS_TRACER_HEAP_PROCESS_ATTACH;
typedef TLS_TRACER_HEAP_PROCESS_ATTACH *PTLS_TRACER_HEAP_PROCESS_ATTACH;

typedef TLS_TRACER_HEAP_DLL_MAIN TLS_TRACER_HEAP_PROCESS_DETACH;
typedef TLS_TRACER_HEAP_PROCESS_DETACH *PTLS_TRACER_HEAP_PROCESS_DETACH;

typedef TLS_TRACER_HEAP_DLL_MAIN TLS_TRACER_HEAP_THREAD_ATTACH;
typedef TLS_TRACER_HEAP_THREAD_ATTACH *PTLS_TRACER_HEAP_THREAD_ATTACH;

typedef TLS_TRACER_HEAP_DLL_MAIN TLS_TRACER_HEAP_THREAD_DETACH;
typedef TLS_TRACER_HEAP_THREAD_DETACH *PTLS_TRACER_HEAP_THREAD_DETACH;

//
// Function declarations.
//

TLS_TRACER_HEAP_SET_TRACER_CONFIG TlsTracerHeapSetTracerConfig;

TLS_TRACER_HEAP_PROCESS_ATTACH TlsTracerHeapProcessAttach;
TLS_TRACER_HEAP_PROCESS_DETACH TlsTracerHeapProcessDetach;
TLS_TRACER_HEAP_THREAD_ATTACH TlsTracerHeapThreadAttach;
TLS_TRACER_HEAP_THREAD_DETACH TlsTracerHeapThreadDetach;

MALLOC TlsHeapMalloc;
CALLOC TlsHeapCalloc;
REALLOC TlsHeapRealloc;
FREE TlsHeapFree;
FREE_POINTER TlsHeapFreePointer;
INITIALIZE_ALLOCATOR TlsHeapInitializeAllocator;
DESTROY_ALLOCATOR TlsHeapDestroyAllocator;

MALLOC TlsAwareMalloc;
CALLOC TlsAwareCalloc;
REALLOC TlsAwareRealloc;
FREE TlsAwareFree;
FREE_POINTER TlsAwareFreePointer;

// vim:set ts=8 sw=4 sts=4 tw=80 expandtab                                     :
