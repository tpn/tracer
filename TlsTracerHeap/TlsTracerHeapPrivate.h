#pragma once

#include "TlsTracerHeap.h"

//
// TlsIndex and TracerConfig are both exported from this DLL.
//

extern ULONG TlsIndex;
extern PTRACER_CONFIG TracerConfig;

//
// Copies of the old allocation routines that will be restored when the last
// thread detaches.
//

extern PMALLOC OriginalMalloc;
extern PCALLOC OriginalCalloc;
extern PREALLOC OriginalRealloc;
extern PFREE OriginalFree;
extern PFREE_POINTER OriginalFreePointer;

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
