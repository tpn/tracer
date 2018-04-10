/*++

Copyright (c) 2018 Trent Nelson <trent@trent.me>

Module Name:

    Memory.h

Abstract:

    This is the header file for memory-related functionality of the Rtl
    component.  It defines function pointer typedefs for standard C memory
    allocation functions like malloc, calloc and free, as well as an ALLOCATOR
    interface.

--*/

#pragma once

#include <minwindef.h>
#include <sal.h>
#include <specstrings.h>

//
// Define malloc-oriented functions.
//

typedef
_Check_return_
_Ret_maybenull_
_Post_writable_byte_size_(Size)
PVOID
(MALLOC)(
    _In_ PVOID Context,
    _In_ SIZE_T Size
    );
typedef MALLOC *PMALLOC;

typedef
_Must_inspect_result_
PVOID
(TRY_MALLOC)(
    _In_ PVOID Context,
    _In_ SIZE_T Size
    );
typedef TRY_MALLOC *PTRY_MALLOC;

typedef
_Must_inspect_result_
PVOID
(MALLOC_WITH_TIMESTAMP)(
    _In_ PVOID Context,
    _In_ SIZE_T Size,
    _In_ PLARGE_INTEGER TimestampPointer
    );
typedef MALLOC_WITH_TIMESTAMP *PMALLOC_WITH_TIMESTAMP;

typedef
_Must_inspect_result_
PVOID
(TRY_MALLOC_WITH_TIMESTAMP)(
    _In_ PVOID Context,
    _In_ SIZE_T Size,
    _In_ PLARGE_INTEGER TimestampPointer
    );
typedef TRY_MALLOC_WITH_TIMESTAMP *PTRY_MALLOC_WITH_TIMESTAMP;

//
// Define aligned malloc-oriented functions.
//

typedef
_Check_return_
_Ret_maybenull_
_Post_writable_byte_size_(Size)
PVOID
(ALIGNED_MALLOC)(
    _In_ PVOID Context,
    _In_ SIZE_T Size,
    _In_ SIZE_T Alignment
    );
typedef ALIGNED_MALLOC *PALIGNED_MALLOC;

typedef
_Check_return_
PVOID
(TRY_ALIGNED_MALLOC)(
    _In_ PVOID Context,
    _In_ SIZE_T Size,
    _In_ SIZE_T Alignment
    );
typedef TRY_ALIGNED_MALLOC *PTRY_ALIGNED_MALLOC;

typedef
_Check_return_
_Ret_maybenull_
_Post_writable_byte_size_(Size)
PVOID
(ALIGNED_MALLOC_WITH_TIMESTAMP)(
    _In_ PVOID Context,
    _In_ SIZE_T Size,
    _In_ SIZE_T Alignment,
    _In_ PLARGE_INTEGER TimestampPointer
    );
typedef ALIGNED_MALLOC_WITH_TIMESTAMP *PALIGNED_MALLOC_WITH_TIMESTAMP;

typedef
_Check_return_
_Ret_maybenull_
_Post_writable_byte_size_(Size)
PVOID
(TRY_ALIGNED_MALLOC_WITH_TIMESTAMP)(
    _In_ PVOID Context,
    _In_ SIZE_T Size,
    _In_ SIZE_T Alignment,
    _In_ PLARGE_INTEGER TimestampPointer
    );
typedef TRY_ALIGNED_MALLOC_WITH_TIMESTAMP *PTRY_ALIGNED_MALLOC_WITH_TIMESTAMP;

//
// Define aligned offset malloc-oriented functions.
//

typedef
_Check_return_
_Ret_maybenull_
_Post_writable_byte_size_(Size)
PVOID
(ALIGNED_OFFSET_MALLOC)(
    _In_ PVOID Context,
    _In_ SIZE_T Size,
    _In_ SIZE_T Alignment,
    _In_ SIZE_T Offset
    );
typedef ALIGNED_OFFSET_MALLOC *PALIGNED_OFFSET_MALLOC;

typedef
_Check_return_
_Ret_maybenull_
_Post_writable_byte_size_(Size)
PVOID
(TRY_ALIGNED_OFFSET_MALLOC)(
    _In_ PVOID Context,
    _In_ SIZE_T Size,
    _In_ SIZE_T Alignment,
    _In_ SIZE_T Offset
    );
