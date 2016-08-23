#pragma once

#include <minwindef.h>
#include <sal.h>
#include <specstrings.h>

//
// Function pointer typedefs for standard C memory allocation functions.
//

typedef
VOID * __restrict
_Must_inspect_result_
(MALLOC)(
    _In_ PVOID Context,
    _In_ SIZE_T Size
    );
typedef MALLOC *PMALLOC;

typedef
VOID * __restrict
_Must_inspect_result_
(CALLOC)(
    _In_ PVOID Context,
    _In_ SIZE_T NumberOfElements,
    _In_ SIZE_T ElementSize
    );
typedef CALLOC *PCALLOC;

typedef
VOID * __restrict
_Must_inspect_result_
_Ret_reallocated_bytes_(Buffer, NewSize)
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
BOOLEAN
(INITIALIZE_ALLOCATOR)(
    _In_ struct _ALLOCATOR *Allocator
    );
typedef INITIALIZE_ALLOCATOR *PINITIALIZE_ALLOCATOR;

typedef
VOID
(DESTROY_ALLOCATOR)(
    _In_opt_ struct _ALLOCATOR *Allocator
    );
typedef DESTROY_ALLOCATOR *PDESTROY_ALLOCATOR;

//
// Allocator structure.  This intentionally mirrors the Python 3.5+
// PyMemAllocatorEx structure up to the Free pointer.
//

typedef struct _ALLOCATOR {
    union {
        PVOID Context;
        PVOID ctx;
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

    PVOID Context2;
    PINITIALIZE_ALLOCATOR Initialize;
    PDESTROY_ALLOCATOR Destroy;

} ALLOCATOR, *PALLOCATOR;


