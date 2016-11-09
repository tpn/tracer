#pragma once

#include <minwindef.h>
#include <sal.h>
#include <specstrings.h>

//
// Function pointer typedefs for standard C memory allocation functions.
//

typedef
_Must_inspect_result_
PVOID
(MALLOC)(
    _In_ PVOID Context,
    _In_ SIZE_T Size
    );
typedef MALLOC *PMALLOC;

typedef
_Must_inspect_result_
PVOID
(CALLOC)(
    _In_ PVOID Context,
    _In_ SIZE_T NumberOfElements,
    _In_ SIZE_T ElementSize
    );
typedef CALLOC *PCALLOC;

typedef
_Must_inspect_result_
_Ret_reallocated_bytes_(Buffer, NewSize)
PVOID
(REALLOC)(
    _In_ PVOID Context,
    _In_ PVOID Buffer,
    _In_ SIZE_T NewSize
    );
typedef REALLOC *PREALLOC;

typedef
VOID
(FREE)(
    _In_ PVOID Context,
    _Frees_ptr_opt_ PVOID Buffer
    );
typedef FREE *PFREE;

typedef
VOID
(FREE_POINTER)(
    _In_ PVOID Context,
    _In_ PPVOID PointerToBuffer
    );
typedef FREE_POINTER *PFREE_POINTER;

typedef
BOOLEAN
(INITIALIZE_ALLOCATOR)(
    _In_ struct _ALLOCATOR *Allocator
    );
typedef INITIALIZE_ALLOCATOR *PINITIALIZE_ALLOCATOR;

typedef
BOOLEAN
(CREATE_AND_INITIALIZE_ALLOCATOR)(
    _Out_ struct _ALLOCATOR **Allocator
    );
typedef CREATE_AND_INITIALIZE_ALLOCATOR *PCREATE_AND_INITIALIZE_ALLOCATOR;

typedef
VOID
(DESTROY_ALLOCATOR)(
    _In_opt_ struct _ALLOCATOR *Allocator
    );
typedef DESTROY_ALLOCATOR *PDESTROY_ALLOCATOR;

typedef struct _ALLOCATOR_FLAGS {
    ULONG IsTlsAware:1;
    ULONG IsTlsRedirectionEnabled:1;
} ALLOCATOR_FLAGS, *PALLOCATOR_FLAGS;

//
// Allocator structure.  This intentionally mirrors the Python 3.5+
// PyMemAllocatorEx structure up to the Free pointer.
//

typedef struct _ALLOCATOR {
    union {
        PVOID Context;
        PVOID ctx;
        struct _ALLOCATOR *Allocator;
    };

    union {
        PMALLOC Malloc;
        PMALLOC malloc;
    };

    union {
        PCALLOC Calloc;
        PCALLOC calloc;
    };

    union {
        PREALLOC Realloc;
        PREALLOC realloc;
    };

    union {
        PFREE Free;
        PFREE free;
    };

    //
    // Remaining fields are specific to us and have no
    // PyMemAllocatorEx counterparts.
    //

    //
    // Additional function pointers.
    //

    PFREE_POINTER FreePointer;
    PINITIALIZE_ALLOCATOR Initialize;
    PDESTROY_ALLOCATOR Destroy;

    union {
        PVOID Context2;
        HANDLE HeapHandle;
        struct _TRACE_STORE *TraceStore;
    };

    //
    // If this is a per-thread allocator, this will be a pointer to the parent
    // (global) allocator.
    //

    struct _ALLOCATOR *Parent;

    //
    // Id of the thread that created this structure.
    //

    ULONG ThreadId;

    //
    // Tls Index used to store per-thread allocators.  This is set by the
    // TlsTracerHeap DLL machinery.
    //

    ULONG TlsIndex;

    //
    // Flags.
    //

    volatile ALLOCATOR_FLAGS Flags;


    //
    // Number of the TlsHeap threads that have been created.
    //

    volatile ULONG NumberOfThreads;

} ALLOCATOR, *PALLOCATOR, **PPALLOCATOR;

typedef
_Success_(return != 0)
BOOLEAN
(GET_OR_CREATE_GLOBAL_ALLOCATOR)(
    _Out_ PPALLOCATOR Allocator
    );
typedef GET_OR_CREATE_GLOBAL_ALLOCATOR *PGET_OR_CREATE_GLOBAL_ALLOCATOR;


FORCEINLINE
VOID
InitializeAllocator(
    _In_ PALLOCATOR Allocator,
    _In_ PVOID Context,
    _In_ PMALLOC Malloc,
    _In_ PCALLOC Calloc,
    _In_ PREALLOC Realloc,
    _In_ PFREE Free,
    _In_ PFREE_POINTER FreePointer,
    _In_ PINITIALIZE_ALLOCATOR Initialize,
    _In_ PDESTROY_ALLOCATOR Destroy,
    _In_ PVOID Context2
    )
{

    Allocator->Context = Context;

    Allocator->Malloc = Malloc;
    Allocator->Calloc = Calloc;
    Allocator->Realloc = Realloc;
    Allocator->Free = Free;
    Allocator->FreePointer = FreePointer;
    Allocator->Initialize = Initialize;
    Allocator->Destroy = Destroy;

    Allocator->Context2 = Context2;

    Allocator->TlsIndex = TLS_OUT_OF_INDEXES;
    Allocator->ThreadId = GetCurrentThreadId();
    Allocator->Parent = NULL;
}

FORCEINLINE
VOID
InitializeTlsAllocator(
    _In_ PALLOCATOR Allocator,
    _In_ PVOID Context,
    _In_ PMALLOC Malloc,
    _In_ PCALLOC Calloc,
    _In_ PREALLOC Realloc,
    _In_ PFREE Free,
    _In_ PFREE_POINTER FreePointer,
    _In_ PINITIALIZE_ALLOCATOR Initialize,
    _In_ PDESTROY_ALLOCATOR Destroy,
    _In_ PVOID Context2,
    _In_ PALLOCATOR Parent,
    _In_ ULONG TlsIndex
    )
{
    InitializeAllocator(
        Allocator,
        Context,
        Malloc,
        Calloc,
        Realloc,
        Free,
        FreePointer,
        Initialize,
        Destroy,
        Context2
    );

    Allocator->TlsIndex = TlsIndex;
    Allocator->Parent = Parent;

}

// vim:set ts=8 sw=4 sts=4 tw=80 expandtab                                     :