typedef TRY_ALIGNED_OFFSET_MALLOC *PTRY_ALIGNED_OFFSET_MALLOC;

typedef
_Check_return_
_Ret_maybenull_
_Post_writable_byte_size_(Size)
PVOID
(ALIGNED_OFFSET_MALLOC_WITH_TIMESTAMP)(
    _In_ PVOID Context,
    _In_ SIZE_T Size,
    _In_ SIZE_T Alignment,
    _In_ SIZE_T Offset,
    _In_ PLARGE_INTEGER TimestampPointer
    );
typedef ALIGNED_OFFSET_MALLOC_WITH_TIMESTAMP
      *PALIGNED_OFFSET_MALLOC_WITH_TIMESTAMP;

typedef
_Check_return_
_Ret_maybenull_
_Post_writable_byte_size_(Size)
PVOID
(TRY_ALIGNED_OFFSET_MALLOC_WITH_TIMESTAMP)(
    _In_ PVOID Context,
    _In_ SIZE_T Size,
    _In_ SIZE_T Alignment,
    _In_ SIZE_T Offset,
    _In_ PLARGE_INTEGER TimestampPointer
    );
typedef TRY_ALIGNED_OFFSET_MALLOC_WITH_TIMESTAMP
      *PTRY_ALIGNED_OFFSET_MALLOC_WITH_TIMESTAMP;

//
// Define calloc-oriented functions.
//

typedef
_Check_return_
_Ret_maybenull_
_Post_writable_byte_size_(NumberOfElements * ElementSize)
PVOID
(CALLOC)(
    _In_ PVOID Context,
    _In_ SIZE_T NumberOfElements,
    _In_ SIZE_T ElementSize
    );
typedef CALLOC *PCALLOC;

typedef
_Check_return_
_Ret_maybenull_
_Post_writable_byte_size_(NumberOfElements * ElementSize)
PVOID
(TRY_CALLOC)(
    _In_ PVOID Context,
    _In_ SIZE_T NumberOfElements,
    _In_ SIZE_T ElementSize
    );
typedef TRY_CALLOC *PTRY_CALLOC;

typedef
_Check_return_
_Ret_maybenull_
_Post_writable_byte_size_(NumberOfElements * ElementSize)
PVOID
(CALLOC_WITH_TIMESTAMP)(
    _In_ PVOID Context,
    _In_ SIZE_T NumberOfElements,
    _In_ SIZE_T ElementSize,
    _In_ PLARGE_INTEGER TimestampPointer
    );
typedef CALLOC_WITH_TIMESTAMP *PCALLOC_WITH_TIMESTAMP;

typedef
_Check_return_
_Ret_maybenull_
_Post_writable_byte_size_(NumberOfElements * ElementSize)
PVOID
(TRY_CALLOC_WITH_TIMESTAMP)(
    _In_ PVOID Context,
    _In_ SIZE_T NumberOfElements,
    _In_ SIZE_T ElementSize,
    _In_ PLARGE_INTEGER TimestampPointer
    );
typedef TRY_CALLOC_WITH_TIMESTAMP *PTRY_CALLOC_WITH_TIMESTAMP;

//
// Define aligned calloc-oriented functions.
//

typedef
_Check_return_
_Ret_maybenull_
_Post_writable_byte_size_(NumberOfElements * ElementSize)
PVOID
(ALIGNED_CALLOC)(
    _In_ PVOID Context,
    _In_ SIZE_T NumberOfElements,
    _In_ SIZE_T ElementSize,
    _In_ SIZE_T Alignment
    );
typedef ALIGNED_CALLOC *PALIGNED_CALLOC;

typedef
_Check_return_
_Ret_maybenull_
_Post_writable_byte_size_(NumberOfElements * ElementSize)
PVOID
(TRY_ALIGNED_CALLOC)(
    _In_ PVOID Context,
    _In_ SIZE_T NumberOfElements,
    _In_ SIZE_T ElementSize,
    _In_ SIZE_T Alignment
    );
typedef TRY_ALIGNED_CALLOC *PTRY_ALIGNED_CALLOC;

