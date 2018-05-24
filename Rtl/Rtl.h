/*++

Copyright (c) 2016 Trent Nelson <trent@trent.me>

Module Name:

    Rtl.h

Abstract:

    This is the main header file for the Rtl (Run-time Library) component.
    It is named after the NT kernel's Rtl component.  It has two main roles:
    provide an interface to useful NT kernel primitives (such as bitmaps,
    prefix trees, hash tables, splay trees, AVL trees, etc) without the need
    to include ntddk.h (which can't be done if also including Windows.h) or
    link to ntdll.lib or ntoskrnl.lib (which requires the DDK to be installed).

    Type definitions for the data structures (e.g. RTL_BITMAP) are mirrored,
    and function type definitions and pointer types are provided for all NT
    functions.  They are made accessible as function pointers through a
    structure named RTL.

    In addition to NT functionality, this module also defines structures and
    functions for additional functionality we implement, such as convenience
    functions for string and path handling.

--*/

#pragma once

#ifdef __cplusplus
extern "C" {
#endif

#ifdef _RTL_INTERNAL_BUILD

//
// This is an internal build of the Rtl component.
//

#define RTL_API __declspec(dllexport)
#define RTL_DATA extern __declspec(dllexport)

#include "stdafx.h"

#elif _RTL_NO_API_EXPORT_IMPORT

//
// We're being included by someone who doesn't want dllexport or dllimport.
// This is useful for creating new .exe-based projects for things like unit
// testing or performance testing/profiling.
//

#define RTL_API
#define RTL_DATA extern

#else

//
// We're being included by an external component.
//

#define RTL_API __declspec(dllimport)
#define RTL_DATA extern __declspec(dllimport)

#include <Windows.h>
#include <sal.h>
#include <Psapi.h>
#include <DbgHelp.h>
#include <combaseapi.h>

//
// Disable inconsistent SAL annotation warnings before importing the
// intrinsics headers.
//

#pragma warning(push)
#pragma warning(disable: 28251)
#include <intrin.h>
#include <mmintrin.h>
#pragma warning(pop)

#endif

#ifdef _DEBUG
#ifndef _RTL_TEST
#define _RTL_TEST
#endif
#endif

#include "DisableWarnings.h"

//
// Define useful WDM macros if they're not already defined.
//

#ifndef PAGE_SIZE
#define PAGE_SIZE 4096
#endif

#ifndef PAGE_SHIFT
#define PAGE_SHIFT 12L
#endif

#ifndef PAGE_ALIGN
#define PAGE_ALIGN(Va) ((PVOID)((ULONG_PTR)(Va) & ~(PAGE_SIZE - 1)))
#endif

#ifndef ROUND_TO_PAGES
#define ROUND_TO_PAGES(Size) (                             \
    ((ULONG_PTR)(Size) + PAGE_SIZE - 1) & ~(PAGE_SIZE - 1) \
)
#endif

#ifndef BYTES_TO_PAGES
#define BYTES_TO_PAGES(Size) (                                  \
    (((Size) >> PAGE_SHIFT) + (((Size) & (PAGE_SIZE - 1)) != 0) \
)
#endif

#define BYTES_TO_QUADWORDS(Bytes) ((Bytes) >> 3)

#define QUADWORD_SIZEOF(Type) (BYTES_TO_QUADWORDS(sizeof(Type)))

#ifndef ADDRESS_AND_SIZE_TO_SPAN_PAGES
#define ADDRESS_AND_SIZE_TO_SPAN_PAGES(Va,Size)                             \
    ((BYTE_OFFSET (Va) + ((SIZE_T) (Size)) + (PAGE_SIZE - 1)) >> PAGE_SHIFT
#endif

#ifndef ALIGN_DOWN
#define ALIGN_DOWN(Address, Alignment)                     \
    ((ULONG_PTR)(Address) & (~((ULONG_PTR)(Alignment)-1)))
#endif

#ifndef ALIGN_UP
#define ALIGN_UP(Address, Alignment) (                        \
    (((ULONG_PTR)(Address)) + (((ULONG_PTR)(Alignment))-1)) & \
    ~(((ULONG_PTR)(Alignment))-1)                             \
)
#endif

#ifndef ALIGN_UP_POINTER
#define ALIGN_UP_POINTER(Address) (ALIGN_UP(Address, sizeof(ULONG_PTR)))
#endif

#ifndef ALIGN_DOWN_POINTER
#define ALIGN_DOWN_POINTER(Address) (ALIGN_DOWN(Address, sizeof(ULONG_PTR)))
#endif

#ifndef ALIGN_DOWN_PAGE
#define ALIGN_DOWN_PAGE(Address) (ALIGN_DOWN(Address, PAGE_SIZE))
#endif

#ifndef ALIGN_DOWN_USHORT_TO_POINTER_SIZE
#define ALIGN_DOWN_USHORT_TO_POINTER_SIZE(Value)                   \
    (USHORT)(ALIGN_DOWN((USHORT)Value, (USHORT)sizeof(ULONG_PTR)))
#endif

#ifndef ALIGN_UP_USHORT_TO_POINTER_SIZE
#define ALIGN_UP_USHORT_TO_POINTER_SIZE(Value)                   \
    (USHORT)(ALIGN_UP((USHORT)Value, (USHORT)sizeof(ULONG_PTR)))
#endif

#define BITMAP_ALIGNMENT 128
#define ALIGN_UP_BITMAP(Address)                  \
    (USHORT)(ALIGN_UP(Address, BITMAP_ALIGNMENT))

#ifndef ARGUMENT_PRESENT
#define ARGUMENT_PRESENT(ArgumentPointer) (                  \
    (CHAR *)((ULONG_PTR)(ArgumentPointer)) != (CHAR *)(NULL) \
)
#endif

#ifndef NOTHING
#define NOTHING
#endif

#ifndef DECLSPEC_RESTRICT
#define DECLSPEC_RESTRICT __declspec(restrict)
#endif

#ifndef DECLSPEC_NOALIAS
#define DECLSPEC_NOALIAS __declspec(noalias)
#endif

#ifndef DECLSPEC_NOINLINE
#define DECLSPEC_NOINLINE __declspec(noinline)
#endif

#ifndef DECLSPEC_THREAD
#define DECLSPEC_THREAD __declspec(thread)
#endif

#ifndef DECLSPEC_DLLEXPORT
#define DECLSPEC_DLLEXPORT __declspec(dllexport)
#endif

#ifndef DECLSPEC_DLLIMPORT
#define DECLSPEC_DLLIMPORT __declspec(dllimport)
#endif

#ifndef NOINLINE
#define NOINLINE __declspec(noinline)
#endif

#ifndef INLINE
#define INLINE __inline
#endif

#ifndef FORCEINLINE
#define FORCEINLINE __forceinline
#endif

#ifndef RtlUpcaseChar
#define RtlUpcaseChar(C)                                         \
    (CHAR)(((C) >= 'a' && (C) <= 'z' ? (C) - ('a' - 'A') : (C)))
#endif

#ifndef WINAPI
#define WINAPI __stdcall
#endif

#ifndef RtlUpcaseUnicodeChar
#define RtlUpcaseUnicodeChar(C)                                   \
    (WCHAR)(((C) >= 'a' && (C) <= 'z' ? (C) - ('a' - 'A') : (C)))
#endif

#ifndef RtlOffsetToPointer
#define RtlOffsetToPointer(B,O)    ((PCHAR)(((PCHAR)(B)) + ((ULONG_PTR)(O))))
#endif

#ifndef RtlOffsetFromPointer
#define RtlOffsetFromPointer(B,O)  ((PCHAR)(((PCHAR)(B)) - ((ULONG_PTR)(O))))
#endif

#ifndef RtlPointerToOffset
#define RtlPointerToOffset(B,P)    ((ULONG_PTR)(((PCHAR)(P)) - ((PCHAR)(B))))
#endif

#ifndef FlagOn
#define FlagOn(_F,_SF)        ((_F) & (_SF))
#endif

#ifndef BooleanFlagOn
#define BooleanFlagOn(F,SF)   ((BOOLEAN)(((F) & (SF)) != 0))
#endif

#ifndef SetFlag
#define SetFlag(_F,_SF)       ((_F) |= (_SF))
#endif

#ifndef ClearFlag
#define ClearFlag(_F,_SF)     ((_F) &= ~(_SF))
#endif

#ifndef ASSERT
#define ASSERT(Condition) \
    if (!(Condition)) {   \
        __debugbreak();   \
    }
#endif

#define NOT_IMPLEMENTED() __debugbreak(); return
#define NOT_IMPLEMENTED_RETURN_NULL() __debugbreak(); return NULL

//
// Helper macros.
//

#define CHECKED(Expr) do { \
    if (!(Expr)) {         \
        goto Error;        \
    }                      \
} while (0)

#define CHECKED_HRESULT(Expr) do {     \
    if (((Result) = (Expr)) != S_OK) { \
        goto Error;                    \
    }                                  \
} while (0)

#define CHECKED_NTSTATUS(Expr) do { \
    if ((Expr) != STATUS_SUCCESS) { \
        goto Error;                 \
    }                               \
} while (0)

#define CHECKED_MSG(Expr, FailureMessage) do {              \
    if (!(Expr)) {                                          \
        OutputDebugStringA("Failed: " FailureMessage "\n"); \
        goto Error;                                         \
    }                                                       \
} while (0)

#define CHECKED_HRESULT_MSG(Expr, FailureMessage) do {      \
    if (((Result) = (Expr)) != S_OK) {                      \
        OutputDebugStringA("Failed: " FailureMessage "\n"); \
        goto Error;                                         \
    }                                                       \
} while (0)

#define CHECKED_NTSTATUS_MSG(Expr, FailureMessage) do {     \
    if (((Status) = (Expr)) != ERROR_SUCCESS) {             \
        OutputDebugStringA("Failed: " FailureMessage "\n"); \
        goto Error;                                         \
    }                                                       \
} while (0)

#define LOAD_LIBRARY_W(Target, Name) do {                  \
    (Target) = LoadLibraryW(L#Name);                       \
    if (!(Target)) {                                       \
        OutputDebugStringA("Failed to load " #Name ".\n"); \
        goto Error;                                        \
    }                                                      \
} while (0)

#define LOAD_LIBRARY_A(Target, Name) do {                  \
    (Target) = LoadLibraryA(#Name);                        \
    if (!(Target)) {                                       \
        OutputDebugStringA("Failed to load " #Name ".\n"); \
        goto Error;                                        \
    }                                                      \
} while (0)

#define RESOLVE_FUNCTION(Target, Module, Type, Name) do {                  \
    (Target) = (Type)GetProcAddress((Module), #Name);                      \
    if (!(Target)) {                                                       \
        OutputDebugStringA("Failed to resolve " #Module " !" #Name ".\n"); \
        goto Error;                                                        \
    }                                                                      \
} while (0)

#define ALLOCATE_TYPE(Target, Type, Allocator) do {                   \
    (Target) = (P##Type)(                                             \
        Allocator->Calloc(                                            \
            Allocator->Context,                                       \
            1,                                                        \
            sizeof(Type)                                              \
        )                                                             \
    );                                                                \
    if (!(Target)) {                                                  \
        OutputDebugStringA("Allocation of type " #Type " failed.\n"); \
        goto Error;                                                   \
    }                                                                 \
} while (0)

#define MAYBE_FREE_POINTER(Target, Allocator) do {      \
    if ((Target)) {                                     \
        Allocator->Free(Allocator->Context, &(Target)); \
    }                                                   \
} while (0)

#define MAYBE_FREE_LIBRARY(Module) do { \
    if ((Module)) {                     \
        FreeLibrary((Module));          \
        Module = NULL;                  \
    }                                   \
} while (0)

#define MAYBE_CLOSE_HANDLE(Handle) do { \
    if ((Handle)) {                     \
        CloseHandle((Handle));          \
        Handle = NULL;                  \
    }                                   \
} while (0)

#define TIMESTAMP_TO_SECONDS    1000000
#define SECONDS_TO_MICROSECONDS 1000000

#define TRY_TSX __try
#define TRY_AVX __try
#define TRY_AVX512 __try
#define TRY_AVX_ALIGNED __try
#define TRY_AVX_UNALIGNED __try

#define TRY_SSE42 __try
#define TRY_SSE42_ALIGNED __try
#define TRY_SSE42_UNALIGNED __try

#define TRY_PROBE_MEMORY __try
#define TRY_MAPPED_MEMORY_OP __try

#define CATCH_EXCEPTION_ILLEGAL_INSTRUCTION __except(     \
    GetExceptionCode() == EXCEPTION_ILLEGAL_INSTRUCTION ? \
        EXCEPTION_EXECUTE_HANDLER :                       \
        EXCEPTION_CONTINUE_SEARCH                         \
    )

#define CATCH_EXCEPTION_ACCESS_VIOLATION __except(     \
    GetExceptionCode() == EXCEPTION_ACCESS_VIOLATION ? \
        EXCEPTION_EXECUTE_HANDLER :                    \
        EXCEPTION_CONTINUE_SEARCH                      \
    )

#define CATCH_STATUS_IN_PAGE_ERROR __except(     \
    GetExceptionCode() == STATUS_IN_PAGE_ERROR ? \
        EXCEPTION_EXECUTE_HANDLER :              \
        EXCEPTION_CONTINUE_SEARCH                \
    )

#define CATCH_STATUS_IN_PAGE_ERROR_OR_ACCESS_VIOLATION __except( \
    GetExceptionCode() == STATUS_IN_PAGE_ERROR ||                \
    GetExceptionCode() == EXCEPTION_ACCESS_VIOLATION ?           \
        EXCEPTION_EXECUTE_HANDLER :                              \
        EXCEPTION_CONTINUE_SEARCH                                \
    )

//
// Build type static strings.
//

#ifdef _BUILD_CONFIG_PGI
static const char BuildConfigString[] = "PGI";
#elif defined(_BUILD_CONFIG_PGU)
static const char BuildConfigString[] = "PGU";
#elif defined(_BUILD_CONFIG_PGO)
static const char BuildConfigString[] = "PGO";
#elif defined(_BUILD_CONFIG_RELEASE)
static const char BuildConfigString[] = "Release";
#elif defined(_BUILD_CONFIG_DEBUG)
static const char BuildConfigString[] = "Debug";
#else
#error Unknown build config type.
#endif

//
// Define start/end markers for IACA.
//

#define IACA_VC_START() __writegsbyte(111, 111)
#define IACA_VC_END()   __writegsbyte(222, 222)

//
// Define common NT-style typedefs.
//

typedef const LONG CLONG;
typedef PVOID *PPVOID;
typedef const PVOID PCVOID;
typedef const PVOID CPVOID;
typedef const BYTE CBYTE;
typedef CBYTE *PCBYTE;
typedef CHAR **PPCHAR;
typedef WCHAR **PPWCHAR;
typedef CHAR **PPSTR;
typedef WCHAR **PPWSTR;
typedef HMODULE *PHMODULE;
typedef PROC *PPROC;
typedef DOUBLE *PDOUBLE;
typedef DOUBLE **PPDOUBLE;
typedef const DOUBLE *PCDOUBLE;
typedef const DOUBLE **PPCDOUBLE;

typedef volatile CHAR *VPCHAR;

typedef const SHORT CSHORT;

#define MAX_USHORT (USHORT)0xffff

typedef struct _RTL *PRTL;

typedef struct _STRING {
    USHORT Length;
    USHORT MaximumLength;
    union {
        LONG Hash;
        LONG Offset;
    };
    PCHAR  Buffer;
} STRING, ANSI_STRING, *PSTRING, *PANSI_STRING, **PPSTRING, **PPANSI_STRING;
typedef const STRING *PCSTRING;

typedef struct _UNICODE_STRING {
    USHORT Length;
    USHORT MaximumLength;
    union {
        LONG Hash;
        LONG Offset;
    };
    PWSTR  Buffer;
} UNICODE_STRING, *PUNICODE_STRING, **PPUNICODE_STRING, ***PPPUNICODE_STRING;
typedef const UNICODE_STRING *PCUNICODE_STRING;
#define UNICODE_NULL ((WCHAR)0)

typedef struct _LINKED_STRING {
    LIST_ENTRY ListEntry;
    union {
        struct {
            USHORT Length;
            USHORT MaximumLength;
            LONG   Hash;
            union {
                PCHAR Buffer;
                PWCHAR WideBuffer;
            };
        };
        STRING String;
        UNICODE_STRING Unicode;
    };
} LINKED_STRING;
typedef LINKED_STRING *PLINKED_STRING;
typedef LINKED_STRING **PPLINKED_STRING;
typedef LINKED_STRING LINKED_LINE;
typedef LINKED_LINE  *PLINKED_LINE;
typedef LINKED_LINE **PPLINKED_LINE;

#include "../Asm/Asm.h"
#include "Time.h"
#include "Memory.h"
#include "HeapAllocator.h"
#include "Commandline.h"

typedef _Null_terminated_ CONST CHAR *PCSZ;
typedef CONST CHAR *PCCHAR;

typedef _Null_terminated_ CONST WCHAR *PCWSZ;
typedef CONST WCHAR *PCWCHAR;

#ifdef _M_X64
#pragma intrinsic(__readgsdword)
FORCEINLINE
DWORD
FastGetCurrentProcessId(VOID)
{
    return __readgsdword(0x40);
}

FORCEINLINE
DWORD
FastGetCurrentThreadId(VOID)
{
    return __readgsdword(0x48);
}
#endif

typedef LONG (MAINPROCA)(
    _In_ LONG NumberOfArguments,
    _In_ PPSTR ArgvA
    );
typedef MAINPROCA *PMAINPROCA;

typedef LONG (MAINPROCW)(
    _In_ LONG NumberOfArguments,
    _In_ PPWSTR ArgvW
    );
typedef MAINPROCW *PMAINPROCW;

typedef VOID (RTL_INIT_STRING)(
    _Out_       PSTRING     DestinationString,
    _In_opt_    PCSZ        SourceString
    );

typedef RTL_INIT_STRING *PRTL_INIT_STRING;

typedef VOID (RTL_COPY_UNICODE_STRING)(
    _Inout_  PUNICODE_STRING  DestinationString,
    _In_opt_ PCUNICODE_STRING SourceString
    );
typedef RTL_COPY_UNICODE_STRING *PRTL_COPY_UNICODE_STRING;

typedef VOID (RTL_COPY_STRING)(
    _Out_    PSTRING  DestinationString,
    _In_opt_ PCSTRING SourceString
    );
typedef RTL_COPY_STRING *PRTL_COPY_STRING;

typedef
NTSTATUS
(NTAPI RTL_APPEND_STRING_TO_STRING)(
    _Inout_       PSTRING Destination,
    _In_    const STRING  *Source
    );
typedef RTL_APPEND_STRING_TO_STRING *PRTL_APPEND_STRING_TO_STRING;

typedef NTSTATUS (*PRTL_APPEND_UNICODE_TO_STRING)(
    _Inout_  PUNICODE_STRING Destination,
    _In_opt_ PCWSTR          Source
    );

typedef NTSTATUS (*PRTL_APPEND_UNICODE_STRING_TO_STRING)(
    _Inout_ PUNICODE_STRING  Destination,
    _In_    PCUNICODE_STRING Source
    );

typedef NTSTATUS (*PRTL_UNICODE_STRING_TO_ANSI_STRING)(
    _Inout_ PANSI_STRING     DestinationString,
    _In_    PCUNICODE_STRING SourceString,
    _In_    BOOLEAN          AllocateDestinationString
    );

typedef ULONG (*PRTL_UNICODE_STRING_TO_ANSI_SIZE)(
    _In_ PUNICODE_STRING UnicodeString
    );

typedef BOOLEAN (*PRTL_EQUAL_STRING)(
    _In_    PCSTRING    String1,
    _In_    PCSTRING    String2,
    _In_    BOOLEAN     CaseInSensitive
    );

typedef BOOLEAN (*PRTL_EQUAL_UNICODE_STRING)(
    _In_    PCUNICODE_STRING    String1,
    _In_    PCUNICODE_STRING    String2,
    _In_    BOOLEAN             CaseInSensitive
    );

typedef LONG (*PRTL_COMPARE_STRING)(
    _In_ PCSTRING String1,
    _In_ PCSTRING String2,
    _In_ BOOL     CaseInSensitive
    );

typedef LONG (*PCOMPARE_STRING_CASE_INSENSITIVE)(
    _In_ PCSTRING String1,
    _In_ PCSTRING String2
    );


typedef struct _RTL_BITMAP {
    ULONG SizeOfBitMap;     // Number of bits.
    PULONG Buffer;
} RTL_BITMAP, *PRTL_BITMAP, **PPRTL_BITMAP;

typedef struct _RTL_BITMAP_RUN {
    ULONG StartingIndex;
    ULONG NumberOfBits;
} RTL_BITMAP_RUN, *PRTL_BITMAP_RUN, **PPRTL_BITMAP_RUN;

//
// The various bitmap find functions return 0xFFFFFFFF
// if they couldn't find the requested bit pattern.
//

#define BITS_NOT_FOUND 0xFFFFFFFF


// 65535 (1 << 16)
#define MAX_STRING  ((USHORT)0xffff)
#define MAX_USTRING ((USHORT)0xffff)

#define RTL_CONSTANT_STRING(s) { \
    sizeof(s) - sizeof((s)[0]),  \
    sizeof( s ),                 \
    0,                           \
    s                            \
}

typedef union _ULONG_INTEGER {
    struct {
        USHORT  LowPart;
        USHORT  HighPart;
    };
    ULONG   LongPart;

} ULONG_INTEGER, *PULONG_INTEGER;

typedef union _LONG_INTEGER {
    struct {
        USHORT  LowPart;
        SHORT   HighPart;
    };
    LONG   LongPart;
} LONG_INTEGER, *PLONG_INTEGER;

typedef union _USHORT_INTEGER {
    struct {
        BYTE  LowPart;
        BYTE  HighPart;
    };
    USHORT   ShortPart;

} USHORT_INTEGER, *PUSHORT_INTEGER;

typedef union _SHORT_INTEGER {
    struct {
        CHAR  LowPart;
        CHAR  HighPart;
    };
    SHORT   ShortPart;

} SHORT_INTEGER, *PSHORT_INTEGER;

typedef union _WIDE_CHARACTER {
    struct {
        CHAR  LowPart;
        CHAR  HighPart;
    };
    WCHAR WidePart;

} WIDE_CHARACTER, *PWIDE_CHARACTER;

typedef union _OCTWORD_INTEGER {
    struct {
        LARGE_INTEGER LowPart;
        LARGE_INTEGER HighPart;
    };
    __m128 OctPart;
} OCTWORD_INTEGER, *POCTWORD_INTEGER, **PPOCTWORD_INTEGER;

typedef _Null_terminated_ CHAR *PSZ;
//typedef const PSZ PCSZ;

typedef BOOL (WINAPI *PGET_PROCESS_MEMORY_INFO)(
    _In_  HANDLE                      Process,
    _Out_ PPROCESS_MEMORY_COUNTERS    ppsmemCounters,
    _In_  DWORD                       cb
    );

typedef
BOOL
(WINAPI GET_PERFORMANCE_INFO)(
    _Out_writes_bytes_all_(cb) PPERFORMANCE_INFORMATION PerfInfo,
    _In_ DWORD cb
    );
typedef GET_PERFORMANCE_INFO *PGET_PERFORMANCE_INFO;

typedef BOOL (WINAPI *PGET_PROCESS_IO_COUNTERS)(
    _In_  HANDLE          Process,
    _Out_ PIO_COUNTERS    lpIoCounters
    );

typedef BOOL (WINAPI *PGET_PROCESS_HANDLE_COUNT)(
    _In_  HANDLE      Process,
    _Out_ PDWORD      pdwHandleCount
    );

typedef
_Check_return_
_Success_(return != 0)
BOOL
(WINAPI INITIALIZE_PROCESS_FOR_WS_WATCH)(
    _In_ HANDLE ProcessHandle
    );
typedef INITIALIZE_PROCESS_FOR_WS_WATCH *PINITIALIZE_PROCESS_FOR_WS_WATCH;

typedef
_Check_return_
_Success_(return != 0)
BOOL
(WINAPI GET_WS_CHANGES)(
    _In_ HANDLE ProcessHandle,
    _Out_writes_bytes_(SizeOfWatchInfoBufferInBytes)
         PPSAPI_WS_WATCH_INFORMATION WatchInfo,
    _In_ ULONG SizeOfWatchInfoBufferInBytes
    );
typedef GET_WS_CHANGES *PGET_WS_CHANGES;

typedef
_Check_return_
_Success_(return != 0)
BOOL
(WINAPI GET_WS_CHANGES_EX)(
    _In_ HANDLE ProcessHandle,

    _Out_writes_bytes_to_(*SizeOfWatchInfoExBufferInBytes,
                          *SizeOfWatchInfoExBufferInBytes)
         PPSAPI_WS_WATCH_INFORMATION_EX WatchInfoEx,

    _Inout_ PULONG SizeOfWatchInfoExBufferInBytes
    );
typedef GET_WS_CHANGES_EX *PGET_WS_CHANGES_EX;

typedef
_Check_return_
_Success_(return != 0)
BOOL
(WINAPI QUERY_WORKING_SET)(
    _In_ HANDLE ProcessHandle,

    _Out_writes_bytes_(SizeOfWorkingSetInfoBufferInBytes)
         PPSAPI_WORKING_SET_INFORMATION WorkingSetInfo,

    _In_ ULONG SizeOfWorkingSetInfoBufferInBytes
    );
typedef QUERY_WORKING_SET *PQUERY_WORKING_SET;

typedef
_Check_return_
_Success_(return != 0)
BOOL
(WINAPI QUERY_WORKING_SET_EX)(
    _In_ HANDLE ProcessHandle,

    _Out_writes_bytes_(SizeOfWorkingSetExInfoBufferInBytes)
         PPSAPI_WORKING_SET_EX_INFORMATION WorkingSetExInfo,

    _In_ ULONG SizeOfWorkingSetExInfoBufferInBytes
    );
typedef QUERY_WORKING_SET_EX *PQUERY_WORKING_SET_EX;

//
// Console Handler routines.
//

typedef
BOOL
(WINAPI HANDLER_ROUTINE)(
    _In_ ULONG ControlType
    );
typedef HANDLER_ROUTINE *PHANDLER_ROUTINE;

typedef
BOOL
(WINAPI SET_CONSOLE_CTRL_HANDLER)(
    _In_opt_ PHANDLER_ROUTINE HandlerRoutine,
    _In_ BOOL Add
    );
typedef SET_CONSOLE_CTRL_HANDLER *PSET_CONSOLE_CTRL_HANDLER;

//
// Rtl Heap related functions.
//

typedef
NTSTATUS
(NTAPI RTL_HEAP_COMMIT_ROUTINE)(
    _In_    PVOID    Base,
    _Inout_ PVOID   *CommitAddress,
    _Inout_ PSIZE_T  CommitSize
    );
typedef RTL_HEAP_COMMIT_ROUTINE *PRTL_HEAP_COMMIT_ROUTINE;

typedef struct _RTL_HEAP_PARAMETERS {
    ULONG Length;
    SIZE_T SegmentReserve;
    SIZE_T SegmentCommit;
    SIZE_T DeCommitFreeBlockThreshold;
    SIZE_T DeCommitTotalFreeThreshold;
    SIZE_T MaximumAllocationSize;
    SIZE_T VirtualMemoryThreshold;
    SIZE_T InitialCommit;
    SIZE_T InitialReserve;
    PRTL_HEAP_COMMIT_ROUTINE CommitRoutine;
    SIZE_T Reserved[2];
} RTL_HEAP_PARAMETERS, *PRTL_HEAP_PARAMETERS;

typedef
_Check_return_
_Success_(return != 0)
PVOID
(NTAPI RTL_ALLOCATE_HEAP)(
    _In_        PVOID   HeapHandle,
    _In_opt_    ULONG   Flags,
    _In_        SIZE_T  SizeInBytes
    );
typedef RTL_ALLOCATE_HEAP *PRTL_ALLOCATE_HEAP;

typedef
_Check_return_
_Success_(return != 0)
PVOID
(NTAPI RTL_CREATE_HEAP)(
    _In_     ULONG                Flags,
    _In_opt_ PVOID                HeapBase,
    _In_opt_ SIZE_T               ReserveSize,
    _In_opt_ SIZE_T               CommitSize,
    _In_opt_ PVOID                Lock,
    _In_opt_ PRTL_HEAP_PARAMETERS Parameters
    );
typedef RTL_CREATE_HEAP *PRTL_CREATE_HEAP;

typedef
_Check_return_
PVOID
(NTAPI RTL_DESTROY_HEAP)(
    _In_ PVOID HeapHandle
    );
typedef RTL_DESTROY_HEAP *PRTL_DESTROY_HEAP;

typedef
_Check_return_
_Success_(return != 0)
BOOLEAN
(NTAPI RTL_FREE_HEAP)(
    _In_     PVOID HeapHandle,
    _In_opt_ ULONG Flags,
    _In_     PVOID HeapBase
    );
typedef RTL_FREE_HEAP *PRTL_FREE_HEAP;

typedef
_Check_return_
NTSTATUS
(NTAPI ZW_ALLOCATE_VIRTUAL_MEMORY)(
    _In_    HANDLE    ProcessHandle,
    _Inout_ PVOID     *BaseAddress,
    _In_    ULONG_PTR ZeroBits,
    _Inout_ PSIZE_T   RegionSize,
    _In_    ULONG     AllocationType,
    _In_    ULONG     Protect
    );
typedef ZW_ALLOCATE_VIRTUAL_MEMORY *PZW_ALLOCATE_VIRTUAL_MEMORY;

typedef
_Check_return_
NTSTATUS
(NTAPI ZW_FREE_VIRTUAL_MEMORY)(
    _In_    HANDLE  ProcessHandle,
    _Inout_ PVOID   *BaseAddress,
    _Inout_ PSIZE_T RegionSize,
    _In_    ULONG   FreeType
    );
typedef ZW_FREE_VIRTUAL_MEMORY *PZW_FREE_VIRTUAL_MEMORY;

typedef
USHORT
(NTAPI RTL_CAPTURE_STACK_BACK_TRACE)(
    _In_      ULONG  FramesToSkip,
    _In_      ULONG  FramesToCapture,
    _Out_     PVOID  *BackTrace,
    _Out_opt_ PULONG BackTraceHash
    );
typedef RTL_CAPTURE_STACK_BACK_TRACE *PRTL_CAPTURE_STACK_BACK_TRACE;

typedef
PVOID
(NTAPI RTL_PC_TO_FILE_HEADER)(
    _In_ PVOID PcValue,
    _Out_ PVOID * BaseOfImage
    );
typedef RTL_PC_TO_FILE_HEADER *PRTL_PC_TO_FILE_HEADER;

//
// Runtime function table function typedefs.
//

typedef
PRUNTIME_FUNCTION
(WINAPI RTL_LOOKUP_FUNCTION_ENTRY)(
    _In_  ULONGLONG  ControlPc,
    _Out_ PULONGLONG ImageBase,
    _Inout_opt_ PUNWIND_HISTORY_TABLE HistoryTable
    );
typedef RTL_LOOKUP_FUNCTION_ENTRY *PRTL_LOOKUP_FUNCTION_ENTRY;

typedef
BOOLEAN
(__cdecl RTL_ADD_FUNCTION_TABLE)(
    _In_reads_(EntryCount) PRUNTIME_FUNCTION FunctionTable,
    _In_ DWORD EntryCount,
    _In_ DWORD64 BaseAddress
    );
typedef RTL_ADD_FUNCTION_TABLE *PRTL_ADD_FUNCTION_TABLE;

typedef
BOOLEAN
(__cdecl RTL_DELETE_FUNCTION_TABLE)(
    _In_ PRUNTIME_FUNCTION FunctionTable
    );
typedef RTL_DELETE_FUNCTION_TABLE *PRTL_DELETE_FUNCTION_TABLE;

typedef
BOOLEAN
(__cdecl RTL_INSTALL_FUNCTION_TABLE_CALLBACK)(
    _In_ DWORD64 TableIdentifier,
    _In_ DWORD64 BaseAddress,
    _In_ DWORD Length,
    _In_ PGET_RUNTIME_FUNCTION_CALLBACK Callback,
    _In_opt_ PVOID Context,
    _In_opt_ PCWSTR OutOfProcessCallbackDll
    );
typedef RTL_INSTALL_FUNCTION_TABLE_CALLBACK
       *PRTL_INSTALL_FUNCTION_TABLE_CALLBACK;

typedef
ULONG
(NTAPI RTL_ADD_GROWABLE_FUNCTION_TABLE)(
    _Out_ PVOID * DynamicTable,
    _In_reads_(MaximumEntryCount) PRUNTIME_FUNCTION FunctionTable,
    _In_ DWORD EntryCount,
    _In_ DWORD MaximumEntryCount,
    _In_ ULONG_PTR RangeBase,
    _In_ ULONG_PTR RangeEnd
    );
typedef RTL_ADD_GROWABLE_FUNCTION_TABLE *PRTL_ADD_GROWABLE_FUNCTION_TABLE;

typedef
VOID
(NTAPI RTL_GROW_FUNCTION_TABLE)(
    _Inout_ PVOID DynamicTable,
    _In_ DWORD NewEntryCount
    );
typedef RTL_GROW_FUNCTION_TABLE *PRTL_GROW_FUNCTION_TABLE;

typedef enum _UWOP_UNWIND_CODE {
    UWOP_PUSH_NONVOL        = 0,        //  1
    UWOP_ALLOC_LARGE        = 1,        //  2 or 3
    UWOP_ALLOC_SMALL        = 2,        //  1
    UWOP_SET_FPREG          = 3,        //  1
    UWOP_SAVE_NONVOL        = 4,        //  2
    UWOP_SAVE_NONVOL_FAR    = 5,        //  3
    UWOP_SPARE_CODE1        = 6,        //  0
    UWOP_SPARE_CODE2        = 7,        //  0
    UWOP_SAVE_XMM128        = 8,        //  2
    UWOP_SAVE_XMM128_FAR    = 9,        //  3
    UWOP_PUSH_MACHFRAME     = 10,       //  1
} UWOP_UNWIND_CODE;
typedef UWOP_UNWIND_CODE *PUWOP_UNWIND_CODE;

typedef enum _UWOP_UNWIND_OPREG {
    UWOP_REG_RAX            = 0,
    UWOP_REG_RCX            = 1,
    UWOP_REG_RDX            = 2,
    UWOP_REG_RBX            = 3,
    UWOP_REG_RSP            = 4,
    UWOP_REG_RBP            = 5,
    UWOP_REG_RSI            = 6,
    UWOP_REG_RDI            = 7,
    UWOP_REG_R8             = 8,
    UWOP_REG_R9             = 9,
    UWOP_REG_R10            = 10,
    UWOP_REG_R11            = 11,
    UWOP_REG_R12            = 12,
    UWOP_REG_R13            = 13,
    UWOP_REG_R14            = 14,
    UWOP_REG_R15            = 15,
} UWOP_UNWIND_OPREG;
typedef UWOP_UNWIND_OPREG *PUWOP_UNWIND_OPREG;

#ifndef UBYTE
#define UBYTE unsigned char
#endif

#pragma pack(push, 1)
typedef struct _UNWIND_CODE {
    UBYTE PrologOffset;
    UBYTE UnwindOpCode:4;
    UBYTE UnwindOpInfo:4;
} UNWIND_CODE;
typedef UNWIND_CODE *PUNWIND_CODE;

typedef struct _UNWIND_CODE_EX {
    UBYTE PrologOffset;
    union {
        struct {
            UBYTE UnwindOpCode:4;
            UBYTE UnwindOpInfo:4;
        };
        struct {
            UWOP_UNWIND_CODE Code;
        };
        struct {
            UBYTE _UnwindOpCode:4;
            UWOP_UNWIND_OPREG Reg;
        };
    };
} UNWIND_CODE_EX;
typedef UNWIND_CODE_EX *PUNWIND_CODE_EX;

typedef struct _UNWIND_INFO {
    UBYTE Version:3;
    UBYTE Flags:5;
    UBYTE SizeOfProlog;
    UBYTE CountOfCodes;
    UBYTE FrameRegister:4;
    UBYTE FrameOffset:4;

    //
    // The array of unwind codes will always be padded out to an even number;
    // thus, the minimum size will be 2 (or 0 if no codes).
    //

    UNWIND_CODE UnwindCode[2];

    union {
        struct {
            ULONG ExceptionHandler;
            ULONG ExceptionData[1];
        };
        union {
            struct {
                ULONG FunctionStartAddress;
                ULONG FunctionEndAddress;
                ULONG UnwindInfoAddress;
            };
            RUNTIME_FUNCTION RuntimeFunction;
        };
    };
} UNWIND_INFO;
typedef UNWIND_INFO *PUNWIND_INFO;

typedef struct _UNWIND_SCOPE_RECORD {
    ULONG BeginAddress;
    ULONG EndAddress;
    ULONG HandlerAddress;
    ULONG JumpTarget;
} UNWIND_SCOPE_RECORD;

typedef struct _UNWIND_SCOPE_TABLE {
    ULONG Count;
    UNWIND_SCOPE_RECORD ScopeRecord[1];
} UNWIND_SCOPE_TABLE;
typedef UNWIND_SCOPE_TABLE *PUNWIND_SCOPE_TABLE;

//
// Helper structs based on UNWIND_INFO with hard-coded code sizes in common
// intervals (4, 6, 8 etc).
//

typedef struct _UNWIND_INFO4 {
    UBYTE Version:3;
    UBYTE Flags:5;
    UBYTE SizeOfProlog;
    UBYTE CountOfCodes;
    UBYTE FrameRegister:4;
    UBYTE FrameOffset:4;
    UNWIND_CODE UnwindCode[4];
    union {
        struct {
            ULONG ExceptionHandler;
            ULONG ExceptionData[1];
        };
        union {
            struct {
                ULONG FunctionStartAddress;
                ULONG FunctionEndAddress;
                ULONG UnwindInfoAddress;
            };
            RUNTIME_FUNCTION RuntimeFunction;
        };
    };
} UNWIND_INFO4;
typedef UNWIND_INFO4 UNWIND_INFO3;
typedef UNWIND_INFO4 *PUNWIND_INFO4;

typedef struct _UNWIND_INFO6 {
    UBYTE Version:3;
    UBYTE Flags:5;
    UBYTE SizeOfProlog;
    UBYTE CountOfCodes;
    UBYTE FrameRegister:4;
    UBYTE FrameOffset:4;
    UNWIND_CODE UnwindCode[6];
    union {
        struct {
            ULONG ExceptionHandler;
            ULONG ExceptionData[1];
        };
        union {
            struct {
                ULONG FunctionStartAddress;
                ULONG FunctionEndAddress;
                ULONG UnwindInfoAddress;
            };
            RUNTIME_FUNCTION RuntimeFunction;
        };
    };
} UNWIND_INFO6;
typedef UNWIND_INFO6 UNWIND_INFO5;
typedef UNWIND_INFO6 *PUNWIND_INFO6;

typedef struct _UNWIND_INFO8 {
    UBYTE Version:3;
    UBYTE Flags:5;
    UBYTE SizeOfProlog;
    UBYTE CountOfCodes;
    UBYTE FrameRegister:4;
    UBYTE FrameOffset:4;
    UNWIND_CODE UnwindCode[8];
    union {
        struct {
            ULONG ExceptionHandler;
            ULONG ExceptionData[1];
        };
        union {
            struct {
                ULONG FunctionStartAddress;
                ULONG FunctionEndAddress;
                ULONG UnwindInfoAddress;
            };
            RUNTIME_FUNCTION RuntimeFunction;
        };
    };
} UNWIND_INFO8;
typedef UNWIND_INFO8 UNWIND_INFO7;
typedef UNWIND_INFO8 *PUNWIND_INFO8;

typedef union _UNWIND_INFO_EX {
    UNWIND_INFO     UnwindInfo;
    UNWIND_INFO3    UnwindInfo3;
    UNWIND_INFO4    UnwindInfo4;
    UNWIND_INFO5    UnwindInfo5;
    UNWIND_INFO6    UnwindInfo6;
    UNWIND_INFO7    UnwindInfo7;
    UNWIND_INFO8    UnwindInfo8;
} UNWIND_INFO_EX;
typedef UNWIND_INFO_EX *PUNWIND_INFO_EX;

#pragma pack(pop)

typedef struct _RUNTIME_FUNCTION_EX {
    PVOID BeginAddress;
    PVOID EndAddress;
    union {
        PUNWIND_INFO UnwindInfo;
        PUNWIND_INFO_EX UnwindInfoEx;
        PVOID UnwindData;
    };
    PVOID HandlerFunction;
    PUNWIND_SCOPE_TABLE ScopeTable;
} RUNTIME_FUNCTION_EX;
typedef RUNTIME_FUNCTION_EX *PRUNTIME_FUNCTION_EX;

typedef
VOID
(NTAPI RTL_DELETE_GROWABLE_FUNCTION_TABLE)(
    _In_ PVOID DynamicTable
    );
typedef RTL_DELETE_GROWABLE_FUNCTION_TABLE *PRTL_DELETE_GROWABLE_FUNCTION_TABLE;

//
// Our remote function copying support.
//

typedef
_Check_return_
_Success_(return != 0)
BOOL
(CALLBACK ADJUST_THUNK_POINTERS)(
    _In_ PRTL Rtl,
    _In_ HANDLE TargetProcessHandle,
    _In_ PBYTE OriginalThunkBuffer,
    _In_ USHORT SizeOfThunkBufferInBytes,
    _Inout_bytecap_(BytesRemaining) PBYTE TemporaryLocalThunkBuffer,
    _In_ USHORT BytesRemaining,
    _In_ PBYTE RemoteThunkBufferAddress,
    _In_ PRUNTIME_FUNCTION RemoteRuntimeFunction,
    _In_ PVOID RemoteCodeBaseAddress,
    _In_ USHORT EntryCount,
    _Out_ PUSHORT NumberOfBytesWritten,
    _Out_ PBYTE *NewDestUserData,
    _Out_ PBYTE *NewRemoteDestUserData
    );
typedef ADJUST_THUNK_POINTERS *PADJUST_THUNK_POINTERS;

typedef
_Check_return_
_Success_(return != 0)
BOOL
(CALLBACK ADJUST_USER_DATA_POINTERS)(
    _In_ PRTL Rtl,
    _In_ HANDLE TargetProcessHandle,
    _In_ PBYTE OriginalDataBuffer,
    _In_ USHORT SizeOfDataBufferInBytes,
    _In_ PBYTE TemporaryLocalDataBuffer,
    _In_ USHORT BytesRemaining,
    _In_ PBYTE RemoteDataBufferAddress
    );
typedef ADJUST_USER_DATA_POINTERS *PADJUST_USER_DATA_POINTERS;

typedef
_Check_return_
_Success_(return != 0)
BOOL
(CALLBACK COPY_FUNCTION)(
    _In_ PRTL Rtl,
    _In_ PALLOCATOR Allocator,
    _In_ HANDLE TargetProcessHandle,
    _In_ PVOID SourceFunction,
    _In_opt_ PVOID SourceHandlerFunction,
    _In_reads_bytes_(SizeOfThunkBufferInBytes) PBYTE ThunkBuffer,
    _In_ USHORT SizeOfThunkBufferInBytes,
    _In_reads_bytes_(SizeOfUserDataInBytes) PBYTE UserData,
    _In_ USHORT SizeOfUserDataInBytes,
    _In_ PADJUST_THUNK_POINTERS AdjustThunkPointers,
    _In_ PADJUST_USER_DATA_POINTERS AdjustUserDataPointers,
    _Out_ PPVOID DestBaseCodeAddressPointer,
    _Out_ PPVOID DestThunkBufferAddressPointer,
    _Out_ PPVOID DestUserDataAddressPointer,
    _Out_ PPVOID DestFunctionPointer,
    _Out_opt_ PPVOID LocalBaseCodeAddressPointer,
    _Out_opt_ PPVOID LocalThunkBufferAddressPointer,
    _Out_opt_ PPVOID LocalUserDataAddressPointer,
    _Out_ PRUNTIME_FUNCTION *DestRuntimeFunctionPointer,
    _When_(SourceHandlerFunction != NULL, _Outptr_result_nullonfailure_)
    _When_(SourceHandlerFunction == NULL, _Out_opt_)
        PRUNTIME_FUNCTION *DestHandlerRuntimeFunctionPointer,
    _Out_ PULONG EntryCountPointer
    );
typedef COPY_FUNCTION *PCOPY_FUNCTION;

//
// Process and Thread support.
//

typedef enum _PROCESSINFOCLASS {
    ProcessBasicInformation = 0x0,
    ProcessQuotaLimits = 0x1,
    ProcessIoCounters = 0x2,
    ProcessVmCounters = 0x3,
    ProcessTimes = 0x4,
    ProcessBasePriority = 0x5,
    ProcessRaisePriority = 0x6,
    ProcessDebugPort = 0x7,
    ProcessExceptionPort = 0x8,
    ProcessAccessToken = 0x9,
    ProcessLdtInformation = 0xA,
    ProcessLdtSize = 0xB,
    ProcessDefaultHardErrorMode = 0xC,
    ProcessIoPortHandlers = 0xD,
    ProcessPooledUsageAndLimits = 0xE,
    ProcessWorkingSetWatch = 0xF,
    ProcessUserModeIOPL = 0x10,
    ProcessEnableAlignmentFaultFixup = 0x11,
    ProcessPriorityClass = 0x12,
    ProcessWx86Information = 0x13,
    ProcessHandleCount = 0x14,
    ProcessAffinityMask = 0x15,
    ProcessPriorityBoost = 0x16,
    ProcessDeviceMap = 0x17,
    ProcessSessionInformation = 0x18,
    ProcessForegroundInformation = 0x19,
    ProcessWow64Information = 0x1A,
    ProcessImageFileName = 0x1B,
    ProcessLUIDDeviceMapsEnabled = 0x1C,
    ProcessBreakOnTermination = 0x1D,
    ProcessDebugObjectHandle = 0x1E,
    ProcessDebugFlags = 0x1F,
    ProcessHandleTracing = 0x20,
    ProcessIoPriority = 0x21,
    ProcessExecuteFlags = 0x22,
    ProcessTlsInformation = 0x23,
    ProcessCookie = 0x24,
    ProcessImageInformation = 0x25,
    ProcessCycleTime = 0x26,
    ProcessPagePriority = 0x27,
    ProcessInstrumentationCallback = 0x28,
    ProcessThreadStackAllocation = 0x29,
    ProcessWorkingSetWatchEx = 0x2A,
    ProcessImageFileNameWin32 = 0x2B,
    ProcessImageFileMapping = 0x2C,
    ProcessAffinityUpdateMode = 0x2D,
    ProcessMemoryAllocationMode = 0x2E,
    ProcessGroupInformation = 0x2F,
    ProcessTokenVirtualizationEnabled = 0x30,
    ProcessConsoleHostProcess = 0x31,
    ProcessWindowInformation = 0x32,
    ProcessHandleInformation = 0x33,
    ProcessMitigationPolicy = 0x34,
    ProcessDynamicFunctionTableInformation = 0x35,
    ProcessHandleCheckingMode = 0x36,
    ProcessKeepAliveCount = 0x37,
    ProcessRevokeFileHandles = 0x38,
    ProcessWorkingSetControl = 0x39,
    MaxProcessInfoClass = 0x3A
} PROCESSINFOCLASS, *PPROCESSINFOCLASS;

typedef enum _THREADINFOCLASS {
    ThreadBasicInformation,
    ThreadTimes,
    ThreadPriority,
    ThreadBasePriority,
    ThreadAffinityMask,
    ThreadImpersonationToken,
    ThreadDescriptorTableEntry,
    ThreadEnableAlignmentFaultFixup,
    ThreadEventPair_Reusable,
    ThreadQuerySetWin32StartAddress,
    ThreadZeroTlsCell,
    ThreadPerformanceCount,
    ThreadAmILastThread,
    ThreadIdealProcessor,
    ThreadPriorityBoost,
    ThreadSetTlsArrayAddress,
    ThreadIsIoPending,
    ThreadHideFromDebugger,
    ThreadBreakOnTermination,
    ThreadSwitchLegacyState,
    ThreadIsTerminated,
    MaxThreadInfoClass
} THREADINFOCLASS, *PTHREADINFOCLASS;

typedef
NTSTATUS
(WINAPI NT_QUERY_INFORMATION_PROCESS)(
    _In_      HANDLE           ProcessHandle,
    _In_      PROCESSINFOCLASS ProcessInfoClass,
    _Out_     PVOID            ProcessInformation,
    _In_      ULONG            ProcessInformationLength,
    _Out_opt_ PULONG           ReturnLength
    );
typedef NT_QUERY_INFORMATION_PROCESS *PNT_QUERY_INFORMATION_PROCESS;
typedef NT_QUERY_INFORMATION_PROCESS   ZW_QUERY_INFORMATION_PROCESS;
typedef ZW_QUERY_INFORMATION_PROCESS *PZW_QUERY_INFORMATION_PROCESS;

typedef struct _PEB_LDR_DATA {
    ULONG Length;
    BOOLEAN Initialized;
    HANDLE SsHandle;
    LIST_ENTRY InLoadOrderModuleList;
    LIST_ENTRY InMemoryOrderModuleList;
    LIST_ENTRY InInitializationOrderModuleList;
    PVOID EntryInProgress;
} PEB_LDR_DATA, *PPEB_LDR_DATA;

typedef struct _LDR_DATA_TABLE_ENTRY {
    LIST_ENTRY InLoadOrderModuleList;
    LIST_ENTRY InMemoryOrderLinks;
    LIST_ENTRY InInitializationOrderModuleList;
    PVOID DllBase;
    PVOID EntryPoint;
    union {
        ULONG SizeOfImage;
        PVOID _Padding;
    };
    UNICODE_STRING FullDllName;
    UNICODE_STRING BaseDllName;
    ULONG Flags;
    USHORT LoadCount;
    USHORT TlsIndex;
    union {
        LIST_ENTRY HashLinks;
        struct {
            PVOID SectionPointer;
            ULONG CheckSum;
        };
    };
    union {
        struct {
            ULONG TimeDateStamp;
        };
        struct {
            PVOID LoadedImports;
        };
    };
} LDR_DATA_TABLE_ENTRY, *PLDR_DATA_TABLE_ENTRY;

#ifndef LDR_DLL_NOTIFICATION_REASON_LOADED
#define LDR_DLL_NOTIFICATION_REASON_LOADED 1
#endif

#ifndef LDR_DLL_NOTIFICATION_REASON_UNLOADED
#define LDR_DLL_NOTIFICATION_REASON_UNLOADED 2
#endif

typedef struct _LDR_DLL_LOADED_NOTIFICATION_DATA {
    ULONG Flags;
    PCUNICODE_STRING FullDllName;
    PCUNICODE_STRING BaseDllName;
    PVOID DllBase;
    ULONG SizeOfImage;
} LDR_DLL_LOADED_NOTIFICATION_DATA, *PLDR_DLL_LOADED_NOTIFICATION_DATA;

typedef struct _LDR_DLL_UNLOADED_NOTIFICATION_DATA {
    ULONG Flags;
    PCUNICODE_STRING FullDllName;
    PCUNICODE_STRING BaseDllName;
    PVOID DllBase;
    ULONG SizeOfImage;
} LDR_DLL_UNLOADED_NOTIFICATION_DATA, *PLDR_DLL_UNLOADED_NOTIFICATION_DATA;

typedef union _LDR_DLL_NOTIFICATION_DATA {
    LDR_DLL_LOADED_NOTIFICATION_DATA Loaded;
    LDR_DLL_UNLOADED_NOTIFICATION_DATA Unloaded;
} LDR_DLL_NOTIFICATION_DATA, *PLDR_DLL_NOTIFICATION_DATA;
typedef const LDR_DLL_NOTIFICATION_DATA *PCLDR_DLL_NOTIFICATION_DATA;

typedef
VOID
(CALLBACK LDR_DLL_NOTIFICATION_FUNCTION)(
    _In_     ULONG                       NotificationReason,
    _In_     PCLDR_DLL_NOTIFICATION_DATA NotificationData,
    _In_opt_ PVOID                       Context
    );
typedef LDR_DLL_NOTIFICATION_FUNCTION *PLDR_DLL_NOTIFICATION_FUNCTION;

typedef
NTSTATUS
(NTAPI LDR_REGISTER_DLL_NOTIFICATION)(
    _In_     ULONG                          Flags,
    _In_     PLDR_DLL_NOTIFICATION_FUNCTION NotificationFunction,
    _In_opt_ PVOID                          Context,
    _Out_    PVOID                          *Cookie
    );
typedef LDR_REGISTER_DLL_NOTIFICATION *PLDR_REGISTER_DLL_NOTIFICATION;

typedef
NTSTATUS
(NTAPI LDR_UNREGISTER_DLL_NOTIFICATION)(
    _In_ PVOID Cookie
    );
typedef LDR_UNREGISTER_DLL_NOTIFICATION *PLDR_UNREGISTER_DLL_NOTIFICATION;

#define LDR_LOCK_LOADER_LOCK_FLAG_RAISE_ON_ERRORS (0x00000001)
#define LDR_LOCK_LOADER_LOCK_FLAG_TRY_ONLY (0x00000002)

#define LDR_LOCK_LOADER_LOCK_DISPOSITION_INVALID (0)
#define LDR_LOCK_LOADER_LOCK_DISPOSITION_LOCK_ACQUIRED (1)
#define LDR_LOCK_LOADER_LOCK_DISPOSITION_LOCK_NOT_ACQUIRED (2)

typedef
NTSTATUS
(NTAPI LDR_LOCK_LOADER_LOCK)(
    _In_ ULONG Flags,
    _Inout_opt_ PULONG Disposition,
    _Out_ PVOID *Cookie
    );
typedef LDR_LOCK_LOADER_LOCK *PLDR_LOCK_LOADER_LOCK;

#define LDR_UNLOCK_LOADER_LOCK_FLAG_RAISE_ON_ERRORS (0x00000001)

typedef
NTSTATUS
(NTAPI LDR_UNLOCK_LOADER_LOCK)(
    _In_ ULONG Flags,
    _Inout_ PVOID Cookie
    );
typedef LDR_UNLOCK_LOADER_LOCK *PLDR_UNLOCK_LOADER_LOCK;

typedef struct _RTL_USER_PROCESS_PARAMETERS {
    BYTE           Reserved1[16];
    PVOID          Reserved2[10];
    UNICODE_STRING ImagePathName;
    UNICODE_STRING CommandLine;
} RTL_USER_PROCESS_PARAMETERS, *PRTL_USER_PROCESS_PARAMETERS;

#define FLS_MAXIMUM_AVAILABLE 128
#define TLS_MINIMUM_AVAILABLE 64
#define TLS_EXPANSION_SLOTS   1024

#define GDI_HANDLE_BUFFER_SIZE  60
typedef ULONG GDI_HANDLE_BUFFER[GDI_HANDLE_BUFFER_SIZE];

typedef struct _PEB_FREE_BLOCK {
    struct _PEB_FREE_BLOCK *Next;
    ULONG Size;
} PEB_FREE_BLOCK, *PPEB_FREE_BLOCK;

typedef
VOID
(PS_POST_PROCESS_INIT_ROUTINE)(
    VOID
    );
typedef PS_POST_PROCESS_INIT_ROUTINE *PPS_POST_PROCESS_INIT_ROUTINE;

typedef struct _PEB {
    BOOLEAN InheritedAddressSpace;
    BOOLEAN ReadImageFileExecOptions;
    BOOLEAN BeingDebugged;
    union {
        BOOLEAN BitField;
        struct {
            BOOLEAN ImageUsesLargePages:1;
            BOOLEAN SpareBits:7;
         };
    };
    HANDLE Mutant;
    PVOID ImageBaseAddress;
    PPEB_LDR_DATA Ldr;
    PRTL_USER_PROCESS_PARAMETERS ProcessParameters;
    PVOID SubSystemData;
    PVOID ProcessHeap;
    PCRITICAL_SECTION FastPebLock;
    PVOID AtlThunkSListPtr;
    PVOID SparePtr2;
    ULONG EnvironmentUpdateCount;
    PVOID KernelCallbackTable;
    ULONG SystemReserved[1];
    ULONG SpareUlong;
    PPEB_FREE_BLOCK FreeList;
    ULONG TlsExpansionCounter;
    PVOID TlsBitmap;
    ULONG TlsBitmapBits[2];
    PVOID ReadOnlySharedMemoryBase;
    PVOID ReadOnlySharedMemoryHeap;
    PPVOID ReadOnlyStaticServerData;
    PVOID AnsiCodePageData;
    PVOID OemCodePageData;
    PVOID UnicodeCaseTableData;
    ULONG NumberOfProcessors;
    ULONG NtGlobalFlag;
    LARGE_INTEGER CriticalSectionTimeout;
    SIZE_T HeapSegmentReserve;
    SIZE_T HeapSegmentCommit;
    SIZE_T HeapDeCommitTotalFreeThreshold;
    SIZE_T HeapDeCommitFreeBlockThreshold;
    ULONG NumberOfHeaps;
    ULONG MaximumNumberOfHeaps;
    PPVOID ProcessHeaps;
    PVOID GdiSharedHandleTable;
    PVOID ProcessStarterHelper;
    ULONG GdiDCAttributeList;
    PCRITICAL_SECTION LoaderLock;
    ULONG OSMajorVersion;
    ULONG OSMinorVersion;
    USHORT OSBuildNumber;
    USHORT OSCSDVersion;
    ULONG OSPlatformId;
    ULONG ImageSubsystem;
    ULONG ImageSubsystemMajorVersion;
    ULONG ImageSubsystemMinorVersion;
    ULONG_PTR ImageProcessAffinityMask;
    GDI_HANDLE_BUFFER GdiHandleBuffer;
    PPS_POST_PROCESS_INIT_ROUTINE PostProcessInitRoutine;
    PVOID TlsExpansionBitmap;
    ULONG TlsExpansionBitmapBits[32];
    ULONG SessionId;
    ULARGE_INTEGER AppCompatFlags;
    ULARGE_INTEGER AppCompatFlagsUser;
    PVOID pShimData;
    PVOID AppCompatInfo;
    UNICODE_STRING CSDVersion;
    const struct _ACTIVATION_CONTEXT_DATA *ActivationContextData;
    struct _ASSEMBLY_STORAGE_MAP *ProcessAssemblyStorageMap;
    const struct _ACTIVATION_CONTEXT_DATA *SystemDefaultActivationContextData;
    struct _ASSEMBLY_STORAGE_MAP *SystemAssemblyStorageMap;
    SIZE_T MinimumStackCommit;
    PPVOID FlsCallback;
    LIST_ENTRY FlsListHead;
    PVOID FlsBitmap;
    ULONG FlsBitmapBits[FLS_MAXIMUM_AVAILABLE / (sizeof(ULONG) * 8)];
    ULONG FlsHighIndex;
} PEB, *PPEB;

typedef struct _ACTIVATION_CONTEXT_STACK {
    struct _RTL_ACTIVATION_CONTEXT_STACK_FRAME *ActiveFrame;
    LIST_ENTRY FrameListCache;
    ULONG Flags;
    ULONG NextCookieSequenceNumber;
    ULONG StackId;
} ACTIVATION_CONTEXT_STACK, * PACTIVATION_CONTEXT_STACK;

typedef const ACTIVATION_CONTEXT_STACK *PCACTIVATION_CONTEXT_STACK;

typedef struct _TEB_ACTIVE_FRAME_CONTEXT {
    ULONG Flags;
    PCSTR FrameName;
} TEB_ACTIVE_FRAME_CONTEXT, *PTEB_ACTIVE_FRAME_CONTEXT;

typedef const TEB_ACTIVE_FRAME_CONTEXT *PCTEB_ACTIVE_FRAME_CONTEXT;

typedef struct _TEB_ACTIVE_FRAME_CONTEXT_EX {
    TEB_ACTIVE_FRAME_CONTEXT BasicContext;
    PCSTR SourceLocation;
} TEB_ACTIVE_FRAME_CONTEXT_EX, *PTEB_ACTIVE_FRAME_CONTEXT_EX;

typedef const TEB_ACTIVE_FRAME_CONTEXT_EX *PCTEB_ACTIVE_FRAME_CONTEXT_EX;

typedef struct _TEB_ACTIVE_FRAME {
    ULONG Flags;
    struct _TEB_ACTIVE_FRAME *Previous;
    PCTEB_ACTIVE_FRAME_CONTEXT Context;
} TEB_ACTIVE_FRAME, *PTEB_ACTIVE_FRAME;

typedef const TEB_ACTIVE_FRAME *PCTEB_ACTIVE_FRAME;

typedef struct _TEB_ACTIVE_FRAME_EX {
    TEB_ACTIVE_FRAME BasicFrame;
    PVOID ExtensionIdentifier;
} TEB_ACTIVE_FRAME_EX, *PTEB_ACTIVE_FRAME_EX;

typedef const TEB_ACTIVE_FRAME_EX *PCTEB_ACTIVE_FRAME_EX;

typedef struct _CLIENT_ID {
    HANDLE ProcessId;
    HANDLE ThreadId;
} CLIENT_ID, *PCLIENT_ID;

#define WIN32_CLIENT_INFO_LENGTH 62
#define STATIC_UNICODE_BUFFER_LENGTH 261

#define GDI_BATCH_BUFFER_SIZE 310

typedef struct _GDI_TEB_BATCH {
    ULONG     Offset;
    ULONG_PTR HDC;
    ULONG     Buffer[GDI_BATCH_BUFFER_SIZE];
} GDI_TEB_BATCH,*PGDI_TEB_BATCH;

typedef struct _TEB {
    NT_TIB NtTib;
    PVOID EnvironmentPointer;
    CLIENT_ID ClientId;
    PVOID ActiveRpcHandle;
    PVOID ThreadLocalStoragePointer;
    PPEB ProcessEnvironmentBlock;
    ULONG LastErrorValue;
    ULONG CountOfOwnedCriticalSections;
    PVOID CsrClientThread;
    PVOID Win32ThreadInfo;
    ULONG User32Reserved[26];
    ULONG UserReserved[5];
    PVOID WOW32Reserved;
    LCID CurrentLocale;
    ULONG FpSoftwareStatusRegister;
    PVOID SystemReserved1[54];
    NTSTATUS ExceptionCode;
    PACTIVATION_CONTEXT_STACK ActivationContextStackPointer;
    UCHAR SpareBytes1[28];
    GDI_TEB_BATCH GdiTebBatch;
    CLIENT_ID RealClientId;
    HANDLE GdiCachedProcessHandle;
    ULONG GdiClientPID;
    ULONG GdiClientTID;
    PVOID GdiThreadLocalInfo;
    ULONG_PTR Win32ClientInfo[WIN32_CLIENT_INFO_LENGTH];
    PVOID glDispatchTable[233];
    ULONG_PTR glReserved1[29];
    PVOID glReserved2;
    PVOID glSectionInfo;
    PVOID glSection;
    PVOID glTable;
    PVOID glCurrentRC;
    PVOID glContext;
    ULONG LastStatusValue;
    UNICODE_STRING StaticUnicodeString;
    WCHAR StaticUnicodeBuffer[STATIC_UNICODE_BUFFER_LENGTH];
    PVOID DeallocationStack;
    PVOID TlsSlots[TLS_MINIMUM_AVAILABLE];
    LIST_ENTRY TlsLinks;
    PVOID Vdm;
    PVOID ReservedForNtRpc;
    PVOID DbgSsReserved[2];
    ULONG HardErrorMode;
    PVOID Instrumentation[14];
    PVOID SubProcessTag;
    PVOID EtwTraceData;
    PVOID WinSockData;
    ULONG GdiBatchCount;
    BOOLEAN InDbgPrint;
    BOOLEAN FreeStackOnTermination;
    BOOLEAN HasFiberData;
    BOOLEAN IdealProcessor;
    ULONG GuaranteedStackBytes;
    PVOID ReservedForPerf;
    PVOID ReservedForOle;
    ULONG WaitingOnLoaderLock;
    ULONG_PTR SparePointer1;
    ULONG_PTR SoftPatchPtr1;
    ULONG_PTR SoftPatchPtr2;
    PPVOID TlsExpansionSlots;
    PVOID DeallocationBStore;
    PVOID BStoreLimit;
    LCID ImpersonationLocale;
    ULONG IsImpersonating;
    PVOID NlsCache;
    PVOID pShimData;
    ULONG HeapVirtualAffinity;
    HANDLE CurrentTransactionHandle;
    PTEB_ACTIVE_FRAME ActiveFrame;
    PVOID FlsData;
    BOOLEAN SafeThunkCall;
    BOOLEAN BooleanSpare[3];
} TEB, *PTEB;

typedef struct _INITIAL_TEB {
    struct {
        PVOID OldStackBase;
        PVOID OldStackLimit;
    } OldInitialTeb;
    PVOID StackBase;
    PVOID StackLimit;
    PVOID StackAllocationBase;
} INITIAL_TEB, *PINITIAL_TEB;

typedef LONG KPRIORITY;

#define NtCurrentPeb() (NtCurrentTeb()->ProcessEnvironmentBlock)

typedef struct _THREAD_BASIC_INFORMATION {
    NTSTATUS ExitStatus;
    PTEB TebBaseAddress;
    CLIENT_ID ClientId;
    ULONG_PTR AffinityMask;
    KPRIORITY Priority;
    LONG BasePriority;
} THREAD_BASIC_INFORMATION;
typedef THREAD_BASIC_INFORMATION *PTHREAD_BASIC_INFORMATION;

typedef struct _PROCESS_BASIC_INFORMATION {
    NTSTATUS ExitStatus;
    PPEB PebBaseAddress;
    ULONG_PTR AffinityMask;
    KPRIORITY BasePriority;
    ULONG_PTR UniqueProcessId;
    ULONG_PTR InheritedFromUniqueProcessId;
} PROCESS_BASIC_INFORMATION;

typedef struct _OBJECT_ATTRIBUTES {
    ULONG Length;
    HANDLE RootDirectory;
    PUNICODE_STRING ObjectName;
    ULONG Attributes;
    PVOID SecurityDescriptor;
    PVOID SecurityQualityOfService;
} OBJECT_ATTRIBUTES, *POBJECT_ATTRIBUTES;

typedef enum _SECTION_INHERIT {
    ViewShare = 1,
    ViewUnmap = 2
} SECTION_INHERIT, *PSECTION_INHERIT;

typedef
NTSTATUS
(NTAPI NT_CREATE_SECTION)(
    _Out_    PHANDLE            SectionHandle,
    _In_     ACCESS_MASK        DesiredAccess,
    _In_opt_ POBJECT_ATTRIBUTES ObjectAttributes,
    _In_opt_ PLARGE_INTEGER     MaximumSize,
    _In_     ULONG              SectionPageProtection,
    _In_     ULONG              AllocationAttributes,
    _In_opt_ HANDLE             FileHandle
    );
typedef NT_CREATE_SECTION *PNT_CREATE_SECTION;
typedef NT_CREATE_SECTION   ZW_CREATE_SECTION;
typedef ZW_CREATE_SECTION *PZW_CREATE_SECTION;

typedef
NTSTATUS
(NTAPI NT_MAP_VIEW_OF_SECTION)(
    _In_        HANDLE          SectionHandle,
    _In_        HANDLE          ProcessHandle,
    _Inout_     PVOID           *BaseAddress,
    _In_        ULONG_PTR       ZeroBits,
    _In_        SIZE_T          CommitSize,
    _Inout_opt_ PLARGE_INTEGER  SectionOffset,
    _Inout_     PSIZE_T         ViewSize,
    _In_        SECTION_INHERIT InheritDisposition,
    _In_        ULONG           AllocationType,
    _In_        ULONG           Win32Protect
    );
typedef NT_MAP_VIEW_OF_SECTION *PNT_MAP_VIEW_OF_SECTION;
typedef NT_MAP_VIEW_OF_SECTION   ZW_MAP_VIEW_OF_SECTION;
typedef ZW_MAP_VIEW_OF_SECTION *PZW_MAP_VIEW_OF_SECTION;

typedef
NTSTATUS
(NTAPI NT_UNMAP_VIEW_OF_SECTION)(
    _In_        HANDLE  ProcessHandle,
    _In_opt_    PVOID   BaseAddress
    );
typedef NT_UNMAP_VIEW_OF_SECTION *PNT_UNMAP_VIEW_OF_SECTION;
typedef NT_UNMAP_VIEW_OF_SECTION   ZW_UNMAP_VIEW_OF_SECTION;
typedef ZW_UNMAP_VIEW_OF_SECTION *PZW_UNMAP_VIEW_OF_SECTION;

typedef
NTSTATUS
(NTAPI NT_CREATE_PROCESS)(
    _Out_ PHANDLE ProcessHandle,
    _In_ ACCESS_MASK DesiredAccess,
    _In_opt_ POBJECT_ATTRIBUTES ObjectAttributes,
    _In_ HANDLE ParentProcess,
    _In_ BOOLEAN InheritObjectTable,
    _In_opt_ HANDLE SectionHandle,
    _In_opt_ HANDLE DebugPort,
    _In_opt_ HANDLE ExceptionPort
    );
typedef NT_CREATE_PROCESS *PNT_CREATE_PROCESS;
typedef NT_CREATE_PROCESS   ZW_CREATE_PROCESS;
typedef ZW_CREATE_PROCESS *PZW_CREATE_PROCESS;

typedef
NTSTATUS
(NTAPI NT_CREATE_PROCESS_EX)(
    _Out_ PHANDLE ProcessHandle,
    _In_ ACCESS_MASK DesiredAccess,
    _In_opt_ POBJECT_ATTRIBUTES ObjectAttributes,
    _In_ HANDLE ParentProcess,
    _In_ BOOLEAN InheritObjectTable,
    _In_opt_ HANDLE SectionHandle,
    _In_opt_ HANDLE DebugPort,
    _In_opt_ HANDLE ExceptionPort,
    _In_ ULONG JobMemberLevel
    );
typedef NT_CREATE_PROCESS_EX *PNT_CREATE_PROCESS_EX;
typedef NT_CREATE_PROCESS_EX   ZW_CREATE_PROCESS_EX;
typedef ZW_CREATE_PROCESS_EX *PZW_CREATE_PROCESS_EX;

typedef
NTSTATUS
(NTAPI NT_CREATE_THREAD)(
    _Out_ PHANDLE ThreadHandle,
    _In_ ACCESS_MASK DesiredAccess,
    _In_opt_ POBJECT_ATTRIBUTES ObjectAttributes,
    _In_ HANDLE ProcessHandle,
    _Out_ PCLIENT_ID ClientId,
    _In_ PCONTEXT ThreadContext,
    _In_ PINITIAL_TEB InitialTeb,
    _In_ BOOLEAN CreateSuspended
    );
typedef NT_CREATE_THREAD *PNT_CREATE_THREAD;
typedef NT_CREATE_THREAD   ZW_CREATE_THREAD;
typedef ZW_CREATE_THREAD *PZW_CREATE_THREAD;

typedef
NTSTATUS
(NTAPI NT_OPEN_THREAD)(
    _Out_ PHANDLE ThreadHandle,
    _In_ ACCESS_MASK DesiredAccess,
    _In_ POBJECT_ATTRIBUTES ObjectAttributes,
    _In_opt_ PCLIENT_ID ClientId
    );
typedef NT_OPEN_THREAD *PNT_OPEN_THREAD;
typedef NT_OPEN_THREAD   ZW_OPEN_THREAD;
typedef ZW_OPEN_THREAD *PZW_OPEN_THREAD;

typedef
NTSTATUS
(NTAPI NT_TERMINATE_THREAD)(
    _In_opt_ HANDLE ThreadHandle,
    _In_ NTSTATUS ExitStatus
    );
typedef NT_TERMINATE_THREAD *PNT_TERMINATE_THREAD;
typedef NT_TERMINATE_THREAD   ZW_TERMINATE_THREAD;
typedef ZW_TERMINATE_THREAD *PZW_TERMINATE_THREAD;

typedef
NTSTATUS
(NTAPI NT_YIELD_EXECUTION)(
    VOID
    );
typedef NT_YIELD_EXECUTION *PNT_YIELD_EXECUTION;

typedef
NTSTATUS
(NTAPI NT_DELAY_EXECUTION)(
    _In_ BOOLEAN Alertable,
    _In_ PLARGE_INTEGER DelayInterval
    );
typedef NT_DELAY_EXECUTION *PNT_DELAY_EXECUTION;

//
// SystemTime-related functions.
//

typedef VOID (WINAPI *PGETSYSTEMTIMEPRECISEASFILETIME)(
    _Out_ LPFILETIME lpSystemTimeAsFileTime
);

typedef NTSTATUS (WINAPI *PNTQUERYSYSTEMTIME)(
    _Out_ PLARGE_INTEGER SystemTime
    );

typedef BOOL (*PGETSYSTEMTIMEPRECISEASLARGEINTEGER)(
    _Out_   PLARGE_INTEGER  SystemTime
);

#define _SYSTEM_TIMER_FUNCTIONS_HEAD                                \
    PGETSYSTEMTIMEPRECISEASFILETIME GetSystemTimePreciseAsFileTime; \
    PNTQUERYSYSTEMTIME NtQuerySystemTime;

FORCEINLINE
VOID
CopySystemTime(
    _In_ PSYSTEMTIME Dest,
    _In_ PSYSTEMTIME Source
    )
{
    __movsq((PULONGLONG)Dest, (PULONGLONG)Source, sizeof(*Dest) >> 3);
}

typedef struct _SYSTEM_TIMER_FUNCTION {
    _SYSTEM_TIMER_FUNCTIONS_HEAD
} SYSTEM_TIMER_FUNCTION, *PSYSTEM_TIMER_FUNCTION, **PPSYSTEM_TIMER_FUNCTION;

BOOL
CallSystemTimer(
    _Out_       PFILETIME               SystemTime,
    _Inout_opt_ PPSYSTEM_TIMER_FUNCTION ppSystemTimerFunction
);

typedef
NTSTATUS
(WINAPI RTL_CHAR_TO_INTEGER)(
    _In_ PCSZ String,
    _In_opt_ ULONG Base,
    _Out_ PULONG Value
    );
typedef RTL_CHAR_TO_INTEGER *PRTL_CHAR_TO_INTEGER;

typedef
DWORD
(WINAPI SEARCHPATHW)(
    _In_opt_    LPCWSTR     lpPath,
    _In_        LPCWSTR     lpFileName,
    _In_opt_    LPCWSTR     lpExtension,
    _In_        DWORD       nBufferLength,
    _Out_       LPWSTR      lpBuffer,
    _Out_opt_   LPWSTR      lpFilePart
    );
typedef SEARCHPATHW *PSEARCHPATHW;

//
// CRT functions.
//

typedef
INT
(__cdecl CRTCOMPARE)(
    _In_    CONST PVOID Key,
    _In_    CONST PVOID Datum
    );
typedef CRTCOMPARE *PCRTCOMPARE;

typedef
PVOID
(BSEARCH)(
    _In_ CPVOID      Key,
    _In_ CPVOID      Base,
    _In_ SIZE_T      NumberOfElements,
    _In_ SIZE_T      WidthOfElement,
    _In_ PCRTCOMPARE Compare
    );
typedef BSEARCH *PBSEARCH;

typedef
VOID
(QSORT)(
    _In_ PVOID       Base,
    _In_ SIZE_T      NumberOfElements,
    _In_ SIZE_T      WidthOfElement,
    _In_ PCRTCOMPARE Compare
    );
typedef QSORT *PQSORT;

//
// atexit-related functions and structures.
//

typedef
VOID
(__cdecl ATEXITFUNC)(
    VOID
    );
typedef ATEXITFUNC *PATEXITFUNC;

typedef
_Success_(return == 0)
INT
(ATEXIT)(
    _In_ PATEXITFUNC AtExitFunc
    );
typedef ATEXIT *PATEXIT;

typedef
VOID
(SET_ATEXIT)(
    _In_ PATEXIT AtExit
    );
typedef SET_ATEXIT *PSET_ATEXIT;

//
// Our extended version of CRT atexit() functionality.  Callers can indicate
// if they wish to suppress exceptions during callback, and additionally provide
// a context parameter passed to the callback function when invoked.
//

typedef struct _Struct_size_bytes_(sizeof(ULONG)) _ATEXITEX_FLAGS {

    //
    // When set, indicates that the caller's atexit function will be wrapped
    // in a __try/__except SEH block that suppresses all exceptions.
    //

    ULONG SuppressExceptions:1;

} ATEXITEX_FLAGS, *PATEXITEX_FLAGS;
C_ASSERT(sizeof(ATEXITEX_FLAGS) == sizeof(ULONG));

typedef
VOID
(CALLBACK ATEXITEX_CALLBACK)(
    _In_ BOOL IsProcessTerminating,
    _In_opt_ PVOID Context
    );
typedef ATEXITEX_CALLBACK *PATEXITEX_CALLBACK;

typedef
_Success_(return == 0)
BOOL
(ATEXITEX)(
    _In_ PATEXITEX_CALLBACK Callback,
    _In_opt_ PATEXITEX_FLAGS Flags,
    _In_opt_ PVOID Context,
    _Outptr_opt_result_nullonfailure_ struct _RTL_ATEXIT_ENTRY **EntryPointer
    );
typedef ATEXITEX *PATEXITEX;

typedef
VOID
(SET_ATEXITEX)(
    _In_ PATEXITEX AtExitEx
    );
typedef SET_ATEXITEX *PSET_ATEXITEX;

typedef
_Success_(return != 0)
_Check_return_
BOOL
(UNREGISTER_RTL_ATEXIT_ENTRY)(
    _In_ _Post_invalid_ struct _RTL_ATEXIT_ENTRY *Entry
    );
typedef UNREGISTER_RTL_ATEXIT_ENTRY *PUNREGISTER_RTL_ATEXIT_ENTRY;

//
// End of atexit-related functionality.
//

typedef
PVOID
(MEMSET)(
    _In_ PVOID Destination,
    _In_ INT Character,
    _In_ SIZE_T Count
    );
typedef MEMSET *PMEMSET;

typedef
PWCHAR
(WMEMSET)(
    _In_ PWCHAR Destination,
    _In_ WCHAR Character,
    _In_ SIZE_T Count
    );
typedef WMEMSET *PWMEMSET;

//
// End of CRT functions.
//

//
// Injection-related typedefs.
//

typedef
_Ret_maybenull_
HMODULE
(WINAPI LOAD_LIBRARY_A)(
    _In_ LPCSTR lpLibFileName
    );
typedef LOAD_LIBRARY_A *PLOAD_LIBRARY_A;

typedef
_Ret_maybenull_
HMODULE
(WINAPI LOAD_LIBRARY_W)(
    _In_ LPWSTR lpLibFileName
    );
typedef LOAD_LIBRARY_W *PLOAD_LIBRARY_W;

typedef
_Ret_maybenull_
HMODULE
(WINAPI LOAD_LIBRARY_EX_W)(
    _In_ LPWSTR lpLibFileName,
    _Reserved_ HANDLE hFile,
    _In_ DWORD Flags
    );
typedef LOAD_LIBRARY_EX_W *PLOAD_LIBRARY_EX_W;

typedef
FARPROC
(WINAPI GET_PROC_ADDRESS)(
    _In_ HMODULE hModule,
    _In_ LPCSTR lpProcName
    );
typedef GET_PROC_ADDRESS *PGET_PROC_ADDRESS;

typedef
_Ret_maybenull_
HANDLE
(WINAPI OPEN_EVENT_A)(
    _In_ DWORD dwDesiredAccess,
    _In_ BOOL bInheritHandle,
    _In_ LPCSTR lpName
    );
typedef OPEN_EVENT_A *POPEN_EVENT_A;

typedef
_Ret_maybenull_
HANDLE
(WINAPI OPEN_EVENT_W)(
    _In_ DWORD dwDesiredAccess,
    _In_ BOOL bInheritHandle,
    _In_ LPCWSTR lpName
    );
typedef OPEN_EVENT_W *POPEN_EVENT_W;

typedef
_Ret_maybenull_
HANDLE
(WINAPI CREATE_EVENT_A)(
    _In_opt_ LPSECURITY_ATTRIBUTES lpEventAttributes,
    _In_ BOOL bManualReset,
    _In_ BOOL bInitialState,
    _In_opt_ LPCSTR lpName
    );
typedef CREATE_EVENT_A *PCREATE_EVENT_A;

typedef
_Ret_maybenull_
HANDLE
(WINAPI CREATE_EVENT_W)(
    _In_opt_ LPSECURITY_ATTRIBUTES lpEventAttributes,
    _In_ BOOL bManualReset,
    _In_ BOOL bInitialState,
    _In_opt_ LPCWSTR lpName
    );
typedef CREATE_EVENT_W *PCREATE_EVENT_W;

typedef
DWORD
(WINAPI WAIT_FOR_INPUT_IDLE)(
    _In_ HANDLE hProcess,
    _In_ DWORD dwMilliseconds);
typedef WAIT_FOR_INPUT_IDLE *PWAIT_FOR_INPUT_IDLE;

typedef
_Check_return_
_Ret_maybenull_
_Post_writable_byte_size_(dwSize)
LPVOID
(WINAPI VIRTUAL_ALLOC)(
    _In_opt_ LPVOID lpAddress,
    _In_     SIZE_T dwSize,
    _In_     DWORD  flAllocationType,
    _In_     DWORD  flProtect
    );
typedef VIRTUAL_ALLOC *PVIRTUAL_ALLOC;

typedef
_Check_return_
_Ret_maybenull_
_Post_writable_byte_size_(dwSize)
LPVOID
(WINAPI VIRTUAL_ALLOC_EX)(
    _In_     HANDLE hProcess,
    _In_opt_ LPVOID lpAddress,
    _In_     SIZE_T dwSize,
    _In_     DWORD  flAllocationType,
    _In_     DWORD  flProtect
    );
typedef VIRTUAL_ALLOC_EX *PVIRTUAL_ALLOC_EX;

typedef
BOOL
(WINAPI VIRTUAL_FREE)(

    _Pre_notnull_
        _When_(dwFreeType == MEM_DECOMMIT, _Post_invalid_)
        _When_(dwFreeType == MEM_RELEASE, _Post_ptr_invalid_)
            LPVOID lpAddress,

    _In_ SIZE_T dwSize,
    _In_ DWORD dwFreeType
    );
typedef VIRTUAL_FREE *PVIRTUAL_FREE;

typedef
BOOL
(WINAPI VIRTUAL_FREE_EX)(
    _In_ HANDLE hProcess,

    _Pre_notnull_
        _When_(dwFreeType == MEM_DECOMMIT, _Post_invalid_)
        _When_(dwFreeType == MEM_RELEASE, _Post_ptr_invalid_)
            LPVOID lpAddress,

    _In_ SIZE_T dwSize,
    _In_ DWORD dwFreeType
    );
typedef VIRTUAL_FREE_EX *PVIRTUAL_FREE_EX;

typedef
_Success_(return != FALSE)
BOOL
(WINAPI VIRTUAL_PROTECT)(
    _In_ LPVOID lpAddress,
    _In_ SIZE_T dwSize,
    _In_ DWORD flNewProtect,
    _Out_ PDWORD lpflOldProtect
    );
typedef VIRTUAL_PROTECT *PVIRTUAL_PROTECT;

typedef
_Success_(return != FALSE)
BOOL
(WINAPI VIRTUAL_PROTECT_EX)(
    _In_ HANDLE hProcess,
    _In_ LPVOID lpAddress,
    _In_ SIZE_T dwSize,
    _In_ DWORD flNewProtect,
    _Out_ PDWORD lpflOldProtect
    );
typedef VIRTUAL_PROTECT_EX *PVIRTUAL_PROTECT_EX;

typedef
SIZE_T
(WINAPI VIRTUAL_QUERY_EX)(
    _In_ HANDLE hProcess,
    _In_opt_ LPCVOID lpAddress,
    _Out_writes_bytes_to_(dwLength, return) PMEMORY_BASIC_INFORMATION lpBuffer,
    _In_ SIZE_T dwLength
    );
typedef VIRTUAL_QUERY_EX *PVIRTUAL_QUERY_EX;

typedef
_Success_(return != FALSE)
BOOL
(WINAPI READ_PROCESS_MEMORY)(
    _In_ HANDLE hProcess,
    _In_ LPCVOID lpBaseAddress,
    _Out_writes_bytes_to_(nSize, *lpNumberOfBytesRead) LPVOID lpBuffer,
    _In_ SIZE_T nSize,
    _Out_opt_ SIZE_T * lpNumberOfBytesRead
    );
typedef READ_PROCESS_MEMORY *PREAD_PROCESS_MEMORY;

typedef
_Success_(return != FALSE)
BOOL
(WINAPI WRITE_PROCESS_MEMORY)(
    _In_ HANDLE hProcess,
    _In_ LPVOID lpBaseAddress,
    _In_reads_bytes_(nSize) LPCVOID lpBuffer,
    _In_ SIZE_T nSize,
    _Out_opt_ SIZE_T * lpNumberOfBytesWritten
    );
typedef WRITE_PROCESS_MEMORY *PWRITE_PROCESS_MEMORY;

typedef
BOOL
(WINAPI DEVICE_IO_CONTROL)(
    _In_ HANDLE hDevice,
    _In_ DWORD dwIoControlCode,
    _In_reads_bytes_opt_(nInBufferSize) LPVOID lpInBuffer,
    _In_ DWORD nInBufferSize,
    _Out_writes_bytes_to_opt_(nOutBufferSize, *lpBytesReturned)
        LPVOID lpOutBuffer,
    _In_ DWORD nOutBufferSize,
    _Out_opt_ LPDWORD lpBytesReturned,
    _Inout_opt_ LPOVERLAPPED lpOverlapped
    );
typedef DEVICE_IO_CONTROL *PDEVICE_IO_CONTROL;

typedef
_When_(lpModuleName == NULL, _Ret_notnull_)
_When_(lpModuleName != NULL, _Ret_maybenull_)
HMODULE
(WINAPI GET_MODULE_HANDLE_A)(
    _In_opt_ LPCSTR lpModuleName
    );
typedef GET_MODULE_HANDLE_A *PGET_MODULE_HANDLE_A;

typedef
_When_(lpModuleName == NULL, _Ret_notnull_)
_When_(lpModuleName != NULL, _Ret_maybenull_)
HMODULE
(WINAPI GET_MODULE_HANDLE_W)(
    _In_opt_ LPCWSTR lpModuleName
    );
typedef GET_MODULE_HANDLE_W *PGET_MODULE_HANDLE_W;

typedef
HANDLE
(WINAPI CREATE_FILE_W)(
    _In_ LPCWSTR lpFileName,
    _In_ DWORD dwDesiredAccess,
    _In_ DWORD dwShareMode,
    _In_opt_ LPSECURITY_ATTRIBUTES lpSecurityAttributes,
    _In_ DWORD dwCreationDisposition,
    _In_ DWORD dwFlagsAndAttributes,
    _In_opt_ HANDLE hTemplateFile
    );
typedef CREATE_FILE_W *PCREATE_FILE_W;

typedef
_Ret_maybenull_
HANDLE
(WINAPI CREATE_FILE_MAPPING_W)(
    _In_ HANDLE hFile,
    _In_opt_ LPSECURITY_ATTRIBUTES lpFileMappingAttributes,
    _In_ DWORD flProtect,
    _In_ DWORD dwMaximumSizeHigh,
    _In_ DWORD dwMaximumSizeLow,
    _In_opt_ LPCWSTR lpName
    );
typedef CREATE_FILE_MAPPING_W *PCREATE_FILE_MAPPING_W;

typedef
_Ret_maybenull_
HANDLE
(WINAPI OPEN_FILE_MAPPING_W)(
    _In_ DWORD dwDesiredAccess,
    _In_ BOOL bInheritHandle,
    _In_ LPCWSTR lpName
    );
typedef OPEN_FILE_MAPPING_W *POPEN_FILE_MAPPING_W;

typedef
_Ret_maybenull_
__out_data_source(FILE)
LPVOID
(WINAPI MAP_VIEW_OF_FILE)(
    _In_ HANDLE hFileMappingObject,
    _In_ DWORD dwDesiredAccess,
    _In_ DWORD dwFileOffsetHigh,
    _In_ DWORD dwFileOffsetLow,
    _In_ SIZE_T dwNumberOfBytesToMap
    );
typedef MAP_VIEW_OF_FILE *PMAP_VIEW_OF_FILE;

typedef
_Ret_maybenull_
__out_data_source(FILE)
LPVOID
(WINAPI MAP_VIEW_OF_FILE_EX)(
    _In_ HANDLE hFileMappingObject,
    _In_ DWORD dwDesiredAccess,
    _In_ DWORD dwFileOffsetHigh,
    _In_ DWORD dwFileOffsetLow,
    _In_ SIZE_T dwNumberOfBytesToMap,
    _In_opt_ LPVOID lpBaseAddress
    );
typedef MAP_VIEW_OF_FILE_EX *PMAP_VIEW_OF_FILE_EX;

typedef
_Ret_maybenull_
__out_data_source(FILE)
LPVOID
(WINAPI MAP_VIEW_OF_FILE_EX_NUMA)(
    _In_ HANDLE hFileMappingObject,
    _In_ DWORD dwDesiredAccess,
    _In_ DWORD dwFileOffsetHigh,
    _In_ DWORD dwFileOffsetLow,
    _In_ SIZE_T dwNumberOfBytesToMap,
    _In_opt_ LPVOID lpBaseAddress,
    _In_ ULONG nndPreferred
    );
typedef MAP_VIEW_OF_FILE_EX_NUMA *PMAP_VIEW_OF_FILE_EX_NUMA;

typedef
_Ret_maybenull_
__out_data_source(FILE)
PVOID
(WINAPI MAP_VIEW_OF_FILE_NUMA2)(
    _In_ HANDLE FileMappingHandle,
    _In_ HANDLE ProcessHandle,
    _In_ ULONG64 Offset,
    _In_opt_ PVOID BaseAddress,
    _In_ SIZE_T ViewSize,
    _In_ ULONG AllocationType,
    _In_ ULONG PageProtection,
    _In_ ULONG PreferredNode
    );
typedef MAP_VIEW_OF_FILE_NUMA2 *PMAP_VIEW_OF_FILE_NUMA2;

typedef
_Ret_maybenull_
__out_data_source(FILE)
PVOID
(WINAPI TRY_MAP_VIEW_OF_FILE_NUMA2)(
    _In_ struct _RTL *Rtl,
    _In_ HANDLE FileMappingHandle,
    _In_ HANDLE ProcessHandle,
    _In_ ULONG64 Offset,
    _In_opt_ PVOID BaseAddress,
    _In_ SIZE_T ViewSize,
    _In_ ULONG AllocationType,
    _In_ ULONG PageProtection,
    _In_ ULONG PreferredNode
    );
typedef TRY_MAP_VIEW_OF_FILE_NUMA2 *PTRY_MAP_VIEW_OF_FILE_NUMA2;

typedef
BOOL
(WINAPI FLUSH_VIEW_OF_FILE)(
    _In_ LPCVOID lpBaseAddress,
    _In_ SIZE_T dwNumberOfBytesToFlush
    );
typedef FLUSH_VIEW_OF_FILE *PFLUSH_VIEW_OF_FILE;

typedef
BOOL
(WINAPI UNMAP_VIEW_OF_FILE)(
    _In_ LPCVOID lpBaseAddress
    );
typedef UNMAP_VIEW_OF_FILE *PUNMAP_VIEW_OF_FILE;

typedef
BOOL
(WINAPI UNMAP_VIEW_OF_FILE_EX)(
    _In_ PVOID BaseAddress,
    _In_ ULONG UnmapFlags
    );
typedef UNMAP_VIEW_OF_FILE_EX *PUNMAP_VIEW_OF_FILE_EX;

typedef
DWORD
(WINAPI WAIT_FOR_SINGLE_OBJECT)(
    _In_ HANDLE hHandle,
    _In_ DWORD dwMilliseconds
    );
typedef WAIT_FOR_SINGLE_OBJECT *PWAIT_FOR_SINGLE_OBJECT;

typedef
DWORD
(WINAPI WAIT_FOR_SINGLE_OBJECT_EX)(
    _In_ HANDLE hHandle,
    _In_ DWORD dwMilliseconds,
    _In_ BOOL bAlertable
    );
typedef WAIT_FOR_SINGLE_OBJECT_EX *PWAIT_FOR_SINGLE_OBJECT_EX;

typedef
ULONG
(WINAPI GET_LAST_ERROR)(
    VOID
    );
typedef GET_LAST_ERROR *PGET_LAST_ERROR;

typedef
VOID
(WINAPI SET_LAST_ERROR)(
    ULONG
    );
typedef SET_LAST_ERROR *PSET_LAST_ERROR;

typedef
BOOL
(WINAPI SET_EVENT)(
    _In_ HANDLE hEvent
    );
typedef SET_EVENT *PSET_EVENT;

typedef
BOOL
(WINAPI RESET_EVENT)(
    _In_ HANDLE hEvent
    );
typedef RESET_EVENT *PRESET_EVENT;

typedef
ULONG
(WINAPI SUSPEND_THREAD)(
    _In_ HANDLE ThreadHandle
    );
typedef SUSPEND_THREAD *PSUSPEND_THREAD;

typedef
ULONG
(WINAPI RESUME_THREAD)(
    _In_ HANDLE ThreadHandle
    );
typedef RESUME_THREAD *PRESUME_THREAD;

typedef
BOOL
(WINAPI GET_THREAD_CONTEXT)(
    _In_ HANDLE ThreadHandle,
    _Inout_ LPCONTEXT Context
    );
typedef GET_THREAD_CONTEXT *PGET_THREAD_CONTEXT;

typedef
BOOL
(WINAPI SET_THREAD_CONTEXT)(
    _In_ HANDLE ThreadHandle,
    _In_ const CONTEXT *Context
    );
typedef SET_THREAD_CONTEXT *PSET_THREAD_CONTEXT;

typedef
DWORD
(WINAPI SIGNAL_OBJECT_AND_WAIT)(
    _In_ HANDLE hObjectToSignal,
    _In_ HANDLE hObjectToWaitOn,
    _In_ DWORD  dwMilliseconds,
    _In_ BOOL   bAlertable
    );
typedef SIGNAL_OBJECT_AND_WAIT *PSIGNAL_OBJECT_AND_WAIT;

typedef
DWORD
(WINAPI SLEEP_EX)(
    _In_ DWORD dwMilliseconds,
    _In_ BOOL bAlertable
    );
typedef SLEEP_EX *PSLEEP_EX;

typedef
VOID
(WINAPI EXIT_THREAD)(
    _In_ DWORD dwExitCode
    );
typedef EXIT_THREAD *PEXIT_THREAD;

typedef
BOOL
(WINAPI TERMINATE_THREAD)(
    _In_ HANDLE hThread,
    _In_ DWORD dwExitCode
    );
typedef TERMINATE_THREAD *PTERMINATE_THREAD;

typedef
_Success_(return != 0)
BOOL
(WINAPI GET_EXIT_CODE_THREAD)(
    _In_ HANDLE hThread,
    _Out_ LPDWORD lpExitCode
    );
typedef GET_EXIT_CODE_THREAD *PGET_EXIT_CODE_THREAD;

typedef
DWORD
(WINAPI QUEUE_USER_APC)(
    _In_ PAPCFUNC pfnAPC,
    _In_ HANDLE hThread,
    _In_ ULONG_PTR dwData
    );
typedef QUEUE_USER_APC *PQUEUE_USER_APC;

typedef
BOOL
(WINAPI CLOSE_HANDLE)(
    _In_ HANDLE hObject
    );
typedef CLOSE_HANDLE *PCLOSE_HANDLE;

typedef
VOID
(WINAPI OUTPUT_DEBUG_STRING_A)(
    _In_opt_ LPCSTR lpOutputString
    );
typedef OUTPUT_DEBUG_STRING_A *POUTPUT_DEBUG_STRING_A;

typedef
VOID
(WINAPI OUTPUT_DEBUG_STRING_W)(
    _In_opt_ LPCWSTR lpOutputString
    );
typedef OUTPUT_DEBUG_STRING_W *POUTPUT_DEBUG_STRING_W;

typedef
VOID
(CALLBACK APC_ROUTINE) (
    _In_opt_ PVOID ApcArgument1,
    _In_opt_ PVOID ApcArgument2,
    _In_opt_ PVOID ApcArgument3
    );
typedef APC_ROUTINE *PAPC_ROUTINE;

typedef
NTSTATUS
(NTAPI NT_QUEUE_APC_THREAD)(
    _In_ HANDLE ThreadHandle,
    _In_ PAPC_ROUTINE ApcRoutine,
    _In_opt_ PVOID ApcArgument1,
    _In_opt_ PVOID ApcArgument2,
    _In_opt_ PVOID ApcArgument3
    );
typedef NT_QUEUE_APC_THREAD *PNT_QUEUE_APC_THREAD;

typedef struct _APC {
    PAPC_ROUTINE Routine;
    PVOID Argument1;
    PVOID Argument2;
    PVOID Argument3;
} APC;
typedef APC *PAPC;

typedef
NTSTATUS
(NTAPI NT_TEST_ALERT)(
    VOID
    );
typedef NT_TEST_ALERT *PNT_TEST_ALERT;

typedef
BOOL
(WINAPI DEBUG_ACTIVE_PROCESS_STOP)(
    _In_ BOOL KillOnExit
    );
typedef DEBUG_ACTIVE_PROCESS_STOP *PDEBUG_ACTIVE_PROCESS_STOP;

typedef
BOOL
(WINAPI DEBUG_SET_PROCESS_KILL_ON_EXIT)(
    _In_ BOOL KillOnExit
    );
typedef DEBUG_SET_PROCESS_KILL_ON_EXIT *PDEBUG_SET_PROCESS_KILL_ON_EXIT;

typedef
_Success_(return != 0)
BOOL
(CREATE_RANDOM_OBJECT_NAMES)(
    _In_ PRTL Rtl,
    _In_ PALLOCATOR TemporaryAllocator,
    _In_ PALLOCATOR WideBufferAllocator,
    _In_ USHORT NumberOfNames,
    _In_ USHORT LengthOfNameInChars,
    _In_opt_ PUNICODE_STRING NamespacePrefix,
    _Inout_ PPUNICODE_STRING NamesArrayPointer,
    _In_opt_ PPUNICODE_STRING PrefixArrayPointer,
    _Out_ PULONG SizeOfWideBufferInBytes,
    _Out_writes_bytes_all_(*SizeOfWideBufferInBytes) PPWSTR WideBufferPointer
    );
typedef CREATE_RANDOM_OBJECT_NAMES *PCREATE_RANDOM_OBJECT_NAMES;

typedef
_Success_(return != 0)
BOOL
(CREATE_SINGLE_RANDOM_OBJECT_NAME)(
    _In_ PRTL Rtl,
    _In_ PALLOCATOR TemporaryAllocator,
    _In_ PALLOCATOR WideBufferAllocator,
    _In_opt_ PCUNICODE_STRING Prefix,
    _Inout_ PUNICODE_STRING Name
    );
typedef CREATE_SINGLE_RANDOM_OBJECT_NAME *PCREATE_SINGLE_RANDOM_OBJECT_NAME;

#include "Injection.h"

//
// Miscellaneous NT/Rtl.
//

typedef NTSYSAPI SIZE_T (NTAPI RTL_COMPARE_MEMORY)(
    _In_ const VOID * Source1,
    _In_ const VOID * Source2,
    _In_ SIZE_T Length
    );

typedef RTL_COMPARE_MEMORY *PRTL_COMPARE_MEMORY;

typedef
EXCEPTION_DISPOSITION
(__cdecl RTL_EXCEPTION_HANDLER)(
    PEXCEPTION_RECORD ExceptionRecord,
    ULONG_PTR Frame,
    PCONTEXT Context,
    struct _DISPATCHER_CONTEXT *Dispatch
    );
typedef RTL_EXCEPTION_HANDLER *PRTL_EXCEPTION_HANDLER;

typedef RTL_EXCEPTION_HANDLER __C_SPECIFIC_HANDLER;
typedef __C_SPECIFIC_HANDLER *P__C_SPECIFIC_HANDLER;

typedef
VOID
(SET_C_SPECIFIC_HANDLER)(
    _In_ P__C_SPECIFIC_HANDLER Handler
    );
typedef SET_C_SPECIFIC_HANDLER *PSET_C_SPECIFIC_HANDLER;

typedef
VOID
(__cdecl __SECURITY_INIT_COOKIE)(
    VOID
    );
typedef __SECURITY_INIT_COOKIE *P__SECURITY_INIT_COOKIE;

//
// Prefix Helpers
//

typedef
BOOLEAN
(NTAPI RTL_PREFIX_STRING)(
    _In_ PCSTRING String1,
    _In_ PCSTRING String2,
    _In_ BOOLEAN CaseInSensitive
    );
typedef RTL_PREFIX_STRING *PRTL_PREFIX_STRING;

typedef
BOOLEAN
(NTAPI RTL_SUFFIX_STRING)(
    _In_ PCSTRING String1,
    _In_ PCSTRING String2,
    _In_ BOOLEAN CaseInSensitive
    );
typedef RTL_SUFFIX_STRING *PRTL_SUFFIX_STRING;

typedef
BOOLEAN
(NTAPI RTL_PREFIX_UNICODE_STRING)(
    _In_ PCUNICODE_STRING String1,
    _In_ PCUNICODE_STRING String2,
    _In_ BOOLEAN CaseInSensitive
    );
typedef RTL_PREFIX_UNICODE_STRING *PRTL_PREFIX_UNICODE_STRING;

typedef
BOOLEAN
(NTAPI RTL_SUFFIX_UNICODE_STRING)(
    _In_ PCUNICODE_STRING String1,
    _In_ PCUNICODE_STRING String2,
    _In_ BOOLEAN CaseInSensitive
    );
typedef RTL_SUFFIX_UNICODE_STRING *PRTL_SUFFIX_UNICODE_STRING;

//
// Splay Links
//

typedef struct _RTL_SPLAY_LINKS {
    struct _RTL_SPLAY_LINKS *Parent;
    struct _RTL_SPLAY_LINKS *LeftChild;
    struct _RTL_SPLAY_LINKS *RightChild;
} RTL_SPLAY_LINKS, *PRTL_SPLAY_LINKS;
C_ASSERT(sizeof(RTL_SPLAY_LINKS) == 24);

typedef PRTL_SPLAY_LINKS (NTAPI PRTL_SPLAY)(
    _Inout_ PRTL_SPLAY_LINKS Links
    );


//
// Generic Tables
//

typedef struct _TABLE_ENTRY_HEADER {
    RTL_SPLAY_LINKS SplayLinks;
    LIST_ENTRY ListEntry;
    LONGLONG UserData;
} TABLE_ENTRY_HEADER, *PTABLE_ENTRY_HEADER;
C_ASSERT(sizeof(TABLE_ENTRY_HEADER) == 48);

//
// _HEADER_HEADER is a terrible name for something, unless you're dealing with
// the header of a header, which is what we're dealing with here.
//

typedef struct _TABLE_ENTRY_HEADER_HEADER {

    RTL_SPLAY_LINKS SplayLinks;
    LIST_ENTRY ListEntry;

} TABLE_ENTRY_HEADER_HEADER, *PTABLE_ENTRY_HEADER_HEADER;

typedef enum _TABLE_SEARCH_RESULT {
    TableEmptyTree,
    TableFoundNode,
    TableInsertAsLeft,
    TableInsertAsRight
} TABLE_SEARCH_RESULT;

struct _RTL_GENERIC_TABLE;

typedef enum _RTL_GENERIC_COMPARE_RESULTS {
    GenericLessThan,
    GenericGreaterThan,
    GenericEqual
} RTL_GENERIC_COMPARE_RESULTS;

typedef RTL_GENERIC_COMPARE_RESULTS (NTAPI *PRTL_GENERIC_COMPARE_ROUTINE)(
    _In_ struct _RTL_GENERIC_TABLE  *Table,
    _In_ PVOID  FirstStruct,
    _In_ PVOID  SecondStruct
    );

typedef PVOID (NTAPI *PRTL_GENERIC_ALLOCATE_ROUTINE) (
    _In_ struct _RTL_GENERIC_TABLE  *Table,
    _In_ CLONG  ByteSize
    );

typedef VOID (NTAPI *PRTL_GENERIC_FREE_ROUTINE)(
    _In_ struct _RTL_GENERIC_TABLE *Table,
    _In_ PVOID  Buffer
    );

typedef struct _RTL_GENERIC_TABLE {
    PRTL_SPLAY_LINKS              TableRoot;
    LIST_ENTRY                    InsertOrderList;
    PLIST_ENTRY                   OrderedPointer;
    ULONG                         WhichOrderedElement;
    ULONG                         NumberGenericTableElements;
    PRTL_GENERIC_COMPARE_ROUTINE  CompareRoutine;
    PRTL_GENERIC_ALLOCATE_ROUTINE AllocateRoutine;
    PRTL_GENERIC_FREE_ROUTINE     FreeRoutine;
    PVOID                         TableContext;
} RTL_GENERIC_TABLE, *PRTL_GENERIC_TABLE;

typedef VOID (NTAPI *PRTL_INITIALIZE_GENERIC_TABLE)(
    _Out_    PRTL_GENERIC_TABLE            Table,
    _In_     PRTL_GENERIC_COMPARE_ROUTINE  CompareRoutine,
    _In_     PRTL_GENERIC_ALLOCATE_ROUTINE AllocateRoutine,
    _In_     PRTL_GENERIC_FREE_ROUTINE     FreeRoutine,
    _In_opt_ PVOID                         TableContext
    );

typedef PVOID (NTAPI *PRTL_INSERT_ELEMENT_GENERIC_TABLE)(
    _In_ PRTL_GENERIC_TABLE Table,
    _In_ PVOID Buffer,
    _In_ CLONG BufferSize,
    _Out_opt_ PBOOLEAN NewElement
    );

typedef PVOID (NTAPI *PRTL_INSERT_ELEMENT_GENERIC_TABLE_FULL)(
    _In_ PRTL_GENERIC_TABLE Table,
    _In_ PVOID Buffer,
    _In_ CLONG BufferSize,
    _Out_opt_ PBOOLEAN NewElement,
    _In_ PVOID NodeOrParent,
    _In_ TABLE_SEARCH_RESULT SearchResult
    );

typedef BOOLEAN (NTAPI *PRTL_DELETE_ELEMENT_GENERIC_TABLE)(
    _In_ PRTL_GENERIC_TABLE Table,
    _In_ PVOID Buffer
    );

typedef PVOID (NTAPI *PRTL_LOOKUP_ELEMENT_GENERIC_TABLE)(
    _In_ PRTL_GENERIC_TABLE Table,
    _In_ PVOID Buffer
    );

typedef PVOID (NTAPI *PRTL_LOOKUP_ELEMENT_GENERIC_TABLE_FULL)(
    _In_ PRTL_GENERIC_TABLE Table,
    _In_ PVOID Buffer,
    _Out_ PVOID *NodeOrParent,
    _Out_ TABLE_SEARCH_RESULT *SearchResult
    );

typedef PVOID (NTAPI *PRTL_ENUMERATE_GENERIC_TABLE)(
    _In_ PRTL_GENERIC_TABLE Table,
    _In_ BOOLEAN Restart
    );

typedef PVOID (NTAPI *PRTL_ENUMERATE_GENERIC_TABLE_WITHOUT_SPLAYING)(
    _In_ PRTL_GENERIC_TABLE Table,
    _Inout_ PVOID *RestartKey
    );

typedef PVOID (NTAPI *PRTL_GET_ELEMENT_GENERIC_TABLE)(
    _In_ PRTL_GENERIC_TABLE Table,
    _In_ ULONG I
    );

typedef PVOID (NTAPI *PRTL_NUMBER_GENERIC_TABLE_ELEMENTS)(
    _In_ PRTL_GENERIC_TABLE Table
    );

typedef BOOLEAN (NTAPI *PRTL_IS_GENERIC_TABLE_EMPTY)(
    _In_ PRTL_GENERIC_TABLE Table
    );

//
// Avl Table
//

typedef struct _RTL_BALANCED_LINKS {
    struct _RTL_BALANCED_LINKS *Parent;
    struct _RTL_BALANCED_LINKS *LeftChild;
    struct _RTL_BALANCED_LINKS *RightChild;
    CHAR Balance;
    UCHAR Reserved[3];
    ULONG Padding;
} RTL_BALANCED_LINKS;
typedef RTL_BALANCED_LINKS *PRTL_BALANCED_LINKS;
C_ASSERT(sizeof(RTL_BALANCED_LINKS) == 32);

typedef RTL_GENERIC_COMPARE_RESULTS (NTAPI *PRTL_AVL_COMPARE_ROUTINE)(
    _In_ struct _RTL_AVL_TABLE *Table,
    _In_ PVOID FirstStruct,
    _In_ PVOID SecondStruct
    );

typedef PVOID (NTAPI *PRTL_AVL_ALLOCATE_ROUTINE)(
    _In_ struct _RTL_AVL_TABLE *Table,
    _In_ CLONG ByteSize
    );

typedef VOID (NTAPI *PRTL_AVL_FREE_ROUTINE)(
    _In_ struct _RTL_AVL_TABLE *Table,
    _In_ __drv_freesMem(Mem) _Post_invalid_ PVOID Buffer
    );

typedef struct _RTL_AVL_TABLE {
    RTL_BALANCED_LINKS BalancedRoot;
    PVOID OrderedPointer;
    ULONG WhichOrderedElement;
    ULONG NumberGenericTableElements;
    ULONG DepthOfTree;
    ULONG Unused1;
    PRTL_BALANCED_LINKS RestartKey;
    ULONG DeleteCount;
    ULONG Unused2;
    PRTL_AVL_COMPARE_ROUTINE CompareRoutine;
    PRTL_AVL_ALLOCATE_ROUTINE AllocateRoutine;
    PRTL_AVL_FREE_ROUTINE FreeRoutine;
    PVOID TableContext;
} RTL_AVL_TABLE, *PRTL_AVL_TABLE;
C_ASSERT(FIELD_OFFSET(RTL_AVL_TABLE, OrderedPointer) == 32);
C_ASSERT(sizeof(RTL_AVL_TABLE) == 32+8+4+4+4+4+8+4+4+8+8+8+8);
C_ASSERT(sizeof(RTL_AVL_TABLE) == 104);

typedef NTSTATUS (NTAPI *PRTL_AVL_MATCH_FUNCTION)(
    _In_ struct _RTL_AVL_TABLE *Table,
    _In_ PVOID UserData,
    _In_ PVOID MatchData
    );

typedef VOID (NTAPI *PRTL_INITIALIZE_GENERIC_TABLE_AVL)(
    _Out_ PRTL_AVL_TABLE Table,
    _In_ PRTL_AVL_COMPARE_ROUTINE CompareRoutine,
    _In_ PRTL_AVL_ALLOCATE_ROUTINE AllocateRoutine,
    _In_ PRTL_AVL_FREE_ROUTINE FreeRoutine,
    _In_opt_ PVOID TableContext
    );

typedef PVOID (NTAPI *PRTL_INSERT_ELEMENT_GENERIC_TABLE_AVL)(
    _In_ PRTL_AVL_TABLE Table,
    _In_ PVOID Buffer,
    _In_ CLONG BufferSize,
    _Out_opt_ PBOOLEAN NewElement
    );

typedef PVOID (NTAPI *PRTL_INSERT_ELEMENT_GENERIC_TABLE_FULL_AVL)(
    _In_ PRTL_AVL_TABLE Table,
    _In_ PVOID Buffer,
    _In_ CLONG BufferSize,
    _Out_opt_ PBOOLEAN NewElement,
    _In_ PVOID NodeOrParent,
    _In_ TABLE_SEARCH_RESULT SearchResult
    );

typedef BOOLEAN (NTAPI *PRTL_DELETE_ELEMENT_GENERIC_TABLE_AVL)(
    _In_ PRTL_AVL_TABLE Table,
    _In_ PVOID Buffer
    );

typedef PVOID (NTAPI *PRTL_LOOKUP_ELEMENT_GENERIC_TABLE_AVL)(
    _In_ PRTL_AVL_TABLE Table,
    _In_ PVOID Buffer
    );

typedef PVOID (NTAPI *PRTL_LOOKUP_ELEMENT_GENERIC_TABLE_FULL_AVL)(
    _In_ PRTL_AVL_TABLE Table,
    _In_ PVOID Buffer,
    _Out_ PVOID *NodeOrParent,
    _Out_ TABLE_SEARCH_RESULT *SearchResult
    );

typedef PVOID (NTAPI *PRTL_ENUMERATE_GENERIC_TABLE_AVL)(
    _In_ PRTL_AVL_TABLE Table,
    _In_ BOOLEAN Restart
    );

typedef PVOID (NTAPI *PRTL_ENUMERATE_GENERIC_TABLE_WITHOUT_SPLAYING_AVL)(
    _In_ PRTL_AVL_TABLE Table,
    _Inout_ PVOID *RestartKey
    );

typedef PVOID (NTAPI *PRTL_LOOKUP_FIRST_MATCHING_ELEMENT_GENERIC_TABLE_AVL)(
    _In_ PRTL_AVL_TABLE Table,
    _In_ PVOID Buffer,
    _Out_ PVOID *RestartKey
    );

typedef PVOID (NTAPI *PRTL_ENUMERATE_GENERIC_TABLE_LIKE_A_DICTIONARY)(
    _In_ PRTL_AVL_TABLE Table,
    _In_opt_ PRTL_AVL_MATCH_FUNCTION MatchFunction,
    _In_opt_ PVOID MatchData,
    _In_ ULONG NextFlag,
    _Inout_ PVOID *RestartKey,
    _Inout_ PULONG DeleteCount,
    _In_ PVOID Buffer
    );

typedef PVOID (NTAPI *PRTL_GET_ELEMENT_GENERIC_TABLE_AVL)(
    _In_ PRTL_AVL_TABLE Table,
    _In_ ULONG I
    );

typedef ULONG (NTAPI *PRTL_NUMBER_GENERIC_TABLE_ELEMENTS_AVL)(
    _In_ PRTL_AVL_TABLE Table
    );

typedef BOOLEAN (NTAPI *PRTL_IS_GENERIC_TABLE_EMPTY_AVL)(
    _In_ PRTL_AVL_TABLE Table
    );

//
// Hash Tables
//

typedef struct _RTL_DYNAMIC_HASH_TABLE_ENTRY {
    LIST_ENTRY Linkage;
    ULONG_PTR Signature;
} RTL_DYNAMIC_HASH_TABLE_ENTRY, *PRTL_DYNAMIC_HASH_TABLE_ENTRY;
C_ASSERT(sizeof(RTL_DYNAMIC_HASH_TABLE_ENTRY) == 24);

typedef struct _RTL_DYNAMIC_HASH_TABLE_CONTEXT {
    PLIST_ENTRY ChainHead;
    PLIST_ENTRY PrevLinkage;
    ULONG_PTR Signature;
} RTL_DYNAMIC_HASH_TABLE_CONTEXT, *PRTL_DYNAMIC_HASH_TABLE_CONTEXT;
C_ASSERT(sizeof(RTL_DYNAMIC_HASH_TABLE_CONTEXT) == 24);

typedef struct _RTL_DYNAMIC_HASH_TABLE_ENUMERATOR {
    union {
       RTL_DYNAMIC_HASH_TABLE_ENTRY HashEntry;
       PLIST_ENTRY CurEntry;
    };
    PLIST_ENTRY ChainHead;
    ULONG BucketIndex;
    ULONG Padding1;
} RTL_DYNAMIC_HASH_TABLE_ENUMERATOR, *PRTL_DYNAMIC_HASH_TABLE_ENUMERATOR;
C_ASSERT(sizeof(RTL_DYNAMIC_HASH_TABLE_ENUMERATOR) == 40);

typedef struct _RTL_DYNAMIC_HASH_TABLE {

    // Entries initialized at creation
    ULONG Flags;
    ULONG Shift;

    // Entries used in bucket computation.
    ULONG TableSize;
    ULONG Pivot;
    ULONG DivisorMask;

    // Counters
    ULONG NumEntries;
    ULONG NonEmptyBuckets;
    ULONG NumEnumerators;

    // The directory. This field is for internal use only.
    PVOID Directory;

} RTL_DYNAMIC_HASH_TABLE, *PRTL_DYNAMIC_HASH_TABLE;

FORCEINLINE
VOID
RtlInitHashTableContext(
    _Inout_ PRTL_DYNAMIC_HASH_TABLE_CONTEXT Context
    )
{
    Context->ChainHead = NULL;
    Context->PrevLinkage = NULL;
}

typedef BOOLEAN (NTAPI RTL_CREATE_HASH_TABLE)(
    _Inout_ PRTL_DYNAMIC_HASH_TABLE *HashTable,
    _In_ ULONG Shift,
    _In_ ULONG Flags
    );
typedef RTL_CREATE_HASH_TABLE *PRTL_CREATE_HASH_TABLE;

typedef BOOLEAN (NTAPI RTL_CREATE_HASH_TABLE_EX)(
    _Inout_ PRTL_DYNAMIC_HASH_TABLE *HashTable,
    _In_ ULONG InitialSize,
    _In_ ULONG Shift,
    _In_ ULONG Flags
    );
typedef RTL_CREATE_HASH_TABLE_EX *PRTL_CREATE_HASH_TABLE_EX;

typedef VOID (NTAPI RTL_DELETE_HASH_TABLE)(
    _In_ PRTL_DYNAMIC_HASH_TABLE HashTable
    );
typedef RTL_DELETE_HASH_TABLE *PRTL_DELETE_HASH_TABLE;

typedef BOOLEAN (NTAPI RTL_INSERT_ENTRY_HASH_TABLE)(
    _In_ PRTL_DYNAMIC_HASH_TABLE HashTable,
    _In_ PRTL_DYNAMIC_HASH_TABLE_ENTRY Entry,
    _In_ ULONG_PTR Signature,
    _Inout_opt_ PRTL_DYNAMIC_HASH_TABLE_CONTEXT Context
    );
typedef RTL_INSERT_ENTRY_HASH_TABLE *PRTL_INSERT_ENTRY_HASH_TABLE;

typedef BOOLEAN (NTAPI *PRTL_REMOVE_ENTRY_HASH_TABLE)(
    _In_ PRTL_DYNAMIC_HASH_TABLE HashTable,
    _In_ PRTL_DYNAMIC_HASH_TABLE_ENTRY Entry,
    _Inout_opt_ PRTL_DYNAMIC_HASH_TABLE_CONTEXT Context
    );

typedef PRTL_DYNAMIC_HASH_TABLE_ENTRY (NTAPI *PRTL_LOOKUP_ENTRY_HASH_TABLE)(
    _In_ PRTL_DYNAMIC_HASH_TABLE HashTable,
    _In_ ULONG_PTR Signature,
    _Out_opt_ PRTL_DYNAMIC_HASH_TABLE_CONTEXT Context
    );

typedef PRTL_DYNAMIC_HASH_TABLE_ENTRY (NTAPI *PRTL_GET_NEXT_ENTRY_HASH_TABLE)(
    _In_ PRTL_DYNAMIC_HASH_TABLE HashTable,
    _In_ PRTL_DYNAMIC_HASH_TABLE_CONTEXT Context
    );

typedef BOOLEAN (NTAPI *PRTL_INIT_ENUMERATION_HASH_TABLE)(
    _In_ PRTL_DYNAMIC_HASH_TABLE HashTable,
    _Out_ PRTL_DYNAMIC_HASH_TABLE_ENUMERATOR Enumerator
    );

typedef PRTL_DYNAMIC_HASH_TABLE_ENTRY (NTAPI *PRTL_ENUMERATE_ENTRY_HASH_TABLE)(
    _In_ PRTL_DYNAMIC_HASH_TABLE HashTable,
    _Inout_ PRTL_DYNAMIC_HASH_TABLE_ENUMERATOR Enumerator
    );

typedef VOID (NTAPI *PRTL_END_ENUMERATION_HASH_TABLE)(
    _In_ PRTL_DYNAMIC_HASH_TABLE HashTable,
    _Inout_ PRTL_DYNAMIC_HASH_TABLE_ENUMERATOR Enumerator
    );

typedef BOOLEAN (NTAPI *PRTL_INIT_WEAK_ENUMERATION_HASH_TABLE)(
    _In_ PRTL_DYNAMIC_HASH_TABLE HashTable,
    _Out_ PRTL_DYNAMIC_HASH_TABLE_ENUMERATOR Enumerator
    );

typedef
PRTL_DYNAMIC_HASH_TABLE_ENTRY
(NTAPI *PRTL_WEAKLY_ENUMERATE_ENTRY_HASH_TABLE)(
    _In_ PRTL_DYNAMIC_HASH_TABLE HashTable,
    _Inout_ PRTL_DYNAMIC_HASH_TABLE_ENUMERATOR Enumerator
    );

typedef VOID (NTAPI *PRTL_END_WEAK_ENUMERATION_HASH_TABLE)(
    _In_ PRTL_DYNAMIC_HASH_TABLE HashTable,
    _Inout_ PRTL_DYNAMIC_HASH_TABLE_ENUMERATOR Enumerator
    );

typedef BOOLEAN (NTAPI *PRTL_INIT_STRONG_ENUMERATION_HASH_TABLE)(
    _In_ PRTL_DYNAMIC_HASH_TABLE HashTable,
    _Out_ PRTL_DYNAMIC_HASH_TABLE_ENUMERATOR Enumerator
    );

typedef
PRTL_DYNAMIC_HASH_TABLE_ENTRY
(NTAPI *PRTL_STRONGLY_ENUMERATE_ENTRY_HASH_TABLE)(
    _In_ PRTL_DYNAMIC_HASH_TABLE HashTable,
    _Inout_ PRTL_DYNAMIC_HASH_TABLE_ENUMERATOR Enumerator
    );

typedef VOID (NTAPI *PRTL_END_STRONG_ENUMERATION_HASH_TABLE)(
    _In_ PRTL_DYNAMIC_HASH_TABLE HashTable,
    _Inout_ PRTL_DYNAMIC_HASH_TABLE_ENUMERATOR Enumerator
    );

typedef BOOLEAN (NTAPI *PRTL_EXPAND_HASH_TABLE)(
    _In_ PRTL_DYNAMIC_HASH_TABLE HashTable
    );

typedef BOOLEAN (NTAPI *PRTL_CONTRACT_HASH_TABLE)(
    _In_ PRTL_DYNAMIC_HASH_TABLE HashTable
    );

//
// Prefix Tables
//

typedef struct _PREFIX_TABLE_ENTRY {
    CSHORT NodeTypeCode;
    CSHORT NameLength;
    struct _PREFIX_TABLE_ENTRY *NextPrefixTree;
    RTL_SPLAY_LINKS Links;
    PSTRING Prefix;
} PREFIX_TABLE_ENTRY, *PPREFIX_TABLE_ENTRY, **PPPREFIX_TABLE_ENTRY;

typedef struct _PREFIX_TABLE {
    CSHORT NodeTypeCode;
    CSHORT NameLength;
    ULONG Unused;
    PPREFIX_TABLE_ENTRY NextPrefixTree;
} PREFIX_TABLE, *PPREFIX_TABLE, **PPPREFIX_TABLE;
C_ASSERT(sizeof(PREFIX_TABLE) == 16);

typedef VOID (NTAPI *PPFX_INITIALIZE)(
    _Out_ PPREFIX_TABLE PrefixTable
    );

typedef BOOLEAN (NTAPI *PPFX_INSERT_PREFIX)(
    _In_ PPREFIX_TABLE PrefixTable,
    _In_ PSTRING Prefix,
    _Out_ PPREFIX_TABLE_ENTRY PrefixTableEntry
    );

typedef VOID (NTAPI *PPFX_REMOVE_PREFIX)(
    _In_ PPREFIX_TABLE PrefixTable,
    _In_ PPREFIX_TABLE_ENTRY PrefixTableEntry
    );

typedef PPREFIX_TABLE_ENTRY (NTAPI *PPFX_FIND_PREFIX)(
    _In_ PPREFIX_TABLE PrefixTable,
    _In_ PSTRING FullName
    );

//
// Unicode Prefix Table
//

typedef struct _UNICODE_PREFIX_TABLE_ENTRY {
    CSHORT NodeTypeCode;
    CSHORT NameLength;
    ULONG Unused1;
    struct _UNICODE_PREFIX_TABLE_ENTRY *NextPrefixTree;
    struct _UNICODE_PREFIX_TABLE_ENTRY *CaseMatch;
    RTL_SPLAY_LINKS Links;
    PUNICODE_STRING Prefix;
} UNICODE_PREFIX_TABLE_ENTRY;
C_ASSERT(FIELD_OFFSET(UNICODE_PREFIX_TABLE_ENTRY, NextPrefixTree) == 8);
C_ASSERT(FIELD_OFFSET(UNICODE_PREFIX_TABLE_ENTRY, Links) == 24);
C_ASSERT(FIELD_OFFSET(UNICODE_PREFIX_TABLE_ENTRY, Prefix) == 48);
C_ASSERT(sizeof(UNICODE_PREFIX_TABLE_ENTRY) == 56);
typedef UNICODE_PREFIX_TABLE_ENTRY *PUNICODE_PREFIX_TABLE_ENTRY;

typedef struct _UNICODE_PREFIX_TABLE {
    CSHORT NodeTypeCode;
    CSHORT NameLength;
    ULONG Unused;
    PUNICODE_PREFIX_TABLE_ENTRY NextPrefixTree;
    PUNICODE_PREFIX_TABLE_ENTRY LastNextEntry;
} UNICODE_PREFIX_TABLE;
typedef UNICODE_PREFIX_TABLE *PUNICODE_PREFIX_TABLE;
C_ASSERT(sizeof(UNICODE_PREFIX_TABLE) == 2+2+4+8+8);
C_ASSERT(sizeof(UNICODE_PREFIX_TABLE) == 24);

typedef VOID (NTAPI *PRTL_INITIALIZE_UNICODE_PREFIX)(
    _In_ PUNICODE_PREFIX_TABLE PrefixTable
    );

typedef BOOLEAN (NTAPI *PRTL_INSERT_UNICODE_PREFIX)(
    _In_ PUNICODE_PREFIX_TABLE PrefixTable,
    _In_ PUNICODE_STRING Prefix,
    _In_ PUNICODE_PREFIX_TABLE_ENTRY PrefixTableEntry
    );

typedef VOID (NTAPI *PRTL_REMOVE_UNICODE_PREFIX)(
    _In_ PUNICODE_PREFIX_TABLE PrefixTable,
    _In_ PUNICODE_PREFIX_TABLE_ENTRY PrefixTableEntry
    );

typedef PUNICODE_PREFIX_TABLE_ENTRY (NTAPI *PRTL_FIND_UNICODE_PREFIX)(
    _In_ PUNICODE_PREFIX_TABLE PrefixTable,
    _In_ PCUNICODE_STRING FullName,
    _In_ ULONG CaseInsensitiveIndex
    );

typedef PUNICODE_PREFIX_TABLE_ENTRY (NTAPI *PRTL_NEXT_UNICODE_PREFIX)(
    _In_ PUNICODE_PREFIX_TABLE PrefixTable,
    _In_ BOOLEAN Restart
    );

//
// Bitmaps
//

typedef VOID (NTAPI *PRTL_INITIALIZE_BITMAP)(
    _Out_ PRTL_BITMAP BitMapHeader,
    _In_opt_ PULONG BitMapBuffer,
    _In_opt_ ULONG SizeOfBitMap
    );

typedef VOID (NTAPI *PRTL_CLEAR_BIT)(
    _In_ PRTL_BITMAP BitMapHeader,
    _In_range_(<, BitMapHeader->SizeOfBitMap) ULONG BitNumber
    );

typedef VOID (NTAPI *PRTL_SET_BIT)(
    _In_ PRTL_BITMAP BitMapHeader,
    _In_range_(<, BitMapHeader->SizeOfBitMap) ULONG BitNumber
    );

typedef BOOLEAN (NTAPI *PRTL_TEST_BIT)(
    _In_ PRTL_BITMAP BitMapHeader,
    _In_range_(<, BitMapHeader->SizeOfBitMap) ULONG BitNumber
    );

typedef VOID (NTAPI *PRTL_CLEAR_ALL_BITS)(
    _In_ PRTL_BITMAP BitMapHeader
    );

typedef VOID (NTAPI *PRTL_SET_ALL_BITS)(
    _In_ PRTL_BITMAP BitMapHeader
    );

typedef ULONG (NTAPI *PRTL_FIND_CLEAR_BITS)(
    _In_ PRTL_BITMAP BitMapHeader,
    _In_ ULONG NumberToFind,
    _In_ ULONG HintIndex
    );

typedef ULONG (NTAPI *PRTL_FIND_SET_BITS)(
    _In_ PRTL_BITMAP BitMapHeader,
    _In_ ULONG NumberToFind,
    _In_ ULONG HintIndex
    );

typedef ULONG (NTAPI *PRTL_FIND_CLEAR_BITS_AND_SET)(
    _In_ PRTL_BITMAP BitMapHeader,
    _In_ ULONG NumberToFind,
    _In_ ULONG HintIndex
    );

typedef ULONG (NTAPI *PRTL_FIND_SET_BITS_AND_CLEAR)(
    _In_ PRTL_BITMAP BitMapHeader,
    _In_ ULONG NumberToFind,
    _In_ ULONG HintIndex
    );

typedef VOID (NTAPI *PRTL_CLEAR_BITS)(
    _In_ PRTL_BITMAP BitMapHeader,
    _In_ ULONG StartingIndex,
    _In_ ULONG NumberToClear
    );

typedef VOID (NTAPI *PRTL_SET_BITS)(
    _In_ PRTL_BITMAP BitMapHeader,
    _In_ ULONG StartingIndex,
    _In_ ULONG NumberToSet
    );

typedef ULONG (NTAPI *PRTL_FIND_CLEAR_BITS)(
    _In_ PRTL_BITMAP BitMapHeader,
    _In_ ULONG NumberToFind,
    _In_ ULONG HintIndex
    );

typedef ULONG (NTAPI *PRTL_FIND_SET_BITS)(
    _In_ PRTL_BITMAP BitMapHeader,
    _In_ ULONG NumberToFind,
    _In_ ULONG HintIndex
    );

typedef ULONG (NTAPI *PRTL_FIND_CLEAR_BITS_AND_SET)(
    _In_ PRTL_BITMAP BitMapHeader,
    _In_ ULONG NumberToFind,
    _In_ ULONG HintIndex
    );

typedef ULONG (NTAPI *PRTL_FIND_SET_BITS_AND_CLEAR)(
    _In_ PRTL_BITMAP BitMapHeader,
    _In_ ULONG NumberToFind,
    _In_ ULONG HintIndex
    );

typedef VOID (NTAPI *PRTL_CLEAR_BITS)(
    _In_ PRTL_BITMAP BitMapHeader,
    _In_ ULONG StartingIndex,
    _In_ ULONG NumberToClear
    );

typedef VOID (NTAPI *PRTL_SET_BITS)(
    _In_ PRTL_BITMAP BitMapHeader,
    _In_ ULONG StartingIndex,
    _In_ ULONG NumberToSet
    );

typedef ULONG (NTAPI *PRTL_FIND_CLEAR_RUNS)(
    _In_ PRTL_BITMAP BitMapHeader,
    _Out_ PRTL_BITMAP_RUN RunArray,
    _In_ ULONG SizeOfRunArray,
    _In_ BOOLEAN LocateLongestRuns
    );

typedef ULONG (NTAPI *PRTL_FIND_LONGEST_RUN_CLEAR)(
    _In_ PRTL_BITMAP BitMapHeader,
    _Out_ PULONG StartingIndex
    );

typedef ULONG (NTAPI *PRTL_FIND_FIRST_RUN_CLEAR)(
    _In_ PRTL_BITMAP BitMapHeader,
    _Out_ PULONG StartingIndex
    );

typedef ULONG (NTAPI *PRTL_NUMBER_OF_CLEAR_BITS)(
    _In_ PRTL_BITMAP BitMapHeader
    );

typedef ULONG (NTAPI *PRTL_NUMBER_OF_SET_BITS)(
    _In_ PRTL_BITMAP BitMapHeader
    );

typedef BOOLEAN (NTAPI *PRTL_ARE_BITS_CLEAR)(
    _In_ PRTL_BITMAP BitMapHeader,
    _In_ ULONG StartingIndex,
    _In_ ULONG Length
    );

typedef BOOLEAN (NTAPI *PRTL_ARE_BITS_SET)(
    _In_ PRTL_BITMAP BitMapHeader,
    _In_ ULONG StartingIndex,
    _In_ ULONG Length
    );

typedef ULONG (NTAPI *PRTL_FIND_NEXT_FORWARD_RUN_CLEAR)(
    _In_ PRTL_BITMAP BitMapHeader,
    _In_ ULONG FromIndex,
    _Out_ PULONG StartingRunIndex
    );

typedef ULONG (NTAPI *PRTL_FIND_LAST_BACKWARD_RUN_CLEAR)(
    _In_ PRTL_BITMAP BitMapHeader,
    _In_ ULONG FromIndex,
    _Out_ PULONG StartingRunIndex
    );

//
// Quad-word Bitmaps.  (Work in progress.)
//

typedef struct _RTL_BITMAP_EX {
    ULONGLONG SizeOfBitMap; // Number of bits.
    PULONGLONG Buffer;
} RTL_BITMAP_EX, *PRTL_BITMAP_EX, **PPRTL_BITMAP_EX;

typedef VOID (NTAPI *PRTL_INITIALIZE_BITMAP_EX)(
    _Out_ PRTL_BITMAP_EX BitMapHeader,
    _In_opt_ __drv_aliasesMem PULONGLONG BitMapBuffer,
    _In_opt_ ULONGLONG SizeOfBitMap
    );

typedef VOID (NTAPI *PRTL_CLEAR_BIT_EX)(
    _In_ PRTL_BITMAP_EX BitMapHeader,
    _In_range_(<, BitMapHeader->SizeOfBitMap) ULONGLONG BitNumber
    );

//
// Sparse bitmaps.  (Work in progress.)
//

typedef union _RTL_SPARSE_BITMAP_CTX_FLAGS {
    LONG AsLong;
    ULONG AsULong;
    struct _Struct_size_bytes_(sizeof(ULONG)) {
        ULONG DefaultBitsSet:1;
        ULONG SparseRangeArray:1;
        ULONG NoInternalLocking:1;
        ULONG SpareFlags:29;
    };
} RTL_SPARSE_BITMAP_CTX_FLAGS, *PRTL_SPARSE_BITMAP_CTX_FLAGS;
C_ASSERT(sizeof(RTL_SPARSE_BITMAP_CTX_FLAGS) == sizeof(ULONG));

typedef SRWLOCK RTL_SPARSE_BITMAP_LOCK;

typedef
_Check_return_
_Success_(return != 0)
PVOID
(NTAPI RTL_SPARSE_BITMAP_ALLOCATION_ROUTINE)(
    _In_ struct _RTL_SPARSE_BITMAP_CTX *Context,
    _In_ CLONG ByteSize
    );
typedef RTL_SPARSE_BITMAP_ALLOCATION_ROUTINE
      *PRTL_SPARSE_BITMAP_ALLOCATION_ROUTINE;

typedef
VOID
(NTAPI RTL_SPARSE_BITMAP_FREE_ROUTINE)(
    _In_ struct _RTL_SPARSE_BITMAP_CTX *Context,
    _In_ _Post_invalid_ PVOID Buffer
    );
typedef RTL_SPARSE_BITMAP_FREE_ROUTINE *PRTL_SPARSE_BITMAP_FREE_ROUTINE;

typedef struct _RTL_SPARSE_BITMAP_RANGE {
    RTL_SPARSE_BITMAP_LOCK Lock;
    struct _SINGLE_LIST_ENTRY Next;
    RTL_BITMAP RangeBitmap;
} RTL_SPARSE_BITMAP_RANGE, *PRTL_SPARSE_BITMAP_RANGE;
typedef RTL_SPARSE_BITMAP_RANGE **PPRTL_SPARSE_BITMAP_RANGE;

typedef struct _RTL_SPARSE_BITMAP_CTX {
    RTL_SPARSE_BITMAP_LOCK Lock;
    PPRTL_SPARSE_BITMAP_RANGE BitmapRanges;
    RTL_BITMAP RangeArrayCommitStatus;
    PRTL_SPARSE_BITMAP_ALLOCATION_ROUTINE AllocationRoutine;
    PRTL_SPARSE_BITMAP_FREE_ROUTINE FreeRoutine;

    ULONG RangeCount;
    ULONG RangeIndexLimit;
    ULONG RangeCountMax;
    ULONG RangeMetadataOffset;
    ULONG MetadataSizePerBit;

    ULONG Padding1;

    union {
        RTL_SPARSE_BITMAP_CTX_FLAGS Flags;
        struct _Struct_size_bytes_(sizeof(ULONG)) {
            ULONG DefaultBitsSet:1;
            ULONG SparseRangeArray:1;
            ULONG NoInternalLocking:1;
            ULONG SpareFlags:29;
        };
    };

    ULONG Padding2;

} RTL_SPARSE_BITMAP_CTX, *PRTL_SPARSE_BITMAP_CTX;
C_ASSERT(FIELD_OFFSET(RTL_SPARSE_BITMAP_CTX, BitmapRanges) == 8);
C_ASSERT(FIELD_OFFSET(RTL_SPARSE_BITMAP_CTX, AllocationRoutine) == 0x20);
C_ASSERT(FIELD_OFFSET(RTL_SPARSE_BITMAP_CTX, FreeRoutine) == 0x28);
C_ASSERT(FIELD_OFFSET(RTL_SPARSE_BITMAP_CTX, RangeCount) == 0x30);
C_ASSERT(FIELD_OFFSET(RTL_SPARSE_BITMAP_CTX, Flags) == 0x48);
C_ASSERT(sizeof(RTL_SPARSE_BITMAP_CTX) == 80);

//
// CRC32 and CRC64
//

typedef ULONG (NTAPI *PRTLCRC32)(
    _In_ const void *Buffer,
    _In_ size_t Size,
    _In_ ULONG InitialCrc
    );

typedef ULONGLONG (NTAPI *PRTLCRC64)(
    _In_ const void *Buffer,
    _In_ size_t Size,
    _In_ ULONGLONG InitialCrc
    );

//
// Tool Help
//

typedef struct tagTHREADENTRY32 *LPTHREADENTRY32;

typedef HANDLE (WINAPI * PCREATE_TOOLHELP32_SNAPSHOT)(
    DWORD dwFlags,
    DWORD th32ProcessID
    );

typedef BOOL (WINAPI *PTHREAD32_FIRST)(
    HANDLE hSnapshot,
    LPTHREADENTRY32 lpte
    );

typedef BOOL (WINAPI *PTHREAD32_NEXT)(
    HANDLE hSnapshot,
    LPTHREADENTRY32 lpte
    );

//
// Misc
//

typedef
_Check_return_
_Success_(return != 0)
BOOL
(CALLBACK INIT_ONCE_CALLBACK)(
    _Inout_     PINIT_ONCE InitOnce,
    _Inout_opt_ PVOID      Parameter,
    _Out_opt_   PVOID      *Context
    );
typedef INIT_ONCE_CALLBACK *PINIT_ONCE_CALLBACK;

typedef VOID (NTAPI *PRTL_PREFETCH_MEMORY_NON_TEMPORAL)(
    _In_ PVOID Source,
    _In_ SIZE_T Length
    );

typedef VOID (*PRTL_MOVE_MEMORY)(
    _Out_       VOID UNALIGNED *Destination,
    _In_  const VOID UNALIGNED *Source,
    _In_        SIZE_T          Length
    );

typedef PVOID (__cdecl *PRTL_COPY_MEMORY)(
    _Out_ PVOID   Destination,
    _In_  LPCVOID Source,
    _In_  SIZE_T  Size
    );

typedef NTSTATUS (NTAPI *PRTL_COPY_MAPPED_MEMORY)(
    _Out_ PVOID   Destination,
    _In_  LPCVOID Source,
    _In_  SIZE_T  Size
    );

typedef PVOID (__cdecl *PRTL_FILL_MEMORY)(
    _Out_ PVOID  Destination,
    _In_  INT    Value,
    _In_  SIZE_T Size
    );

typedef BOOLEAN (WINAPI *PRTL_LOCAL_TIME_TO_SYSTEM_TIME)(
    _In_    PLARGE_INTEGER  LocalTime,
    _In_    PLARGE_INTEGER  SystemTime
    );

typedef BOOLEAN (WINAPI *PRTL_TIME_TO_SECONDS_SINCE_1970)(
    _In_    PLARGE_INTEGER  Time,
    _Out_   PULONG          ElapsedSeconds
    );

typedef
PVOID
(MM_GET_MAXIMUM_FILE_SECTION_SIZE)(VOID);
typedef MM_GET_MAXIMUM_FILE_SECTION_SIZE *PMM_GET_MAXIMUM_FILE_SECTION_SIZE;

//
// Address Windowing Extensions
//

typedef
_Success_(return != FALSE)
BOOL
(WINAPI ALLOCATE_USER_PHYSICAL_PAGES)(
    _In_ HANDLE hProcess,
    _Inout_ PULONG_PTR NumberOfPages,
    _Out_writes_to_(*NumberOfPages, *NumberOfPages) PULONG_PTR PageArray
    );
typedef ALLOCATE_USER_PHYSICAL_PAGES *PALLOCATE_USER_PHYSICAL_PAGES;

typedef
_Success_(return != FALSE)
BOOL
(WINAPI ALLOCATE_USER_PHYSICAL_PAGES_NUMA)(
    _In_ HANDLE hProcess,
    _Inout_ PULONG_PTR NumberOfPages,
    _Out_writes_to_(*NumberOfPages, *NumberOfPages) PULONG_PTR PageArray,
    _In_ DWORD nndPreferred
    );
typedef ALLOCATE_USER_PHYSICAL_PAGES_NUMA *PALLOCATE_USER_PHYSICAL_PAGES_NUMA;

typedef
_Success_(return != FALSE)
BOOL
(WINAPI FREE_USER_PHYSICAL_PAGES)(
    _In_ HANDLE hProcess,
    _Inout_ PULONG_PTR NumberOfPages,
    _In_reads_(*NumberOfPages) PULONG_PTR PageArray
    );
typedef FREE_USER_PHYSICAL_PAGES *PFREE_USER_PHYSICAL_PAGES;

typedef
_Success_(return != FALSE)
BOOL
(WINAPI MAP_USER_PHYSYICAL_PAGES)(
    _In_ PVOID VirtualAddress,
    _In_ ULONG_PTR NumberOfPages,
    _In_reads_opt_(NumberOfPages) PULONG_PTR PageArray
    );
typedef MAP_USER_PHYSYICAL_PAGES *PMAP_USER_PHYSICAL_PAGES;

typedef
_Success_(return != FALSE)
BOOL
(WINAPI MAP_USER_PHYSYICAL_PAGES_SCATTER)(
    _In_ PVOID *VirtualAddresses,
    _In_ ULONG_PTR NumberOfPages,
    _In_reads_opt_(NumberOfPages) PULONG_PTR PageArray
    );
typedef MAP_USER_PHYSYICAL_PAGES_SCATTER *PMAP_USER_PHYSICAL_PAGES_SCATTER;

typedef
_Success_(return != FALSE)
BOOL
(WINAPI FREE_USER_PHYSICAL_PAGES)(
    _In_ HANDLE hProcess,
    _Inout_ PULONG_PTR NumberOfPages,
    _In_reads_(*NumberOfPages) PULONG_PTR PageArray
    );
typedef FREE_USER_PHYSICAL_PAGES *PFREE_USER_PHYSICAL_PAGES;

typedef
_Success_(return != FALSE)
BOOL
(WINAPI MAP_USER_PHYSYICAL_PAGES)(
    _In_ PVOID VirtualAddress,
    _In_ ULONG_PTR NumberOfPages,
    _In_reads_opt_(NumberOfPages) PULONG_PTR PageArray
    );
typedef MAP_USER_PHYSYICAL_PAGES *PMAP_USER_PHYSICAL_PAGES;

#ifdef _M_X64

#ifndef BitTestAndSet
#define BitTestAndSet _bittestandset
#endif

#define FastSetBit(Bitmap, BitNumber) (             \
    BitTestAndSet((PLONG)Bitmap->Buffer, BitNumber) \
)

#else

#define FastSetBit(Bitmap, BitNumber) ( \
    Rtl->RtlSetBit(Bitmap, BitNumber)   \
)

#endif

#undef RtlFillMemory
#undef RtlMoveMemory
#undef RtlCopyMemory

//
// Our RTL_PATH structure.
//

typedef enum _RTL_PATH_LINK_TYPE {

    //
    // No linkage is being used.
    //

    PathLinkTypeNone = 0,

    //
    // Linkage type is LIST_ENTRY.
    //

    PathLinkTypeListEntry,

    //
    // Linkage type is SLIST_ENTRY.
    //

    PathLinkTypeSListEntry,

    //
    // Linkage type is UNICODE_PREFIX_TABLE_ENTRY.
    //

    PathLinkTypePrefixTableEntry,

    //
    // Linkage type is RTL_DYNAMIC_HASH_TABLE_ENTRY.
    //

    PathLinkHashTableEntry

} RTL_PATH_LINK_TYPE, *PRTL_PATH_LINK_TYPE;

typedef struct _RTL_PATH_LINK {

    RTL_PATH_LINK_TYPE Type;                                    // 0    4   4

    //
    // Pad to an 16 byte boundary in order to ensure SLIST_ENTRY is aligned
    // properly.
    //

    ULONG Padding1[3];                                          // 4    12  16

    union {
        LIST_ENTRY ListEntry;                                   // 16   16  32
        DECLSPEC_ALIGN(16) SLIST_ENTRY SListEntry;              // 16   8   24
        struct _UNICODE_PREFIX_TABLE_ENTRY PrefixTableEntry;    // 16   56  72
        struct _RTL_DYNAMIC_HASH_TABLE_ENTRY HashTableEntry;    // 16   24  40
    };

} RTL_PATH_LINK, *PRTL_PATH_LINK;
C_ASSERT(FIELD_OFFSET(RTL_PATH_LINK, SListEntry) == 16);
C_ASSERT(sizeof(RTL_PATH_LINK) == 80);

typedef union _RTL_PATH_FLAGS {
    struct _Struct_size_bytes_(sizeof(ULONG)) {
        ULONG IsFile:1;
        ULONG IsDirectory:1;
        ULONG IsSymlink:1;
        ULONG IsFullyQualified:1;
        ULONG HasParent:1;
        ULONG HasChildren:1;

        //
        // When set, indicates that the path resides within the Windows
        // directory (e.g. "C:\Windows").  This includes the Windows directory
        // itself.
        //
        // N.B. The Windows directory is obtained via GetWindowsDirectory();
        //      if Windows was installed into the root directory of a drive,
        //      e.g. C:\, this bit will be set for every fully-qualified path.
        //
        // Invariants:
        //
        //      If WithinWindowsDirectory == TRUE:
        //          Assert IsFullyQualified == TRUE
        //

        ULONG WithinWindowsDirectory:1;

        //
        // When set, indicates that the path resides within the Windows
        // side-by-side assembly directory (e.g. "C:\Windows\WinSxS").  This
        // includes the WinSxS directory itself.
        //
        // N.B. This path is obtained by calling GetWindowsDirectory() and
        //      appending "WinSxS" to the returned path.
        //
        // Invariants:
        //
        //      If WithinWindowsSxSDirectory == TRUE:
        //          Assert IsFullyQualified == TRUE
        //

        ULONG WithinWindowsSxSDirectory:1;

        //
        // When set, indicates that the path resides within the Windows
        // system directory (e.g. "C:\Windows\system32").  This includes the
        // Windows sytem directory itself.
        //
        // Invariants:
        //
        //      If WithinWindowsSystemDirectory == TRUE:
        //          Assert IsFullyQualified == TRUE
        //

        ULONG WithinWindowsSystemDirectory:1;
    };
    LONG AsLong;
    ULONG AsULong;
} RTL_PATH_FLAGS;
C_ASSERT(sizeof(RTL_PATH_FLAGS) == sizeof(ULONG));
typedef RTL_PATH_FLAGS *PRTL_PATH_FLAGS;

typedef union _RTL_PATH_CREATE_FLAGS {
    struct _Struct_size_bytes_(sizeof(ULONG)) {
        ULONG CheckType:1;
        ULONG EnsureQualified:1;
        ULONG Unused:30;
    };
    LONG AsLong;
    ULONG AsULong;
} RTL_PATH_CREATE_FLAGS;
C_ASSERT(sizeof(RTL_PATH_CREATE_FLAGS) == sizeof(ULONG));
typedef RTL_PATH_CREATE_FLAGS *PRTL_PATH_CREATE_FLAGS;

typedef enum _RTL_PATH_TYPE_INTENT {

    PathTypeDontCare = 0,
    PathTypeLookup,
    PathTypeKnownFile,
    PathTypeKnownSymlink,
    PathTypeKnownDirectory

} RTL_PATH_TYPE_INTENT;

typedef struct _Struct_size_bytes_(StructSize) _RTL_PATH {

    //
    // Size of the structure, in bytes.
    //

    _Field_range_(==, sizeof(struct _RTL_PATH)) USHORT StructSize; // 0    2   2

    //
    // Drive letter.
    //

    WCHAR Drive;                                                // 2    2   4

    //
    // Number of slashes and dots.  Saves having to do RtlNumberOfSetBits().
    //

    USHORT NumberOfSlashes;                                     // 4    2   6
    USHORT NumberOfDots;                                        // 6    2   8

    //
    // We get a free ULONG slot in each RTL_BITMAP structure due to the buffer
    // pointer needing to be 8 bytes aligned.  Leverage these to store AllocSize
    // and Flags.
    //

    union {
        struct {
            ULONG SizeOfReversedSlashesBitMap;                  // 8    4   12

            //
            // Total number of bytes allocated for the structure, including
            // StructSize.  This includes the bitmap buffers and Unicode string
            // buffer (all of which will typically trail this structure in
            // memory).
            //

            ULONG AllocSize;                                    // 12   4   16
            PULONG ReversedSlashesBitMapBuffer;                 // 16   8   24
        };
        RTL_BITMAP ReversedSlashesBitmap;                       // 8    16  24
    };

    union {
        struct {
            ULONG SizeOfReversedDotsBitMap;                     // 24   4   28
            RTL_PATH_FLAGS Flags;                               // 28   4   32
            PULONG ReversedDotsBitMapBuffer;                    // 32   8   40
        };
        RTL_BITMAP ReversedDotsBitmap;                          // 24   16  40
    };

    //
    // Optional allocator used to allocate this structure.
    //

    PALLOCATOR Allocator;                                       // 40   8   48

    //
    // Unicode strings for path details.
    //

    UNICODE_STRING Full;                                        // 48   16  64
    UNICODE_STRING Name;                                        // 64   16  80
    UNICODE_STRING Directory;                                   // 80   16  96
    UNICODE_STRING Extension;                                   // 96   16  112

    //
    // Path linkage.
    //

    RTL_PATH_LINK Link;                                         // 112  80  192

} RTL_PATH;
typedef RTL_PATH *PRTL_PATH;
typedef RTL_PATH **PPRTL_PATH;
C_ASSERT(FIELD_OFFSET(RTL_PATH, ReversedSlashesBitmap) == 8);
C_ASSERT(FIELD_OFFSET(RTL_PATH, ReversedDotsBitmap) == 24);
C_ASSERT(FIELD_OFFSET(RTL_PATH, Allocator) == 40);
C_ASSERT(FIELD_OFFSET(RTL_PATH, Full) == 48);
C_ASSERT(FIELD_OFFSET(RTL_PATH, Name) == 64);
C_ASSERT(FIELD_OFFSET(RTL_PATH, Link) == 112);
C_ASSERT(sizeof(RTL_PATH) == 192);

typedef
_Success_(return != 0)
BOOL
(UNICODE_STRING_TO_RTL_PATH)(
    _In_ PRTL Rtl,
    _In_ PUNICODE_STRING String,
    _In_ PALLOCATOR Allocator,
    _Out_ PPRTL_PATH PathPointer
    );
typedef UNICODE_STRING_TO_RTL_PATH *PUNICODE_STRING_TO_RTL_PATH;

typedef
_Success_(return != 0)
BOOL
(STRING_TO_RTL_PATH)(
    _In_ PRTL Rtl,
    _In_ PSTRING String,
    _In_ PALLOCATOR Allocator,
    _Out_ PPRTL_PATH PathPointer
    );
typedef STRING_TO_RTL_PATH *PSTRING_TO_RTL_PATH;

typedef
_Success_(return != 0)
BOOL
(STRING_TO_EXISTING_RTL_PATH)(
    _In_ PRTL Rtl,
    _In_ PSTRING AnsiString,
    _In_ PALLOCATOR BitmapAllocator,
    _In_ PALLOCATOR UnicodeStringBufferAllocator,
    _In_ PRTL_PATH Path,
    _In_ PLARGE_INTEGER TimestampPointer
    );
typedef STRING_TO_EXISTING_RTL_PATH *PSTRING_TO_EXISTING_RTL_PATH;

typedef
_Success_(return != 0)
BOOL
(UNICODE_STRING_TO_EXISTING_RTL_PATH)(
    _In_ PRTL Rtl,
    _In_ PUNICODE_STRING UnicodeString,
    _In_ PALLOCATOR BitmapAllocator,
    _In_ PALLOCATOR UnicodeStringBufferAllocator,
    _In_ PRTL_PATH Path,
    _In_ PLARGE_INTEGER TimestampPointer
    );
typedef UNICODE_STRING_TO_EXISTING_RTL_PATH \
      *PUNICODE_STRING_TO_EXISTING_RTL_PATH;

typedef
_Success_(return != 0)
BOOL
(DESTROY_RTL_PATH)(
    _Inout_opt_ PPRTL_PATH PathPointer
    );
typedef DESTROY_RTL_PATH *PDESTROY_RTL_PATH;

typedef
_Success_(return != 0)
BOOL
(GET_MODULE_RTL_PATH)(
    _In_ PRTL Rtl,
    _In_ HMODULE Module,
    _In_ PALLOCATOR Allocator,
    _Out_ PPRTL_PATH PathPointer
    );
typedef GET_MODULE_RTL_PATH *PGET_MODULE_RTL_PATH;

//
// Our RTL_FILE structure.
//

//
// Flags for the RTL_FILE structure.
//

typedef union _RTL_FILE_FLAGS {
    struct _Struct_size_bytes_(sizeof(ULONG)) {

        ULONG Valid:1;

        //
        // Which copy method was used to copy the source file contents into the
        // destination.
        //

        ULONG Avx2Copy:1;
        ULONG MovsqCopy:1;
    };
    LONG AsLong;
    ULONG AsULong;
} RTL_FILE_FLAGS, *PRTL_FILE_FLAGS;

typedef enum _RTL_FILE_TYPE {
    RtlFileNullType = 0,
    RtlFileTextFileType = 1,
    RtlFileImageFileType,
    RtlFileInvalidType
} RTL_FILE_TYPE, *PRTL_FILE_TYPE;

//
// A structure for capturing details about a text file.  This is used for source
// code files.
//

typedef struct _Struct_size_bytes_(StructSizeInBytes) _RTL_TEXT_FILE {

    //
    // Size of the structure, in bytes.
    //

    _Field_range_(==, sizeof(struct _RTL_TEXT_FILE)) ULONG StructSizeInBytes;

    //
    // Number of lines in the file.
    //

    ULONG NumberOfLines;

    //
    // An array of STRING structures, one for each line in the file.
    // Number of elements in the array is governed by NumberOfLines.
    //

    PSTRING Lines;

    //
    // Line ending bitmaps.
    //

    PRTL_BITMAP CarriageReturnBitmap;
    PRTL_BITMAP LineFeedBitmap;

    //
    // The two bitmaps above are combined (literally AND'd together) to
    // create the following line ending bitmap, where each set bit
    // indicates a line ending character.
    //

    PRTL_BITMAP LineEndingBitmap;

    //
    // This bitmap is then inverted, such that each set bit indicates a
    // normal non-line-ending character.
    //

    PRTL_BITMAP LineBitmap;

    //
    // Bitmaps for capturing whitespace/tabs.
    //

    PRTL_BITMAP WhitespaceBitmap;
    PRTL_BITMAP TabBitmap;

    //
    // Set bits correlate to whitespace/tabs that indicate indentation;
    // i.e.  longest run of space/tab after the last line-ending bit.
    //

    PRTL_BITMAP IndentBitmap;

    //
    // Set bits correlate to trailing whitespace.
    //

    PRTL_BITMAP TrailingWhitespaceBitmap;

    //
    // (80 bytes consumed.)
    //

} RTL_TEXT_FILE, *PRTL_TEXT_FILE;
C_ASSERT(sizeof(RTL_TEXT_FILE) == 80);

#define NUMBER_OF_RTL_TEXT_FILE_BITMAPS 8

//
// This structure is used to capture information about image files (e.g. dll,
// exe).

typedef struct _RTL_IMAGE_FILE {

    //
    // Preferred base address, if set.
    //

    PVOID PreferredBaseAddress;

    //
    // Actual base address the module was loaded at.
    //

    PVOID BaseAddress;

    //
    // Pointer to the entry point of the image if applicable.  This will be
    // relative to the loaded BaseAddress.
    //

    PVOID EntryPoint;

    //
    // (24 bytes consumed.)
    //

    //
    // Size of the image, in bytes.
    //

    ULONG SizeOfImage;

    //
    // HeaderSum and CheckSum are filled in after CheckSumMappedFile() is
    // called.
    //

    ULONG HeaderSum;
    ULONG CheckSum;

    //
    // Timestamp receives the return value from GetTimestampForLoadedLibrary().
    //

    ULONG Timestamp;

    //
    // (40 bytes consumed.)
    //

    //
    // If a PDB file could be located, this will point to it.
    //

    PRTL_PATH PdbPath;

    //
    // Captures the module handle.
    //

    HMODULE ModuleHandle;

    //
    // (56 bytes consumed.)
    //

    //
    // Pointer to an IMAGEHLP_MODULE64 structure if symbol tracing is enabled
    // and symbols were successfully loaded.
    //

    PIMAGEHLP_MODULEW64 ModuleInfo;

} RTL_IMAGE_FILE, *PRTL_IMAGE_FILE;
C_ASSERT(FIELD_OFFSET(RTL_IMAGE_FILE, SizeOfImage) == 24);
C_ASSERT(FIELD_OFFSET(RTL_IMAGE_FILE, ModuleInfo) == 56);
C_ASSERT(sizeof(RTL_IMAGE_FILE) == 64);

//
// This structure is used to capture information about a file that participates
// in a trace session some how (e.g. text source code, binary image, etc).
//

typedef struct _RTL_FILE {

    //
    // Singly-linked list entry allowing the record to be pushed and popped
    // onto interlocked lists.
    //

    union {
        SLIST_ENTRY ListEntry;
        struct {
            PSLIST_ENTRY Next;
            PVOID Unused;
        };
    };

    //
    // File information.
    //

    //
    // Inline FILE_BASIC_INFO.
    //

    union {
        FILE_BASIC_INFO BasicInfo;
        struct {
            LARGE_INTEGER CreationTime;
            LARGE_INTEGER LastAccessTime;
            LARGE_INTEGER LastWriteTime;
            LARGE_INTEGER ChangeTime;
            DWORD FileAttributes;

            //
            // Stash the number of pages here as we get a free DWORD/ULONG in
            // order to maintain alignment.  Note that this isn't part of the
            // FILE_BASIC_INFO structure.
            //

            ULONG NumberOfPages;
        };
    };

    LARGE_INTEGER EndOfFile;
    LARGE_INTEGER AllocationSize;

    //
    // (72 bytes consumed.)
    //

    //
    // File ID related information.
    //

    LARGE_INTEGER FileId;

    //
    // Inline FILE_ID_INFO.
    //

    union {
        FILE_ID_INFO FileIdInfo;
        struct {
            ULONGLONG VolumeSerialNumber;
            FILE_ID_128 FileId128;
        };
    };

    //
    // (104 bytes consumed.)
    //

    //
    // Checksums.
    //

    BYTE MD5[16];
    BYTE SHA1[20];

    //
    // (116 bytes consumed.)
    //

    //
    // Define type and flags.
    //

    RTL_FILE_TYPE Type;
    RTL_FILE_FLAGS Flags;

    //
    // Pad to 8 byte boundary with an elapsed field that captures ticks required
    // to copy the file from the memory mapping to our allocated memory.
    //

    ULONG Elapsed;

    //
    // A pointer to the first byte of data for the file, once it has been
    // loaded.
    //

    PCHAR Content;

    //
    // Capture all path related information via Rtl's RTL_PATH structure.
    //

    RTL_PATH Path;

    //
    // (360 bytes consumed.)
    //

    //
    // Additional information about the loading speed.
    //

    LARGE_INTEGER CopyTimeInMicroseconds;
    LARGE_INTEGER CopiedBytesPerSecond;

    //
    // (376 bytes consumed.)
    //

    union {

        //
        // Source code specific structure.
        //

        RTL_TEXT_FILE SourceCode;

        //
        // Image file specific structure.
        //

        RTL_IMAGE_FILE ImageFile;

        //
        // Pad out to 480 bytes such that we've got 32 bytes at the end of the
        // structure spare.
        //

        BYTE Reserved[112];
    };

    //
    // (480 bytes consumed.)
    //

    //
    // These will have values as long as the handles are open.
    //

    HANDLE FileHandle;
    HANDLE MappingHandle;
    PVOID MappedAddress;

    //
    // Pad out to a final 512 bytes.
    //

    BYTE Reserved2[8];

} RTL_FILE, *PRTL_FILE, **PPRTL_FILE;
C_ASSERT(FIELD_OFFSET(RTL_FILE, CreationTime) == 16);
C_ASSERT(FIELD_OFFSET(RTL_FILE, EndOfFile) == 56);
C_ASSERT(FIELD_OFFSET(RTL_FILE, FileId) == 72);
C_ASSERT(FIELD_OFFSET(RTL_FILE, FileIdInfo) == 80);
C_ASSERT(FIELD_OFFSET(RTL_FILE, MD5) == 104);
C_ASSERT(FIELD_OFFSET(RTL_FILE, SHA1) == 120);
C_ASSERT(FIELD_OFFSET(RTL_FILE, Type) == 140);
C_ASSERT(FIELD_OFFSET(RTL_FILE, Flags) == 144);
C_ASSERT(FIELD_OFFSET(RTL_FILE, Elapsed) == 148);
C_ASSERT(FIELD_OFFSET(RTL_FILE, Content) == 152);
C_ASSERT(FIELD_OFFSET(RTL_FILE, Path) == 160);
C_ASSERT(FIELD_OFFSET(RTL_FILE, CopyTimeInMicroseconds) == 352);
C_ASSERT(FIELD_OFFSET(RTL_FILE, SourceCode) == 368);
C_ASSERT(FIELD_OFFSET(RTL_FILE, FileHandle) == 480);
C_ASSERT(FIELD_OFFSET(RTL_FILE, Reserved2) == 504);
C_ASSERT(sizeof(RTL_FILE) == 512);

//
// Inline functions for pushing/popping RTL_FILE structures to/from interlocked
// singly-linked list heads.
//

FORCEINLINE
_Check_return_
_Success_(return != 0)
BOOL
PopRtlFile(
    _In_  PSLIST_HEADER ListHead,
    _Out_ PPRTL_FILE File
    )
{
    PSLIST_ENTRY ListEntry;

    ListEntry = InterlockedPopEntrySList(ListHead);
    if (!ListEntry) {
        return FALSE;
    }

    *File = CONTAINING_RECORD(ListEntry,
                              RTL_FILE,
                              ListEntry);

    return TRUE;
}

FORCEINLINE
VOID
PushRtlFile(
    _In_ PSLIST_HEADER ListHead,
    _In_ PRTL_FILE File
    )
{
    InterlockedPushEntrySList(ListHead, &File->ListEntry);
}

//
// This structure provides bitfields that customize the behavior of the routine
// InitializeRtlFile().
//

typedef union _RTL_FILE_INIT_FLAGS {
    ULONG AsLong;
    struct _Struct_size_bytes_(sizeof(ULONG)) {
        ULONG IsSourceCode:1;
        ULONG IsImageFile:1;
        ULONG InitPath:1;
        ULONG CopyContents:1;
        ULONG CopyViaMovsq:1;
        ULONG KeepViewMapped:1;
        ULONG KeepFileHandleOpen:1;
        ULONG KeepMappingHandleOpen:1;
    };
} RTL_FILE_INIT_FLAGS, *PRTL_FILE_INIT_FLAGS;
C_ASSERT(sizeof(RTL_FILE_INIT_FLAGS) == sizeof(ULONG));

typedef
_Check_return_
_Success_(return != 0)
BOOL
(INITIALIZE_RTL_FILE)(
    _In_ PRTL Rtl,
    _In_opt_ PUNICODE_STRING UnicodeStringPath,
    _In_opt_ PSTRING AnsiStringPath,
    _In_opt_ PALLOCATOR BitmapAllocator,
    _In_opt_ PALLOCATOR UnicodeStringBufferAllocator,
    _In_opt_ PALLOCATOR FileContentsAllocator,
    _In_opt_ PALLOCATOR LineAllocator,
    _In_opt_ PALLOCATOR RtlFileAllocator,
    _In_ RTL_FILE_INIT_FLAGS InitFlags,
    _Inout_opt_ PPRTL_FILE FilePointer,
    _In_ PLARGE_INTEGER Timestamp
    );
typedef INITIALIZE_RTL_FILE *PINITIALIZE_RTL_FILE;

#define _RTLFUNCTIONS_HEAD                                                                             \
    PRTL_CHAR_TO_INTEGER RtlCharToInteger;                                                             \
    PRTL_INITIALIZE_GENERIC_TABLE RtlInitializeGenericTable;                                           \
    PRTL_INSERT_ELEMENT_GENERIC_TABLE RtlInsertElementGenericTable;                                    \
    PRTL_INSERT_ELEMENT_GENERIC_TABLE_FULL RtlInsertElementGenericTableFull;                           \
    PRTL_DELETE_ELEMENT_GENERIC_TABLE RtlDeleteElementGenericTable;                                    \
    PRTL_LOOKUP_ELEMENT_GENERIC_TABLE RtlLookupElementGenericTable;                                    \
    PRTL_LOOKUP_ELEMENT_GENERIC_TABLE_FULL RtlLookupElementGenericTableFull;                           \
    PRTL_ENUMERATE_GENERIC_TABLE RtlEnumerateGenericTable;                                             \
    PRTL_ENUMERATE_GENERIC_TABLE_WITHOUT_SPLAYING RtlEnumerateGenericTableWithoutSplaying;             \
    PRTL_GET_ELEMENT_GENERIC_TABLE RtlGetElementGenericTable;                                          \
    PRTL_NUMBER_GENERIC_TABLE_ELEMENTS RtlNumberGenericTableElements;                                  \
    PRTL_IS_GENERIC_TABLE_EMPTY RtlIsGenericTableEmpty;                                                \
    PRTL_INITIALIZE_GENERIC_TABLE_AVL RtlInitializeGenericTableAvl;                                    \
    PRTL_INSERT_ELEMENT_GENERIC_TABLE_AVL RtlInsertElementGenericTableAvl;                             \
    PRTL_INSERT_ELEMENT_GENERIC_TABLE_FULL_AVL RtlInsertElementGenericTableFullAvl;                    \
    PRTL_DELETE_ELEMENT_GENERIC_TABLE_AVL RtlDeleteElementGenericTableAvl;                             \
    PRTL_LOOKUP_ELEMENT_GENERIC_TABLE_AVL RtlLookupElementGenericTableAvl;                             \
    PRTL_LOOKUP_ELEMENT_GENERIC_TABLE_FULL_AVL RtlLookupElementGenericTableFullAvl;                    \
    PRTL_ENUMERATE_GENERIC_TABLE_AVL RtlEnumerateGenericTableAvl;                                      \
    PRTL_ENUMERATE_GENERIC_TABLE_WITHOUT_SPLAYING_AVL RtlEnumerateGenericTableWithoutSplayingAvl;      \
    PRTL_LOOKUP_FIRST_MATCHING_ELEMENT_GENERIC_TABLE_AVL RtlLookupFirstMatchingElementGenericTableAvl; \
    PRTL_ENUMERATE_GENERIC_TABLE_LIKE_A_DICTIONARY RtlEnumerateGenericTableLikeADirectory;             \
    PRTL_GET_ELEMENT_GENERIC_TABLE_AVL RtlGetElementGenericTableAvl;                                   \
    PRTL_NUMBER_GENERIC_TABLE_ELEMENTS_AVL RtlNumberGenericTableElementsAvl;                           \
    PRTL_IS_GENERIC_TABLE_EMPTY_AVL RtlIsGenericTableEmptyAvl;                                         \
    PPFX_INITIALIZE PfxInitialize;                                                                     \
    PPFX_INSERT_PREFIX PfxInsertPrefix;                                                                \
    PPFX_REMOVE_PREFIX PfxRemovePrefix;                                                                \
    PPFX_FIND_PREFIX PfxFindPrefix;                                                                    \
    PRTL_PREFIX_STRING RtlPrefixString;                                                                \
    PRTL_PREFIX_UNICODE_STRING RtlPrefixUnicodeString;                                                 \
    PRTL_CREATE_HASH_TABLE RtlCreateHashTable;                                                         \
    PRTL_DELETE_HASH_TABLE RtlDeleteHashTable;                                                         \
    PRTL_INSERT_ENTRY_HASH_TABLE RtlInsertEntryHashTable;                                              \
    PRTL_REMOVE_ENTRY_HASH_TABLE RtlRemoveEntryHashTable;                                              \
    PRTL_LOOKUP_ENTRY_HASH_TABLE RtlLookupEntryHashTable;                                              \
    PRTL_GET_NEXT_ENTRY_HASH_TABLE RtlGetNextEntryHashTable;                                           \
    PRTL_ENUMERATE_ENTRY_HASH_TABLE RtlEnumerateEntryHashTable;                                        \
    PRTL_END_ENUMERATION_HASH_TABLE RtlEndEnumerationHashTable;                                        \
    PRTL_INIT_WEAK_ENUMERATION_HASH_TABLE RtlInitWeakEnumerationHashTable;                             \
    PRTL_WEAKLY_ENUMERATE_ENTRY_HASH_TABLE RtlWeaklyEnumerateEntryHashTable;                           \
    PRTL_END_WEAK_ENUMERATION_HASH_TABLE RtlEndWeakEnumerationHashTable;                               \
    PRTL_EXPAND_HASH_TABLE RtlExpandHashTable;                                                         \
    PRTL_CONTRACT_HASH_TABLE RtlContractHashTable;                                                     \
    PRTL_INITIALIZE_BITMAP RtlInitializeBitMap;                                                        \
    PRTL_CLEAR_BIT RtlClearBit;                                                                        \
    PRTL_SET_BIT RtlSetBit;                                                                            \
    PRTL_TEST_BIT RtlTestBit;                                                                          \
    PRTL_CLEAR_ALL_BITS RtlClearAllBits;                                                               \
    PRTL_SET_ALL_BITS RtlSetAllBits;                                                                   \
    PRTL_FIND_CLEAR_BITS RtlFindClearBits;                                                             \
    PRTL_FIND_SET_BITS RtlFindSetBits;                                                                 \
    PRTL_FIND_CLEAR_BITS_AND_SET RtlFindClearBitsAndSet;                                               \
    PRTL_FIND_SET_BITS_AND_CLEAR RtlFindSetBitsAndClear;                                               \
    PRTL_CLEAR_BITS RtlClearBits;                                                                      \
    PRTL_SET_BITS RtlSetBits;                                                                          \
    PRTL_FIND_CLEAR_RUNS RtlFindClearRuns;                                                             \
    PRTL_FIND_LONGEST_RUN_CLEAR RtlFindLongestRunClear;                                                \
    PRTL_NUMBER_OF_CLEAR_BITS RtlNumberOfClearBits;                                                    \
    PRTL_NUMBER_OF_SET_BITS RtlNumberOfSetBits;                                                        \
    PRTL_ARE_BITS_CLEAR RtlAreBitsClear;                                                               \
    PRTL_ARE_BITS_SET RtlAreBitsSet;                                                                   \
    PRTL_FIND_FIRST_RUN_CLEAR RtlFindFirstRunClear;                                                    \
    PRTL_FIND_NEXT_FORWARD_RUN_CLEAR RtlFindNextForwardRunClear;                                       \
    PRTL_FIND_LAST_BACKWARD_RUN_CLEAR RtlFindLastBackwardRunClear;                                     \
    PRTL_INITIALIZE_UNICODE_PREFIX RtlInitializeUnicodePrefix;                                         \
    PRTL_INSERT_UNICODE_PREFIX RtlInsertUnicodePrefix;                                                 \
    PRTL_REMOVE_UNICODE_PREFIX RtlRemoveUnicodePrefix;                                                 \
    PRTL_FIND_UNICODE_PREFIX RtlFindUnicodePrefix;                                                     \
    PRTL_NEXT_UNICODE_PREFIX RtlNextUnicodePrefix;                                                     \
    PRTL_COPY_UNICODE_STRING RtlCopyUnicodeString;                                                     \
    PRTL_INIT_STRING RtlInitString;                                                                    \
    PRTL_COPY_STRING RtlCopyString;                                                                    \
    PRTL_APPEND_UNICODE_TO_STRING RtlAppendUnicodeToString;                                            \
    PRTL_APPEND_UNICODE_STRING_TO_STRING RtlAppendUnicodeStringToString;                               \
    PRTL_UNICODE_STRING_TO_ANSI_SIZE RtlUnicodeStringToAnsiSize;                                       \
    PRTL_UNICODE_STRING_TO_ANSI_STRING RtlUnicodeStringToAnsiString;                                   \
    PRTL_EQUAL_STRING RtlEqualString;                                                                  \
    PRTL_EQUAL_UNICODE_STRING RtlEqualUnicodeString;                                                   \
    PRTL_COMPARE_STRING RtlCompareString;                                                              \
    PRTL_COMPARE_MEMORY RtlCompareMemory;                                                              \
    PRTL_PREFETCH_MEMORY_NON_TEMPORAL RtlPrefetchMemoryNonTemporal;                                    \
    PRTL_MOVE_MEMORY RtlMoveMemory;                                                                    \
    PRTL_COPY_MEMORY RtlCopyMemory;                                                                    \
    PRTL_COPY_MAPPED_MEMORY RtlCopyMappedMemory;                                                       \
    PRTL_FILL_MEMORY RtlFillMemory;                                                                    \
    PRTL_LOCAL_TIME_TO_SYSTEM_TIME RtlLocalTimeToSystemTime;                                           \
    PRTL_TIME_TO_SECONDS_SINCE_1970 RtlTimeToSecondsSince1970;                                         \
    PBSEARCH bsearch;                                                                                  \
    PQSORT qsort;                                                                                      \
    PMEMSET memset;                                                                                    \
    PSET_CONSOLE_CTRL_HANDLER SetConsoleCtrlHandler;                                                   \
    PMM_GET_MAXIMUM_FILE_SECTION_SIZE MmGetMaximumFileSectionSize;                                     \
    PGET_PROCESS_MEMORY_INFO K32GetProcessMemoryInfo;                                                  \
    PGET_PERFORMANCE_INFO K32GetPerformanceInfo;                                                       \
    PGET_PROCESS_IO_COUNTERS GetProcessIoCounters;                                                     \
    PGET_PROCESS_HANDLE_COUNT GetProcessHandleCount;                                                   \
    PINITIALIZE_PROCESS_FOR_WS_WATCH K32InitializeProcessForWsWatch;                                   \
    PGET_WS_CHANGES K32GetWsChanges;                                                                   \
    PGET_WS_CHANGES_EX K32GetWsChangesEx;                                                              \
    PQUERY_WORKING_SET K32QueryWorkingSet;                                                             \
    PQUERY_WORKING_SET_EX K32QueryWorkingSetEx;                                                        \
    PZW_QUERY_INFORMATION_PROCESS ZwQueryInformationProcess;                                           \
    PLDR_REGISTER_DLL_NOTIFICATION LdrRegisterDllNotification;                                         \
    PLDR_UNREGISTER_DLL_NOTIFICATION LdrUnregisterDllNotification;                                     \
    PLDR_LOCK_LOADER_LOCK LdrLockLoaderLock;                                                           \
    PLDR_UNLOCK_LOADER_LOCK LdrUnlockLoaderLock;                                                       \
    PZW_ALLOCATE_VIRTUAL_MEMORY ZwAllocateVirtualMemory;                                               \
    PZW_FREE_VIRTUAL_MEMORY ZwFreeVirtualMemory;                                                       \
    PALLOCATE_USER_PHYSICAL_PAGES AllocateUserPhysicalPages;                                           \
    PALLOCATE_USER_PHYSICAL_PAGES_NUMA AllocateUserPhysicalPagesNuma;                                  \
    PMAP_USER_PHYSICAL_PAGES MapUserPhysicalPages;                                                     \
    PMAP_USER_PHYSICAL_PAGES_SCATTER MapUserPhysicalPagesScatter;                                      \
    PFREE_USER_PHYSICAL_PAGES FreeUserPhysicalPages;                                                   \
    PRTL_CREATE_HEAP RtlCreateHeap;                                                                    \
    PRTL_DESTROY_HEAP RtlDestroyHeap;                                                                  \
    PRTL_ALLOCATE_HEAP RtlAllocateHeap;                                                                \
    PRTL_FREE_HEAP RtlFreeHeap;                                                                        \
    PRTL_CAPTURE_STACK_BACK_TRACE RtlCaptureStackBackTrace;                                            \
    PRTL_PC_TO_FILE_HEADER RtlPcToFileHeader;                                                          \
    PRTL_LOOKUP_FUNCTION_ENTRY RtlLookupFunctionEntry;                                                 \
    PRTL_ADD_FUNCTION_TABLE RtlAddFunctionTable;                                                       \
    PRTL_DELETE_FUNCTION_TABLE RtlDeleteFunctionTable;                                                 \
    PRTL_INSTALL_FUNCTION_TABLE_CALLBACK RtlInstallFunctionTableCallback;                              \
    PRTL_ADD_GROWABLE_FUNCTION_TABLE RtlAddGrowableFunctionTable;                                      \
    PRTL_GROW_FUNCTION_TABLE RtlGrowFunctionTable;                                                     \
    PRTL_DELETE_GROWABLE_FUNCTION_TABLE RtlDeleteGrowableFunctionTable;                                \
    PZW_CREATE_SECTION ZwCreateSection;                                                                \
    PZW_MAP_VIEW_OF_SECTION ZwMapViewOfSection;                                                        \
    PZW_UNMAP_VIEW_OF_SECTION ZwUnmapViewOfSection;                                                    \
    PZW_CREATE_PROCESS ZwCreateProcess;                                                                \
    PZW_CREATE_PROCESS_EX ZwCreateProcessEx;                                                           \
    PZW_CREATE_THREAD ZwCreateThread;                                                                  \
    PZW_OPEN_THREAD ZwOpenThread;                                                                      \
    PZW_TERMINATE_THREAD ZwTerminateThread;                                                            \
    PNT_QUEUE_APC_THREAD NtQueueApcThread;                                                             \
    PNT_TEST_ALERT NtTestAlert;                                                                        \
    PNT_YIELD_EXECUTION NtYieldExecution;                                                              \
    PNT_DELAY_EXECUTION NtDelayExecution;                                                              \
    PSEARCHPATHW SearchPathW;                                                                          \
    PCREATE_TOOLHELP32_SNAPSHOT CreateToolhelp32Snapshot;                                              \
    PLOAD_LIBRARY_A LoadLibraryA;                                                                      \
    PLOAD_LIBRARY_W LoadLibraryW;                                                                      \
    PLOAD_LIBRARY_EX_W LoadLibraryExW;                                                                 \
    PGET_PROC_ADDRESS GetProcAddress;                                                                  \
    PCLOSE_HANDLE CloseHandle;                                                                         \
    POPEN_EVENT_A OpenEventA;                                                                          \
    POPEN_EVENT_W OpenEventW;                                                                          \
    PGET_LAST_ERROR GetLastError;                                                                      \
    PSET_LAST_ERROR SetLastError;                                                                      \
    PSET_EVENT SetEvent;                                                                               \
    PRESET_EVENT ResetEvent;                                                                           \
    PSUSPEND_THREAD SuspendThread;                                                                     \
    PRESUME_THREAD ResumeThread;                                                                       \
    PGET_THREAD_CONTEXT GetThreadContext;                                                              \
    PSET_THREAD_CONTEXT SetThreadContext;                                                              \
    PWAIT_FOR_SINGLE_OBJECT WaitForSingleObject;                                                       \
    PWAIT_FOR_SINGLE_OBJECT_EX WaitForSingleObjectEx;                                                  \
    PSIGNAL_OBJECT_AND_WAIT SignalObjectAndWait;                                                       \
    PSLEEP_EX SleepEx;                                                                                 \
    PEXIT_THREAD ExitThread;                                                                           \
    PGET_EXIT_CODE_THREAD GetExitCodeThread;                                                           \
    PTERMINATE_THREAD TerminateThread;                                                                 \
    PQUEUE_USER_APC QueueUserAPC;                                                                      \
    PDEVICE_IO_CONTROL DeviceIoControl;                                                                \
    PGET_MODULE_HANDLE_A GetModuleHandleA;                                                             \
    PGET_MODULE_HANDLE_W GetModuleHandleW;                                                             \
    PCREATE_FILE_W CreateFileW;                                                                        \
    PCREATE_FILE_MAPPING_W CreateFileMappingW;                                                         \
    POPEN_FILE_MAPPING_W OpenFileMappingW;                                                             \
    PMAP_VIEW_OF_FILE_EX MapViewOfFileEx;                                                              \
    PFLUSH_VIEW_OF_FILE FlushViewOfFile;                                                               \
    PUNMAP_VIEW_OF_FILE_EX UnmapViewOfFileEx;                                                          \
    PVIRTUAL_ALLOC_EX VirtualAllocEx;                                                                  \
    PVIRTUAL_FREE_EX VirtualFreeEx;                                                                    \
    PVIRTUAL_PROTECT_EX VirtualProtectEx;                                                              \
    PVIRTUAL_QUERY_EX VirtualQueryEx;                                                                  \
    PDEBUG_ACTIVE_PROCESS_STOP DebugActiveProcessStop;                                                 \
    PDEBUG_SET_PROCESS_KILL_ON_EXIT DebugSetProcessKillOnExit;                                         \
    PTHREAD32_FIRST Thread32First;                                                                     \
    PTHREAD32_NEXT Thread32Next;

typedef struct _RTLFUNCTIONS {
    _RTLFUNCTIONS_HEAD
} RTLFUNCTIONS, *PRTLFUNCTIONS, **PPRTLFUNCTIONS;

// Win 8
typedef ULONG (NTAPI *PRTL_NUMBER_OF_CLEAR_BITS_IN_RANGE)(
    _In_ PRTL_BITMAP BitMapHeader,
    _In_ ULONG StartingIndex,
    _In_ ULONG Length
    );

// Win 8
typedef ULONG (NTAPI *PRTL_NUMBER_OF_SET_BITS_IN_RANGE)(
    _In_ PRTL_BITMAP BitMapHeader,
    _In_ ULONG StartingIndex,
    _In_ ULONG Length
    );

#define _RTLFUNCTIONS8_HEAD                                                      \
    PRTLCRC32 RtlCrc32;                                                          \
    PRTLCRC64 RtlCrc64;                                                          \
    PRTL_STRONGLY_ENUMERATE_ENTRY_HASH_TABLE RtlStronglyEnumerateEntryHashTable; \
    PRTL_END_STRONG_ENUMERATION_HASH_TABLE RtlEndStrongEnumerationHashTable;     \
    PRTL_INIT_STRONG_ENUMERATION_HASH_TABLE RtlInitStrongEnumerationHashTable;   \
    PRTL_CREATE_HASH_TABLE_EX RtlCreateHashTableEx;                              \
    PRTL_NUMBER_OF_CLEAR_BITS_IN_RANGE RtlNumberOfClearBitsInRange;              \
    PRTL_NUMBER_OF_SET_BITS_IN_RANGE RtlNumberOfSetBitsInRange;

//
// Functions that aren't currently resolving.
//
#define _RTL_XXX                                       \
    PRTL_SUFFIX_UNICODE_STRING RtlSuffixUnicodeString; \
    PRTL_FIND_FIRST_RUN_CLEAR RtlFindFirstRunClear;    \
    PRTL_SPLAY_LINKS RtlSplayLinks;

//
// RtlEx functions.
//

typedef BOOLEAN (*PRTL_CHECK_BIT)(
    _In_ PRTL_BITMAP BitMapHeader,
    _In_ ULONG BitPosition
    );

typedef VOID (*PRTL_INITIALIZE_SPLAY_LINKS)(
    _Out_ PRTL_SPLAY_LINKS Links
    );

typedef PRTL_SPLAY_LINKS (*PRTL_PARENT)(
    _In_ PRTL_SPLAY_LINKS Links
    );

typedef PRTL_SPLAY_LINKS (*PRTL_LEFT_CHILD)(
    _In_ PRTL_SPLAY_LINKS Links
    );

typedef PRTL_SPLAY_LINKS (*PRTL_RIGHT_CHILD)(
    _In_ PRTL_SPLAY_LINKS Links
    );

typedef BOOLEAN (*PRTL_IS_ROOT)(
    _In_ PRTL_SPLAY_LINKS Links
    );

typedef BOOLEAN (*PRTL_IS_LEFT_CHILD)(
    _In_ PRTL_SPLAY_LINKS Links
    );

typedef BOOLEAN (*PRTL_IS_RIGHT_CHILD)(
    _In_ PRTL_SPLAY_LINKS Links
    );

typedef VOID (*PRTL_INSERT_AS_LEFT_CHILD)(
    _Inout_ PRTL_SPLAY_LINKS ParentLinks,
    _Inout_ PRTL_SPLAY_LINKS ChildLinks
    );

typedef VOID (*PRTL_INSERT_AS_RIGHT_CHILD)(
    _Inout_ PRTL_SPLAY_LINKS ParentLinks,
    _Inout_ PRTL_SPLAY_LINKS ChildLinks
    );



//
// Our types
//

typedef struct _BITMAP_INDEX {
    USHORT NumberOfUniqueChars;
    USHORT Alignment;
    RTL_BITMAP Forward;
    RTL_BITMAP Reverse;
    CHAR Char[sizeof(ULONG_PTR)];
} BITMAP_INDEX, *PBITMAP_INDEX, **PPBITMAP_INDEX;

typedef struct _STRINGEX {
    USHORT NumberOfBitmapIndexes;

    //
    // Inline STRING structure.
    //

    union {
        STRING String;
        struct {
            USHORT Length;
            USHORT MaximumLength;
            PCHAR  Buffer;
        };
    };

    BITMAP_INDEX BitmapIndexes[1];

} STRINGEX, *PSTRINGEX, **PPSTRINGEX;

#if 0
typedef struct _PAGE_COPY_TYPE {
    ULONG Movsb:1;
    ULONG Movsw:1;
    ULONG Movsq:1;
    ULONG Avx2C:1;
    ULONG Avx2NonTemporal:1;
} PAGE_COPY_TYPE, *PPAGE_COPY_TYPE;

typedef
VOID
(COPY_PAGES)(
    _Out_writes_bytes_all_(NumberOfPages << PAGE_SHIFT) PCHAR Dest,
    _In_reads_bytes_(NumberOfPages << PAGE_SHIFT) _Const_ PCHAR Source,
    _In_ ULONG NumberOfPages
    );
typedef COPY_PAGES *PCOPY_PAGES;

typedef
VOID
(COPY_PAGES_EX)(
    _Out_writes_bytes_all_(NumberOfPages << PAGE_SHIFT) PCHAR Dest,
    _In_reads_bytes_(NumberOfPages << PAGE_SHIFT) _Const_ PCHAR Source,
    _In_ ULONG NumberOfPages,
    _Out_opt_ PPAGE_COPY_TYPE PageCopyType
    );
typedef COPY_PAGES_EX *PCOPY_PAGES_EX;
#endif

//
// Our functions
//

typedef
PVOID
(COPY_TO_MEMORY_MAPPED_MEMORY)(
    _In_ PRTL Rtl,
    _In_ PVOID Destination,
    _In_ LPCVOID Source,
    _In_ SIZE_T Size
    );
typedef COPY_TO_MEMORY_MAPPED_MEMORY *PCOPY_TO_MEMORY_MAPPED_MEMORY;

typedef BOOL (FIND_CHARS_IN_UNICODE_STRING)(
    _In_     PRTL                Rtl,
    _In_     PCUNICODE_STRING    String,
    _In_     WCHAR               Char,
    _Inout_  PRTL_BITMAP         Bitmap,
    _In_     BOOL                Reverse
    );

typedef FIND_CHARS_IN_UNICODE_STRING *PFIND_CHARS_IN_UNICODE_STRING;

typedef
BOOL
(CREATE_BITMAP_INDEX_FOR_UNICODE_STRING)(
    _In_     PRTL                Rtl,
    _In_     PCUNICODE_STRING    String,
    _In_     WCHAR               Char,
    _Inout_  PHANDLE             HeapHandlePointer,
    _Inout_  PPRTL_BITMAP        BitmapPointer,
    _In_     BOOL                Reverse,
    _In_opt_ PFIND_CHARS_IN_UNICODE_STRING FindCharsFunction
    );

typedef CREATE_BITMAP_INDEX_FOR_UNICODE_STRING \
       *PCREATE_BITMAP_INDEX_FOR_UNICODE_STRING;

typedef
BOOL
(FIND_CHARS_IN_STRING)(
    _In_     PRTL           Rtl,
    _In_     PCSTRING       String,
    _In_     CHAR           Char,
    _Inout_  PRTL_BITMAP    Bitmap,
    _In_     BOOL           Reverse
    );

typedef FIND_CHARS_IN_STRING *PFIND_CHARS_IN_STRING;

typedef
ULONGLONG
(NTAPI FIND_AND_REPLACE_BYTE)(
    _In_ ULONGLONG SizeOfBufferInBytes,
    _Inout_bytecap_(SizeOfBufferInBytes) PBYTE Buffer,
    _In_ BYTE Find,
    _In_ BYTE Replace
    );
typedef FIND_AND_REPLACE_BYTE *PFIND_AND_REPLACE_BYTE;

typedef
_Success_(return != 0)
BOOL
(NTAPI MAKE_RANDOM_STRING)(
    _In_ struct _RTL *Rtl,
    _In_ struct _ALLOCATOR *Allocator,
    _In_ ULONG BufferSize,
    _Outptr_result_buffer_(BufferSize) PBYTE *Buffer
    );
typedef MAKE_RANDOM_STRING *PMAKE_RANDOM_STRING;

typedef
_Success_(return != 0)
BOOL
(NTAPI CREATE_BUFFER)(
    _In_ struct _RTL *Rtl,
    _In_opt_ PHANDLE TargetProcessHandle,
    _In_ USHORT NumberOfPages,
    _In_opt_ PULONG AdditionalProtectionFlags,
    _Out_ PULONGLONG UsableBufferSizeInBytes,
    _Out_ PPVOID BufferAddress
    );
typedef CREATE_BUFFER *PCREATE_BUFFER;

typedef
_Success_(return != 0)
BOOL
(NTAPI DESTROY_BUFFER)(
    _In_ struct _RTL *Rtl,
    _In_ HANDLE ProcessHandle,
    _Out_ PPVOID BufferAddress
    );
typedef DESTROY_BUFFER *PDESTROY_BUFFER;

typedef
BOOL
(NTAPI TEST_CREATE_AND_DESTROY_BUFFER)(
    _In_ struct _RTL *Rtl,
    _In_ PCREATE_BUFFER CreateBuffer,
    _In_ PDESTROY_BUFFER DestroyBuffer
    );
typedef TEST_CREATE_AND_DESTROY_BUFFER *PTEST_CREATE_AND_DESTROY_BUFFER;


typedef
VOID
(NTAPI FILL_BUFFER_WITH_256_BYTES)(
    _Out_writes_all_(SizeOfDest) PBYTE Dest,
    _In_reads_bytes_(256) PCBYTE Source,
    ULONGLONG SizeOfDest
    );
typedef FILL_BUFFER_WITH_256_BYTES *PFILL_BUFFER_WITH_256_BYTES;

typedef
BOOL
(CREATE_BITMAP_INDEX_FOR_STRING)(
    _In_     PRTL           Rtl,
    _In_     PCSTRING       String,
    _In_     CHAR           Char,
    _Inout_  PHANDLE        HeapHandlePointer,
    _Inout_  PPRTL_BITMAP   BitmapPointer,
    _In_     BOOL           Reverse,
    _In_opt_ PFIND_CHARS_IN_STRING FindCharsFunction
    );

typedef CREATE_BITMAP_INDEX_FOR_STRING \
       *PCREATE_BITMAP_INDEX_FOR_STRING;

#define MAYBE_FREE_BITMAP_BUFFER(BitmapPointer, StackBitmapBuffer)          \
    if ((ULONG_PTR)StackBitmapBuffer != (ULONG_PTR)BitmapPointer->Buffer && \
        HeapHandle != NULL) {                                               \
        HeapFree(HeapHandle, 0, BitmapPointer->Buffer);                     \
    }

typedef VOID *PALLOCATION_CONTEXT;

typedef PVOID (ALLOCATION_ROUTINE)(
    _In_opt_ PALLOCATION_CONTEXT AllocationContext,
    _In_ const ULONG ByteSize
    );

typedef ALLOCATION_ROUTINE *PALLOCATION_ROUTINE;

typedef
_Must_inspect_result_
PVOID
(CALLOC_ROUTINE)(
    _In_ PVOID Context,
    _In_ SIZE_T NumberOfElements,
    _In_ SIZE_T ElementSize
    );
typedef CALLOC_ROUTINE *PCALLOC_ROUTINE;

typedef VOID *PFREE_CONTEXT;

typedef VOID (FREE_ROUTINE)(
    _In_opt_ PFREE_CONTEXT Context,
    _In_ PVOID Buffer
    );

typedef FREE_ROUTINE *PFREE_ROUTINE;

typedef
_Check_return_
_Success_(return != 0)
BOOL (FILES_EXISTW)(
    _In_      PRTL             Rtl,
    _In_      PUNICODE_STRING  Directory,
    _In_      USHORT           NumberOfFilenames,
    _In_      PPUNICODE_STRING Filenames,
    _Out_     PBOOL            Exists,
    _Out_opt_ PUSHORT          WhichIndex,
    _Out_opt_ PPUNICODE_STRING WhichFilename
    );

typedef FILES_EXISTW *PFILES_EXISTW;

typedef
_Check_return_
_Success_(return != 0)
BOOL
(FILES_EXISTA)(
    _In_      PRTL     Rtl,
    _In_      PSTRING  Directory,
    _In_      USHORT   NumberOfFilenames,
    _In_      PPSTRING Filenames,
    _Out_     PBOOL    Exists,
    _Out_opt_ PUSHORT  WhichIndex,
    _Out_opt_ PPSTRING WhichFilename
    );

typedef FILES_EXISTA *PFILES_EXISTA;

typedef struct _Struct_size_bytes_(Size) _DIRECTORY_CONTAINING_FILES {

    //
    // Size of the structure, in bytes.
    //

    _Field_range_(==, sizeof(struct _DIRECTORY_CONTAINING_FILES)) USHORT Size;

    //
    // Number of child filenames that matched.
    //

    USHORT NumberOfFiles;

    //
    // Padding out to 8-bytes.
    //

    USHORT Padding[2];

    //
    // The directory name.
    //

    UNICODE_STRING Directory;

    //
    // The allocator used to allocate this structure.
    //

    PALLOCATOR Allocator;

    //
    // List entry in DIRECTORIES_CONTAINING_FILES.ListHead.
    //

    LIST_ENTRY ListEntry;

    //
    // List head of child entries.
    //

    LIST_ENTRY Children;

} DIRECTORY_CONTAINING_FILE, *PDIRECTORY_CONTAINING_FILE;

typedef struct _Struct_size_bytes_(Size) _DIRECTORIES_CONTAINING_FILES {

    //
    // Size of the structure, in bytes.
    //

    _Field_range_(==, sizeof(struct _DIRECTORY_CONTAINING_FILES)) USHORT Size;

    //
    // Number of directory entries.
    //

    //
    // Padding out to 8-bytes.
    //

} DIRECTORIES_CONTAINING_FILES, *PDIRECTORIES_CONTAINING_FILES;

typedef
_Check_return_
_Success_(return != 0)
BOOL
(FILES_EXIST_IN_DIRECTORIESW)(
    _In_ PRTL Rtl,
    _In_ USHORT NumberOfDirectories,
    _In_reads_bytes_(NumberOfDirectories * sizeof(PUNICODE_STRING))
        PPUNICODE_STRING Directories,
    _In_ USHORT NumberOfFilenames,
    _In_reads_bytes_(NumberOfFilenames * sizeof(PUNICODE_STRING))
        PPUNICODE_STRING Filenames,
    _In_ PALLOCATOR Allocator
    );

typedef FILES_EXISTW *PFILES_EXISTW;


typedef BOOL (*PCOPY_UNICODE_STRING)(
    _In_  PRTL                  Rtl,
    _In_  PCUNICODE_STRING      Source,
    _Out_ PPUNICODE_STRING      Destination,
    _In_  PALLOCATION_ROUTINE   AllocationRoutine,
    _In_  PVOID                 AllocationContext
    );

typedef BOOL (*PCOPY_STRING)(
    _In_  PRTL                  Rtl,
    _In_  PCSTRING              Source,
    _Out_ PPSTRING              Destination,
    _In_  PALLOCATION_ROUTINE   AllocationRoutine,
    _In_  PVOID                 AllocationContext
    );

typedef BOOL (*PHASH_UNICODE_STRING_TO_ATOM)(
    _In_  PUNICODE_STRING String,
    _Out_ PULONG Hash
    );

typedef BOOL (*PTEST_EXCEPTION_HANDLER)(VOID);

typedef BOOL (*PLOAD_SHLWAPI)(PRTL Rtl);
typedef BOOL (*PLOAD_DBGHELP)(PRTL Rtl);
typedef BOOL (*PLOAD_DBGENG)(PRTL Rtl);

typedef BOOL (*PPATH_CANONICALIZEA)(
        _Out_   LPSTR   Dest,
        _In_    LPCSTR  Source
    );

#define PATH_ENV_NAME L"Path"

typedef struct _Struct_size_bytes_(StructSize) _PATH_ENV_VAR {

    _Field_range_(==, sizeof(struct _PATH_ENV_VAR)) USHORT StructSize;

    USHORT FirstAlignedAllocSizeInBytes;
    USHORT SecondAlignedAllocSizeInBytes;

    USHORT NumberOfElements;
    USHORT ReservedUnicodeBufferSizeInBytes;

    //
    // Pad out to an 8-byte boundary.
    //

    USHORT Padding[3];

    //
    // The allocator used to create this structure.
    //

    PALLOCATOR Allocator;

    //
    // Bitmap for directory end points.
    //

    RTL_BITMAP Bitmap;

    //
    // A UNICODE_STRING wrapping the PATH environment variable value.
    //

    UNICODE_STRING Paths;

    //
    // A UNICODE_STRING that will wrap the new PATH environment variable to
    // be set, if there is one.
    //

    UNICODE_STRING NewPaths;

    UNICODE_PREFIX_TABLE PathsPrefixTable;
    UNICODE_PREFIX_TABLE PathsToAddPrefixTable;
    UNICODE_PREFIX_TABLE PathsToRemovePrefixTable;

    //
    // Pointer to the first element of an array of UNICODE_STRING structs used
    // to capture each directory in the path.
    //

    PUNICODE_STRING Directories;

    //
    // Pointer to the first element of an array of UNICODE_PREFIX_TABLE_ENTRY
    // structs used for the PathsPrefixTable.
    //

    PUNICODE_PREFIX_TABLE_ENTRY PathsPrefixTableEntries;

    //
    // Pointer to the first element of an array of UNICODE_PREFIX_TABLE_ENTRY
    // structs used for the PathsToRemovePrefixTable.
    //

    PUNICODE_PREFIX_TABLE_ENTRY PathsToRemovePrefixTableEntries;

} PATH_ENV_VAR, *PPATH_ENV_VAR, **PPPATH_ENV_VAR;

typedef
_Success_(return != 0)
PPATH_ENV_VAR
(LOAD_PATH_ENVIRONMENT_VARIABLE)(
    _In_ PRTL Rtl,
    _In_ PALLOCATOR Allocator,
    _In_ USHORT ReservedUnicodeBufferSizeInBytes
    );
typedef LOAD_PATH_ENVIRONMENT_VARIABLE *PLOAD_PATH_ENVIRONMENT_VARIABLE;

typedef
VOID
(DESTROY_PATH_ENVIRONMENT_VARIABLE)(
    _Inout_ PPPATH_ENV_VAR PathPointer
    );
typedef DESTROY_PATH_ENVIRONMENT_VARIABLE *PDESTROY_PATH_ENVIRONMENT_VARIABLE;

typedef
_Success_(return != 0)
_Check_return_
PUNICODE_STRING
(CURRENT_DIRECTORY_TO_UNICODE_STRING)(
    _In_ PALLOCATOR Allocator
    );
typedef CURRENT_DIRECTORY_TO_UNICODE_STRING \
      *PCURRENT_DIRECTORY_TO_UNICODE_STRING,\
    **PPCURRENT_DIRECTORY_TO_UNICODE_STRING;

typedef
_Success_(return != 0)
_Check_return_
PRTL_PATH
(CURRENT_DIRECTORY_TO_RTL_PATH)(
    _In_ PALLOCATOR Allocator
    );
typedef CURRENT_DIRECTORY_TO_RTL_PATH \
      *PCURRENT_DIRECTORY_TO_RTL_PATH,\
    **PPCURRENT_DIRECTORY_TO_RTL_PATH;

typedef
_Check_return_
_Success_(return != 0)
BOOL
(WRITE_REGISTRY_STRING)(
    _In_ PRTL Rtl,
    _In_ PALLOCATOR Allocator,
    _In_ HKEY RegistryKey,
    _In_ PWSTR Name,
    _In_ PUNICODE_STRING Value
    );
typedef WRITE_REGISTRY_STRING *PWRITE_REGISTRY_STRING;

typedef
_Check_return_
_Success_(return != 0)
BOOL
(WRITE_ENV_VAR_TO_REGISTRY)(
    _In_ PRTL Rtl,
    _In_ PALLOCATOR Allocator,
    _In_ HKEY RegistryKey,
    _In_ PWSTR EnvironmentVariableName,
    _In_opt_ PWSTR RegistryKeyName
    );
typedef WRITE_ENV_VAR_TO_REGISTRY *PWRITE_ENV_VAR_TO_REGISTRY;

typedef
VOID
(DESTROY_RTL)(
    _In_opt_ struct _RTL **RtlPointer
    );
typedef DESTROY_RTL *PDESTROY_RTL, **PPDESTROY_RTL;

typedef
BOOL
(PREFAULT_PAGES)(
    _In_ PVOID Address,
    _In_ ULONG NumberOfPages
    );
typedef PREFAULT_PAGES *PPREFAULT_PAGES, **PPPREFAULT_PAGES;

typedef
_Success_(return != 0)
BOOL
(SET_PRIVILEGE)(
    _In_ PWSTR PrivilegeName,
    _In_ BOOL Enable
    );
typedef SET_PRIVILEGE *PSET_PRIVILEGE;

typedef
_Success_(return != 0)
BOOL
(ENABLE_PRIVILEGE)(
    _In_ PWSTR PrivilegeName
    );
typedef ENABLE_PRIVILEGE *PENABLE_PRIVILEGE;

typedef
_Success_(return != 0)
BOOL
(DISABLE_PRIVILEGE)(
    _In_ PWSTR PrivilegeName
    );
typedef DISABLE_PRIVILEGE *PDISABLE_PRIVILEGE;

typedef
_Success_(return != 0)
BOOL
(ENABLE_MANAGE_VOLUME_PRIVILEGE)(
    VOID
    );
typedef ENABLE_MANAGE_VOLUME_PRIVILEGE *PENABLE_MANAGE_VOLUME_PRIVILEGE;

typedef
_Success_(return != 0)
BOOL
(DISABLE_MANAGE_VOLUME_PRIVILEGE)(
    VOID
    );
typedef DISABLE_MANAGE_VOLUME_PRIVILEGE *PDISABLE_MANAGE_VOLUME_PRIVILEGE;

typedef
_Success_(return != 0)
BOOL
(ENABLE_LOCK_MEMORY_PRIVILEGE)(
    VOID
    );
typedef ENABLE_LOCK_MEMORY_PRIVILEGE *PENABLE_LOCK_MEMORY_PRIVILEGE;

typedef
_Success_(return != 0)
BOOL
(DISABLE_LOCK_MEMORY_PRIVILEGE)(
    VOID
    );
typedef DISABLE_LOCK_MEMORY_PRIVILEGE *PDISABLE_LOCK_MEMORY_PRIVILEGE;

typedef
_Success_(return != 0)
BOOL
(ENABLE_DEBUG_PRIVILEGE)(
    VOID
    );
typedef ENABLE_DEBUG_PRIVILEGE *PENABLE_DEBUG_PRIVILEGE;

typedef
_Success_(return != 0)
BOOL
(DISABLE_DEBUG_PRIVILEGE)(
    VOID
    );
typedef DISABLE_DEBUG_PRIVILEGE *PDISABLE_DEBUG_PRIVILEGE;

typedef
_Success_(return != 0)
BOOL
(ENABLE_SYSTEM_PROFILE_PRIVILEGE)(
    VOID
    );
typedef ENABLE_SYSTEM_PROFILE_PRIVILEGE *PENABLE_SYSTEM_PROFILE_PRIVILEGE;

typedef
_Success_(return != 0)
BOOL
(DISABLE_SYSTEM_PROFILE_PRIVILEGE)(
    VOID
    );
typedef DISABLE_SYSTEM_PROFILE_PRIVILEGE *PDISABLE_SYSTEM_PROFILE_PRIVILEGE;

typedef
_Success_(return != 0)
BOOL
(ENABLE_PROFILE_SINGLE_PROCESS_PRIVILEGE)(
    VOID
    );
typedef ENABLE_PROFILE_SINGLE_PROCESS_PRIVILEGE
      *PENABLE_PROFILE_SINGLE_PROCESS_PRIVILEGE;

typedef
_Success_(return != 0)
BOOL
(DISABLE_PROFILE_SINGLE_PROCESS_PRIVILEGE)(
    VOID
    );
typedef DISABLE_PROFILE_SINGLE_PROCESS_PRIVILEGE
      *PDISABLE_PROFILE_SINGLE_PROCESS_PRIVILEGE;

typedef
_Success_(return != 0)
BOOL
(ENABLE_INCREASE_WORKING_SET_PRIVILEGE)(
    VOID
    );
typedef ENABLE_INCREASE_WORKING_SET_PRIVILEGE
      *PENABLE_INCREASE_WORKING_SET_PRIVILEGE;

typedef
_Success_(return != 0)
BOOL
(DISABLE_INCREASE_WORKING_SET_PRIVILEGE)(
    VOID
    );
typedef DISABLE_INCREASE_WORKING_SET_PRIVILEGE
      *PDISABLE_INCREASE_WORKING_SET_PRIVILEGE;

typedef
_Success_(return != 0)
BOOL
(ENABLE_CREATE_SYMBOLIC_LINK_PRIVILEGE)(
    VOID
    );
typedef ENABLE_CREATE_SYMBOLIC_LINK_PRIVILEGE
      *PENABLE_CREATE_SYMBOLIC_LINK_PRIVILEGE;

typedef
_Success_(return != 0)
BOOL
(DISABLE_CREATE_SYMBOLIC_LINK_PRIVILEGE)(
    VOID
    );
typedef DISABLE_CREATE_SYMBOLIC_LINK_PRIVILEGE
      *PDISABLE_CREATE_SYMBOLIC_LINK_PRIVILEGE;

//
// Loader hooking.
//

#define LDR_DLL_NOTIFICATION_REASON_LOADED_INITIAL 5

//
// This is a custom structure we use to overlay the DLL notification reasons
// above.  Bit 1 indicates both LOADED and LOADED_INITIAL.
//

typedef union _DLL_NOTIFICATION_REASON {
    ULONG AsLong;
    struct {
        ULONG Loaded:1;
        ULONG Unloaded:1;
        ULONG LoadedInitial:1;
    };
} DLL_NOTIFICATION_REASON;

typedef DLL_NOTIFICATION_REASON *PDLL_NOTIFICATION_REASON;

typedef struct _DLL_NOTIFICATION_DATA {
    PLDR_DATA_TABLE_ENTRY LoaderDataTableEntry;

    //
    // The following entries are only filled in when the notification reason is
    // LDR_DLL_NOTIFICATION_REASON_LOADED (i.e. not LOADED_INITIAL).
    //

    PCLDR_DLL_NOTIFICATION_DATA NotificationData;
    ULONG NumberOfStackBackTraceFrames;
    ULONG StackBackTraceHash;
    PVOID StackBackTrace;
} DLL_NOTIFICATION_DATA, *PDLL_NOTIFICATION_DATA;

typedef
VOID
(DLL_NOTIFICATION_CALLBACK)(
    _In_ DLL_NOTIFICATION_REASON NotificationReason,
    _In_ PDLL_NOTIFICATION_DATA NotificationData,
    _In_ PVOID Context
    );
typedef DLL_NOTIFICATION_CALLBACK *PDLL_NOTIFICATION_CALLBACK;

typedef struct _DLL_NOTIFICATION_FLAGS {
    ULONG Unused:1;
} DLL_NOTIFICATION_FLAGS, *PDLL_NOTIFICATION_FLAGS;

typedef
_Check_return_
_Success_(return != 0)
BOOL
(REGISTER_DLL_NOTIFICATION)(
    _In_ struct _RTL *Rtl,
    _In_ PDLL_NOTIFICATION_CALLBACK NotificationCallback,
    _In_opt_ PDLL_NOTIFICATION_FLAGS NotificationFlags,
    _In_opt_ PVOID Context,
    _Outptr_opt_result_nullonfailure_ PPVOID Cookie
    );
typedef REGISTER_DLL_NOTIFICATION *PREGISTER_DLL_NOTIFICATION;

typedef
_Success_(return != 0)
BOOL
(UNREGISTER_DLL_NOTIFICATION)(
    _In_ PVOID Cookie
    );
typedef UNREGISTER_DLL_NOTIFICATION *PUNREGISTER_DLL_NOTIFICATION;

#define PrefaultPage(Address) (*(volatile *)(PCHAR)(Address))

#define PrefaultNextPage(Address)                          \
    (*(volatile *)(PCHAR)((ULONG_PTR)Address + PAGE_SIZE))

typedef
_Check_return_
_Success_(return != 0)
BOOL
(LOAD_SYMBOLS)(
    _In_count_(NumberOfSymbolNames) CONST PCSZ *SymbolNameArray,
    _In_ ULONG NumberOfSymbolNames,
    _In_count_(NumberOfSymbolAddresses) PULONG_PTR SymbolAddressArray,
    _In_ ULONG NumberOfSymbolAddresses,
    _In_ HMODULE Module,
    _In_ PRTL_BITMAP FailedSymbols,
    _In_ BOOL SkipJumpInstructions,
    _Out_ PULONG NumberOfResolvedSymbolsPointer
    );
typedef LOAD_SYMBOLS *PLOAD_SYMBOLS;

typedef
_Check_return_
_Success_(return != 0)
BOOL
(LOAD_SYMBOLS_FROM_MULTIPLE_MODULES)(
    _In_count_(NumberOfSymbolNames) CONST PCSZ *SymbolNameArray,
    _In_ ULONG NumberOfSymbolNames,
    _In_count_(NumberOfSymbolAddresses) PULONG_PTR SymbolAddressArray,
    _In_ ULONG NumberOfSymbolAddresses,
    _In_count_(NumberOfModules) PHMODULE ModuleArray,
    _In_ USHORT NumberOfModules,
    _In_ PRTL_BITMAP FailedSymbols,
    _In_ BOOL SkipJumpInstructions,
    _Out_ PULONG NumberOfResolvedSymbolsPointer
    );
typedef LOAD_SYMBOLS_FROM_MULTIPLE_MODULES
      *PLOAD_SYMBOLS_FROM_MULTIPLE_MODULES;

typedef union _LOAD_FILE_FLAGS {
    struct _Struct_size_bytes_(sizeof(ULONG)) {
        ULONG Unused:32;
    };
    LONG AsLong;
    ULONG AsULong;
} LOAD_FILE_FLAGS;
typedef LOAD_FILE_FLAGS *PLOAD_FILE_FLAGS;

typedef
BOOL
(LOAD_FILE)(
    _In_ PRTL Rtl,
    _In_ LOAD_FILE_FLAGS Flags,
    _In_ PCUNICODE_STRING Path,
    _Out_ PHANDLE FileHandlePointer,
    _Out_ PHANDLE MappingHandlePointer,
    _Out_ PPVOID BaseAddressPointer
    );
typedef LOAD_FILE *PLOAD_FILE;

#ifdef _RTL_TEST
typedef
BOOL
(TEST_LOAD_SYMBOLS)(
    VOID
    );
typedef TEST_LOAD_SYMBOLS *PTEST_LOAD_SYMBOLS;

typedef
BOOL
(TEST_LOAD_SYMBOLS_FROM_MULTIPLE_MODULES)(
    VOID
    );
typedef TEST_LOAD_SYMBOLS_FROM_MULTIPLE_MODULES
      *PTEST_LOAD_SYMBOLS_FROM_MULTIPLE_MODULES;
#endif

//
// Helper string routines for buffer manipulation.
//

typedef
VOID
(NTAPI APPEND_INTEGER_TO_CHAR_BUFFER)(
    _Inout_ PPCHAR BufferPointer,
    _In_ ULONGLONG Integer
    );
typedef APPEND_INTEGER_TO_CHAR_BUFFER *PAPPEND_INTEGER_TO_CHAR_BUFFER;

typedef
BOOLEAN
(NTAPI APPEND_INTEGER_TO_CHAR_BUFFER_EX)(
    _Inout_ PPCHAR BufferPointer,
    _In_ ULONGLONG Integer,
    _In_ BYTE NumberOfDigits,
    _In_ CHAR Pad,
    _In_opt_ CHAR Trailer
    );
typedef APPEND_INTEGER_TO_CHAR_BUFFER_EX *PAPPEND_INTEGER_TO_CHAR_BUFFER_EX;

typedef
VOID
(NTAPI APPEND_STRING_TO_CHAR_BUFFER)(
    _Inout_ PPCHAR BufferPointer,
    _In_ PCSTRING String
    );
typedef APPEND_STRING_TO_CHAR_BUFFER *PAPPEND_STRING_TO_CHAR_BUFFER;

typedef
VOID
(NTAPI APPEND_CHAR_BUFFER_TO_CHAR_BUFFER)(
    _Inout_ PPCHAR BufferPointer,
    _In_ PCCHAR String,
    _In_ ULONG SizeInBytes
    );
typedef APPEND_CHAR_BUFFER_TO_CHAR_BUFFER *PAPPEND_CHAR_BUFFER_TO_CHAR_BUFFER;

typedef
VOID
(NTAPI APPEND_CHAR_TO_CHAR_BUFFER)(
    _Inout_ PPCHAR BufferPointer,
    _In_ CHAR Char
    );
typedef APPEND_CHAR_TO_CHAR_BUFFER *PAPPEND_CHAR_TO_CHAR_BUFFER;

typedef
VOID
(NTAPI APPEND_CSTR_TO_CHAR_BUFFER)(
    _Inout_ PPCHAR BufferPointer,
    _In_ PCSZ String
    );
typedef APPEND_CSTR_TO_CHAR_BUFFER *PAPPEND_CSTR_TO_CHAR_BUFFER;

//
// Wide character versions.
//

typedef
VOID
(NTAPI APPEND_INTEGER_TO_WIDE_CHAR_BUFFER)(
    _Inout_ PPWCHAR BufferPointer,
    _In_ ULONGLONG Integer
    );
typedef APPEND_INTEGER_TO_WIDE_CHAR_BUFFER *PAPPEND_INTEGER_TO_WIDE_CHAR_BUFFER;

typedef
BOOLEAN
(NTAPI APPEND_INTEGER_TO_WIDE_CHAR_BUFFER_EX)(
    _Inout_ PPWCHAR BufferPointer,
    _In_ ULONGLONG Integer,
    _In_ BYTE NumberOfDigits,
    _In_ WCHAR Pad,
    _In_opt_ WCHAR Trailer
    );
typedef APPEND_INTEGER_TO_WIDE_CHAR_BUFFER_EX
      *PAPPEND_INTEGER_TO_WIDE_CHAR_BUFFER_EX;

typedef
VOID
(NTAPI APPEND_UNICODE_STRING_TO_WIDE_CHAR_BUFFER)(
    _Inout_ PPWCHAR BufferPointer,
    _In_ PCUNICODE_STRING UnicodeString
    );
typedef APPEND_UNICODE_STRING_TO_WIDE_CHAR_BUFFER
      *PAPPEND_UNICODE_STRING_TO_WIDE_CHAR_BUFFER;

typedef
VOID
(NTAPI APPEND_WIDE_CHAR_BUFFER_TO_WIDE_CHAR_BUFFER)(
    _Inout_ PPWCHAR BufferPointer,
    _In_ PCWCHAR String,
    _In_ ULONG SizeInBytes
    );
typedef APPEND_WIDE_CHAR_BUFFER_TO_WIDE_CHAR_BUFFER
      *PAPPEND_WIDE_CHAR_BUFFER_TO_WIDE_CHAR_BUFFER;

typedef
VOID
(NTAPI APPEND_WIDE_CHAR_TO_WIDE_CHAR_BUFFER)(
    _Inout_ PPWCHAR BufferPointer,
    _In_ WCHAR Char
    );
typedef APPEND_WIDE_CHAR_TO_WIDE_CHAR_BUFFER
      *PAPPEND_WIDE_CHAR_TO_WIDE_CHAR_BUFFER;

typedef
VOID
(NTAPI APPEND_WIDE_CSTR_TO_WIDE_CHAR_BUFFER)(
    _Inout_ PPWCHAR BufferPointer,
    _In_ PCWSZ String
    );
typedef APPEND_WIDE_CSTR_TO_WIDE_CHAR_BUFFER
      *PAPPEND_WIDE_CSTR_TO_WIDE_CHAR_BUFFER;


#define _RTLEXFUNCTIONS_HEAD                                                           \
    PDESTROY_RTL DestroyRtl;                                                           \
    PARGVW_TO_ARGVA ArgvWToArgvA;                                                      \
    PCOPY_PAGES_EX CopyPagesMovsq;                                                     \
    PCOPY_PAGES_EX CopyPagesAvx2;                                                      \
    PCOPY_PAGES CopyPagesMovsq_C;                                                      \
    PCOPY_PAGES CopyPagesAvx2_C;                                                       \
    PCOPY_TO_MEMORY_MAPPED_MEMORY CopyToMemoryMappedMemory;                            \
    PCREATE_BITMAP_INDEX_FOR_STRING CreateBitmapIndexForString;                        \
    PCREATE_BITMAP_INDEX_FOR_UNICODE_STRING CreateBitmapIndexForUnicodeString;         \
    PCURRENT_DIRECTORY_TO_RTL_PATH CurrentDirectoryToRtlPath;                          \
    PCURRENT_DIRECTORY_TO_UNICODE_STRING CurrentDirectoryToUnicodeString;              \
    PDESTROY_PATH_ENVIRONMENT_VARIABLE DestroyPathEnvironmentVariable;                 \
    PDESTROY_RTL_PATH DestroyRtlPath;                                                  \
    PDISABLE_CREATE_SYMBOLIC_LINK_PRIVILEGE DisableCreateSymbolicLinkPrivilege;        \
    PDISABLE_DEBUG_PRIVILEGE DisableDebugPrivilege;                                    \
    PDISABLE_INCREASE_WORKING_SET_PRIVILEGE DisableIncreaseWorkingSetPrivilege;        \
    PDISABLE_LOCK_MEMORY_PRIVILEGE DisableLockMemoryPrivilege;                         \
    PDISABLE_MANAGE_VOLUME_PRIVILEGE DisableManageVolumePrivilege;                     \
    PDISABLE_PRIVILEGE DisablePrivilege;                                               \
    PDISABLE_PROFILE_SINGLE_PROCESS_PRIVILEGE DisableProfileSingleProcessPrivilege;    \
    PDISABLE_SYSTEM_PROFILE_PRIVILEGE DisableSystemProfilePrivilege;                   \
    PENABLE_CREATE_SYMBOLIC_LINK_PRIVILEGE EnableCreateSymbolicLinkPrivilege;          \
    PENABLE_DEBUG_PRIVILEGE EnableDebugPrivilege;                                      \
    PENABLE_INCREASE_WORKING_SET_PRIVILEGE EnableIncreaseWorkingSetPrivilege;          \
    PENABLE_LOCK_MEMORY_PRIVILEGE EnableLockMemoryPrivilege;                           \
    PENABLE_MANAGE_VOLUME_PRIVILEGE EnableManageVolumePrivilege;                       \
    PENABLE_PRIVILEGE EnablePrivilege;                                                 \
    PENABLE_PROFILE_SINGLE_PROCESS_PRIVILEGE EnableProfileSingleProcessPrivilege;      \
    PENABLE_SYSTEM_PROFILE_PRIVILEGE EnableSystemProfilePrivilege;                     \
    PFILES_EXISTA FilesExistA;                                                         \
    PFILES_EXISTW FilesExistW;                                                         \
    PFIND_CHARS_IN_STRING FindCharsInString;                                           \
    PFIND_CHARS_IN_UNICODE_STRING FindCharsInUnicodeString;                            \
    PGET_MODULE_RTL_PATH GetModuleRtlPath;                                             \
    PINITIALIZE_RTL_FILE InitializeRtlFile;                                            \
    PLOAD_FILE LoadFile;                                                               \
    PLOAD_DBGENG LoadDbgEng;                                                           \
    PLOAD_DBGHELP LoadDbgHelp;                                                         \
    PLOAD_PATH_ENVIRONMENT_VARIABLE LoadPathEnvironmentVariable;                       \
    PLOAD_SHLWAPI LoadShlwapi;                                                         \
    PPREFAULT_PAGES PrefaultPages;                                                     \
    PREGISTER_DLL_NOTIFICATION RegisterDllNotification;                                \
    PRTL_CHECK_BIT RtlCheckBit;                                                        \
    PRTL_INITIALIZE_SPLAY_LINKS RtlInitializeSplayLinks;                               \
    PRTL_INSERT_AS_LEFT_CHILD RtlInsertAsLeftChild;                                    \
    PRTL_INSERT_AS_RIGHT_CHILD RtlInsertAsRightChild;                                  \
    PRTL_IS_LEFT_CHILD RtlIsLeftChild;                                                 \
    PRTL_IS_RIGHT_CHILD RtlIsRightChild;                                               \
    PRTL_IS_ROOT RtlIsRoot;                                                            \
    PRTL_LEFT_CHILD RtlLeftChild;                                                      \
    PRTL_PARENT RtlParent;                                                             \
    PRTL_RIGHT_CHILD RtlRightChild;                                                    \
    PSET_PRIVILEGE SetPrivilege;                                                       \
    PSTRING_TO_EXISTING_RTL_PATH StringToExistingRtlPath;                              \
    PSTRING_TO_RTL_PATH StringToRtlPath;                                               \
    PTEST_EXCEPTION_HANDLER TestExceptionHandler;                                      \
    PUNICODE_STRING_TO_EXISTING_RTL_PATH UnicodeStringToExistingRtlPath;               \
    PUNICODE_STRING_TO_RTL_PATH UnicodeStringToRtlPath;                                \
    PUNREGISTER_DLL_NOTIFICATION UnregisterDllNotification;                            \
    PWRITE_ENV_VAR_TO_REGISTRY WriteEnvVarToRegistry;                                  \
    PWRITE_REGISTRY_STRING WriteRegistryString;                                        \
    PAPPEND_INTEGER_TO_CHAR_BUFFER AppendIntegerToCharBuffer;                          \
    PAPPEND_INTEGER_TO_CHAR_BUFFER_EX AppendIntegerToCharBufferEx;                     \
    PAPPEND_STRING_TO_CHAR_BUFFER AppendStringToCharBuffer;                            \
    PAPPEND_CHAR_BUFFER_TO_CHAR_BUFFER AppendCharBufferToCharBuffer;                   \
    PAPPEND_CSTR_TO_CHAR_BUFFER AppendCStrToCharBuffer;                                \
    PAPPEND_CHAR_TO_CHAR_BUFFER AppendCharToCharBuffer;                                \
    PAPPEND_UNICODE_STRING_TO_WIDE_CHAR_BUFFER AppendUnicodeStringToWideCharBuffer;    \
    PAPPEND_WIDE_CSTR_TO_WIDE_CHAR_BUFFER AppendWideCStrToWideCharBuffer;              \
    PAPPEND_WIDE_CHAR_BUFFER_TO_WIDE_CHAR_BUFFER AppendWideCharBufferToWideCharBuffer; \
    PAPPEND_WIDE_CHAR_TO_WIDE_CHAR_BUFFER AppendWideCharToWideCharBuffer;              \
    PAPPEND_INTEGER_TO_WIDE_CHAR_BUFFER AppendIntegerToWideCharBuffer;


typedef struct _RTLEXFUNCTIONS {
    _RTLEXFUNCTIONS_HEAD
} RTLEXFUNCTIONS, *PRTLEXFUNCTIONS, **PPRTLEXFUNCTIONS;

#define _SHLWAPIFUNCTIONS_HEAD             \
    PPATH_CANONICALIZEA PathCanonicalizeA;

typedef struct _SHLWAPI_FUNCTIONS {
    _SHLWAPIFUNCTIONS_HEAD
} SHLWAPI_FUNCTIONS, *PSHLWAPI_FUNCTIONS, **PPSHLWAPI_FUNCTIONS;

//
// DbgHelp related functions.
//

#include "DbgHelpFunctionPointerTypedefs.h"

#define _DBGHELP_FUNCTIONS_HEAD                                                 \
    PENUM_DIR_TREE EnumDirTree;                                                 \
    PENUM_DIR_TREE_W EnumDirTreeW;                                              \
    PENUMERATE_LOADED_MODULES64 EnumerateLoadedModules64;                       \
    PENUMERATE_LOADED_MODULES EnumerateLoadedModules;                           \
    PENUMERATE_LOADED_MODULES_EX EnumerateLoadedModulesEx;                      \
    PENUMERATE_LOADED_MODULES_EX_W EnumerateLoadedModulesExW;                   \
    PENUMERATE_LOADED_MODULES_W64 EnumerateLoadedModulesW64;                    \
    PFIND_FILE_IN_PATH FindFileInPath;                                          \
    PFIND_FILE_IN_SEARCH_PATH FindFileInSearchPath;                             \
    PGET_SYM_LOAD_ERROR GetSymLoadError;                                        \
    PGET_TIMESTAMP_FOR_LOADED_LIBRARY GetTimestampForLoadedLibrary;             \
    PMAKE_SURE_DIRECTORY_PATH_EXISTS MakeSureDirectoryPathExists;               \
    PRANGE_MAP_ADD_PE_IMAGE_SECTIONS RangeMapAddPeImageSections;                \
    PRANGE_MAP_READ RangeMapRead;                                               \
    PRANGE_MAP_REMOVE RangeMapRemove;                                           \
    PRANGE_MAP_WRITE RangeMapWrite;                                             \
    PREPORT_SYMBOL_LOAD_SUMMARY ReportSymbolLoadSummary;                        \
    PSEARCH_TREE_FOR_FILE SearchTreeForFile;                                    \
    PSEARCH_TREE_FOR_FILE_W SearchTreeForFileW;                                 \
    PSTACK_WALK64 StackWalk64;                                                  \
    PSTACK_WALK_EX StackWalkEx;                                                 \
    PSTACK_WALK StackWalk;                                                      \
    PSYM_ADDR_INCLUDE_INLINE_TRACE SymAddrIncludeInlineTrace;                   \
    PSYM_ADD_SOURCE_STREAM_A SymAddSourceStreamA;                               \
    PSYM_ADD_SOURCE_STREAM SymAddSourceStream;                                  \
    PSYM_ADD_SOURCE_STREAM_W SymAddSourceStreamW;                               \
    PSYM_ADD_SYMBOL SymAddSymbol;                                               \
    PSYM_ADD_SYMBOL_W SymAddSymbolW;                                            \
    PSYM_CLEANUP SymCleanup;                                                    \
    PSYM_COMPARE_INLINE_TRACE SymCompareInlineTrace;                            \
    PSYM_DELETE_SYMBOL SymDeleteSymbol;                                         \
    PSYM_DELETE_SYMBOL_W SymDeleteSymbolW;                                      \
    PSYM_ENUMERATE_MODULES64 SymEnumerateModules64;                             \
    PSYM_ENUMERATE_MODULES SymEnumerateModules;                                 \
    PSYM_ENUMERATE_MODULES_W64 SymEnumerateModulesW64;                          \
    PSYM_ENUMERATE_SYMBOLS64 SymEnumerateSymbols64;                             \
    PSYM_ENUMERATE_SYMBOLS SymEnumerateSymbols;                                 \
    PSYM_ENUMERATE_SYMBOLS_W64 SymEnumerateSymbolsW64;                          \
    PSYM_ENUMERATE_SYMBOLS_W SymEnumerateSymbolsW;                              \
    PSYM_ENUM_LINES SymEnumLines;                                               \
    PSYM_ENUM_LINES_W SymEnumLinesW;                                            \
    PSYM_ENUM_PROCESSES SymEnumProcesses;                                       \
    PSYM_ENUM_SOURCE_FILES SymEnumSourceFiles;                                  \
    PSYM_ENUM_SOURCE_FILES_W SymEnumSourceFilesW;                               \
    PSYM_ENUM_SOURCE_FILE_TOKENS SymEnumSourceFileTokens;                       \
    PSYM_ENUM_SOURCE_LINES SymEnumSourceLines;                                  \
    PSYM_ENUM_SOURCE_LINES_W SymEnumSourceLinesW;                               \
    PSYM_ENUM_SYMBOLS_EX SymEnumSymbolsEx;                                      \
    PSYM_ENUM_SYMBOLS_EX_W SymEnumSymbolsExW;                                   \
    PSYM_ENUM_SYMBOLS_FOR_ADDR SymEnumSymbolsForAddr;                           \
    PSYM_ENUM_SYMBOLS_FOR_ADDR_W SymEnumSymbolsForAddrW;                        \
    PSYM_ENUM_SYMBOLS SymEnumSymbols;                                           \
    PSYM_ENUM_SYMBOLS_W SymEnumSymbolsW;                                        \
    PSYM_ENUM_SYM SymEnumSym;                                                   \
    PSYM_ENUM_TYPES_BY_NAME SymEnumTypesByName;                                 \
    PSYM_ENUM_TYPES_BY_NAME_W SymEnumTypesByNameW;                              \
    PSYM_ENUM_TYPES SymEnumTypes;                                               \
    PSYM_ENUM_TYPES_W SymEnumTypesW;                                            \
    PSYM_FIND_FILE_IN_PATH SymFindFileInPath;                                   \
    PSYM_FIND_FILE_IN_PATH_W SymFindFileInPathW;                                \
    PSYM_FROM_ADDR SymFromAddr;                                                 \
    PSYM_FROM_ADDR_W SymFromAddrW;                                              \
    PSYM_FROM_INDEX SymFromIndex;                                               \
    PSYM_FROM_INDEX_W SymFromIndexW;                                            \
    PSYM_FROM_INLINE_CONTEXT SymFromInlineContext;                              \
    PSYM_FROM_INLINE_CONTEXT_W SymFromInlineContextW;                           \
    PSYM_FROM_NAME SymFromName;                                                 \
    PSYM_FROM_NAME_W SymFromNameW;                                              \
    PSYM_FROM_TOKEN SymFromToken;                                               \
    PSYM_FROM_TOKEN_W SymFromTokenW;                                            \
    PSYM_GET_FILE_LINE_OFFSETS64 SymGetFileLineOffsets64;                       \
    PSYM_GET_LINE_FROM_ADDR64 SymGetLineFromAddr64;                             \
    PSYM_GET_LINE_FROM_ADDR SymGetLineFromAddr;                                 \
    PSYM_GET_LINE_FROM_ADDR_W64 SymGetLineFromAddrW64;                          \
    PSYM_GET_LINE_FROM_INLINE_CONTEXT SymGetLineFromInlineContext;              \
    PSYM_GET_LINE_FROM_INLINE_CONTEXT_W SymGetLineFromInlineContextW;           \
    PSYM_GET_LINE_FROM_NAME64 SymGetLineFromName64;                             \
    PSYM_GET_LINE_FROM_NAME SymGetLineFromName;                                 \
    PSYM_GET_LINE_FROM_NAME_W64 SymGetLineFromNameW64;                          \
    PSYM_GET_LINE_NEXT64 SymGetLineNext64;                                      \
    PSYM_GET_LINE_NEXT SymGetLineNext;                                          \
    PSYM_GET_LINE_NEXT_W64 SymGetLineNextW64;                                   \
    PSYM_GET_LINE_PREV64 SymGetLinePrev64;                                      \
    PSYM_GET_LINE_PREV SymGetLinePrev;                                          \
    PSYM_GET_LINE_PREV_W64 SymGetLinePrevW64;                                   \
    PSYM_GET_MODULE_BASE SymGetModuleBase;                                      \
    PSYM_GET_MODULE_INFO64 SymGetModuleInfo64;                                  \
    PSYM_GET_MODULE_INFO SymGetModuleInfo;                                      \
    PSYM_GET_MODULE_INFO_W64 SymGetModuleInfoW64;                               \
    PSYM_GET_MODULE_INFO_W SymGetModuleInfoW;                                   \
    PSYM_GET_OMAPS SymGetOmaps;                                                 \
    PSYM_GET_OPTIONS SymGetOptions;                                             \
    PSYM_GET_SCOPE SymGetScope;                                                 \
    PSYM_GET_SCOPE_W SymGetScopeW;                                              \
    PSYM_GET_SEARCH_PATH SymGetSearchPath;                                      \
    PSYM_GET_SEARCH_PATH_W SymGetSearchPathW;                                   \
    PSYM_GET_SOURCE_FILE_FROM_TOKEN SymGetSourceFileFromToken;                  \
    PSYM_GET_SOURCE_FILE_FROM_TOKEN_W SymGetSourceFileFromTokenW;               \
    PSYM_GET_SOURCE_FILE SymGetSourceFile;                                      \
    PSYM_GET_SOURCE_FILE_TOKEN SymGetSourceFileToken;                           \
    PSYM_GET_SOURCE_FILE_TOKEN_W SymGetSourceFileTokenW;                        \
    PSYM_GET_SOURCE_FILE_W SymGetSourceFileW;                                   \
    PSYM_GET_SOURCE_VAR_FROM_TOKEN SymGetSourceVarFromToken;                    \
    PSYM_GET_SOURCE_VAR_FROM_TOKEN_W SymGetSourceVarFromTokenW;                 \
    PSYM_GET_SYMBOL_FILE SymGetSymbolFile;                                      \
    PSYM_GET_SYMBOL_FILE_W SymGetSymbolFileW;                                   \
    PSYM_GET_SYM_FROM_ADDR64 SymGetSymFromAddr64;                               \
    PSYM_GET_SYM_FROM_ADDR SymGetSymFromAddr;                                   \
    PSYM_GET_SYM_FROM_NAME64 SymGetSymFromName64;                               \
    PSYM_GET_SYM_FROM_NAME SymGetSymFromName;                                   \
    PSYM_GET_SYM_NEXT64 SymGetSymNext64;                                        \
    PSYM_GET_SYM_NEXT SymGetSymNext;                                            \
    PSYM_GET_SYM_PREV64 SymGetSymPrev64;                                        \
    PSYM_GET_SYM_PREV SymGetSymPrev;                                            \
    PSYM_GET_TYPE_FROM_NAME SymGetTypeFromName;                                 \
    PSYM_GET_TYPE_FROM_NAME_W SymGetTypeFromNameW;                              \
    PSYM_GET_TYPE_INFO_EX SymGetTypeInfoEx;                                     \
    PSYM_GET_TYPE_INFO SymGetTypeInfo;                                          \
    PSYM_GET_UNWIND_INFO SymGetUnwindInfo;                                      \
    PSYM_INITIALIZE SymInitialize;                                              \
    PSYM_INITIALIZE_W SymInitializeW;                                           \
    PSYM_LOAD_MODULE SymLoadModule;                                             \
    PSYM_LOAD_MODULE_EX SymLoadModuleEx;                                        \
    PSYM_LOAD_MODULE_EX_W SymLoadModuleExW;                                     \
    PSYM_MATCH_FILE_NAME SymMatchFileName;                                      \
    PSYM_MATCH_FILE_NAME_W SymMatchFileNameW;                                   \
    PSYM_MATCH_STRING_A SymMatchStringA;                                        \
    PSYM_MATCH_STRING SymMatchString;                                           \
    PSYM_MATCH_STRING_W SymMatchStringW;                                        \
    PSYM_NEXT SymNext;                                                          \
    PSYM_NEXT_W SymNextW;                                                       \
    PSYM_PREV SymPrev;                                                          \
    PSYM_PREV_W SymPrevW;                                                       \
    PSYM_QUERY_INLINE_TRACE SymQueryInlineTrace;                                \
    PSYM_REFRESH_MODULE_LIST SymRefreshModuleList;                              \
    PSYM_REGISTER_CALLBACK64 SymRegisterCallback64;                             \
    PSYM_REGISTER_CALLBACK SymRegisterCallback;                                 \
    PSYM_REGISTER_CALLBACK_W64 SymRegisterCallbackW64;                          \
    PSYM_REGISTER_FUNCTION_ENTRY_CALLBACK64 SymRegisterFunctionEntryCallback64; \
    PSYM_REGISTER_FUNCTION_ENTRY_CALLBACK SymRegisterFunctionEntryCallback;     \
    PSYM_SEARCH SymSearch;                                                      \
    PSYM_SEARCH_W SymSearchW;                                                   \
    PSYM_SET_CONTEXT SymSetContext;                                             \
    PSYM_SET_OPTIONS SymSetOptions;                                             \
    PSYM_SET_PARENT_WINDOW SymSetParentWindow;                                  \
    PSYM_SET_SCOPE_FROM_ADDR SymSetScopeFromAddr;                               \
    PSYM_SET_SCOPE_FROM_INDEX SymSetScopeFromIndex;                             \
    PSYM_SET_SCOPE_FROM_INLINE_CONTEXT SymSetScopeFromInlineContext;            \
    PSYM_SET_SEARCH_PATH SymSetSearchPath;                                      \
    PSYM_SET_SEARCH_PATH_W SymSetSearchPathW;                                   \
    PSYM_SRV_GET_FILE_INDEXES SymSrvGetFileIndexes;                             \
    PSYM_SRV_GET_FILE_INDEXES_W SymSrvGetFileIndexesW;                          \
    PSYM_SRV_GET_FILE_INDEX_INFO SymSrvGetFileIndexInfo;                        \
    PSYM_SRV_GET_FILE_INDEX_INFO_W SymSrvGetFileIndexInfoW;                     \
    PSYM_SRV_GET_FILE_INDEX_STRING SymSrvGetFileIndexString;                    \
    PSYM_SRV_GET_FILE_INDEX_STRING_W SymSrvGetFileIndexStringW;                 \
    PSYM_SRV_IS_STORE SymSrvIsStore;                                            \
    PSYM_SRV_IS_STORE_W SymSrvIsStoreW;                                         \
    PSYM_UNDNAME64 SymUnDName64;                                                \
    PSYM_UNDNAME SymUnDName;                                                    \
    PSYM_UNLOAD_MODULE64 SymUnloadModule64;                                     \
    PSYM_UNLOAD_MODULE SymUnloadModule;                                         \
    PUNDECORATE_SYMBOL_NAME UnDecorateSymbolName;                               \
    PUNDECORATE_SYMBOL_NAME_W UnDecorateSymbolNameW;                            \
    PIMAGE_NT_HEADER ImageNtHeader;                                             \
    PIMAGE_DIRECTORY_ENTRY_TO_DATA ImageDirectoryEntryToData;                   \
    PIMAGE_DIRECTORY_ENTRY_TO_DATA_EX ImageDirectoryEntryToDataEx;              \
    PIMAGE_RVA_TO_SECTION ImageRvaToSection;                                    \
    PIMAGE_RVA_TO_VA ImageRvaToV;

//
// Helper/convenience enums/bitflags for the Flags, Scope, Type and Tag fields
// of the SYMBOL_INFO structure.
//

typedef enum _Enum_is_bitflag_ _SYMFLAG {
    SymFlagValuePresent      = 0x00000001,
    SymFlagRegister          = 0x00000008,
    SymFlagRegRel            = 0x00000010,
    SymFlagFrameRel          = 0x00000020,
    SymFlagParameter         = 0x00000040,
    SymFlagLocal             = 0x00000080,
    SymFlagConstant          = 0x00000100,
    SymFlagExport            = 0x00000200,
    SymFlagForwarder         = 0x00000400,
    SymFlagFunction          = 0x00000800,
    SymFlagVirtual           = 0x00001000,
    SymFlagThunk             = 0x00002000,
    SymFlagTlsRel            = 0x00004000,
    SymFlagSlot              = 0x00008000,
    SymFlagIlRel             = 0x00010000,
    SymFlagMetadata          = 0x00020000,
    SymFlagClrToken          = 0x00040000,
    SymFlagNull              = 0x00080000,
    SymFlagFuncNoReturn      = 0x00100000,
    SymFlagSyntheticZeroBase = 0x00200000,
    SymFlagPublicCode        = 0x00400000
} SYMFLAG;
typedef SYMFLAG *PSYMFLAG;

typedef union _SYM_FLAGS {
    struct _Struct_size_bytes_(sizeof(ULONG)) {
        ULONG ValuePresent:1;       //      0x00000001,
        ULONG Unused1:2;
        ULONG Register:1;           //      0x00000008
        ULONG RegRel:1;             //      0x00000010
        ULONG FrameRel:1;           //      0x00000020
        ULONG Parameter:1;          //      0x00000040
        ULONG Local:1;              //      0x00000080
        ULONG Constant:1;           //      0x00000100
        ULONG Export:1;             //      0x00000200
        ULONG Forwarder:1;          //      0x00000400
        ULONG Function:1;           //      0x00000800
        ULONG Virtual:1;            //      0x00001000
        ULONG Thunk:1;              //      0x00002000
        ULONG TlsRel:1;             //      0x00004000
        ULONG Slot:1;               //      0x00008000
        ULONG IlRel:1;              //      0x00010000
        ULONG Metadata:1;           //      0x00020000
        ULONG ClrToken:1;           //      0x00040000
        ULONG Null:1;               //      0x00080000
        ULONG FuncNoReturn:1;       //      0x00100000
        ULONG SyntheticZeroBase:1;  //      0x00200000
        ULONG PublicCode:1;         //      0x00400000
        ULONG SpareBits:9;
    };
    LONG AsLong;
    ULONG AsULong;
} SYM_FLAGS;
C_ASSERT(sizeof(SYM_FLAGS) == sizeof(ULONG));
typedef SYM_FLAGS *PSYM_FLAGS;

typedef enum SymTagEnum SYMTAG;

typedef struct _SYMINFO {
    ULONG       SizeOfStruct;
    SYM_TYPE    TypeIndex;
    ULONG64     Reserved[2];
    ULONG       Index;
    ULONG       Size;
    ULONG64     ModBase;
    SYMFLAG     Flags;
    ULONG64     Value;
    ULONG64     Address;
    ULONG       Register;
    ULONG       Scope;
    ULONG       Tag;
    ULONG       NameLen;
    ULONG       MaxNameLen;
    CHAR        Name[1];
} SYMINFO;

typedef struct _DBG {
    _DBGHELP_FUNCTIONS_HEAD
} DBG, *PDBG;

typedef
HRESULT
(STDAPICALLTYPE DEBUG_CREATE)(
    _In_ REFIID InterfaceId,
    _Out_ PPVOID Interface
    );
typedef DEBUG_CREATE *PDEBUG_CREATE;

#define _DBGENG_FUNCTIONS_HEAD \
    PDEBUG_CREATE DebugCreate;

typedef struct _DBGENG {
    _DBGENG_FUNCTIONS_HEAD
} DBGENG, *PDBGENG;

//
// COM-related typedefs.
//

typedef
HRESULT
(CO_INITIALIZE_EX)(
    _In_opt_ LPVOID Reserved,
    _In_ DWORD CoInit
    );
typedef CO_INITIALIZE_EX *PCO_INITIALIZE_EX;

typedef
_Check_return_
_Success_(return != 0)
BOOL
(INITIALIZE_COM)(
    _In_ PRTL Rtl
    );
typedef INITIALIZE_COM *PINITIALIZE_COM;

typedef
_Check_return_
HRESULT
(STDAPICALLTYPE DLL_GET_CLASS_OBJECT)(
    _In_ REFCLSID rclsid,
    _In_ REFIID riid,
    _Outptr_ LPVOID FAR* ppv
    );
typedef DLL_GET_CLASS_OBJECT *PDLL_GET_CLASS_OBJECT;

typedef
__control_entrypoint(DllExport)
HRESULT
(STDAPICALLTYPE DLL_CAN_UNLOAD_NOW)(
    VOID
    );
typedef DLL_CAN_UNLOAD_NOW *PDLL_CAN_UNLOAD_NOW;

#define DEFINE_GUID_EX(name, l, w1, w2, b1, b2, b3, b4, b5, b6, b7, b8) \
    const GUID DECLSPEC_SELECTANY name                                  \
        = { l, w1, w2, { b1, b2,  b3,  b4,  b5,  b6,  b7,  b8 } }

typedef GUID *PGUID;
typedef const GUID CGUID;
typedef GUID const *PCGUID;

//
// IUnknown
//

typedef
HRESULT
(STDAPICALLTYPE IUNKNOWN_QUERY_INTERFACE)(
    _In_ struct _IUNKNOWN *This,
    _In_ REFIID InterfaceId,
    _Out_ PPVOID Interface
    );
typedef IUNKNOWN_QUERY_INTERFACE *PIUNKNOWN_QUERY_INTERFACE;

typedef
ULONG
(STDAPICALLTYPE IUNKNOWN_ADD_REF)(
    _In_ struct _IUNKNOWN *This
    );
typedef IUNKNOWN_ADD_REF *PIUNKNOWN_ADD_REF;

typedef
ULONG
(STDAPICALLTYPE IUNKNOWN_RELEASE)(
    _In_ struct _IUNKNOWN *This
    );
typedef IUNKNOWN_RELEASE *PIUNKNOWN_RELEASE;

typedef struct _IUNKNOWN_VTBL {
    PIUNKNOWN_QUERY_INTERFACE QueryInterface;
    PIUNKNOWN_ADD_REF AddRef;
    PIUNKNOWN_RELEASE Release;
} IUNKNOWN_VTBL;
typedef IUNKNOWN_VTBL *PIUNKNOWN_VTBL;

typedef struct _IUNKNOWN {
    PIUNKNOWN_VTBL lpVtbl;
} IUNKNOWN;
typedef IUNKNOWN *PIUNKNOWN;

//
// IClassFactory
//

typedef
HRESULT
(STDAPICALLTYPE ICLASS_FACTORY_CREATE_INSTANCE)(
    _In_ struct _ICLASS_FACTORY *This,
    _In_opt_ PIUNKNOWN pUnkOuter,
    _In_ REFIID riid,
    _COM_Outptr_result_maybenull_ PPVOID ppvObject
    );
typedef ICLASS_FACTORY_CREATE_INSTANCE *PICLASS_FACTORY_CREATE_INSTANCE;

typedef
HRESULT
(STDAPICALLTYPE ICLASS_FACTORY_LOCK_SERVER)(
    _In_ struct _ICLASS_FACTORY *This,
    _In_opt_ BOOL Lock
    );
typedef ICLASS_FACTORY_LOCK_SERVER *PICLASS_FACTORY_LOCK_SERVER;

typedef struct _ICLASS_FACTORY_VTBL {
    PIUNKNOWN_QUERY_INTERFACE QueryInterface;
    PIUNKNOWN_ADD_REF AddRef;
    PIUNKNOWN_RELEASE Release;
    PICLASS_FACTORY_CREATE_INSTANCE CreateInstance;
    PICLASS_FACTORY_LOCK_SERVER LockServer;
} ICLASS_FACTORY_VTBL;
typedef ICLASS_FACTORY_VTBL *PICLASS_FACTORY_VTBL;

//
// Crypt-related typedefs.
//

typedef
BOOL
(CRYPT_GEN_RANDOM)(
    _In_ PRTL Rtl,
    _In_ ULONG SizeOfBufferInBytes,
    _Inout_updates_bytes_(SizeOfBufferInBytes) PBYTE Buffer
    );
typedef CRYPT_GEN_RANDOM *PCRYPT_GEN_RANDOM;

typedef
_Success_(return != 0)
BOOL
(WINAPI CRYPT_BINARY_TO_STRING_A)(
    _In_reads_bytes_(cbBinary) CONST BYTE *pbBinary,
    _In_ DWORD cbBinary,
    _In_ DWORD dwFlags,
    _Out_writes_to_opt_(*pcchString, *pcchString) LPSTR pszString,
    _Inout_ DWORD *pcchString
    );
typedef CRYPT_BINARY_TO_STRING_A *PCRYPT_BINARY_TO_STRING_A;

typedef
_Success_(return != 0)
BOOL
(WINAPI CRYPT_BINARY_TO_STRING_W)(
    _In_reads_bytes_(cbBinary) CONST BYTE *pbBinary,
    _In_ DWORD cbBinary,
    _In_ DWORD dwFlags,
    _Out_writes_to_opt_(*pcchString, *pcchString) LPWSTR pszString,
    _Inout_ DWORD *pcchString
    );
typedef CRYPT_BINARY_TO_STRING_W *PCRYPT_BINARY_TO_STRING_W;

typedef
LONG
(CALLBACK RTL_INJECTION_REMOTE_THREAD_ENTRY)(
    _In_ struct _RTL_INJECTION_CONTEXT *Context
    );
typedef RTL_INJECTION_REMOTE_THREAD_ENTRY *PRTL_INJECTION_REMOTE_THREAD_ENTRY;

typedef
BOOL
(CALLBACK RTL_SET_DLL_PATH)(
    _Inout_ PRTL Rtl,
    _In_ PALLOCATOR Allocator,
    _In_ PCUNICODE_STRING RtlDllPath
    );
typedef RTL_SET_DLL_PATH *PRTL_SET_DLL_PATH;

typedef
BOOL
(CALLBACK RTL_SET_INJECTION_THUNK_DLL_PATH)(
    _Inout_ PRTL Rtl,
    _In_ PALLOCATOR Allocator,
    _In_ PCUNICODE_STRING InjectionThunkDllPath
    );
typedef RTL_SET_INJECTION_THUNK_DLL_PATH
      *PRTL_SET_INJECTION_THUNK_DLL_PATH;

typedef
VOID
(RUNDOWN_GLOBAL_ATEXIT_FUNCTIONS)(
    _In_opt_ BOOL IsProcessTerminating
    );
typedef RUNDOWN_GLOBAL_ATEXIT_FUNCTIONS *PRUNDOWN_GLOBAL_ATEXIT_FUNCTIONS;
extern RUNDOWN_GLOBAL_ATEXIT_FUNCTIONS RundownGlobalAtExitFunctions;

typedef
_Check_return_
_Success_(return != 0)
BOOL
(CALLBACK RTL_CREATE_NAMED_EVENT)(
    _In_ PRTL Rtl,
    _In_ PALLOCATOR Allocator,
    _Inout_ PHANDLE HandlePointer,
    _In_opt_ LPSECURITY_ATTRIBUTES EventAttributes,
    _In_ BOOL ManualReset,
    _In_ BOOL InitialState,
    _In_opt_ PCUNICODE_STRING Prefix,
    _In_opt_ PCUNICODE_STRING Suffix,
    _Inout_ PUNICODE_STRING EventName
    );
typedef RTL_CREATE_NAMED_EVENT *PRTL_CREATE_NAMED_EVENT;

typedef
_Check_return_
_Success_(return != 0)
BOOL
(CALLBACK PROBE_FOR_READ)(
    _In_ PRTL Rtl,
    _In_reads_bytes_((NumberOfBytes + 4096 - 1) & ~(4096 - 1)) PVOID Address,
    _In_ SIZE_T NumberOfBytes,
    _Outptr_result_maybenull_ PULONG NumberOfValidPages
    );
typedef PROBE_FOR_READ *PPROBE_FOR_READ;

typedef
_Check_return_
_Success_(return != 0)
BOOL
(CALLBACK INITIALIZE_INJECTION)(
    _In_ struct _RTL *Rtl
    );
typedef INITIALIZE_INJECTION *PINITIALIZE_INJECTION;

typedef
_Check_return_
_Success_(return != 0)
BOOL
(GET_CU)(
    _Inout_ PRTL Rtl,
    _Outptr_result_nullonfailure_ struct _CU **CuPointer
    );
typedef GET_CU *PGET_CU;

typedef union _RTL_FLAGS {
    struct _Struct_size_bytes_(sizeof(ULONG)) {

        //
        // When set, indicates Intel's TSX extensions are available.
        //

        ULONG TsxAvailable:1;

        //
        // When set, indicates COM has been initialized.
        //

        ULONG ComInitialized:1;

        //
        // When set, indicates large pages are available.
        //

        ULONG IsLargePageEnabled:1;

        //
        // When set, indicates the nvcuda library has been loaded.
        //

        ULONG NvcudaInitialized:1;

        //
        // When set, indicates Crypt32 module has been load and the various
        // Crypt functions in RTL are available.
        //

        ULONG Crypt32Initialized:1;

    };
    LONG AsLong;
    ULONG AsULong;
} RTL_FLAGS;

typedef struct _Struct_size_bytes_(SizeOfStruct) _RTL {

    _Field_range_(==, sizeof(struct _RTL)) ULONG SizeOfStruct;

    RTL_FLAGS Flags;

    HMODULE     NtdllModule;
    HMODULE     Kernel32Module;
    HMODULE     KernelBaseModule;
    HMODULE     NtosKrnlModule;
    HMODULE     ShlwapiModule;
    HMODULE     Ole32Module;
    HMODULE     DbgHelpModule;
    HMODULE     DbgEngModule;
    HMODULE     InjectionThunkModule;
    HMODULE     NvcudaModule;
    HMODULE     Crypt32Module;

    PINITIALIZE_COM InitializeCom;
    PCO_INITIALIZE_EX CoInitializeEx;

    PGET_CU GetCu;
    struct _CU *Cu;

    DWORD LastError;
    ULONG Padding;

    SIZE_T LargePageMinimum;
    PALLOCATOR LargePageAllocator;
    PVIRTUAL_ALLOC TryLargePageVirtualAlloc;
    PVIRTUAL_ALLOC_EX TryLargePageVirtualAllocEx;

    PMAP_VIEW_OF_FILE_EX_NUMA MapViewOfFileExNuma;
    PMAP_VIEW_OF_FILE_NUMA2 MapViewOfFileNuma2;
    PTRY_MAP_VIEW_OF_FILE_NUMA2 TryMapViewOfFileNuma2;

    PATEXIT atexit;
    PATEXITEX AtExitEx;

    PRUNDOWN_GLOBAL_ATEXIT_FUNCTIONS RundownGlobalAtExitFunctions;

    PCOPY_PAGES CopyPages;
    PFILL_PAGES FillPages;
    PPROBE_FOR_READ ProbeForRead;

    PRTL_CREATE_NAMED_EVENT CreateNamedEvent;

    P__C_SPECIFIC_HANDLER __C_specific_handler;
    P__SECURITY_INIT_COOKIE __security_init_cookie;

    PVOID MaximumFileSectionSize;
    struct _RTL_LDR_NOTIFICATION_TABLE *LoaderNotificationTable;

    HANDLE HeapHandle;

    LARGE_INTEGER Frequency;
    LARGE_INTEGER Multiplicand;

    UNICODE_STRING WindowsDirectory;
    UNICODE_STRING WindowsSxSDirectory;
    UNICODE_STRING WindowsSystemDirectory;

    HCRYPTPROV CryptProv;
    PCRYPT_GEN_RANDOM CryptGenRandom;
    PCRYPT_BINARY_TO_STRING_A CryptBinaryToStringA;
    PCRYPT_BINARY_TO_STRING_W CryptBinaryToStringW;

    POUTPUT_DEBUG_STRING_A OutputDebugStringA;
    POUTPUT_DEBUG_STRING_W OutputDebugStringW;

    PCREATE_EVENT_A CreateEventA;
    PCREATE_EVENT_W CreateEventW;

    //
    // Fully-qualified module path name set by SetDllPath().
    //

    UNICODE_STRING RtlDllPath;
    PRTL_SET_DLL_PATH SetDllPath;

    //
    // Fully-qualified module path name set by SetInjectionThunkDllPath().
    //

    UNICODE_STRING InjectionThunkDllPath;
    PRTL_SET_INJECTION_THUNK_DLL_PATH SetInjectionThunkDllPath;

    PCOPY_FUNCTION CopyFunction;
    PCREATE_RANDOM_OBJECT_NAMES CreateRandomObjectNames;
    PCREATE_SINGLE_RANDOM_OBJECT_NAME CreateSingleRandomObjectName;
    PCREATE_BUFFER CreateBuffer;
    PDESTROY_BUFFER DestroyBuffer;
    PMAKE_RANDOM_STRING MakeRandomString;
    PFIND_AND_REPLACE_BYTE FindAndReplaceByte;

    PVOID InjectionThunkRoutine;
    PINJECT Inject;
    PINITIALIZE_INJECTION InitializeInjection;
    PADJUST_THUNK_POINTERS AdjustThunkPointers;

    INJECTION_FUNCTIONS InjectionFunctions;

    PLOAD_SYMBOLS LoadSymbols;
    PLOAD_SYMBOLS_FROM_MULTIPLE_MODULES LoadSymbolsFromMultipleModules;

    PINITIALIZE_ALLOCATOR InitializeHeapAllocator;
    PDESTROY_ALLOCATOR DestroyHeapAllocator;

    union {
        SYSTEM_TIMER_FUNCTION   SystemTimerFunction;
        struct {
            _SYSTEM_TIMER_FUNCTIONS_HEAD
        };
    };

    union {
        RTLFUNCTIONS RtlFunctions;
        struct {
            _RTLFUNCTIONS_HEAD
        };
    };

    union {
        RTLEXFUNCTIONS RtlExFunctions;
        struct {
            _RTLEXFUNCTIONS_HEAD
        };
    };

    union {
        SHLWAPI_FUNCTIONS ShlwapiFunctions;
        struct {
            _SHLWAPIFUNCTIONS_HEAD
        };
    };

    union {
        DBG Dbg;
        struct {
            _DBGHELP_FUNCTIONS_HEAD
        };
    };

    union {
        DBGENG DbgEng;
        struct {
            _DBGENG_FUNCTIONS_HEAD
        };
    };

#ifdef _RTL_TEST
    PTEST_LOAD_SYMBOLS TestLoadSymbols;
    PTEST_LOAD_SYMBOLS_FROM_MULTIPLE_MODULES TestLoadSymbolsFromMultipleModules;
    PTEST_CREATE_AND_DESTROY_BUFFER TestCreateAndDestroyBuffer;
#endif

} RTL, *PRTL, **PPRTL;

FORCEINLINE
ULONG
FilterLargePageFlags(
    _In_ PRTL Rtl,
    _In_ ULONG Flags
    )
{
    if (!Rtl->Flags.IsLargePageEnabled) {
        return Flags & ~(MEM_LARGE_PAGES | SEC_LARGE_PAGES);
    } else {
        return Flags;
    }
}

FORCEINLINE
ULONG
TrailingZeros(
    _In_ ULONG Integer
    )
{
    return _tzcnt_u32(Integer);
}

FORCEINLINE
ULONG
LeadingZeros(
    _In_ ULONG Integer
    )
{
    return _lzcnt_u32(Integer);
}

FORCEINLINE
ULONGLONG
TrailingZeros64(
    _In_ ULONGLONG Integer
    )
{
    return _tzcnt_u64(Integer);
}

FORCEINLINE
ULONGLONG
LeadingZeros64(
    _In_ ULONGLONG Integer
    )
{
    return _lzcnt_u64(Integer);
}

FORCEINLINE
ULONG
PopulationCount32(
    _In_ ULONG Integer
    )
{
    return __popcnt(Integer);
}

FORCEINLINE
ULONGLONG
GetAddressAlignment(_In_ PVOID Address)
{
    ULONGLONG Integer = (ULONGLONG)Address;
    ULONGLONG NumTrailingZeros = TrailingZeros64(Integer);
    return (1ULL << NumTrailingZeros);
}

FORCEINLINE
BOOL
PointerToOffsetCrossesPageBoundary(
    _In_ PVOID Pointer,
    _In_ LONG_PTR Offset
    )
{
    LONG_PTR ThisPage;
    LONG_PTR NextPage;

    ThisPage = ALIGN_DOWN(Pointer, PAGE_SIZE);
    NextPage = ALIGN_DOWN(((ULONG_PTR)(Pointer)+Offset), PAGE_SIZE);

    return (ThisPage != NextPage);
}

FORCEINLINE
BOOL
IsSamePage(
    _In_ PVOID Left,
    _In_ PVOID Right
    )
{
    LONG_PTR LeftPage;
    LONG_PTR RightPage;

    LeftPage = ALIGN_DOWN(Left, PAGE_SIZE);
    RightPage = ALIGN_DOWN(Right, PAGE_SIZE);
    return (LeftPage == RightPage);
}

FORCEINLINE
BOOL
AssertTrue(
    _In_ PSTR Expression,
    _In_ BOOL Value
    )
{
    if (!Value) {
        OutputDebugStringA(Expression);
        __debugbreak();
        return FALSE;
    }
    return TRUE;
}

FORCEINLINE
BOOL
AssertFalse(
    _In_ PSTR Expression,
    _In_ BOOL Value
    )
{
    if (Value) {
        OutputDebugStringA(Expression);
        __debugbreak();
        return FALSE;
    }
    return TRUE;
}

_Success_(return != 0)
FORCEINLINE
BOOL
AssertAligned(
    _In_ PVOID Address,
    _In_ USHORT Alignment
    )
{
    ULONGLONG CurrentAlignment = GetAddressAlignment(Address);
    ULONGLONG ExpectedAlignment = ALIGN_UP(CurrentAlignment, Alignment);
    if (CurrentAlignment < ExpectedAlignment) {
#ifdef _DEBUG
        __debugbreak();
#endif
        OutputDebugStringA("Alignment failed!\n");
        return FALSE;
    }
    return TRUE;
}

#define AssertAligned8(Address)      AssertAligned((PVOID)Address, 8)
#define AssertAligned16(Address)     AssertAligned((PVOID)Address, 16)
#define AssertAligned32(Address)     AssertAligned((PVOID)Address, 32)
#define AssertAligned64(Address)     AssertAligned((PVOID)Address, 64)
#define AssertAligned512(Address)    AssertAligned((PVOID)Address, 512)
#define AssertAligned1024(Address)   AssertAligned((PVOID)Address, 1024)
#define AssertAligned2048(Address)   AssertAligned((PVOID)Address, 2048)
#define AssertAligned4096(Address)   AssertAligned((PVOID)Address, 4096)
#define AssertAligned8192(Address)   AssertAligned((PVOID)Address, 8192)

#define AssertPageAligned(Address)  AssertAligned4096(Address)

FORCEINLINE
_Success_(return != 0)
BOOL
IsAligned(
    _In_ PVOID Address,
    _In_ USHORT Alignment
    )
{
    ULONGLONG CurrentAlignment = GetAddressAlignment(Address);
    ULONGLONG ExpectedAlignment = ALIGN_UP(CurrentAlignment, Alignment);
    return CurrentAlignment == ExpectedAlignment;
}

#define IsAligned8(Address)      IsAligned((PVOID)Address, 8)
#define IsAligned16(Address)     IsAligned((PVOID)Address, 16)
#define IsAligned32(Address)     IsAligned((PVOID)Address, 32)
#define IsAligned64(Address)     IsAligned((PVOID)Address, 64)
#define IsAligned512(Address)    IsAligned((PVOID)Address, 512)
#define IsAligned1024(Address)   IsAligned((PVOID)Address, 1024)
#define IsAligned2048(Address)   IsAligned((PVOID)Address, 2048)
#define IsAligned4096(Address)   IsAligned((PVOID)Address, 4096)
#define IsAligned8192(Address)   IsAligned((PVOID)Address, 8192)

#define IsPageAligned(Address)  IsAligned4096(Address)

#define IsPowerOf2(X) (((X) & ((X)-1)) == 0)


////////////////////////////////////////////////////////////////////////////////
// SIMD Utilities
////////////////////////////////////////////////////////////////////////////////

typedef __m128i DECLSPEC_ALIGN(16) XMMWORD, *PXMMWORD, **PPXMMWORD;
typedef __m256i DECLSPEC_ALIGN(32) YMMWORD, *PYMMWORD, **PPYMMWORD;
typedef __m512i DECLSPEC_ALIGN(64) ZMMWORD, *PZMMWORD, **PPZMMWORD;

#pragma optimize("", off)
static
NOINLINE
VOID
CanWeUseAvx512(PBOOLEAN UseAvx512Pointer)
{
    BOOLEAN UseAvx512 = TRUE;
    TRY_AVX512 {
        ZMMWORD Test1 = _mm512_set1_epi64(1);
        ZMMWORD Test2 = _mm512_add_epi64(Test1, Test1);
        UNREFERENCED_PARAMETER(Test2);
    } CATCH_EXCEPTION_ILLEGAL_INSTRUCTION{
        UseAvx512 = FALSE;
    }
    *UseAvx512Pointer = UseAvx512;
}
#pragma optimize("", on)

FORCEINLINE
VOID
StoreXmm(
    _In_ XMMWORD *Destination,
    _In_ XMMWORD  Source
    )
{
    TRY_SSE42_ALIGNED {

        _mm_store_si128(Destination, Source);

    } CATCH_EXCEPTION_ACCESS_VIOLATION {

        _mm_storeu_si128(Destination, Source);
    }
}

FORCEINLINE
VOID
Store2Xmm(
    _In_ XMMWORD *Destination128Low,
    _In_ XMMWORD *Destination128High,
    _In_ XMMWORD  Source128Low,
    _In_ XMMWORD  Source128High
    )
{
    TRY_SSE42_ALIGNED {

        _mm_store_si128(Destination128Low, Source128Low);
        _mm_store_si128(Destination128High, Source128High);

    } CATCH_EXCEPTION_ACCESS_VIOLATION {

        _mm_storeu_si128(Destination128Low, Source128Low);
        _mm_storeu_si128(Destination128High, Source128High);

    }
}


FORCEINLINE
VOID
StoreYmmFallbackXmm(
    _In_ PYMMWORD Destination,
    _In_ PXMMWORD Destination128Low,
    _In_ PXMMWORD Destination128High,
    _In_ YMMWORD  Source,
    _In_ XMMWORD  Source128Low,
    _In_ XMMWORD  Source128High
    )
{
    TRY_AVX {

        TRY_AVX_ALIGNED {

            _mm256_store_si256(Destination, Source);

        } CATCH_EXCEPTION_ILLEGAL_INSTRUCTION {

            Store2Xmm(
                Destination128Low,
                Destination128High,
                Source128Low,
                Source128High
            );

        }

    } CATCH_EXCEPTION_ACCESS_VIOLATION {

        _mm256_storeu_si256(Destination, Source);
    }
}

FORCEINLINE
ULONG
CompressUlongNaive(
    _In_ ULONG Input,
    _In_ ULONG Mask
    )
{
    ULONG Bit;
    ULONG Shift;
    ULONG Result;

    Shift = 0;
    Result = 0;

    do {
        Bit = Mask & 1;
        Result = Result | ((Input & Bit) << Shift);
        Shift = Shift + Bit;
        Input = Input >> 1;
        Mask = Mask >> 1;
    } while (Mask != 0);

    return Result;
}

typedef DECLSPEC_ALIGN(32) union _PARALLEL_SUFFIX_MOVE_MASK32 {
    struct {
        ULONG Mask;
        union {
            struct {
                ULONG Move0;
                ULONG Move1;
                ULONG Move2;
                ULONG Move3;
                ULONG Move4;
                ULONG Unused5;
                ULONG Unused6;
            };
            ULONG Moves[7];
        };
    };
    YMMWORD Move256;
    struct {
        XMMWORD MoveLow128;
        XMMWORD MoveHigh128;
    };
} PARALLEL_SUFFIX_MOVE_MASK32, *PPARALLEL_SUFFIX_MOVE_MASK32;

FORCEINLINE
VOID
CreateParallelSuffixMoveMask(
    _In_ ULONG Mask,
    _In_ PPARALLEL_SUFFIX_MOVE_MASK32 ParallelSuffixPointer
    )
{
    BYTE Index;
    ULONG Key;
    ULONG Move;
    ULONG Parallel;
    PPARALLEL_SUFFIX_MOVE_MASK32 Dest;

    PARALLEL_SUFFIX_MOVE_MASK32 Suffix;

    Suffix.Mask = Mask;

    //
    // Count zeros to the right.
    //

    Key = ~Mask << 1;

    for (Index = 0; Index < 5; Index++) {

        //
        // Compute the parallel suffix.
        //

        Parallel = Key ^ (Key << 1);

        Parallel = Parallel ^ (Parallel << 2);
        Parallel = Parallel ^ (Parallel << 4);
        Parallel = Parallel ^ (Parallel << 8);
        Parallel = Parallel ^ (Parallel << 16);

        //
        // Calculate how many bits to move.
        //

        Move = Suffix.Moves[Index] = Parallel & Mask;

        //
        // Compress the mask.
        //

        Mask = Mask ^ Move | (Move >> (1 << Index));

        Key = Key & ~Parallel;
    }

    Dest = ParallelSuffixPointer;

    StoreYmmFallbackXmm(
        &(Dest->Move256),
        &(Dest->MoveLow128),
        &(Dest->MoveHigh128),
        Suffix.Move256,
        Suffix.MoveLow128,
        Suffix.MoveHigh128
    );

    return;
}

FORCEINLINE
ULONG
CompressUlongParallelSuffixDynamicMask(
    _In_ ULONG Input,
    _In_ ULONG Mask
    )
{
    BYTE Index;
    ULONG Key;
    ULONG Parallel;
    ULONG Move;
    ULONG Bit;

    //
    // Clear irrelevant bits.
    //

    Input = Input & Mask;

    //
    // Count zeros to the right.
    //

    Key = ~Mask << 1;

    for (Index = 0; Index < 5; Index++) {

        //
        // Compute the parallel suffix.
        //

        Parallel = Key ^ (Key << 1);

        Parallel = Parallel ^ (Parallel << 2);
        Parallel = Parallel ^ (Parallel << 4);
        Parallel = Parallel ^ (Parallel << 8);
        Parallel = Parallel ^ (Parallel << 16);

        //
        // Calculate how many bits to move.
        //

        Move = Parallel & Mask;

        //
        // Compress the mask.
        //

        Mask = Mask ^ Move | (Move >> (1 << Index));

        Bit = Input & Move;

        //
        // Compress input.
        //

        Input = Input ^ Bit | (Bit >> (1 << Index));

        Key = Key & ~Parallel;
    }

    return Input;
}

FORCEINLINE
ULONG
CompressUlongParallelSuffixMem(
    _In_ ULONG Input,
    _In_ PPARALLEL_SUFFIX_MOVE_MASK32 Suffix
    )
{
    ULONG Bit;
    ULONG Mask = Suffix->Mask;

    Input = Input & Mask;

    Bit = Input & Suffix->Move0; Input = Input ^ Bit | (Bit >> 1);
    Bit = Input & Suffix->Move1; Input = Input ^ Bit | (Bit >> 2);
    Bit = Input & Suffix->Move2; Input = Input ^ Bit | (Bit >> 4);
    Bit = Input & Suffix->Move3; Input = Input ^ Bit | (Bit >> 8);
    Bit = Input & Suffix->Move4; Input = Input ^ Bit | (Bit >> 16);

    return Input;
}

FORCEINLINE
ULONG
CompressUlongParallelSuffix(
    _In_ ULONG Input,
    _In_ PPARALLEL_SUFFIX_MOVE_MASK32 SuffixPointer
    )
{
    ULONG Bit;
    ULONG Mask;
    YMMWORD Move;

    Move = _mm256_load_si256(&SuffixPointer->Move256);

    Mask = SuffixPointer->Mask;

    Input = Input & Mask;

    Bit = Input & Move.m256i_u32[1]; Input = Input ^ Bit | (Bit >> 1);
    Bit = Input & Move.m256i_u32[2]; Input = Input ^ Bit | (Bit >> 2);
    Bit = Input & Move.m256i_u32[3]; Input = Input ^ Bit | (Bit >> 4);
    Bit = Input & Move.m256i_u32[4]; Input = Input ^ Bit | (Bit >> 8);
    Bit = Input & Move.m256i_u32[5]; Input = Input ^ Bit | (Bit >> 16);

    return Input;
}

typedef
_Success_(return != 0)
_Check_return_
BOOL
(INITIALIZE_RTL)(
    _Out_bytecap_(*SizeOfRtl) PRTL   Rtl,
    _Inout_                   PULONG SizeOfRtl
    );
typedef INITIALIZE_RTL *PINITIALIZE_RTL;

FORCEINLINE
VOID
InitializeStringFromString(
    _Out_   PSTRING     Dest,
    _In_    PCSTRING    Source
    )
{
    Dest->Length = Source->Length;
    Dest->MaximumLength = Source->MaximumLength;
    Dest->Buffer = Source->Buffer;
}

FORCEINLINE
VOID
InitializeUnicodeStringFromUnicodeString(
    _Out_   PUNICODE_STRING     Dest,
    _In_    PCUNICODE_STRING    Source
    )
{
    Dest->Length = Source->Length;
    Dest->MaximumLength = Source->MaximumLength;
    Dest->Buffer = Source->Buffer;
}

FORCEINLINE
BOOL
IsValidNullTerminatedUnicodeStringWithMinimumLengthInChars(
    _In_ PCUNICODE_STRING String,
    _In_ USHORT MinimumLengthInChars
    )
{
    //
    // Add 1 to account for the NULL.
    //

    USHORT Length = (MinimumLengthInChars + 1) * sizeof(WCHAR);
    USHORT MaximumLength = Length + sizeof(WCHAR);

    return (
        String != NULL &&
        String->Buffer != NULL &&
        String->Length >= Length &&
        String->MaximumLength >= MaximumLength &&
        sizeof(WCHAR) == (String->MaximumLength - String->Length) &&
        String->Buffer[String->Length >> 1] == L'\0'
    );
}

FORCEINLINE
BOOL
IsValidNullTerminatedStringWithMinimumLengthInChars(
    _In_ PCSTRING String,
    _In_ USHORT MinimumLengthInChars
    )
{
    //
    // Add 1 to account for the NULL.
    //

    USHORT Length = (MinimumLengthInChars + 1) * sizeof(CHAR);
    USHORT MaximumLength = Length + sizeof(CHAR);

    return (
        String != NULL &&
        String->Buffer != NULL &&
        String->Length >= Length &&
        String->MaximumLength >= MaximumLength &&
        sizeof(CHAR) == (String->MaximumLength - String->Length) &&
        String->Buffer[String->Length >> 1] == '\0'
    );
}

FORCEINLINE
BOOL
IsValidUnicodeStringWithMinimumLengthInChars(
    _In_ PCUNICODE_STRING String,
    _In_ USHORT MinimumLengthInChars
    )
{
    USHORT Length = MinimumLengthInChars * sizeof(WCHAR);

    return (
        String != NULL &&
        String->Buffer != NULL &&
        String->Length >= Length &&
        String->MaximumLength >= Length &&
        String->MaximumLength >= String->Length
    );
}

FORCEINLINE
BOOL
IsValidStringWithMinimumLengthInChars(
    _In_ PCSTRING String,
    _In_ USHORT MinimumLengthInChars
    )
{
    USHORT Length = MinimumLengthInChars * sizeof(CHAR);

    return (
        String != NULL &&
        String->Buffer != NULL &&
        String->Length >= Length &&
        String->MaximumLength >= Length &&
        String->MaximumLength >= String->Length
    );
}

FORCEINLINE
BOOL
IsValidMinimumDirectoryUnicodeString(
    _In_ PCUNICODE_STRING String
    )
{
    return IsValidUnicodeStringWithMinimumLengthInChars(
        String,
        4
    );
}

FORCEINLINE
BOOL
IsValidNullTerminatedUnicodeString(
    _In_ PCUNICODE_STRING String
    )
{
    return IsValidNullTerminatedUnicodeStringWithMinimumLengthInChars(
        String,
        1
    );
}

FORCEINLINE
BOOL
IsValidNullTerminatedString(
    _In_ PCSTRING String
    )
{
    return IsValidNullTerminatedStringWithMinimumLengthInChars(String, 1);
}

FORCEINLINE
BOOL
IsValidMinimumDirectoryNullTerminatedUnicodeString(
    _In_ PCUNICODE_STRING String
    )
{
    //
    // Minimum length: "C:\a" -> 4.
    //

    return IsValidNullTerminatedUnicodeStringWithMinimumLengthInChars(
        String,
        4
    );
}

FORCEINLINE
BOOL
IsValidUnicodeString(
    _In_ PCUNICODE_STRING String
    )
{
    return (
        String != NULL &&
        String->Buffer != NULL &&
        String->Length >= 1 &&
        String->MaximumLength >= 1
    );
}

FORCEINLINE
BOOL
AppendUnicodeCharToUnicodeString(
    _Inout_ PUNICODE_STRING Destination,
    _In_    WCHAR           Char
    )
{
    USHORT NewOffset = Destination->Length >> 1;
    USHORT NewLength = Destination->Length + sizeof(WCHAR);

    if (NewLength > Destination->MaximumLength) {
        return FALSE;
    }

    Destination->Buffer[NewOffset] = Char;
    Destination->Length = NewLength;

    return TRUE;
}

FORCEINLINE
BOOL
AppendCharToString(
    _Inout_ PSTRING Destination,
    _In_    CHAR    Char
    )
{
    USHORT NewOffset = Destination->Length;
    USHORT NewLength = Destination->Length + sizeof(CHAR);

    if (NewLength > Destination->MaximumLength) {
        return FALSE;
    }

    Destination->Buffer[NewOffset] = Char;
    Destination->Length = NewLength;

    return TRUE;
}

FORCEINLINE
BOOL
AppendStringAndCharToString(
    _Inout_ PSTRING  Destination,
    _In_    PCSTRING String,
    _In_    CHAR     Char
    )
{
    USHORT NewOffset = Destination->Length;
    USHORT NewLength = NewOffset + String->Length + 1;

    if (NewLength > Destination->MaximumLength) {
        return FALSE;
    }

    __movsb((PBYTE)&Destination->Buffer[NewOffset],
            (PBYTE)String->Buffer,
            String->Length);

    NewOffset += String->Length;
    Destination->Buffer[NewOffset] = Char;
    Destination->Length = NewLength;

    return TRUE;
}

FORCEINLINE
BOOL
AppendCharAndStringToString(
    _Inout_ PSTRING  Destination,
    _In_    CHAR     Char,
    _In_    PCSTRING String
    )
{
    USHORT NewOffset = Destination->Length;
    USHORT NewLength = NewOffset + String->Length + 1;

    if (NewLength > Destination->MaximumLength) {
        return FALSE;
    }

    Destination->Buffer[NewOffset] = Char;
    NewOffset += 1;

    __movsb((PBYTE)&Destination->Buffer[NewOffset],
            (PBYTE)String->Buffer,
            String->Length);

    NewOffset += String->Length;
    Destination->Buffer[NewOffset] = Char;
    Destination->Length = NewLength;

    return TRUE;
}

FORCEINLINE
BOOL
AppendCharAndStringAndCharToString(
    _Inout_ PSTRING Destination,
    _In_    CHAR    FirstChar,
    _In_    PSTRING String,
    _In_    CHAR    SecondChar
    )
{
    USHORT NewOffset = Destination->Length;
    USHORT NewLength = NewOffset + String->Length + 2;

    if (NewLength > Destination->MaximumLength) {
        return FALSE;
    }

    Destination->Buffer[NewOffset] = FirstChar;
    NewOffset += 1;

    __movsb((PBYTE)&Destination->Buffer[NewOffset],
            (PBYTE)String->Buffer,
            String->Length);

    NewOffset += String->Length;
    Destination->Buffer[NewOffset] = SecondChar;
    Destination->Length = NewLength;

    return TRUE;
}

_Success_(return != 0)
FORCEINLINE
BOOL
CreateUnicodeStringInline(
    _In_  PRTL                  Rtl,
    _In_  PCUNICODE_STRING      Source,
    _Out_ PPUNICODE_STRING      Destination,
    _In_  PALLOCATION_ROUTINE   AllocationRoutine,
    _In_  PVOID                 AllocationContext
    )
{
    PUNICODE_STRING String;
    USHORT Length = Source->Length;
    ULONG AllocationSize = Length + sizeof(UNICODE_STRING);

    String = (PUNICODE_STRING)AllocationRoutine(AllocationContext,
                                                AllocationSize);

    if (!String) {
        return FALSE;
    }

    String->Length = 0;
    String->MaximumLength = Length;
    String->Buffer = (PWSTR)RtlOffsetToPointer(String, sizeof(UNICODE_STRING));

    Rtl->RtlCopyUnicodeString(String, Source);
    *Destination = String;

    return TRUE;
}

FORCEINLINE
VOID
CopyUnicodeStringInline(
    _In_    PRTL                  Rtl,
    _In_    PCUNICODE_STRING      Source,
    _Inout_ PUNICODE_STRING       Destination
    )
{
    Destination->Length = 0;
    Destination->MaximumLength = Source->Length;
    Destination->Buffer = (PWSTR)RtlOffsetToPointer(Destination,
                                                    sizeof(UNICODE_STRING));

    Rtl->RtlCopyUnicodeString(Destination, Source);
}

_Success_(return != 0)
FORCEINLINE
BOOL
CopyString(
    _In_     PSTRING Destination,
    _In_opt_ PCSTRING Source
    )
{
    USHORT Length;

    if (!ARGUMENT_PRESENT(Destination)) {
        return FALSE;
    }

    if (!ARGUMENT_PRESENT(Source)) {
        Destination->Length = 0;
        return FALSE;
    }

    Length = min(Destination->MaximumLength, Source->Length);

    __movsb((PBYTE)Destination->Buffer,
            (PBYTE)Source->Buffer,
            Length);

    Destination->Length = Length;

    return TRUE;
}

_Success_(return != 0)
FORCEINLINE
BOOLEAN
AllocateAndCopyWideString(
    _In_ PALLOCATOR Allocator,
    _In_ USHORT SizeInBytes,
    _In_reads_z_(SizeInBytes) PWCHAR Buffer,
    _In_ PUNICODE_STRING String
    )
/*++

Routine Description:

    This routine creates a copy of a wide character string using an allocator.

Arguments:

    Allocator - Supplies a pointer to an ALLOCATOR structure.

    SizeInBytes - Supplies the length of the Buffer parameter, in bytes,
        including the trailing NULL.

    Buffer - Supplies a pointer to an array of wide characters to copy.

    String - Supplies a pointer to a UNICODE_STRING structure that will be
        configured to point at the newly-allocated wide string character
        buffer.

Return Value:

    TRUE on success, FALSE on failure.

--*/
{
    USHORT AlignedSizeInBytes;

    AlignedSizeInBytes = ALIGN_UP_USHORT_TO_POINTER_SIZE(SizeInBytes);

    String->Buffer = (PWCHAR)(
        Allocator->Calloc(
            Allocator->Context,
            1,
            AlignedSizeInBytes
        )
    );

    if (!String->Buffer) {
        return FALSE;
    }

    //
    // Length has 2 subtracted to account for the trailing NULL.
    //

    String->Length = (SizeInBytes - 2);
    String->MaximumLength = AlignedSizeInBytes;

    //
    // Copy the string in WCHAR chunks.
    //

    __movsw((PWORD)String->Buffer, (PWORD)Buffer, SizeInBytes >> 1);

    return TRUE;
}

_Success_(return != 0)
FORCEINLINE
BOOLEAN
AllocateAndCopyUnicodeString(
    _In_ PALLOCATOR Allocator,
    _In_ PCUNICODE_STRING SourceString,
    _In_ PUNICODE_STRING DestString
    )
/*++

Routine Description:

    This routine creates a copy of a UNICODE_STRING.

Arguments:

    Allocator - Supplies a pointer to an ALLOCATOR structure.

    SourceString - Supplies a pointer to a UNICODE_STRING structure to copy.

    DestString - Supplies a pointer to a UNICODE_STRING structure that will
        have its Length, MaximumLength and Buffer fields updated to match the
        newly-allocated buffer.

Return Value:

    TRUE on success, FALSE on failure.

--*/
{
    USHORT AlignedSizeInBytes;

    AlignedSizeInBytes = ALIGN_UP_USHORT_TO_POINTER_SIZE(SourceString->Length);

    DestString->Buffer = (PWCHAR)(
        Allocator->Calloc(
            Allocator->Context,
            1,
            AlignedSizeInBytes
        )
    );

    if (!DestString->Buffer) {
        return FALSE;
    }

    DestString->Length = SourceString->Length;
    DestString->MaximumLength = AlignedSizeInBytes;

    //
    // Copy the string in WCHAR chunks.
    //

    __movsw((PWORD)DestString->Buffer,
            (PWORD)SourceString->Buffer,
            DestString->Length >> 1);

    return TRUE;
}

FORCEINLINE
VOID
ClearUnicodeString(_Inout_ PUNICODE_STRING String)
{
    String->Length = 0;
    String->MaximumLength = 0;
    String->Buffer = NULL;
}

FORCEINLINE
VOID
ClearString(_Inout_ PSTRING String)
{
    String->Length = 0;
    String->MaximumLength = 0;
    String->Buffer = NULL;
}

FORCEINLINE
ULONG
HashUnicodeToAtom(_In_ PCWSTR String)
{
    WCHAR const *Buffer;
    WCHAR Char;
    ULONG Hash;

    Hash = 0;
    Buffer = String;
    while (*Buffer != UNICODE_NULL) {
        Char = RtlUpcaseUnicodeChar(*Buffer++);
        Hash = Hash + (Char << 1) + (Char >> 1) + Char;
    }

    return Hash;
}

FORCEINLINE
ULONG
HashUnicodeStringToAtom(_In_ PCUNICODE_STRING String)
{
    WCHAR const *Buffer;
    WCHAR Char;
    ULONG Hash;
    USHORT Index;

    Hash = 0;
    Buffer = String->Buffer;

    for (Index = 0; Index < String->Length >> 1; Index++) {
        Char = RtlUpcaseUnicodeChar(Buffer[Index]);
        Hash = Hash + (Char << 1) + (Char >> 1) + Char;
    }

    return Hash;
}

FORCEINLINE
ULONG
HashAnsiToAtom(_In_ PCSTR String)
{
    CHAR const *Pointer;
    CHAR Char;
    ULONG Hash;

    Hash = 0;
    Pointer = String;
    while (*Pointer != '\0') {
        Char = RtlUpcaseChar(*Pointer++);
        Hash = Hash + (Char << 1) + (Char >> 1) + Char;
    }

    return Hash;
}

FORCEINLINE
ULONG
HashAnsiStringToAtom(_In_ PCSTRING String)
{
    CHAR const *Buffer;
    CHAR Char;
    ULONG Hash;
    USHORT Index;

    Hash = 0;
    Buffer = String->Buffer;

    for (Index = 0; Index < String->Length; Index++) {
        Char = RtlUpcaseChar(Buffer[Index]);
        Hash = Hash + (Char << 1) + (Char >> 1) + Char;
    }

    return Hash;
}

FORCEINLINE
VOID
PrintStringToDebugStream(
    _In_ PSTRING String
    )
{
    CHAR Temp = String->Buffer[String->Length];
    String->Buffer[String->Length] = '\0';
    OutputDebugStringA(String->Buffer);
    String->Buffer[String->Length] = Temp;
}

FORCEINLINE
VOID
PrintUnicodeStringToDebugStream(
    _In_ PUNICODE_STRING String
    )
{
    USHORT Index = String->Length >> 1;
    WCHAR Temp = String->Buffer[Index];
    String->Buffer[Index] = L'\0';
    OutputDebugStringW(String->Buffer);
    String->Buffer[Index] = Temp;
}

FORCEINLINE
VOID
PrintAsciiToDebugStream(
    _In_ PSTR Text,
    _In_opt_ SIZE_T Length
    )
{
    STRING String;

    if (!Length) {
        Length = strlen(Text);
    }

    //
    // Cap the length at MAX_USHORT.
    //

    String.Length = (USHORT)(min(Length, (SIZE_T)MAX_USHORT));
    String.MaximumLength = String.Length;
    String.Buffer = Text;
    PrintStringToDebugStream(&String);
}

FORCEINLINE
BOOL
InitializeRtlManuallyInline(PRTL Rtl, PULONG SizeOfRtl)
{
    PROC Proc;
    BOOL Success;
    HMODULE Module;
    PINITIALIZE_RTL InitializeRtl;

    Module = LoadLibraryA("Rtl");

    if (!Module) {
        return FALSE;
    }

    Proc = GetProcAddress(Module, "InitializeRtl");
    if (!Proc) {
        __debugbreak();
    }

    InitializeRtl = (PINITIALIZE_RTL)Proc;
    if (!InitializeRtl) {
        __debugbreak();
    }

    Success = InitializeRtl(Rtl, SizeOfRtl);
    if (!Success) {
        __debugbreak();
    }

    return TRUE;
}

typedef
_Check_return_
_Success_(return != 0)
RTL_API
BOOL
(CREATE_AND_INITIALIZE_RTL)(
    _In_ struct _TRACER_CONFIG *TracerConfig,
    _In_ PALLOCATOR Allocator,
    _Out_ PPRTL RtlPointer
    );

FORCEINLINE
VOID
InlineFindCharsInString(
    _In_     PCSTRING     String,
    _In_     CHAR         CharToFind,
    _Inout_  PRTL_BITMAP  Bitmap
    )
{
    USHORT Index;
    USHORT NumberOfCharacters = String->Length;
    CHAR Char;

    for (Index = 0; Index < NumberOfCharacters; Index++) {
        Char = String->Buffer[Index];
        if (Char == CharToFind) {
            FastSetBit(Bitmap, Index);
        }
    }
}

FORCEINLINE
VOID
InlineFindWideCharsInUnicodeString(
    _In_ PCUNICODE_STRING String,
    _In_ WCHAR            CharToFind,
    _In_ PRTL_BITMAP      Bitmap
    )
{
    USHORT Index;
    USHORT NumberOfCharacters = String->Length;
    WCHAR Char;

    for (Index = 0; Index < NumberOfCharacters; Index++) {
        Char = String->Buffer[Index];
        if (Char == CharToFind) {
            FastSetBit(Bitmap, Index);
        }
    }
}

FORCEINLINE
VOID
InlineFindTwoWideCharsInUnicodeStringReversed(
    _In_ PCUNICODE_STRING String,
    _In_ WCHAR            Char1ToFind,
    _In_ WCHAR            Char2ToFind,
    _In_ PRTL_BITMAP      Bitmap1,
    _In_ PRTL_BITMAP      Bitmap2
    )
{
    USHORT Index;
    USHORT NumberOfCharacters = String->Length >> 1;
    WCHAR  Char;
    ULONG  Bit;

    for (Index = 0; Index < NumberOfCharacters; Index++) {
        Char = String->Buffer[Index];
        Bit = NumberOfCharacters - Index;
        if (Char == Char1ToFind) {
            FastSetBit(Bitmap1, Bit);
        }
        if (Char == Char2ToFind) {
            FastSetBit(Bitmap2, Bit);
        }
    }
}



FORCEINLINE
BOOL
ConvertUtf16StringToUtf8StringSlow(
    _In_ PCUNICODE_STRING Utf16,
    _Out_ PPSTRING Utf8Pointer,
    _In_ PALLOCATOR Allocator,
    _In_ PLARGE_INTEGER TimestampPointer
    )
/*++

Routine Description:

    Converts a UTF-16 unicode string to a UTF-8 string using the provided
    allocator.  The 'Slow' suffix on this function name indicates that the
    WideCharToMultiByte() function is called first in order to get the required
    buffer size prior to allocating the buffer.

    (ConvertUtf16StringToUtf8String() is an alternate version of this method
     that optimizes for the case where there are no multi-byte characters.)

Arguments:

    Utf16 - Supplies a pointer to a UNICODE_STRING structure to be converted.

    Utf8Pointer - Supplies a pointer that receives the address of the newly
        allocated and converted UTF-8 STRING version of the UTF-16 input string.

    Allocator - Supplies a pointer to the memory allocator that will be used
        for all allocations.

Return Value:

    TRUE on success, FALSE on failure.

--*/
{
    USHORT NewLength;
    USHORT NumberOfCharacters;
    LONG BufferSizeInBytes;
    LONG BytesCopied;
    ULONG_INTEGER AlignedBufferSizeInBytes;
    ULONG_INTEGER AllocSize;
    PSTRING Utf8;

    //
    // Validate arguments.
    //

    if (!ARGUMENT_PRESENT(Utf16)) {
        return FALSE;
    }

    if (!ARGUMENT_PRESENT(Utf8Pointer)) {
        return FALSE;
    }

    if (!ARGUMENT_PRESENT(Allocator)) {
        return FALSE;
    }

    //
    // Clear the caller's pointer straight away.
    //

    *Utf8Pointer = NULL;

    //
    // Calculate the number of bytes required to hold a UTF-8 encoding of the
    // UTF-16 input string.
    //

    NumberOfCharacters = Utf16->Length >> 1;

    BufferSizeInBytes = WideCharToMultiByte(
        CP_UTF8,                        // CodePage
        0,                              // dwFlags
        Utf16->Buffer,                  // lpWideCharStr
        NumberOfCharacters,             // cchWideChar
        NULL,                           // lpMultiByteStr
        0,                              // cbMultiByte
        NULL,                           // lpDefaultChar
        NULL                            // lpUsedDefaultChar
    );

    if (BufferSizeInBytes <= 0) {
        return FALSE;
    }

    //
    // Account for the trailing NULL.
    //

    NewLength = (USHORT)BufferSizeInBytes;
    BufferSizeInBytes += 1;

    //
    // Align the buffer.
    //

    AlignedBufferSizeInBytes.LongPart = (ULONG)(
        ALIGN_UP_POINTER(
            BufferSizeInBytes
        )
    );

    //
    // Sanity check the buffer size isn't over MAX_USHORT or under the number
    // of bytes for the unicode buffer.
    //

    if (AlignedBufferSizeInBytes.HighPart != 0) {
        return FALSE;
    }

    if (AlignedBufferSizeInBytes.LowPart > Utf16->Length) {
        return FALSE;
    }

    //
    // Calculate the total allocation size required, factoring in the overhead
    // of the STRING struct.
    //

    AllocSize.LongPart = (

        sizeof(STRING) +

        AlignedBufferSizeInBytes.LowPart

    );

    //
    // Try allocate space for the buffer.
    //

    Utf8 = (PSTRING)(
        Allocator->CallocWithTimestamp(
            Allocator->Context,
            1,
            AlignedBufferSizeInBytes.LowPart,
            TimestampPointer
        )
    );

    if (!Utf8) {
        return FALSE;
    }

    //
    // Successfully allocated space.  Point the STRING buffer at the memory
    // trailing the struct.
    //

    Utf8->Buffer = (PCHAR)(
        RtlOffsetToPointer(
            Utf8,
            sizeof(STRING)
        )
    );

    //
    // Initialize the lengths.
    //

    Utf8->Length = NewLength;
    Utf8->MaximumLength = AlignedBufferSizeInBytes.LowPart;

    //
    // Attempt the conversion.
    //

    BytesCopied = WideCharToMultiByte(
        CP_UTF8,                // CodePage
        0,                      // dwFlags
        Utf16->Buffer,          // lpWideCharStr
        NumberOfCharacters,     // cchWideChar
        Utf8->Buffer,           // lpMultiByteStr
        Utf8->Length,           // cbMultiByte
        NULL,                   // lpDefaultChar
        NULL                    // lpUsedDefaultChar
    );

    if (BytesCopied != Utf8->Length) {
        goto Error;
    }

    //
    // We calloc'd the buffer, so no need for zeroing the trailing NULL(s).
    //

    //
    // Update the caller's pointer and return success.
    //

    *Utf8Pointer = Utf8;

    return TRUE;

Error:

    if (Utf8) {

        //
        // Try free the underlying buffer.
        //

        Allocator->Free(Allocator->Context, Utf8);
        Utf8 = NULL;
    }

    return FALSE;
}

FORCEINLINE
BOOL
ConvertUtf16StringToUtf8String(
    _In_ PCUNICODE_STRING Utf16,
    _Out_ PPSTRING Utf8Pointer,
    _In_ PALLOCATOR Allocator
    )
/*++

Routine Description:

    Converts a UTF-16 unicode string to a UTF-8 string using the provided
    allocator.  This method is optimized for the case where there are no
    multi-byte characters in the unicode string, where the buffer size required
    to hold the UTF-8 string is simply half the size of the UTF-16 buffer.  If
    the first attempt at conversion (via WideCharToMultiByte()) indicates that
    there are multibyte characters (ERROR_INSUFFICIENT_BUFFER is returned by
    GetLastError()), the first allocated buffer will be freed and this routine
    will call ConvertUtf16StringToUtf8StringSlow().

Arguments:

    Utf16 - Supplies a pointer to a UNICODE_STRING structure to be converted.

    Utf8Pointer - Supplies a pointer that receives the address of the newly
        allocated and converted UTF-8 STRING version of the UTF-16 input string.

    Allocator - Supplies a pointer to the memory allocator that will be used
        for all allocations.

Return Value:

    TRUE on success, FALSE on failure.

--*/
{
    USHORT NumberOfCharacters;
    LONG BufferSizeInBytes;
    LONG BytesCopied;
    ULONG_INTEGER AllocSize;
    ULONG_INTEGER AlignedBufferSizeInBytes;
    PSTRING Utf8;

    //
    // Validate arguments.
    //

    if (!ARGUMENT_PRESENT(Utf16)) {
        return FALSE;
    }

    if (!ARGUMENT_PRESENT(Utf8Pointer)) {
        return FALSE;
    }

    if (!ARGUMENT_PRESENT(Allocator)) {
        return FALSE;
    }

    //
    // Clear the caller's pointer straight away.
    //

    *Utf8Pointer = NULL;

    //
    // Calculate the number of bytes required to hold a UTF-8 encoding of the
    // UTF-16 input string.
    //

    NumberOfCharacters = Utf16->Length >> 1;

    //
    // Account for the trailing NULL.
    //

    BufferSizeInBytes = NumberOfCharacters + 1;

    //
    // Align the buffer size on a pointer boundary.
    //

    AlignedBufferSizeInBytes.LongPart = (
        ALIGN_UP_USHORT_TO_POINTER_SIZE(
            BufferSizeInBytes
        )
    );

    //
    // Sanity check the buffer size isn't over MAX_USHORT or under the number
    // of bytes for the unicode buffer.
    //

    if (AlignedBufferSizeInBytes.HighPart != 0) {
        return FALSE;
    }

    if (AlignedBufferSizeInBytes.LowPart > Utf16->Length) {
        return FALSE;
    }

    //
    // Calculate the total allocation size required, factoring in the overhead
    // of the STRING struct.
    //

    AllocSize.LongPart = (

        sizeof(STRING) +

        AlignedBufferSizeInBytes.LowPart

    );

    //
    // Try allocate space for the buffer.
    //

    Utf8 = (PSTRING)(
        Allocator->Calloc(
            Allocator->Context,
            1,
            AllocSize.LowPart
        )
    );

    if (!Utf8) {
        return FALSE;
    }

    //
    // Successfully allocated space.  Point the STRING buffer at the memory
    // trailing the struct.
    //

    Utf8->Buffer = (PCHAR)(
        RtlOffsetToPointer(
            Utf8,
            sizeof(STRING)
        )
    );

    //
    // Initialize the lengths.
    //

    Utf8->Length = NumberOfCharacters;
    Utf8->MaximumLength = AlignedBufferSizeInBytes.LowPart;

    //
    // Attempt the conversion.
    //

    BytesCopied = WideCharToMultiByte(
        CP_UTF8,                // CodePage
        0,                      // dwFlags
        Utf16->Buffer,          // lpWideCharStr
        NumberOfCharacters,     // cchWideChar
        Utf8->Buffer,           // lpMultiByteStr
        Utf8->Length,           // cbMultiByte
        NULL,                   // lpDefaultChar
        NULL                    // lpUsedDefaultChar
    );

    if (BytesCopied != Utf8->Length) {
        goto Error;
    }

    //
    // We calloc'd the buffer, so no need for zeroing the trailing NULL(s).
    //

    //
    // Update the caller's pointer and return success.
    //

    *Utf8Pointer = Utf8;

    return TRUE;

Error:

    if (Utf8) {
        Allocator->Free(Allocator->Context, Utf8);
        Utf8 = NULL;
    }

    return FALSE;
}

FORCEINLINE
BOOL
ConvertUtf8StringToUtf16StringSlow(
    _In_ PSTRING Utf8,
    _Out_ PPUNICODE_STRING Utf16Pointer,
    _In_ PALLOCATOR Allocator,
    _In_opt_ PLARGE_INTEGER TimestampPointer
    )
/*++

Routine Description:

    Converts a UTF-8 Unicode string to a UTF-16 string using the provided
    allocator.  The 'Slow' suffix on this function name indicates that the
    MultiByteToWideChar() function is called first in order to get the required
    buffer size prior to allocating the buffer.

    (ConvertUtf8StringToUtf16String() is an alternate version of this method
     that optimizes for the case where there are no multi-byte characters.)

Arguments:

    Utf8 - Supplies a pointer to a STRING structure to be converted.

    Utf16Pointer - Supplies a pointer that receives the address of the newly
        allocated and converted UTF-16 STRING version of the UTF-8 input string.

    Allocator - Supplies a pointer to the memory allocator that will be used
        for all allocations.

    TimestampPointer - Supplies a timestamp to associate with allocations.

Return Value:

    TRUE on success, FALSE on failure.

--*/
{
    USHORT NewLengthInChars;
    LONG CharsCopied;
    LONG BufferSizeInBytes;
    LONG BufferSizeInChars;
    LONG MaximumBufferSizeInChars;
    ULONG_INTEGER AllocSize;
    ULONG_INTEGER AlignedBufferSizeInBytes;
    PUNICODE_STRING Utf16;
    LARGE_INTEGER LocalTimestamp;

    //
    // Validate arguments.
    //

    if (!ARGUMENT_PRESENT(Utf8)) {
        return FALSE;
    }

    if (!ARGUMENT_PRESENT(Utf16Pointer)) {
        return FALSE;
    }

    if (!ARGUMENT_PRESENT(Allocator)) {
        return FALSE;
    }

    if (!ARGUMENT_PRESENT(TimestampPointer)) {
        TimestampPointer = &LocalTimestamp;
    }

    //
    // Clear the caller's pointer straight away.
    //

    *Utf16Pointer = NULL;

    //
    // Calculate the number of bytes required to hold a UTF-16 encoding of the
    // UTF-8 input string.
    //

    BufferSizeInChars = MultiByteToWideChar(
        CP_UTF8,                        // CodePage
        0,                              // dwFlags
        Utf8->Buffer,                   // lpMultiByteStr
        Utf8->Length,                   // cbMultiByte
        NULL,                           // lpWideCharStr
        0                               // cchWideChar
    );

    if (BufferSizeInChars <= 0) {
        return FALSE;
    }

    //
    // Account for the trailing NULL.
    //

    NewLengthInChars = (USHORT)BufferSizeInChars + 1;

    //
    // Convert character buffer size into bytes.
    //

    BufferSizeInBytes = NewLengthInChars << 1;

    //
    // Align the buffer.
    //

    AlignedBufferSizeInBytes.LongPart = (ULONG)(
        ALIGN_UP_POINTER(
            BufferSizeInBytes
        )
    );

    //
    // Sanity check the buffer size isn't over MAX_USHORT or under the number
    // of bytes for the input UTF-8 buffer.
    //

    if (AlignedBufferSizeInBytes.HighPart != 0) {
        return FALSE;
    }

    if (AlignedBufferSizeInBytes.LowPart < Utf8->Length) {
        return FALSE;
    }

    //
    // Calculate the total allocation size required, factoring in the overhead
    // of the UNICODE_STRING struct.
    //

    AllocSize.LongPart = (

        sizeof(UNICODE_STRING) +

        AlignedBufferSizeInBytes.LowPart

    );

    //
    // Try allocate space for the buffer.
    //

    Utf16 = (PUNICODE_STRING)(
        Allocator->CallocWithTimestamp(
            Allocator->Context,
            1,
            AllocSize.LongPart,
            TimestampPointer
        )
    );

    if (!Utf16) {
        return FALSE;
    }

    //
    // Successfully allocated space.  Point the UNICODE_STRING buffer at the
    // memory trailing the struct.
    //

    Utf16->Buffer = (PWCHAR)(
        RtlOffsetToPointer(
            Utf16,
            sizeof(UNICODE_STRING)
        )
    );

    //
    // Initialize the lengths.
    //

    Utf16->Length = (NewLengthInChars-1) << 1;
    Utf16->MaximumLength = AlignedBufferSizeInBytes.LowPart;

    MaximumBufferSizeInChars = Utf16->MaximumLength >> 1;

    //
    // Attempt the conversion.
    //

    CharsCopied = MultiByteToWideChar(
        CP_UTF8,                    // CodePage
        0,                          // dwFlags
        Utf8->Buffer,               // lpMultiByteStr
        Utf8->Length,               // cbMultiByte
        Utf16->Buffer,              // lpWideCharStr
        MaximumBufferSizeInChars    // cchWideChar
    );

    if (CharsCopied != NewLengthInChars-1) {
        goto Error;
    }

    //
    // We calloc'd the buffer, so no need for zeroing the trailing NULL(s).
    //

    //
    // Update the caller's pointer and return success.
    //

    *Utf16Pointer = Utf16;

    return TRUE;

Error:

    if (Utf16) {

        //
        // Try free the underlying buffer.
        //

        Allocator->FreePointer(Allocator->Context, (PPVOID)&Utf16);
    }

    return FALSE;
}

FORCEINLINE
BOOL
ConvertUtf8StringToUtf16StringFast(
    _In_ PSTRING Utf8,
    _Out_ PPUNICODE_STRING Utf16Pointer,
    _In_ PALLOCATOR Allocator
    )
/*++

Routine Description:

    Converts a UTF-8 Unicode string to a UTF-16 string using the provided
    allocator.  This method is optimized for the case where there are no
    multi-byte characters in the string, where the buffer size required to hold
    the UTF-16 string is simply double the size of the UTF-8 buffer.  If the
    first attempt at conversion (via MultiByteToWideChar()) indicates that there
    are multi-byte characters (ERROR_INSUFFICIENT_BUFFER is returned by
    GetLastError()), the first allocated buffer will be freed and this routine
    will call ConvertUtf8StringToUtf16StringSlow().

Arguments:

    Utf8 - Supplies a pointer to a STRING structure to be converted.

    Utf16Pointer - Supplies a pointer that receives the address of the newly
        allocated and converted UTF-16 STRING version of the UTF-8 input string.

    Allocator - Supplies a pointer to the memory allocator that will be used
        for all allocations.

Return Value:

    TRUE on success, FALSE on failure.

--*/
{
    USHORT NumberOfCharacters;
    LONG CharsCopied;
    LONG BufferSizeInChars;
    LONG BufferSizeInBytes;
    LONG MaximumBufferSizeInChars;
    ULONG_INTEGER AllocSize;
    ULONG_INTEGER AlignedBufferSizeInBytes;
    PUNICODE_STRING Utf16;

    //
    // Validate arguments.
    //

    if (!ARGUMENT_PRESENT(Utf8)) {
        return FALSE;
    }

    if (!ARGUMENT_PRESENT(Utf16Pointer)) {
        return FALSE;
    }

    if (!ARGUMENT_PRESENT(Allocator)) {
        return FALSE;
    }

    //
    // Clear the caller's pointer straight away.
    //

    *Utf16Pointer = NULL;

    //
    // Calculate the number of bytes required to hold a UTF-8 encoding of the
    // UTF-16 input string.
    //

    NumberOfCharacters = Utf8->Length >> 1;

    //
    // Account for the trailing NULL.
    //

    BufferSizeInChars = NumberOfCharacters + 1;

    //
    // Convert character buffer size into bytes.
    //

    BufferSizeInBytes = BufferSizeInChars << 1;

    //
    // Align the buffer.
    //

    AlignedBufferSizeInBytes.LongPart = (ULONG)(
        ALIGN_UP_POINTER(
            BufferSizeInBytes
        )
    );

    //
    // Sanity check the buffer size isn't over MAX_USHORT or under the number
    // of bytes for the input UTF-8 buffer.
    //

    if (AlignedBufferSizeInBytes.HighPart != 0) {
        return FALSE;
    }

    if (AlignedBufferSizeInBytes.LowPart > Utf8->Length) {
        return FALSE;
    }

    //
    // Calculate the total allocation size required, factoring in the overhead
    // of the UNICODE_STRING struct.
    //

    AllocSize.LongPart = (

        sizeof(UNICODE_STRING) +

        AlignedBufferSizeInBytes.LowPart

    );

    //
    // Try allocate space for the buffer.
    //

    Utf16 = (PUNICODE_STRING)(
        Allocator->Calloc(
            Allocator->Context,
            1,
            AllocSize.LongPart
        )
    );

    if (!Utf16) {
        return FALSE;
    }

    //
    // Successfully allocated space.  Point the UNICODE_STRING buffer at the
    // memory trailing the struct.
    //

    Utf16->Buffer = (PWCHAR)(
        RtlOffsetToPointer(
            Utf16,
            sizeof(UNICODE_STRING)
        )
    );

    //
    // Initialize the lengths.
    //

    Utf16->Length = NumberOfCharacters << 1;
    Utf16->MaximumLength = AlignedBufferSizeInBytes.LowPart;

    MaximumBufferSizeInChars = Utf16->MaximumLength >> 1;

    //
    // Attempt the conversion.
    //

    CharsCopied = MultiByteToWideChar(
        CP_UTF8,                    // CodePage
        0,                          // dwFlags
        Utf8->Buffer,               // lpMultiByteStr
        Utf8->Length,               // cbMultiByte
        Utf16->Buffer,              // lpWideCharStr
        MaximumBufferSizeInChars    // cchWideChar
    );

    if (CharsCopied != Utf8->Length) {
        goto Error;
    }

    //
    // We calloc'd the buffer, so no need for zeroing the trailing NULL(s).
    //

    //
    // Update the caller's pointer and return success.
    //

    *Utf16Pointer = Utf16;

    return TRUE;

Error:

    if (Utf16) {
        Allocator->Free(Allocator->Context, Utf16);
        Utf16 = NULL;
    }

    return FALSE;
}


#ifdef RTL_SECURE_ZERO_MEMORY
FORCEINLINE
PVOID
RtlSecureZeroMemory(
    _Out_writes_bytes_all_(cnt) PVOID ptr,
    _In_ SIZE_T cnt
    )
{
    volatile char *vptr = (volatile char *)ptr;

#if defined(_M_AMD64)

    __stosb((PUCHAR)((ULONG64)vptr), 0, cnt);

#else

    while (cnt) {

#if !defined(_M_CEE) && (defined(_M_ARM) || defined(_M_ARM64))

        __iso_volatile_store8(vptr, 0);

#else

        *vptr = 0;

#endif

        vptr++;
        cnt--;
    }

#endif // _M_AMD64

    return ptr;
}
#endif

static CONST WCHAR IntegerToWCharTable[] = {
    L'0',
    L'1',
    L'2',
    L'3',
    L'4',
    L'5',
    L'6',
    L'7',
    L'8',
    L'9',
    L'A',
    L'B',
    L'C',
    L'D',
    L'E',
    L'F'
};

FORCEINLINE
BYTE
CountNumberOfDigitsInline(_In_ ULONG Value)
{
    BYTE Count = 0;

    do {
        Count++;
        Value = Value / 10;
    } while (Value != 0);

    return Count;
}

FORCEINLINE
BYTE
CountNumberOfLongLongDigitsInline(_In_ ULONGLONG Value)
{
    BYTE Count = 0;

    do {
        Count++;
        Value = Value / 10;
    } while (Value != 0);

    return Count;
}

FORCEINLINE
BOOLEAN
AppendIntegerToUnicodeString(
    _In_ PUNICODE_STRING String,
    _In_ ULONG Integer,
    _In_ USHORT NumberOfDigits,
    _In_opt_ WCHAR Trailer
    )
/*++

Routine Description:

    This is a helper routine that allows construction of unicode strings out
    of integer values.

Arguments:

    String - Supplies a pointer to a UNICODE_STRING that will be appended to.
        Sufficient buffer space must exist for the entire string to be written.

    Integer - The integer value to be appended to the string.

    NumberOfDigits - The expected number of digits for the value.  If Integer
        has less digits than this number, it will be left-padded with zeros.

    Trailer - An optional trailing wide character to append.

Return Value:

    TRUE on success, FALSE on failure.

--*/
{
    USHORT ActualNumberOfDigits;
    USHORT BytesRequired;
    USHORT BytesRemaining;
    USHORT NumberOfZerosToPad;
    const ULONG Base = 10;
    ULONG Digit;
    ULONG Value;
    ULONG Count;
    ULONG Bytes;
    WCHAR Char;
    PWCHAR Dest;

    //
    // Verify the unicode string has sufficient space.
    //

    BytesRequired = NumberOfDigits * sizeof(WCHAR);

    if (Trailer) {
        BytesRequired += (1 * sizeof(Trailer));
    }

    BytesRemaining = (
        String->MaximumLength -
        String->Length
    );

    if (BytesRemaining < BytesRequired) {
        return FALSE;
    }

    //
    // Make sure the integer value doesn't have more digits than
    // specified.
    //

    ActualNumberOfDigits = CountNumberOfDigitsInline(Integer);

    if (ActualNumberOfDigits > NumberOfDigits) {
        return FALSE;
    }

    //
    // Initialize our destination pointer to the last digit.  (We write
    // back-to-front.)
    //

    Dest = (PWCHAR)(
        RtlOffsetToPointer(
            String->Buffer,
            String->Length + (
                (NumberOfDigits - 1) *
                sizeof(WCHAR)
            )
        )
    );
    Count = 0;
    Bytes = 0;

    //
    // Convert each digit into the corresponding character and copy to the
    // string buffer, retreating the pointer as we go.
    //

    Value = Integer;

    do {
        Count++;
        Bytes += 2;
        Digit = Value % Base;
        Value = Value / Base;
        Char = IntegerToWCharTable[Digit];
        *Dest-- = Char;
    } while (Value != 0);

    //
    // Pad the string with zeros if necessary.
    //

    NumberOfZerosToPad = NumberOfDigits - ActualNumberOfDigits;

    if (NumberOfZerosToPad) {
        do {
            Count++;
            Bytes += 2;
            *Dest-- = L'0';
        } while (--NumberOfZerosToPad);
    }

    //
    // Update the string with the new length.
    //

    String->Length += (USHORT)Bytes;

    //
    // Add the trailer if applicable.
    //

    if (Trailer) {
        String->Length += sizeof(WCHAR);
        String->Buffer[(String->Length - 1) >> 1] = Trailer;
    }

    return TRUE;
}

FORCEINLINE
BOOLEAN
AppendLongLongIntegerToUnicodeString(
    _In_ PUNICODE_STRING String,
    _In_ ULONGLONG Integer,
    _In_ USHORT NumberOfDigits,
    _In_opt_ WCHAR Trailer
    )
/*++

Routine Description:

    This is a helper routine that allows construction of unicode strings out
    of integer values.

Arguments:

    String - Supplies a pointer to a UNICODE_STRING that will be appended to.
        Sufficient buffer space must exist for the entire string to be written.

    Integer - Supplies the long long integer value to be appended to the string.

    NumberOfDigits - The expected number of digits for the value.  If Integer
        has less digits than this number, it will be left-padded with zeros.

    Trailer - An optional trailing wide character to append.

Return Value:

    TRUE on success, FALSE on failure.

--*/
{
    USHORT ActualNumberOfDigits;
    USHORT BytesRequired;
    USHORT BytesRemaining;
    USHORT NumberOfZerosToPad;
    const ULONGLONG Base = 10;
    ULONGLONG Digit;
    ULONGLONG Value;
    ULONGLONG Count;
    ULONGLONG Bytes;
    WCHAR Char;
    PWCHAR Dest;

    //
    // Verify the unicode string has sufficient space.
    //

    BytesRequired = NumberOfDigits * sizeof(WCHAR);

    if (Trailer) {
        BytesRequired += (1 * sizeof(Trailer));
    }

    BytesRemaining = (
        String->MaximumLength -
        String->Length
    );

    if (BytesRemaining < BytesRequired) {
        return FALSE;
    }

    //
    // Make sure the integer value doesn't have more digits than
    // specified.
    //

    ActualNumberOfDigits = CountNumberOfLongLongDigitsInline(Integer);

    if (ActualNumberOfDigits > NumberOfDigits) {
        return FALSE;
    }

    //
    // Initialize our destination pointer to the last digit.  (We write
    // back-to-front.)
    //

    Dest = (PWCHAR)(
        RtlOffsetToPointer(
            String->Buffer,
            String->Length + (
                (NumberOfDigits - 1) *
                sizeof(WCHAR)
            )
        )
    );
    Count = 0;
    Bytes = 0;

    //
    // Convert each digit into the corresponding character and copy to the
    // string buffer, retreating the pointer as we go.
    //

    Value = Integer;

    do {
        Count++;
        Bytes += 2;
        Digit = Value % Base;
        Value = Value / Base;
        Char = IntegerToWCharTable[Digit];
        *Dest-- = Char;
    } while (Value != 0);

    //
    // Pad the string with zeros if necessary.
    //

    NumberOfZerosToPad = NumberOfDigits - ActualNumberOfDigits;

    if (NumberOfZerosToPad) {
        do {
            Count++;
            Bytes += 2;
            *Dest-- = L'0';
        } while (--NumberOfZerosToPad);
    }

    //
    // Update the string with the new length.
    //

    String->Length += (USHORT)Bytes;

    //
    // Add the trailer if applicable.
    //

    if (Trailer) {
        String->Length += sizeof(WCHAR);
        String->Buffer[(String->Length - 1) >> 1] = Trailer;
    }

    return TRUE;
}

//
// Output helpers.
//

#define OUTPUT_RAW(String)                                               \
    Rtl->AppendCharBufferToCharBuffer(&Output, String, sizeof(String)-1)

#define OUTPUT_STRING(String) Rtl->AppendStringToCharBuffer(&Output, String)

#define OUTPUT_CSTR(Str) Rtl->AppendCStrToCharBuffer(&Output, Str)
#define OUTPUT_CHR(Char) Rtl->AppendCharToCharBuffer(&Output, Char)
#define OUTPUT_SEP() Rtl->AppendCharToCharBuffer(&Output, ',')
#define OUTPUT_LF() Rtl->AppendCharToCharBuffer(&Output, '\n')

#define OUTPUT_INT(Value)                           \
    Rtl->AppendIntegerToCharBuffer(&Output, Value);

#define OUTPUT_FLUSH_CONSOLE()                                               \
    BytesToWrite.QuadPart = ((ULONG_PTR)Output) - ((ULONG_PTR)OutputBuffer); \
    Success = WriteConsoleA(OutputHandle,                                    \
                            OutputBuffer,                                    \
                            BytesToWrite.LowPart,                            \
                            &CharsWritten,                                   \
                            NULL);                                           \
    ASSERT(Success);                                                         \
    Output = OutputBuffer

#define OUTPUT_FLUSH_FILE()                                                    \
    BytesToWrite.QuadPart = ((ULONG_PTR)Output) - ((ULONG_PTR)OutputBuffer)-1; \
    Success = WriteFile(OutputHandle,                                          \
                        OutputBuffer,                                          \
                        BytesToWrite.LowPart,                                  \
                        &BytesWritten,                                         \
                        NULL);                                                 \
    ASSERT(Success);                                                           \
    Output = OutputBuffer

#define OUTPUT_FLUSH()                                                       \
    BytesToWrite.QuadPart = ((ULONG_PTR)Output) - ((ULONG_PTR)OutputBuffer); \
    Success = WriteConsoleA(OutputHandle,                                    \
                            OutputBuffer,                                    \
                            BytesToWrite.LowPart,                            \
                            &CharsWritten,                                   \
                            NULL);                                           \
    if (!Success) {                                                          \
        Success = WriteFile(OutputHandle,                                    \
                            OutputBuffer,                                    \
                            BytesToWrite.LowPart,                            \
                            &BytesWritten,                                   \
                            NULL);                                           \
        ASSERT(Success);                                                     \
    }                                                                        \
    Output = OutputBuffer

//
// Wide output helpers.
//

#define WIDE_OUTPUT_RAW(WideOutput, WideString)  \
    Rtl->AppendWideCharBufferToWideCharBuffer(   \
        &WideOutput,                             \
        WideString,                              \
        sizeof(WideString)-sizeof(WideString[0]) \
    )

#define WIDE_OUTPUT_UNICODE_STRING(WideOutput, UnicodeString)            \
    Rtl->AppendUnicodeStringToWideCharBuffer(&WideOutput, UnicodeString)

#define WIDE_OUTPUT_WCSTR(WideOutput, WideCStr)                \
    Rtl->AppendWideCStrToWideCharBuffer(&WideOutput, WideCStr)

#define WIDE_OUTPUT_WCHR(WideOutput, WideChar)                 \
    Rtl->AppendWideCharToWideCharBuffer(&WideOutput, WideChar)

#define WIDE_OUTPUT_SEP(WideOutput)                        \
    Rtl->AppendWideCharToWideCharBuffer(&WideOutput, L',')

#define WIDE_OUTPUT_LF(WideOutput)                          \
    Rtl->AppendWideCharToWideCharBuffer(&WideOutput, L'\n')

#define WIDE_OUTPUT_INT(WideOutputValue)                    \
    Rtl->AppendIntegerToWideCharBuffer(&WideOutput, Value);

#define WIDE_OUTPUT_FLUSH_CONSOLE()                         \
    BytesToWrite.QuadPart = (                               \
        ((ULONG_PTR)WideOutput) -                           \
        ((ULONG_PTR)WideOutputBuffer)                       \
    );                                                      \
    WideCharsToWrite.QuadPart = BytesToWrite.QuadPart >> 1; \
    Success = WriteConsoleW(WideOutputHandle,               \
                            WideOutputBuffer,               \
                            WideCharsToWrite.LowPart,       \
                            &WideCharsWritten,              \
                            NULL);                          \
    ASSERT(Success);                                        \
    WideOutput = WideOutputBuffer

#define WIDE_OUTPUT_FLUSH_FILE()              \
    BytesToWrite.QuadPart = (                 \
        ((ULONG_PTR)WideOutput) -             \
        ((ULONG_PTR)WideOutputBuffer)         \
    ) - 1;                                    \
    Success = WriteFile(WideOutputHandle,     \
                        WideOutputBuffer,     \
                        BytesToWrite.LowPart, \
                        &BytesWritten,        \
                        NULL);                \
    ASSERT(Success);                          \
    WideOutput = WideOutputBuffer

#define WIDE_OUTPUT_FLUSH()                                 \
    BytesToWrite.QuadPart = (                               \
        ((ULONG_PTR)WideOutput) -                           \
        ((ULONG_PTR)WideOutputBuffer)                       \
    );                                                      \
    WideCharsToWrite.QuadPart = BytesToWrite.QuadPart >> 1; \
    Success = WriteConsoleW(WideOutputHandle,               \
                            WideOutputBuffer,               \
                            WideCharsToWrite.LowPart,       \
                            &WideCharsWritten,              \
                            NULL);                          \
    if (!Success) {                                         \
        Success = WriteFile(WideOutputHandle,               \
                            WideOutputBuffer,               \
                            BytesToWrite.LowPart,           \
                            &BytesWritten,                  \
                            NULL);                          \
        ASSERT(Success);                                    \
    }                                                       \
    WideOutput = WideOutputBuffer

//
// Timestamp glue for benchmarking.
//

#define TIMESTAMP_TO_MICROSECONDS 1000000ULL
#define TIMESTAMP_TO_NANOSECONDS  1000000000ULL

typedef struct _TIMESTAMP {
    ULONGLONG Id;
    ULONGLONG Count;
    STRING Name;
    union {
        ULONG Aux;
        ULONG CpuId[4];
    };
    LARGE_INTEGER Start;
    LARGE_INTEGER End;
    ULARGE_INTEGER StartTsc;
    ULARGE_INTEGER EndTsc;
    ULARGE_INTEGER Tsc;
    ULARGE_INTEGER TotalTsc;
    ULARGE_INTEGER MinimumTsc;
    ULARGE_INTEGER MaximumTsc;
    ULARGE_INTEGER Cycles;
    ULARGE_INTEGER MinimumCycles;
    ULARGE_INTEGER MaximumCycles;
    ULARGE_INTEGER TotalCycles;
    ULARGE_INTEGER Nanoseconds;
    ULARGE_INTEGER TotalNanoseconds;
    ULARGE_INTEGER MinimumNanoseconds;
    ULARGE_INTEGER MaximumNanoseconds;
} TIMESTAMP;
typedef TIMESTAMP *PTIMESTAMP;

#define INIT_TIMESTAMP(Idx, Namex)                         \
    ZeroStructPointer(Timestamp);                          \
    Timestamp->Id = Idx;                                   \
    Timestamp->Name.Length = sizeof(Namex)-1;              \
    Timestamp->Name.MaximumLength = sizeof(Namex);         \
    Timestamp->Name.Buffer = Namex;                        \
    Timestamp->TotalTsc.QuadPart = 0;                      \
    Timestamp->TotalCycles.QuadPart = 0;                   \
    Timestamp->TotalNanoseconds.QuadPart = 0;              \
    Timestamp->MinimumTsc.QuadPart = (ULONGLONG)-1;        \
    Timestamp->MinimumCycles.QuadPart = (ULONGLONG)-1;     \
    Timestamp->MinimumNanoseconds.QuadPart = (ULONGLONG)-1

#define INIT_TIMESTAMP_FROM_STRING(Idx, String)            \
    ZeroStructPointer(Timestamp);                          \
    Timestamp->Id = Idx;                                   \
    Timestamp->Name.Length = String->Length;               \
    Timestamp->Name.MaximumLength = String->MaximumLength; \
    Timestamp->Name.Buffer = String->Buffer;               \
    Timestamp->TotalTsc.QuadPart = 0;                      \
    Timestamp->TotalCycles.QuadPart = 0;                   \
    Timestamp->TotalNanoseconds.QuadPart = 0;              \
    Timestamp->MinimumTsc.QuadPart = (ULONGLONG)-1;        \
    Timestamp->MinimumCycles.QuadPart = (ULONGLONG)-1;     \
    Timestamp->MinimumNanoseconds.QuadPart = (ULONGLONG)-1

#define RESET_TIMESTAMP()                                   \
    Timestamp->Count = 0;                                   \
    Timestamp->TotalTsc.QuadPart = 0;                       \
    Timestamp->TotalCycles.QuadPart = 0;                    \
    Timestamp->TotalNanoseconds.QuadPart = 0;               \
    Timestamp->MinimumTsc.QuadPart = (ULONGLONG)-1;         \
    Timestamp->MaximumTsc.QuadPart = 0;                     \
    Timestamp->MinimumCycles.QuadPart = (ULONGLONG)-1;      \
    Timestamp->MaximumCycles.QuadPart = 0;                  \
    Timestamp->MinimumNanoseconds.QuadPart = (ULONGLONG)-1; \
    Timestamp->MaximumNanoseconds.QuadPart = 0

#define START_TIMESTAMP_CPUID()                 \
    ++Timestamp->Count;                         \
    QueryPerformanceCounter(&Timestamp->Start); \
    __cpuid((PULONG)&Timestamp->CpuId, 0);      \
    Timestamp->StartTsc.QuadPart = __rdtsc()

#define START_TIMESTAMP_RDTSCP()                             \
    ++Timestamp->Count;                                      \
    QueryPerformanceCounter(&Timestamp->Start);              \
    Timestamp->StartTsc.QuadPart = __rdtscp(&Timestamp->Aux)

#define START_TIMESTAMP_RDTSC()                 \
    ++Timestamp->Count;                         \
    QueryPerformanceCounter(&Timestamp->Start); \
    Timestamp->StartTsc.QuadPart = __rdtsc()

#define END_TIMESTAMP_COMMON()                             \
    Timestamp->Tsc.QuadPart = (                            \
        Timestamp->EndTsc.QuadPart -                       \
        Timestamp->StartTsc.QuadPart                       \
    );                                                     \
    Timestamp->Cycles.QuadPart = (                         \
        Timestamp->End.QuadPart -                          \
        Timestamp->Start.QuadPart                          \
    );                                                     \
    Timestamp->TotalTsc.QuadPart += (                      \
        Timestamp->Tsc.QuadPart                            \
    );                                                     \
    Timestamp->TotalCycles.QuadPart += (                   \
        Timestamp->Cycles.QuadPart                         \
    );                                                     \
    Timestamp->Nanoseconds.QuadPart = (                    \
        Timestamp->Cycles.QuadPart *                       \
        TIMESTAMP_TO_NANOSECONDS                           \
    );                                                     \
    Timestamp->Nanoseconds.QuadPart /= Frequency.QuadPart; \
    Timestamp->TotalNanoseconds.QuadPart += (              \
        Timestamp->Nanoseconds.QuadPart                    \
    );                                                     \
    if (Timestamp->MinimumNanoseconds.QuadPart >           \
        Timestamp->Nanoseconds.QuadPart) {                 \
            Timestamp->MinimumNanoseconds.QuadPart = (     \
                Timestamp->Nanoseconds.QuadPart            \
            );                                             \
    }                                                      \
    if (Timestamp->MaximumNanoseconds.QuadPart <           \
        Timestamp->Nanoseconds.QuadPart) {                 \
            Timestamp->MaximumNanoseconds.QuadPart = (     \
                Timestamp->Nanoseconds.QuadPart            \
            );                                             \
    }                                                      \
    if (Timestamp->MinimumTsc.QuadPart >                   \
        Timestamp->Tsc.QuadPart) {                         \
            Timestamp->MinimumTsc.QuadPart = (             \
                Timestamp->Tsc.QuadPart                    \
            );                                             \
    }                                                      \
    if (Timestamp->MaximumTsc.QuadPart <                   \
        Timestamp->Tsc.QuadPart) {                         \
            Timestamp->MaximumTsc.QuadPart = (             \
                Timestamp->Tsc.QuadPart                    \
            );                                             \
    }                                                      \
    if (Timestamp->MinimumCycles.QuadPart >                \
        Timestamp->Cycles.QuadPart) {                      \
            Timestamp->MinimumCycles.QuadPart = (          \
                Timestamp->Cycles.QuadPart                 \
            );                                             \
    }                                                      \
    if (Timestamp->MaximumCycles.QuadPart <                \
        Timestamp->Cycles.QuadPart) {                      \
            Timestamp->MaximumCycles.QuadPart = (          \
                Timestamp->Cycles.QuadPart                 \
            );                                             \
    }

#define END_TIMESTAMP_CPUID()                 \
    __cpuid((PULONG)&Timestamp->CpuId, 0);    \
    Timestamp->EndTsc.QuadPart = __rdtsc();   \
    QueryPerformanceCounter(&Timestamp->End); \
    END_TIMESTAMP_COMMON(Id)

#define END_TIMESTAMP_RDTSCP()                              \
    Timestamp->EndTsc.QuadPart = __rdtscp(&Timestamp->Aux); \
    QueryPerformanceCounter(&Timestamp->End);               \
    END_TIMESTAMP_COMMON()

#define END_TIMESTAMP_RDTSC()                 \
    Timestamp->EndTsc.QuadPart = __rdtsc();   \
    QueryPerformanceCounter(&Timestamp->End); \
    END_TIMESTAMP_COMMON()

#define FINISH_TIMESTAMP_EXAMPLE(Id, Length, Iterations) \
    OUTPUT_STRING(&Timestamp->Name);                     \
    OUTPUT_SEP();                                        \
    OUTPUT_INT(*Length);                                 \
    OUTPUT_SEP();                                        \
    OUTPUT_INT(Iterations);                              \
    OUTPUT_SEP();                                        \
    OUTPUT_INT(Timestamp->MinimumTsc.QuadPart);          \
    OUTPUT_SEP();                                        \
    OUTPUT_INT(Timestamp->MaximumTsc.QuadPart);          \
    OUTPUT_SEP();                                        \
    OUTPUT_INT(Timestamp->TotalTsc.QuadPart);            \
    OUTPUT_SEP();                                        \
    OUTPUT_INT(Timestamp->MinimumCycles.QuadPart);       \
    OUTPUT_SEP();                                        \
    OUTPUT_INT(Timestamp->MaximumCycles.QuadPart);       \
    OUTPUT_SEP();                                        \
    OUTPUT_INT(Timestamp->TotalCycles.QuadPart);         \
    OUTPUT_SEP();                                        \
    OUTPUT_INT(Timestamp->MinimumNanoseconds.QuadPart);  \
    OUTPUT_SEP();                                        \
    OUTPUT_INT(Timestamp->MaximumNanoseconds.QuadPart);  \
    OUTPUT_SEP();                                        \
    OUTPUT_INT(Timestamp->TotalNanoseconds.QuadPart);    \
    OUTPUT_LF()

//
// Registry helper macros.
//

/*++

    VOID
    READ_REG_QWORD(
        _In_ HKEY Key,
        _In_ LITERAL Name,
        _In_ PULONGLONG QwordPointer
        );

Routine Description:

    This is a helper macro for reading REG_QWORD values from the registry.

Arguments:

    Key - Supplies an HKEY handle that represents an open registry key with
        appropriate read access.

    Name - Name of the registry key to read.  This is converted into a literal
        wide character string by the macro (e.g. MaxNoneRefCount will become
        L"MaxNoneRefCount").

    QwordPointer - Supplies a pointer to a ULONGLONG that will receive the
        registry key value.

Return Value:

    None.

    N.B. If an error occurs, 0 will be written to QwordPointer.

--*/

#define READ_REG_QWORD(Key, Name, QwordPointer) do { \
    ULONG QwordLength = sizeof(*QwordPointer);       \
    Result = RegGetValueW(                           \
        Key,                                         \
        NULL,                                        \
        L#Name,                                      \
        RRF_RT_REG_QWORD,                            \
        NULL,                                        \
        (PVOID)QwordPointer,                         \
        &QwordLength                                 \
    );                                               \
    if (Result != ERROR_SUCCESS) {                   \
        *QwordPointer = 0;                           \
    }                                                \
} while (0)


/*++

    VOID
    WRITE_REG_QWORD(
        _In_ HKEY Key,
        _In_ LITERAL Name,
        _In_ ULONGLONG Value
        );

Routine Description:

    This is a helper macro for writing REG_QWORD values to the registry.

Arguments:

    Key - Supplies an HKEY handle that represents an open registry key with
        appropriate write access.

    Name - Name of the registry key to write.  This is converted into a literal
        wide character string by the macro (e.g. MaxNoneRefCount will become
        L"MaxNoneRefCount").

    Value - Supplies a ULONGLONG value to write.

Return Value:

    None.

--*/

#define WRITE_REG_QWORD(Key, Name, Value) do { \
    ULONGLONG Qword = Value;                   \
    ULONG QwordLength = sizeof(Qword);         \
    RegSetValueExW(                            \
        Key,                                   \
        L#Name,                                \
        0,                                     \
        REG_QWORD,                             \
        (const BYTE*)&Qword,                   \
        QwordLength                            \
    );                                         \
} while (0)

/*++

    VOID
    READ_REG_DWORD(
        _In_ HKEY Key,
        _In_ LITERAL Name,
        _In_ PDWORD DwordPointer
        );

Routine Description:

    This is a helper macro for reading REG_DWORD values from the registry.

Arguments:

    Key - Supplies an HKEY handle that represents an open registry key with
        appropriate read access.

    Name - Name of the registry key to read.  This is converted into a literal
        wide character string by the macro (e.g. MaxNoneRefCount will become
        L"MaxNoneRefCount").

    DwordPointer - Supplies a pointer to a DWORD that will receive the
        registry key value.

Return Value:

    None.

    N.B. If an error occurs, 0 will be written to DwordPointer.

--*/

#define READ_REG_DWORD(Key, Name, DwordPointer) do { \
    ULONG DwordLength = sizeof(*DwordPointer);       \
    Result = RegGetValueW(                           \
        Key,                                         \
        NULL,                                        \
        L#Name,                                      \
        RRF_RT_REG_DWORD,                            \
        NULL,                                        \
        (PVOID)DwordPointer,                         \
        &DwordLength                                 \
    );                                               \
    if (Result != ERROR_SUCCESS) {                   \
        *DwordPointer = 0;                           \
    }                                                \
} while (0)


/*++

    VOID
    WRITE_REG_DWORD(
        _In_ HKEY Key,
        _In_ LITERAL Name,
        _In_ ULONG Value
        );

Routine Description:

    This is a helper macro for writing REG_DWORD values to the registry.

Arguments:

    Key - Supplies an HKEY handle that represents an open registry key with
        appropriate write access.

    Name - Name of the registry key to write.  This is converted into a literal
        wide character string by the macro (e.g. MaxNoneRefCount will become
        L"MaxNoneRefCount").

    Value - Supplies a ULONG value to write.

Return Value:

    None.

--*/

#define WRITE_REG_DWORD(Key, Name, Value) do { \
    ULONG Dword = Value;                       \
    ULONG DwordLength = sizeof(Dword);         \
    RegSetValueExW(                            \
        Key,                                   \
        L#Name,                                \
        0,                                     \
        REG_DWORD,                             \
        (const BYTE*)&Dword,                   \
        DwordLength                            \
    );                                         \
} while (0)


/*++

    VOID
    UPDATE_MAX_REG_QWORD(
        _In_ HKEY Key,
        _In_ LITERAL Name,
        _In_ ULONGLONG Value
        );

Routine Description:

    This is a helper macro for writing REG_QWORD values to the registry if they
    exceed the value already present.

Arguments:

    Key - Supplies an HKEY handle that represents an open registry key with
        appropriate read/write access.

    Name - Name of the registry key to write.  This is converted into a literal
        wide character string by the macro (e.g. MaxNoneRefCount will become
        L"MaxNoneRefCount").

    Value - Supplies a ULONGLONG value to potentially write.

Return Value:

    None.

--*/

#define UPDATE_MAX_REG_QWORD(Key, Name, Value) do { \
    ULONGLONG Existing;                             \
    READ_REG_QWORD(Key, Name, &Existing);           \
    if (Value > Existing) {                         \
        WRITE_REG_QWORD(Key, Name, Value);          \
    }                                               \
} while (0)

//
// Verbatim copy of the doubly-linked list inline methods.
//

#define RTL_STATIC_LIST_HEAD(x) LIST_ENTRY x = { &x, &x }

FORCEINLINE
VOID
InitializeListHead(
    _Out_ PLIST_ENTRY ListHead
    )

{

    ListHead->Flink = ListHead->Blink = ListHead;
    return;
}

FORCEINLINE
BOOLEAN
IsListEmpty(
    _In_ const LIST_ENTRY * ListHead
    )

{
    return (BOOLEAN)(ListHead->Flink == ListHead);
}

FORCEINLINE
BOOLEAN
RemoveEntryList(
    _In_ PLIST_ENTRY Entry
    )
{

    PLIST_ENTRY Blink;
    PLIST_ENTRY Flink;

    Flink = Entry->Flink;
    Blink = Entry->Blink;
    Blink->Flink = Flink;
    Flink->Blink = Blink;
    return (BOOLEAN)(Flink == Blink);
}

FORCEINLINE
PLIST_ENTRY
RemoveHeadList(
    _Inout_ PLIST_ENTRY ListHead
    )
{

    PLIST_ENTRY Flink;
    PLIST_ENTRY Entry;

    Entry = ListHead->Flink;
    Flink = Entry->Flink;
    ListHead->Flink = Flink;
    Flink->Blink = ListHead;
    return Entry;
}



FORCEINLINE
PLIST_ENTRY
RemoveTailList(
    _Inout_ PLIST_ENTRY ListHead
    )

{

    PLIST_ENTRY Blink;
    PLIST_ENTRY Entry;

    Entry = ListHead->Blink;
    Blink = Entry->Blink;
    ListHead->Blink = Blink;
    Blink->Flink = ListHead;
    return Entry;
}


FORCEINLINE
VOID
InsertTailList(
    _Inout_ PLIST_ENTRY ListHead,
    _Inout_ __drv_aliasesMem PLIST_ENTRY Entry
    )
{

    PLIST_ENTRY Blink;

    Blink = ListHead->Blink;
    Entry->Flink = ListHead;
    Entry->Blink = Blink;
    Blink->Flink = Entry;
    ListHead->Blink = Entry;
    return;
}


FORCEINLINE
VOID
InsertHeadList(
    _Inout_ PLIST_ENTRY ListHead,
    _Inout_ __drv_aliasesMem PLIST_ENTRY Entry
    )
{

    PLIST_ENTRY Flink;

    Flink = ListHead->Flink;
    Entry->Flink = Flink;
    Entry->Blink = ListHead;
    Flink->Blink = Entry;
    ListHead->Flink = Entry;
    return;
}

FORCEINLINE
VOID
AppendTailList(
    _Inout_ PLIST_ENTRY ListHead,
    _Inout_ PLIST_ENTRY ListToAppend
    )
{
    PLIST_ENTRY ListEnd = ListHead->Blink;

    ListHead->Blink->Flink = ListToAppend;
    ListHead->Blink = ListToAppend->Blink;
    ListToAppend->Blink->Flink = ListHead;
    ListToAppend->Blink = ListEnd;
    return;
}

#define FOR_EACH_LIST_ENTRY(Head, Entry) \
    for (Entry = Head->Flink;            \
         Entry != Head;                  \
         Entry = Entry->Flink)

#define FOR_EACH_LIST_ENTRY_REVERSE(Head, Entry) \
    for (Entry = Head->Blink;                    \
         Entry != Head;                          \
         Entry = Entry->Blink)

//
// Interlocked doubly-linked list.
//

typedef struct _GUARDED_LIST {
    SRWLOCK Lock;

    _Guarded_by_(Lock)
    volatile LONGLONG NumberOfEntries;

    _Guarded_by_(Lock)
    LIST_ENTRY ListHead;

} GUARDED_LIST;
C_ASSERT(sizeof(GUARDED_LIST) == 32);
typedef const GUARDED_LIST CGUARDED_LIST;
typedef GUARDED_LIST *PGUARDED_LIST;
typedef GUARDED_LIST **PPGUARDED_LIST;
typedef GUARDED_LIST const *PCGUARDED_LIST;

_Acquires_shared_lock_(GuardedList->Lock)
FORCEINLINE
VOID
AcquireGuardedListLockShared(
    _In_ PGUARDED_LIST GuardedList
    )
{
    AcquireSRWLockShared(&GuardedList->Lock);
}

_Releases_shared_lock_(GuardedList->Lock)
FORCEINLINE
VOID
ReleaseGuardedListLockShared(
    _In_ PGUARDED_LIST GuardedList
    )
{
    ReleaseSRWLockShared(&GuardedList->Lock);
}

_Acquires_exclusive_lock_(GuardedList->Lock)
FORCEINLINE
VOID
AcquireGuardedListLockExclusive(
    _In_ PGUARDED_LIST GuardedList
    )
{
    AcquireSRWLockExclusive(&GuardedList->Lock);
}

_Releases_exclusive_lock_(GuardedList->Lock)
FORCEINLINE
VOID
ReleaseGuardedListLockExclusive(
    _In_ PGUARDED_LIST GuardedList
    )
{
    ReleaseSRWLockExclusive(&GuardedList->Lock);
}

FORCEINLINE
VOID
InitializeGuardedListHead(
    _Out_ PGUARDED_LIST GuardedList
    )
{
    InitializeSRWLock(&GuardedList->Lock);
    InitializeListHead(&GuardedList->ListHead);
    GuardedList->NumberOfEntries = 0;
    return;
}

FORCEINLINE
BOOLEAN
IsGuardedListEmpty(
    _In_ PGUARDED_LIST GuardedList
    )
{
    BOOLEAN IsEmpty;
    AcquireGuardedListLockShared(GuardedList);
    IsEmpty = IsListEmpty(&GuardedList->ListHead);
    ReleaseGuardedListLockShared(GuardedList);
    return IsEmpty;
}

FORCEINLINE
PLIST_ENTRY
RemoveHeadGuardedList(
    _Inout_ PGUARDED_LIST GuardedList
    )
{
    PLIST_ENTRY Entry;

    AcquireGuardedListLockExclusive(GuardedList);
    Entry = RemoveHeadList(&GuardedList->ListHead);
    GuardedList->NumberOfEntries--;
    ReleaseGuardedListLockExclusive(GuardedList);
    return Entry;
}

//
// We use _No_competing_thread_ for the TSX versions to pacify SAL.
//

_No_competing_thread_
FORCEINLINE
PLIST_ENTRY
RemoveHeadGuardedListTsxInline(
    _Inout_ PGUARDED_LIST GuardedList
    )
{
    ULONG Status;
    PLIST_ENTRY Entry;

Retry:
    Status = _xbegin();
    if (Status & _XABORT_RETRY) {
        goto Retry;
    } else if (Status != _XBEGIN_STARTED) {
        return RemoveHeadGuardedList(GuardedList);
    }

    Entry = RemoveHeadList(&GuardedList->ListHead);
    GuardedList->NumberOfEntries--;
    _xend();
    return Entry;
}

typedef
_No_competing_thread_
PLIST_ENTRY
(REMOVE_HEAD_GUARDED_LIST_TSX)(
    _Inout_ PGUARDED_LIST GuardedList
    );
typedef REMOVE_HEAD_GUARDED_LIST_TSX *PREMOVE_HEAD_GUARDED_LIST_TSX;
RTL_API REMOVE_HEAD_GUARDED_LIST_TSX RemoveHeadGuardedListTsx;

FORCEINLINE
PLIST_ENTRY
RemoveTailGuardedList(
    _Inout_ PGUARDED_LIST GuardedList
    )

{
    PLIST_ENTRY Entry;

    AcquireGuardedListLockExclusive(GuardedList);
    Entry = RemoveTailList(&GuardedList->ListHead);
    GuardedList->NumberOfEntries--;
    ReleaseGuardedListLockExclusive(GuardedList);
    return Entry;
}

_No_competing_thread_
FORCEINLINE
PLIST_ENTRY
RemoveTailGuardedListTsxInline(
    _Inout_ PGUARDED_LIST GuardedList
    )
{
    ULONG Status;
    PLIST_ENTRY Entry;

Retry:
    Status = _xbegin();
    if (Status & _XABORT_RETRY) {
        goto Retry;
    } else if (Status != _XBEGIN_STARTED) {
        return RemoveTailGuardedList(GuardedList);
    }

    Entry = RemoveTailList(&GuardedList->ListHead);
    GuardedList->NumberOfEntries--;
    _xend();
    return Entry;
}

typedef
_No_competing_thread_
PLIST_ENTRY
(REMOVE_TAIL_GUARDED_LIST_TSX)(
    _Inout_ PGUARDED_LIST GuardedList
    );
typedef REMOVE_TAIL_GUARDED_LIST_TSX *PREMOVE_TAIL_GUARDED_LIST_TSX;
RTL_API REMOVE_TAIL_GUARDED_LIST_TSX RemoveTailGuardedListTsx;

FORCEINLINE
VOID
InsertTailGuardedList(
    _Inout_ PGUARDED_LIST GuardedList,
    _Inout_ __drv_aliasesMem PLIST_ENTRY Entry
    )
{
    AcquireGuardedListLockExclusive(GuardedList);
    InsertTailList(&GuardedList->ListHead, Entry);
    GuardedList->NumberOfEntries++;
    ReleaseGuardedListLockExclusive(GuardedList);
    return;
}

_No_competing_thread_
FORCEINLINE
VOID
InsertTailGuardedListTsxInline(
    _Inout_ PGUARDED_LIST GuardedList,
    _Inout_ __drv_aliasesMem PLIST_ENTRY Entry
    )
{
    ULONG Status;

Retry:
    Status = _xbegin();

    if (Status & _XABORT_RETRY) {
        goto Retry;
    } else if (Status != _XBEGIN_STARTED) {
        InsertTailGuardedList(GuardedList, Entry);
        return;
    }

    InsertTailList(&GuardedList->ListHead, Entry);
    GuardedList->NumberOfEntries++;
    _xend();
    return;
}

typedef
_No_competing_thread_
VOID
(INSERT_TAIL_GUARDED_LIST_TSX)(
    _Inout_ PGUARDED_LIST GuardedList,
    _Inout_ __drv_aliasesMem PLIST_ENTRY Entry
    );
typedef INSERT_TAIL_GUARDED_LIST_TSX *PINSERT_TAIL_GUARDED_LIST_TSX;
RTL_API INSERT_TAIL_GUARDED_LIST_TSX InsertTailGuardedListTsx;

FORCEINLINE
VOID
InsertHeadGuardedList(
    _Inout_ PGUARDED_LIST GuardedList,
    _Inout_ __drv_aliasesMem PLIST_ENTRY Entry
    )
{
    AcquireGuardedListLockExclusive(GuardedList);
    InsertHeadList(&GuardedList->ListHead, Entry);
    GuardedList->NumberOfEntries++;
    ReleaseGuardedListLockExclusive(GuardedList);
    return;
}

FORCEINLINE
VOID
AppendTailGuardedList(
    _Inout_ PGUARDED_LIST GuardedList,
    _Inout_ __drv_aliasesMem PLIST_ENTRY Entry
    )
{
    AcquireGuardedListLockExclusive(GuardedList);
    AppendTailList(&GuardedList->ListHead, Entry);
    GuardedList->NumberOfEntries++;
    ReleaseGuardedListLockExclusive(GuardedList);
    return;
}

_No_competing_thread_
FORCEINLINE
VOID
AppendTailGuardedListTsxInline(
    _Inout_ PGUARDED_LIST GuardedList,
    _Inout_ __drv_aliasesMem PLIST_ENTRY ListEntry
    )
{
    ULONG Status;

Retry:
    Status = _xbegin();
    if (Status & _XABORT_RETRY) {
        goto Retry;
    } else if (Status != _XBEGIN_STARTED) {
        AppendTailGuardedList(GuardedList, ListEntry);
        return;
    }

    AppendTailList(&GuardedList->ListHead, ListEntry);
    GuardedList->NumberOfEntries++;
    _xend();
    return;
}

typedef
_No_competing_thread_
VOID
(APPEND_TAIL_GUARDED_LIST_TSX)(
    _Inout_ PGUARDED_LIST GuardedList,
    _Inout_ __drv_aliasesMem PLIST_ENTRY ListEntry
    );
typedef APPEND_TAIL_GUARDED_LIST_TSX *PAPPEND_TAIL_GUARDED_LIST_TSX;
RTL_API APPEND_TAIL_GUARDED_LIST_TSX AppendTailGuardedListTsx;

RTL_API
LONG
CompareStringCaseInsensitive(
    _In_ PCSTRING String1,
    _In_ PCSTRING String2
    );

RTL_API
_Check_return_
BOOL
FindCharsInUnicodeString(
    _In_     PRTL                Rtl,
    _In_     PCUNICODE_STRING    String,
    _In_     WCHAR               Char,
    _Inout_  PRTL_BITMAP         Bitmap,
    _In_     BOOL                Reverse
    );

RTL_API
_Check_return_
BOOL
CreateBitmapIndexForUnicodeString(
    _In_     PRTL                Rtl,
    _In_     PCUNICODE_STRING    String,
    _In_     WCHAR               Char,
    _Inout_  PHANDLE             HeapHandlePointer,
    _Inout_  PPRTL_BITMAP        BitmapPointer,
    _In_     BOOL                Reverse,
    _In_opt_ PFIND_CHARS_IN_UNICODE_STRING FindCharsFunction
    );

RTL_API
_Check_return_
BOOL
FindCharsInString(
    _In_     PRTL           Rtl,
    _In_     PCSTRING       String,
    _In_     CHAR           Char,
    _Inout_  PRTL_BITMAP    Bitmap,
    _In_     BOOL           Reverse
    );

RTL_API
_Check_return_
BOOL
CreateBitmapIndexForString(
    _In_     PRTL           Rtl,
    _In_     PCSTRING       String,
    _In_     CHAR           Char,
    _Inout_  PHANDLE        HeapHandlePointer,
    _Inout_  PPRTL_BITMAP   BitmapPointer,
    _In_     BOOL           Reverse,
    _In_opt_ PFIND_CHARS_IN_STRING FindCharsFunction
    );

RTL_API
VOID
Debugbreak();

RTL_API
BOOL
CreateUnicodeString(
    _In_  PRTL                  Rtl,
    _In_  PCUNICODE_STRING      Source,
    _Out_ PPUNICODE_STRING      Destination,
    _In_  PALLOCATION_ROUTINE   AllocationRoutine,
    _In_  PVOID                 AllocationContext
    );

RTL_API
_Check_return_
BOOL
FilesExistW(
    _In_      PRTL             Rtl,
    _In_      PUNICODE_STRING  Directory,
    _In_      USHORT           NumberOfFilenames,
    _In_      PPUNICODE_STRING Filenames,
    _Out_     PBOOL            Exists,
    _Out_opt_ PUSHORT          WhichIndex,
    _Out_opt_ PPUNICODE_STRING WhichFilename
    );

RTL_API
_Check_return_
BOOL
FilesExistA(
    _In_      PRTL     Rtl,
    _In_      PSTRING  Directory,
    _In_      USHORT   NumberOfFilenames,
    _In_      PPSTRING Filenames,
    _Out_     PBOOL    Exists,
    _Out_opt_ PUSHORT  WhichIndex,
    _Out_opt_ PPSTRING WhichFilename
    );

RTL_API
BOOL
TestExceptionHandler(VOID);

RTL_API
BOOL
InitializeRtlManually(PRTL Rtl, PULONG SizeOfRtl);

RTL_API COPY_FUNCTION CopyFunction;
RTL_API LOAD_SYMBOLS LoadSymbols;
RTL_API LOAD_SYMBOLS_FROM_MULTIPLE_MODULES LoadSymbolsFromMultipleModules;
RTL_API COPY_PAGES_EX CopyPagesAvx2;
RTL_API COPY_PAGES_EX CopyPagesMovsq;
RTL_API COPY_PAGES CopyPagesAvx2_C;
RTL_API COPY_PAGES CopyPagesNonTemporalAvx2;
RTL_API COPY_PAGES CopyPagesMovsq_C;
RTL_API COPY_TO_MEMORY_MAPPED_MEMORY CopyToMemoryMappedMemory;
RTL_API CREATE_AND_INITIALIZE_RTL CreateAndInitializeRtl;
RTL_API CURRENT_DIRECTORY_TO_RTL_PATH CurrentDirectoryToRtlPath;
RTL_API CURRENT_DIRECTORY_TO_UNICODE_STRING CurrentDirectoryToUnicodeString;
RTL_API CREATE_RANDOM_OBJECT_NAMES CreateRandomObjectNames;
RTL_API CREATE_SINGLE_RANDOM_OBJECT_NAME CreateSingleRandomObjectName;
RTL_API DESTROY_PATH_ENVIRONMENT_VARIABLE DestroyPathEnvironmentVariable;
RTL_API DESTROY_RTL DestroyRtl;
RTL_API DESTROY_RTL_PATH DestroyRtlPath;
RTL_API DISABLE_LOCK_MEMORY_PRIVILEGE DisableLockMemoryPrivilege;
RTL_API DISABLE_MANAGE_VOLUME_PRIVILEGE DisableManageVolumePrivilege;
RTL_API DISABLE_DEBUG_PRIVILEGE DisableDebugPrivilege;
RTL_API DISABLE_SYSTEM_PROFILE_PRIVILEGE DisableSystemProfilePrivilege;
RTL_API DISABLE_PROFILE_SINGLE_PROCESS_PRIVILEGE DisableProfileSingleProcessPrivilege;
RTL_API DISABLE_INCREASE_WORKING_SET_PRIVILEGE DisableIncreaseWorkingSetPrivilege;
RTL_API DISABLE_CREATE_SYMBOLIC_LINK_PRIVILEGE DisableCreateSymbolicLinkPrivilege;
RTL_API DISABLE_PRIVILEGE DisablePrivilege;
RTL_API ENABLE_LOCK_MEMORY_PRIVILEGE EnableLockMemoryPrivilege;
RTL_API ENABLE_MANAGE_VOLUME_PRIVILEGE EnableManageVolumePrivilege;
RTL_API ENABLE_DEBUG_PRIVILEGE EnableDebugPrivilege;
RTL_API ENABLE_SYSTEM_PROFILE_PRIVILEGE EnableSystemProfilePrivilege;
RTL_API ENABLE_PROFILE_SINGLE_PROCESS_PRIVILEGE EnableProfileSingleProcessPrivilege;
RTL_API ENABLE_INCREASE_WORKING_SET_PRIVILEGE EnableIncreaseWorkingSetPrivilege;
RTL_API ENABLE_CREATE_SYMBOLIC_LINK_PRIVILEGE EnableCreateSymbolicLinkPrivilege;
RTL_API ENABLE_PRIVILEGE EnablePrivilege;
RTL_API GET_MODULE_RTL_PATH GetModuleRtlPath;
RTL_API INITIALIZE_RTL InitializeRtl;
RTL_API INITIALIZE_RTL_FILE InitializeRtlFile;
RTL_API LOAD_PATH_ENVIRONMENT_VARIABLE LoadPathEnvironmentVariable;
RTL_API PREFAULT_PAGES PrefaultPages;
RTL_API REGISTER_DLL_NOTIFICATION RegisterDllNotification;
RTL_API SET_PRIVILEGE SetPrivilege;
RTL_API STRING_TO_EXISTING_RTL_PATH StringToExistingRtlPath;
RTL_API STRING_TO_RTL_PATH StringToRtlPath;
RTL_API UNICODE_STRING_TO_EXISTING_RTL_PATH UnicodeStringToExistingRtlPath;
RTL_API UNICODE_STRING_TO_RTL_PATH UnicodeStringToRtlPath;
RTL_API UNREGISTER_DLL_NOTIFICATION UnregisterDllNotification;
RTL_API UNREGISTER_RTL_ATEXIT_ENTRY UnregisterRtlAtExitEntry;
RTL_API WRITE_ENV_VAR_TO_REGISTRY WriteEnvVarToRegistry;
RTL_API WRITE_REGISTRY_STRING WriteRegistryString;
RTL_API APPEND_INTEGER_TO_CHAR_BUFFER AppendIntegerToCharBuffer;
RTL_API APPEND_STRING_TO_CHAR_BUFFER AppendStringToCharBuffer;
RTL_API APPEND_CHAR_BUFFER_TO_CHAR_BUFFER AppendCharBufferToCharBuffer;
RTL_API APPEND_CHAR_TO_CHAR_BUFFER AppendCharToCharBuffer;

//
// Inline function for helping loading Rtl and the heap allocator routines.
//

typedef struct _RTL_BOOTSTRAP {
    PINITIALIZE_RTL InitializeRtl;
    PINITIALIZE_ALLOCATOR InitializeHeapAllocator;
    PDESTROY_ALLOCATOR DestroyHeapAllocator;
    PLOAD_SYMBOLS LoadSymbols;
} RTL_BOOTSTRAP;
typedef RTL_BOOTSTRAP *PRTL_BOOTSTRAP;

FORCEINLINE
BOOLEAN
BootstrapRtl(
    HMODULE *RtlModulePointer,
    PRTL_BOOTSTRAP Bootstrap
    )
{
    BOOL Success;
    PROC Proc;
    HMODULE Module;
    ULONG NumberOfResolvedSymbols;
    ULONG ExpectedNumberOfResolvedSymbols;
    PLOAD_SYMBOLS LoadSymbols;

    CONST PCSTR Names[] = {
        "InitializeRtl",
        "RtlHeapAllocatorInitialize",
        "RtlHeapAllocatorDestroy",
        "LoadSymbols",
    };

    ULONG BitmapBuffer[(ALIGN_UP(ARRAYSIZE(Names), sizeof(ULONG) << 3) >> 5)+1];
    RTL_BITMAP FailedBitmap = { ARRAYSIZE(Names)+1, (PULONG)&BitmapBuffer };

    ExpectedNumberOfResolvedSymbols = ARRAYSIZE(Names);

    Module = LoadLibraryA("Rtl.dll");
    if (!Module) {
        return FALSE;
    }

    Proc = GetProcAddress(Module, "LoadSymbols");
    if (!Proc) {
        FreeLibrary(Module);
        return FALSE;
    }

    LoadSymbols = (PLOAD_SYMBOLS)Proc;

    Success = LoadSymbols(
        Names,
        ARRAYSIZE(Names),
        (PULONG_PTR)Bootstrap,
        sizeof(*Bootstrap) / sizeof(ULONG_PTR),
        Module,
        &FailedBitmap,
        TRUE,
        &NumberOfResolvedSymbols
    );

    if (!Success) {
        FreeLibrary(Module);
        return FALSE;
    }

    if (ExpectedNumberOfResolvedSymbols != NumberOfResolvedSymbols) {
        FreeLibrary(Module);
        return FALSE;
    }

    *RtlModulePointer = Module;

    return TRUE;
}


#ifdef __cplusplus
} // extern "C"
#endif

// vim:set ts=8 sw=4 sts=4 tw=80 expandtab nowrap                              :