typedef
_Check_return_
_Ret_maybenull_
_Post_writable_byte_size_(NumberOfElements * ElementSize)
PVOID
(ALIGNED_CALLOC_WITH_TIMESTAMP)(
    _In_ PVOID Context,
    _In_ SIZE_T NumberOfElements,
    _In_ SIZE_T ElementSize,
    _In_ SIZE_T Alignment,
    _In_ PLARGE_INTEGER TimestampPointer
    );
typedef ALIGNED_CALLOC_WITH_TIMESTAMP *PALIGNED_CALLOC_WITH_TIMESTAMP;

typedef
_Check_return_
_Ret_maybenull_
_Post_writable_byte_size_(NumberOfElements * ElementSize)
PVOID
(TRY_ALIGNED_CALLOC_WITH_TIMESTAMP)(
    _In_ PVOID Context,
    _In_ SIZE_T NumberOfElements,
    _In_ SIZE_T ElementSize,
    _In_ SIZE_T Alignment,
    _In_ PLARGE_INTEGER TimestampPointer
    );
typedef TRY_ALIGNED_CALLOC_WITH_TIMESTAMP *PTRY_ALIGNED_CALLOC_WITH_TIMESTAMP;

//
// Define aligned offset calloc-oriented functions.
//

typedef
_Check_return_
_Ret_maybenull_
_Post_writable_byte_size_(NumberOfElements * ElementSize)
PVOID
(ALIGNED_OFFSET_CALLOC)(
    _In_ PVOID Context,
    _In_ SIZE_T NumberOfElements,
    _In_ SIZE_T ElementSize,
    _In_ SIZE_T Alignment,
    _In_ SIZE_T Offset
    );
typedef ALIGNED_OFFSET_CALLOC *PALIGNED_OFFSET_CALLOC;

typedef
_Check_return_
_Ret_maybenull_
_Post_writable_byte_size_(NumberOfElements * ElementSize)
PVOID
(TRY_ALIGNED_OFFSET_CALLOC)(
    _In_ PVOID Context,
    _In_ SIZE_T NumberOfElements,
    _In_ SIZE_T ElementSize,
    _In_ SIZE_T Alignment,
    _In_ SIZE_T Offset
    );
typedef TRY_ALIGNED_OFFSET_CALLOC *PTRY_ALIGNED_OFFSET_CALLOC;

typedef
_Check_return_
_Ret_maybenull_
_Post_writable_byte_size_(NumberOfElements * ElementSize)
PVOID
(ALIGNED_OFFSET_CALLOC_WITH_TIMESTAMP)(
    _In_ PVOID Context,
    _In_ SIZE_T NumberOfElements,
    _In_ SIZE_T ElementSize,
    _In_ SIZE_T Alignment,
    _In_ SIZE_T Offset,
    _In_ PLARGE_INTEGER TimestampPointer
    );
typedef ALIGNED_OFFSET_CALLOC_WITH_TIMESTAMP
      *PALIGNED_OFFSET_CALLOC_WITH_TIMESTAMP;

typedef
_Check_return_
_Ret_maybenull_
_Post_writable_byte_size_(NumberOfElements * ElementSize)
PVOID
(TRY_ALIGNED_OFFSET_CALLOC_WITH_TIMESTAMP)(
    _In_ PVOID Context,
    _In_ SIZE_T NumberOfElements,
    _In_ SIZE_T ElementSize,
    _In_ SIZE_T Alignment,
    _In_ SIZE_T Offset,
    _In_ PLARGE_INTEGER TimestampPointer
    );
typedef TRY_ALIGNED_OFFSET_CALLOC_WITH_TIMESTAMP
      *PTRY_ALIGNED_OFFSET_CALLOC_WITH_TIMESTAMP;

//
// Define realloc-oriented functions.
//

typedef
_Check_return_
_Ret_maybenull_
_Ret_reallocated_bytes_(Buffer, NewSize)
PVOID
(REALLOC)(
    _In_ PVOID Context,
    _In_ PVOID Buffer,
    _In_ SIZE_T NewSize
    );
typedef REALLOC *PREALLOC;

//
// Define free-oriented functions.
//

typedef
VOID
(FREE)(
    _In_ PVOID Context,
    _Pre_maybenull_ _Post_invalid_ PVOID Buffer
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
VOID
(ALIGNED_FREE)(
    _In_ PVOID Context,
    _Pre_maybenull_ _Post_invalid_ PVOID Buffer
    );
typedef ALIGNED_FREE *PALIGNED_FREE;

typedef
VOID
(ALIGNED_FREE_POINTER)(
    _In_ PVOID Context,
    _In_ PPVOID PointerToBuffer
    );
typedef ALIGNED_FREE_POINTER *PALIGNED_FREE_POINTER;

//
// Define copy memory functions.
//

typedef
BOOLEAN
(WRITE_BYTES)(
    _In_ PVOID Context,
    _In_ const VOID *Source,
    _In_ SIZE_T SizeInBytes
    );
typedef WRITE_BYTES *PWRITE_BYTES;

//
// Define initialization functions.
//

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

typedef union _ALLOCATOR_FLAGS {
    struct _Struct_size_bytes_(sizeof(ULONG)) {
        ULONG IsTlsAware:1;
        ULONG IsTlsRedirectionEnabled:1;
        ULONG IsLargePageEnabled:1;
        ULONG Unused:29;
    };
    LONG AsLong;
    ULONG AsULong;
} ALLOCATOR_FLAGS;
C_ASSERT(sizeof(ALLOCATOR_FLAGS) == sizeof(ULONG));
typedef ALLOCATOR_FLAGS *PALLOCATOR_FLAGS;

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
    PWRITE_BYTES WriteBytes;

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
    // These will be set if a non-blocking malloc/calloc interface is available.
    //

    PTRY_MALLOC TryMalloc;
    PTRY_CALLOC TryCalloc;

    PMALLOC_WITH_TIMESTAMP MallocWithTimestamp;
    PCALLOC_WITH_TIMESTAMP CallocWithTimestamp;

    PTRY_MALLOC_WITH_TIMESTAMP TryMallocWithTimestamp;
    PTRY_CALLOC_WITH_TIMESTAMP TryCallocWithTimestamp;

    //
    // These will be set if an aligned allocation interface is present.
    //

    PALIGNED_MALLOC AlignedMalloc;
    PTRY_ALIGNED_MALLOC TryAlignedMalloc;
    PALIGNED_MALLOC_WITH_TIMESTAMP AlignedMallocWithTimestamp;
    PTRY_ALIGNED_MALLOC_WITH_TIMESTAMP TryAlignedMallocWithTimestamp;

    PALIGNED_OFFSET_MALLOC AlignedOffsetMalloc;
    PTRY_ALIGNED_OFFSET_MALLOC TryAlignedOffsetMalloc;
    PALIGNED_OFFSET_MALLOC_WITH_TIMESTAMP AlignedOffsetMallocWithTimestamp;
    PTRY_ALIGNED_OFFSET_MALLOC_WITH_TIMESTAMP TryAlignedOffsetMallocWithTimestamp;

    PALIGNED_CALLOC AlignedCalloc;
    PTRY_ALIGNED_CALLOC TryAlignedCalloc;
    PALIGNED_CALLOC_WITH_TIMESTAMP AlignedCallocWithTimestamp;
    PTRY_ALIGNED_CALLOC_WITH_TIMESTAMP TryAlignedCallocWithTimestamp;

    PALIGNED_OFFSET_CALLOC AlignedOffsetCalloc;
    PTRY_ALIGNED_OFFSET_CALLOC TryAlignedOffsetCalloc;
    PALIGNED_OFFSET_CALLOC_WITH_TIMESTAMP AlignedOffsetCallocWithTimestamp;
    PTRY_ALIGNED_OFFSET_CALLOC_WITH_TIMESTAMP TryAlignedOffsetCallocWithTimestamp;

    PALIGNED_FREE AlignedFree;
    PALIGNED_FREE_POINTER AlignedFreePointer;

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

} ALLOCATOR;
typedef ALLOCATOR *PALLOCATOR;
typedef ALLOCATOR **PPALLOCATOR;
//C_ASSERT(sizeof(ALLOCATOR) == 144);

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
    _Inout_ PALLOCATOR Allocator,
    _In_ PVOID Context,
    _In_ PMALLOC Malloc,
    _In_ PCALLOC Calloc,
    _In_ PREALLOC Realloc,
    _In_ PFREE Free,
    _In_ PFREE_POINTER FreePointer,
    _In_ PINITIALIZE_ALLOCATOR Initialize,
    _In_ PDESTROY_ALLOCATOR Destroy,
    _In_opt_ PWRITE_BYTES WriteBytes,
    _In_opt_ PTRY_MALLOC TryMalloc,
    _In_opt_ PTRY_CALLOC TryCalloc,
    _In_opt_ PMALLOC_WITH_TIMESTAMP MallocWithTimestamp,
    _In_opt_ PCALLOC_WITH_TIMESTAMP CallocWithTimestamp,
    _In_opt_ PTRY_MALLOC_WITH_TIMESTAMP TryMallocWithTimestamp,
    _In_opt_ PTRY_CALLOC_WITH_TIMESTAMP TryCallocWithTimestamp,
    _In_opt_ PALIGNED_MALLOC AlignedMalloc,
    _In_opt_ PTRY_ALIGNED_MALLOC TryAlignedMalloc,
    _In_opt_ PALIGNED_MALLOC_WITH_TIMESTAMP AlignedMallocWithTimestamp,
    _In_opt_ PTRY_ALIGNED_MALLOC_WITH_TIMESTAMP TryAlignedMallocWithTimestamp,
    _In_opt_ PALIGNED_OFFSET_MALLOC AlignedOffsetMalloc,
    _In_opt_ PTRY_ALIGNED_OFFSET_MALLOC TryAlignedOffsetMalloc,
    _In_opt_ PALIGNED_OFFSET_MALLOC_WITH_TIMESTAMP AlignedOffsetMallocWithTimestamp,
    _In_opt_ PTRY_ALIGNED_OFFSET_MALLOC_WITH_TIMESTAMP TryAlignedOffsetMallocWithTimestamp,
    _In_opt_ PALIGNED_CALLOC AlignedCalloc,
    _In_opt_ PTRY_ALIGNED_CALLOC TryAlignedCalloc,
    _In_opt_ PALIGNED_CALLOC_WITH_TIMESTAMP AlignedCallocWithTimestamp,
    _In_opt_ PTRY_ALIGNED_CALLOC_WITH_TIMESTAMP TryAlignedCallocWithTimestamp,
    _In_opt_ PALIGNED_OFFSET_CALLOC AlignedOffsetCalloc,
    _In_opt_ PTRY_ALIGNED_OFFSET_CALLOC TryAlignedOffsetCalloc,
    _In_opt_ PALIGNED_OFFSET_CALLOC_WITH_TIMESTAMP AlignedOffsetCallocWithTimestamp,
    _In_opt_ PTRY_ALIGNED_OFFSET_CALLOC_WITH_TIMESTAMP TryAlignedOffsetCallocWithTimestamp,
    _In_opt_ PALIGNED_FREE AlignedFree,
    _In_opt_ PALIGNED_FREE_POINTER AlignedFreePointer,
    _In_opt_ PVOID Context2
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
    Allocator->WriteBytes = WriteBytes;

    Allocator->Context2 = Context2;

    Allocator->TryMalloc = TryMalloc;
    Allocator->TryCalloc = TryCalloc;

    Allocator->MallocWithTimestamp = MallocWithTimestamp;
    Allocator->CallocWithTimestamp = CallocWithTimestamp;

    Allocator->TryMallocWithTimestamp = TryMallocWithTimestamp;
    Allocator->TryCallocWithTimestamp = TryCallocWithTimestamp;

    Allocator->AlignedMalloc = AlignedMalloc;
    Allocator->TryAlignedMalloc = TryAlignedMalloc;
    Allocator->AlignedMallocWithTimestamp = AlignedMallocWithTimestamp;
    Allocator->TryAlignedMallocWithTimestamp = TryAlignedMallocWithTimestamp;
    Allocator->AlignedOffsetMalloc = AlignedOffsetMalloc;
    Allocator->TryAlignedOffsetMalloc = TryAlignedOffsetMalloc;
    Allocator->AlignedOffsetMallocWithTimestamp = AlignedOffsetMallocWithTimestamp;
    Allocator->TryAlignedOffsetMallocWithTimestamp = TryAlignedOffsetMallocWithTimestamp;
    Allocator->AlignedCalloc = AlignedCalloc;
    Allocator->TryAlignedCalloc = TryAlignedCalloc;
    Allocator->AlignedCallocWithTimestamp = AlignedCallocWithTimestamp;
    Allocator->TryAlignedCallocWithTimestamp = TryAlignedCallocWithTimestamp;
    Allocator->AlignedOffsetCalloc = AlignedOffsetCalloc;
    Allocator->TryAlignedOffsetCalloc = TryAlignedOffsetCalloc;
    Allocator->AlignedOffsetCallocWithTimestamp = AlignedOffsetCallocWithTimestamp;
    Allocator->TryAlignedOffsetCallocWithTimestamp = TryAlignedOffsetCallocWithTimestamp;

    Allocator->AlignedFree = AlignedFree;
    Allocator->AlignedFreePointer = AlignedFreePointer;

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
    _In_opt_ PWRITE_BYTES WriteBytes,
    _In_opt_ PTRY_MALLOC TryMalloc,
    _In_opt_ PTRY_CALLOC TryCalloc,
    _In_opt_ PMALLOC_WITH_TIMESTAMP MallocWithTimestamp,
    _In_opt_ PCALLOC_WITH_TIMESTAMP CallocWithTimestamp,
    _In_opt_ PTRY_MALLOC_WITH_TIMESTAMP TryMallocWithTimestamp,
    _In_opt_ PTRY_CALLOC_WITH_TIMESTAMP TryCallocWithTimestamp,
    _In_opt_ PALIGNED_MALLOC AlignedMalloc,
    _In_opt_ PTRY_ALIGNED_MALLOC TryAlignedMalloc,
    _In_opt_ PALIGNED_MALLOC_WITH_TIMESTAMP AlignedMallocWithTimestamp,
    _In_opt_ PTRY_ALIGNED_MALLOC_WITH_TIMESTAMP TryAlignedMallocWithTimestamp,
    _In_opt_ PALIGNED_OFFSET_MALLOC AlignedOffsetMalloc,
    _In_opt_ PTRY_ALIGNED_OFFSET_MALLOC TryAlignedOffsetMalloc,
    _In_opt_ PALIGNED_OFFSET_MALLOC_WITH_TIMESTAMP AlignedOffsetMallocWithTimestamp,
    _In_opt_ PTRY_ALIGNED_OFFSET_MALLOC_WITH_TIMESTAMP TryAlignedOffsetMallocWithTimestamp,
    _In_opt_ PALIGNED_CALLOC AlignedCalloc,
    _In_opt_ PTRY_ALIGNED_CALLOC TryAlignedCalloc,
    _In_opt_ PALIGNED_CALLOC_WITH_TIMESTAMP AlignedCallocWithTimestamp,
    _In_opt_ PTRY_ALIGNED_CALLOC_WITH_TIMESTAMP TryAlignedCallocWithTimestamp,
    _In_opt_ PALIGNED_OFFSET_CALLOC AlignedOffsetCalloc,
    _In_opt_ PTRY_ALIGNED_OFFSET_CALLOC TryAlignedOffsetCalloc,
    _In_opt_ PALIGNED_OFFSET_CALLOC_WITH_TIMESTAMP AlignedOffsetCallocWithTimestamp,
    _In_opt_ PTRY_ALIGNED_OFFSET_CALLOC_WITH_TIMESTAMP TryAlignedOffsetCallocWithTimestamp,
    _In_opt_ PALIGNED_FREE AlignedFree,
    _In_opt_ PALIGNED_FREE_POINTER AlignedFreePointer,
    _In_opt_ PVOID Context2,
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
        WriteBytes,
        TryMalloc,
        TryCalloc,
        MallocWithTimestamp,
        CallocWithTimestamp,
        TryMallocWithTimestamp,
        TryCallocWithTimestamp,
        AlignedMalloc,
        TryAlignedMalloc,
        AlignedMallocWithTimestamp,
        TryAlignedMallocWithTimestamp,
        AlignedOffsetMalloc,
        TryAlignedOffsetMalloc,
        AlignedOffsetMallocWithTimestamp,
        TryAlignedOffsetMallocWithTimestamp,
        AlignedCalloc,
        TryAlignedCalloc,
        AlignedCallocWithTimestamp,
        TryAlignedCallocWithTimestamp,
        AlignedOffsetCalloc,
        TryAlignedOffsetCalloc,
        AlignedOffsetCallocWithTimestamp,
        TryAlignedOffsetCallocWithTimestamp,
        AlignedFree,
        AlignedFreePointer,
        Context2
    );

    Allocator->TlsIndex = TlsIndex;
    Allocator->Parent = Parent;

}

//
// Helper routines.
//

FORCEINLINE
BOOL
CopyMemoryQuadwords(
    PCHAR Dest,
    PCHAR Source,
    SIZE_T SizeInBytes
    )
{
    PCHAR TrailingDest;
    PCHAR TrailingSource;
    SIZE_T TrailingBytes;
    SIZE_T NumberOfQuadwords;

    NumberOfQuadwords = SizeInBytes >> 3;
    TrailingBytes = SizeInBytes % 8;

    TRY_MAPPED_MEMORY_OP {

        if (NumberOfQuadwords) {

            __movsq((PDWORD64)Dest,
                    (PDWORD64)Source,
                    NumberOfQuadwords);

        }

        if (TrailingBytes) {

            TrailingDest = (Dest + (SizeInBytes - TrailingBytes));
            TrailingSource = (Source + (SizeInBytes - TrailingBytes));

            __movsb((PBYTE)TrailingDest,
                    (PBYTE)TrailingSource,
                    TrailingBytes);

        }

    } CATCH_STATUS_IN_PAGE_ERROR {
        return FALSE;
    }

    return TRUE;
}

#ifdef CopyMemory
#undef CopyMemory
#endif

FORCEINLINE
VOID
CopyMemory(
    _Out_writes_bytes_all_(SizeInBytes) PVOID Dst,
    _In_ const VOID *Src,
    _In_ SIZE_T SizeInBytes
    )
{
    PCHAR Dest = (PCHAR)Dst;
    PCHAR Source = (PCHAR)Src;
    PCHAR TrailingDest;
    PCHAR TrailingSource;
    SIZE_T TrailingBytes;
    SIZE_T NumberOfQuadwords;

    NumberOfQuadwords = SizeInBytes >> 3;
    TrailingBytes = SizeInBytes % 8;

    if (!NumberOfQuadwords) {
        goto HandleTrailingBytes;
    }

    __movsq((PDWORD64)Dest,
            (PDWORD64)Source,
            NumberOfQuadwords);

    if (TrailingBytes) {
HandleTrailingBytes:

        TrailingDest = (Dest + (SizeInBytes - TrailingBytes));
        TrailingSource = (Source + (SizeInBytes - TrailingBytes));

        __movsb((PBYTE)TrailingDest,
                (PBYTE)TrailingSource,
                TrailingBytes);

    }
}

FORCEINLINE
BOOL
SecureZeroMemoryQuadwords(
    PVOID pDest,
    SIZE_T SizeInBytes
    )
{
    VPCHAR Dest = (VPCHAR)pDest;
    VPCHAR TrailingDest;
    SIZE_T TrailingBytes;
    SIZE_T NumberOfQuadwords;

    NumberOfQuadwords = SizeInBytes >> 3;
    TrailingBytes = SizeInBytes % 8;

    if (NumberOfQuadwords) {
        __stosq((PDWORD64)Dest, 0, NumberOfQuadwords);
    }

    if (TrailingBytes) {
        TrailingDest = (Dest + (SizeInBytes - TrailingBytes));
        __stosb((PBYTE)TrailingDest, 0, TrailingBytes);
    }

    return TRUE;
}

#ifdef SecureZeroMemory
#undef SecureZeroMemory
#endif
#define SecureZeroMemory SecureZeroMemoryQuadwords

#define ZeroStruct(Name)        __stosq((PDWORD64)&Name, 0, sizeof(Name)  >> 3)
#define ZeroStructPointer(Name) __stosq((PDWORD64)Name,  0, sizeof(*Name) >> 3)

// vim:set ts=8 sw=4 sts=4 tw=80 expandtab                                     :
