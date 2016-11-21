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


#ifndef PAGE_SIZE
#define PAGE_SIZE 4096
#endif

#ifndef PAGE_ALIGN
#define PAGE_ALIGN(Va) ((PVOID)((ULONG_PTR)(Va) & ~(PAGE_SIZE - 1)))
#endif

#ifndef ARGUMENT_PRESENT
#define ARGUMENT_PRESENT(ArgumentPointer)    (                 \
    (CHAR *)((ULONG_PTR)(ArgumentPointer)) != (CHAR *)(NULL) )
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

typedef const LONG CLONG;
typedef PVOID *PPVOID;
typedef const PVOID PCVOID;
typedef const PVOID CPVOID;
typedef CHAR **PPCHAR;
typedef WCHAR **PPWCHAR;
typedef CHAR **PPSTR;
typedef WCHAR **PPWSTR;

typedef const SHORT CSHORT;

typedef struct _RTL *PRTL;

typedef struct _STRING {
    USHORT Length;
    USHORT MaximumLength;
    LONG   Hash;
    PCHAR  Buffer;
} STRING, ANSI_STRING, *PSTRING, *PANSI_STRING, **PPSTRING, **PPANSI_STRING;
typedef const STRING *PCSTRING;

typedef struct _UNICODE_STRING {
    USHORT Length;
    USHORT MaximumLength;
    LONG   Hash;
    PWSTR  Buffer;
} UNICODE_STRING, *PUNICODE_STRING, **PPUNICODE_STRING, ***PPPUNICODE_STRING;
typedef const UNICODE_STRING *PCUNICODE_STRING;
#define UNICODE_NULL ((WCHAR)0)

#include "Memory.h"
#include "Commandline.h"

typedef CONST char *PCSZ;

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

RTL_API
LONG
CompareStringCaseInsensitive(
    _In_ PCSTRING String1,
    _In_ PCSTRING String2
    );


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
    _Out_ PPSAPI_WS_WATCH_INFORMATION WatchInfo,
    _In_ ULONG SizeOfWatchInfoBufferInBytes
    );
typedef GET_WS_CHANGES *PGET_WS_CHANGES;

typedef
_Check_return_
_Success_(return != 0)
BOOL
(WINAPI GET_WS_CHANGES_EX)(
    _In_ HANDLE ProcessHandle,
    _Out_ PPSAPI_WS_WATCH_INFORMATION_EX WatchInfoEx,
    _Inout_ PULONG SizeOfWatchInfoExBufferInBytes
    );
typedef GET_WS_CHANGES_EX *PGET_WS_CHANGES_EX;

typedef
_Check_return_
_Success_(return != 0)
BOOL
(WINAPI QUERY_WORKING_SET)(
    _In_ HANDLE ProcessHandle,
    _Out_ PPSAPI_WORKING_SET_INFORMATION WorkingSetInfo,
    _In_ ULONG SizeOfWorkingSetInfoBufferInBytes
    );
typedef QUERY_WORKING_SET *PQUERY_WORKING_SET;

typedef
_Check_return_
_Success_(return != 0)
BOOL
(WINAPI QUERY_WORKING_SET_EX)(
    _In_ HANDLE ProcessHandle,
    _Out_ PPSAPI_WORKING_SET_EX_INFORMATION WorkingSetExInfo,
    _In_ ULONG SizeOfWorkingSetExInfoBufferInBytes
    );
typedef QUERY_WORKING_SET_EX *PQUERY_WORKING_SET_EX;

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

typedef struct _SYSTEM_TIMER_FUNCTION {
    _SYSTEM_TIMER_FUNCTIONS_HEAD
} SYSTEM_TIMER_FUNCTION, *PSYSTEM_TIMER_FUNCTION, **PPSYSTEM_TIMER_FUNCTION;

BOOL
CallSystemTimer(
    _Out_       PFILETIME               SystemTime,
    _Inout_opt_ PPSYSTEM_TIMER_FUNCTION ppSystemTimerFunction
);

typedef NTSTATUS (WINAPI *PRTLCHARTOINTEGER)(
    _In_ PCSZ String,
    _In_opt_ ULONG Base,
    _Out_ PULONG Value
);

typedef DWORD (WINAPI *PSEARCHPATHW)(
    _In_opt_    LPCWSTR     lpPath,
    _In_        LPCWSTR     lpFileName,
    _In_opt_    LPCWSTR     lpExtension,
    _In_        DWORD       nBufferLength,
    _Out_       LPWSTR      lpBuffer,
    _Out_opt_   LPWSTR      lpFilePart
    );


//
// CRT functions.
//

typedef INT (__cdecl *PCRTCOMPARE)(
    _In_    CONST PVOID Key,
    _In_    CONST PVOID Datum
    );

typedef PVOID (*PBSEARCH)(
    _In_ CPVOID      Key,
    _In_ CPVOID      Base,
    _In_ SIZE_T      NumberOfElements,
    _In_ SIZE_T      WidthOfElement,
    _In_ PCRTCOMPARE Compare
    );

typedef VOID (*PQSORT)(
    _In_ PVOID       Base,
    _In_ SIZE_T      NumberOfElements,
    _In_ SIZE_T      WidthOfElement,
    _In_ PCRTCOMPARE Compare
    );

//
// End of CRT functions.
//

typedef NTSYSAPI SIZE_T (NTAPI RTL_COMPARE_MEMORY)(
    _In_ const VOID * Source1,
    _In_ const VOID * Source2,
    _In_ SIZE_T Length
    );

typedef RTL_COMPARE_MEMORY *PRTL_COMPARE_MEMORY;

typedef
EXCEPTION_DISPOSITION
(__cdecl __C_SPECIFIC_HANDLER)(
    PEXCEPTION_RECORD ExceptionRecord,
    ULONG_PTR Frame,
    PCONTEXT Context,
    struct _DISPATCHER_CONTEXT *Dispatch
    );
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

typedef BOOLEAN (NTAPI *PRTL_PREFIX_STRING)(
    _In_ PCSTRING String1,
    _In_ PCSTRING String2,
    _In_ BOOLEAN CaseInSensitive
    );

typedef BOOLEAN (NTAPI *PRTL_SUFFIX_STRING)(
    _In_ PCSTRING String1,
    _In_ PCSTRING String2,
    _In_ BOOLEAN CaseInSensitive
    );

typedef BOOLEAN (NTAPI *PRTL_PREFIX_UNICODE_STRING)(
    _In_ PCUNICODE_STRING String1,
    _In_ PCUNICODE_STRING String2,
    _In_ BOOLEAN CaseInSensitive
    );

typedef BOOLEAN (NTAPI *PRTL_SUFFIX_UNICODE_STRING)(
    _In_ PCUNICODE_STRING String1,
    _In_ PCUNICODE_STRING String2,
    _In_ BOOLEAN CaseInSensitive
    );

//
// Splay Links
//

typedef struct _RTL_SPLAY_LINKS {
    struct _RTL_SPLAY_LINKS *Parent;
    struct _RTL_SPLAY_LINKS *LeftChild;
    struct _RTL_SPLAY_LINKS *RightChild;
} RTL_SPLAY_LINKS, *PRTL_SPLAY_LINKS;

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
} RTL_BALANCED_LINKS;
typedef RTL_BALANCED_LINKS *PRTL_BALANCED_LINKS;

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
    PRTL_BALANCED_LINKS RestartKey;
    ULONG DeleteCount;
    PRTL_AVL_COMPARE_ROUTINE CompareRoutine;
    PRTL_AVL_ALLOCATE_ROUTINE AllocateRoutine;
    PRTL_AVL_FREE_ROUTINE FreeRoutine;
    PVOID TableContext;
} RTL_AVL_TABLE, *PRTL_AVL_TABLE;

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

typedef PVOID (NTAPI *PRTL_NUMBER_GENERIC_TABLE_ELEMENTS_AVL)(
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

typedef struct _RTL_DYNAMIC_HASH_TABLE_CONTEXT {
    PLIST_ENTRY ChainHead;
    PLIST_ENTRY PrevLinkage;
    ULONG_PTR Signature;
} RTL_DYNAMIC_HASH_TABLE_CONTEXT, *PRTL_DYNAMIC_HASH_TABLE_CONTEXT;

typedef struct _RTL_DYNAMIC_HASH_TABLE_ENUMERATOR {
    union {
       RTL_DYNAMIC_HASH_TABLE_ENTRY HashEntry;
       PLIST_ENTRY CurEntry;
    };
    PLIST_ENTRY ChainHead;
    ULONG BucketIndex;
} RTL_DYNAMIC_HASH_TABLE_ENUMERATOR, *PRTL_DYNAMIC_HASH_TABLE_ENUMERATOR;

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
    PPREFIX_TABLE_ENTRY NextPrefixTree;
} PREFIX_TABLE, *PPREFIX_TABLE, **PPPREFIX_TABLE;

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
    struct _UNICODE_PREFIX_TABLE_ENTRY *NextPrefixTree;
    struct _UNICODE_PREFIX_TABLE_ENTRY *CaseMatch;
    RTL_SPLAY_LINKS Links;
    PUNICODE_STRING Prefix;
} UNICODE_PREFIX_TABLE_ENTRY;
typedef UNICODE_PREFIX_TABLE_ENTRY *PUNICODE_PREFIX_TABLE_ENTRY;

typedef struct _UNICODE_PREFIX_TABLE {
    CSHORT NodeTypeCode;
    CSHORT NameLength;
    PUNICODE_PREFIX_TABLE_ENTRY NextPrefixTree;
    PUNICODE_PREFIX_TABLE_ENTRY LastNextEntry;
} UNICODE_PREFIX_TABLE;
typedef UNICODE_PREFIX_TABLE *PUNICODE_PREFIX_TABLE;

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

#pragma pack(push, 1)

typedef enum _PATH_LINK_TYPE {

    //
    // No linkage is being used.
    //

    PathLinkTypeNone = 0,
    PathLinkTypeListEntry,
    PathLinkTypePrefixTableEntry,
    PathLinkHashTableEntry

} PATH_LINK_TYPE, *PPATH_LINK_TYPE;

typedef struct _PATH_LINK {

    PATH_LINK_TYPE Type;

    union {
        LIST_ENTRY ListEntry;
        struct _UNICODE_PREFIX_TABLE_ENTRY PrefixTableEntry;
        struct _RTL_DYNAMIC_HASH_TABLE_ENTRY HashTableEntry;
    };

} PATH_LINK, *PPATH_LINK;

typedef _Struct_size_bytes_(sizeof(USHORT)) struct _PATH_FLAGS {
    USHORT IsFile:1;
    USHORT IsDirectory:1;
    USHORT IsSymlink:1;
    USHORT IsFullyQualified:1;
    USHORT HasParent:1;
    USHORT HasChildren:1;
} PATH_FLAGS, *PPATH_FLAGS;

typedef _Struct_size_bytes_(sizeof(ULONG)) struct _PATH_CREATE_FLAGS {
    ULONG CheckType:1;
    ULONG EnsureQualified:1;
} PATH_CREATE_FLAGS, *PPATH_CREATE_FLAGS;

typedef enum _PATH_TYPE_INTENT {

    PathTypeDontCare = 0,
    PathTypeLookup,
    PathTypeKnownFile,
    PathTypeKnownSymlink,
    PathTypeKnownDirectory

} PATH_TYPE_INTENT;

typedef _Struct_size_bytes_(StructSize) struct _PATH {

    //
    // Size of the structure, in bytes.
    //

    _Field_range_(==, sizeof(struct _PATH)) USHORT StructSize;      // 0    2

    //
    // Pad out to 4-bytes in order to get the bitmap buffer aligned on a
    // pointer.
    //

    WCHAR Drive;                                                    // 2    4

    union {
        struct {
            ULONG SizeOfReversedSlashesBitMap;                      // 4    8
            PULONG ReversedSlashesBitMapBuffer;                     // 8    16
        };
        RTL_BITMAP ReversedSlashesBitmap;                           // 8    16
    };

    //
    // Total number of bytes allocated for the structure, including StructSize.
    // This includes the bitmap buffers and unicode string buffer (all of which
    // will typically trail this structure in memory).
    //

    USHORT AllocSize;                                               // 2    18

    PATH_FLAGS Flags;                                               // 2    20

    union {
        struct {
            ULONG SizeOfReversedDotsBitMap;                         // 4    24
            PULONG ReversedDotsBitMapBuffer;                        // 8    32
        };
        RTL_BITMAP ReversedDotsBitmap;                              // 8    32
    };

    //
    // Allocator used to allocate this structure.
    //

    PALLOCATOR Allocator;                                           // 40   48

    //
    // Unicode strings for path details.
    //

    UNICODE_STRING Full;
    UNICODE_STRING Name;
    UNICODE_STRING Directory;
    UNICODE_STRING Extension;

    //
    // Path linkage.
    //

    PATH_LINK Link;

    //
    // Number of slashes and dots.  Saves having to do RtlNumberOfSetBits().
    //

    USHORT NumberOfSlashes;

    USHORT NumberOfDots;

} PATH, *PPATH, **PPPATH;

#pragma pack(pop)

typedef
_Success_(return != 0)
BOOL
(UNICODE_STRING_TO_PATH)(
    _In_ PRTL Rtl,
    _In_ PUNICODE_STRING String,
    _In_ PALLOCATOR Allocator,
    _Out_ PPPATH PathPointer
    );
typedef UNICODE_STRING_TO_PATH *PUNICODE_STRING_TO_PATH;
RTL_API UNICODE_STRING_TO_PATH UnicodeStringToPath;

typedef
_Success_(return != 0)
BOOL
(DESTROY_PATH)(
    _Inout_opt_ PPPATH PathPointer
    );
typedef DESTROY_PATH *PDESTROY_PATH;
RTL_API DESTROY_PATH DestroyPath;

typedef
_Success_(return != 0)
BOOL
(GET_MODULE_PATH)(
    _In_ PRTL Rtl,
    _In_ HMODULE Module,
    _In_ PALLOCATOR Allocator,
    _Out_ PPPATH PathPointer
    );
typedef GET_MODULE_PATH *PGET_MODULE_PATH;
RTL_API GET_MODULE_PATH GetModulePath;

typedef
PVOID
(MM_GET_MAXIMUM_FILE_SECTION_SIZE)(VOID);
typedef MM_GET_MAXIMUM_FILE_SECTION_SIZE *PMM_GET_MAXIMUM_FILE_SECTION_SIZE;

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

#define _RTLFUNCTIONS_HEAD                                                                             \
    PRTLCHARTOINTEGER RtlCharToInteger;                                                                \
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
    PVOID MmHighestUserAddress;                                                                        \
    PMM_GET_MAXIMUM_FILE_SECTION_SIZE MmGetMaximumFileSectionSize;                                     \
    PGET_PROCESS_MEMORY_INFO K32GetProcessMemoryInfo;                                                  \
    PGET_PROCESS_IO_COUNTERS GetProcessIoCounters;                                                     \
    PGET_PROCESS_HANDLE_COUNT GetProcessHandleCount;                                                   \
    PINITIALIZE_PROCESS_FOR_WS_WATCH K32InitializeProcessForWsWatch;                                   \
    PGET_WS_CHANGES K32GetWsChanges;                                                                   \
    PGET_WS_CHANGES_EX K32GetWsChangesEx;                                                              \
    PQUERY_WORKING_SET K32QueryWorkingSet;                                                             \
    PQUERY_WORKING_SET_EX K32QueryWorkingSetEx;                                                        \
    PSEARCHPATHW SearchPathW;                                                                          \
    PCREATE_TOOLHELP32_SNAPSHOT CreateToolhelp32Snapshot;                                              \
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
RTL_API COPY_TO_MEMORY_MAPPED_MEMORY CopyToMemoryMappedMemory;

typedef BOOL (FIND_CHARS_IN_UNICODE_STRING)(
    _In_     PRTL                Rtl,
    _In_     PUNICODE_STRING     String,
    _In_     WCHAR               Char,
    _Inout_  PRTL_BITMAP         Bitmap,
    _In_     BOOL                Reverse
    );

typedef FIND_CHARS_IN_UNICODE_STRING *PFIND_CHARS_IN_UNICODE_STRING;

RTL_API
_Check_return_
BOOL
FindCharsInUnicodeString(
    _In_     PRTL                Rtl,
    _In_     PUNICODE_STRING     String,
    _In_     WCHAR               Char,
    _Inout_  PRTL_BITMAP         Bitmap,
    _In_     BOOL                Reverse
    );

typedef BOOL (CREATE_BITMAP_INDEX_FOR_UNICODE_STRING)(
    _In_     PRTL                Rtl,
    _In_     PUNICODE_STRING     String,
    _In_     WCHAR               Char,
    _Inout_  PHANDLE             HeapHandlePointer,
    _Inout_  PPRTL_BITMAP        BitmapPointer,
    _In_     BOOL                Reverse,
    _In_opt_ PFIND_CHARS_IN_UNICODE_STRING FindCharsFunction
    );

typedef CREATE_BITMAP_INDEX_FOR_UNICODE_STRING \
       *PCREATE_BITMAP_INDEX_FOR_UNICODE_STRING;

RTL_API
_Check_return_
BOOL
CreateBitmapIndexForUnicodeString(
    _In_     PRTL                Rtl,
    _In_     PUNICODE_STRING     String,
    _In_     WCHAR               Char,
    _Inout_  PHANDLE             HeapHandlePointer,
    _Inout_  PPRTL_BITMAP        BitmapPointer,
    _In_     BOOL                Reverse,
    _In_opt_ PFIND_CHARS_IN_UNICODE_STRING FindCharsFunction
);

typedef BOOL (FIND_CHARS_IN_STRING)(
    _In_     PRTL           Rtl,
    _In_     PSTRING        String,
    _In_     CHAR           Char,
    _Inout_  PRTL_BITMAP    Bitmap,
    _In_     BOOL           Reverse
    );

typedef FIND_CHARS_IN_STRING *PFIND_CHARS_IN_STRING;

RTL_API
_Check_return_
BOOL
FindCharsInString(
    _In_     PRTL           Rtl,
    _In_     PSTRING        String,
    _In_     CHAR           Char,
    _Inout_  PRTL_BITMAP    Bitmap,
    _In_     BOOL           Reverse
    );

typedef BOOL (CREATE_BITMAP_INDEX_FOR_STRING)(
    _In_     PRTL           Rtl,
    _In_     PSTRING        String,
    _In_     CHAR           Char,
    _Inout_  PHANDLE        HeapHandlePointer,
    _Inout_  PPRTL_BITMAP   BitmapPointer,
    _In_     BOOL           Reverse,
    _In_opt_ PFIND_CHARS_IN_STRING FindCharsFunction
    );

typedef CREATE_BITMAP_INDEX_FOR_STRING \
       *PCREATE_BITMAP_INDEX_FOR_STRING;

RTL_API
_Check_return_
BOOL
CreateBitmapIndexForString(
    _In_     PRTL           Rtl,
    _In_     PSTRING        String,
    _In_     CHAR           Char,
    _Inout_  PHANDLE        HeapHandlePointer,
    _Inout_  PPRTL_BITMAP   BitmapPointer,
    _In_     BOOL           Reverse,
    _In_opt_ PFIND_CHARS_IN_STRING FindCharsFunction
    );

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

typedef _Struct_size_bytes_(Size) struct _DIRECTORY_CONTAINING_FILES {

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

typedef _Struct_size_bytes_(Size) struct _DIRECTORIES_CONTAINING_FILES {

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

typedef BOOL (*PPATH_CANONICALIZEA)(
        _Out_   LPSTR   Dest,
        _In_    LPCSTR  Source
    );

#define PATH_ENV_NAME L"Path"

typedef _Struct_size_bytes_(StructSize) struct _PATH_ENV_VAR {

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
RTL_API LOAD_PATH_ENVIRONMENT_VARIABLE LoadPathEnvironmentVariable;

typedef
VOID
(DESTROY_PATH_ENVIRONMENT_VARIABLE)(
    _Inout_ PPPATH_ENV_VAR PathPointer
    );
typedef DESTROY_PATH_ENVIRONMENT_VARIABLE *PDESTROY_PATH_ENVIRONMENT_VARIABLE;
RTL_API DESTROY_PATH_ENVIRONMENT_VARIABLE DestroyPathEnvironmentVariable;

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
RTL_API CURRENT_DIRECTORY_TO_UNICODE_STRING CurrentDirectoryToUnicodeString;

typedef
_Success_(return != 0)
_Check_return_
PPATH
(CURRENT_DIRECTORY_TO_PATH)(
    _In_ PALLOCATOR Allocator
    );
typedef CURRENT_DIRECTORY_TO_PATH \
      *PCURRENT_DIRECTORY_TO_PATH,\
    **PPCURRENT_DIRECTORY_TO_PATH;
RTL_API CURRENT_DIRECTORY_TO_PATH CurrentDirectoryToPath;

typedef
VOID
(DESTROY_RTL)(
    _In_opt_ struct _RTL **RtlPointer
    );
typedef DESTROY_RTL *PDESTROY_RTL, **PPDESTROY_RTL;
RTL_API DESTROY_RTL DestroyRtl;

typedef
BOOL
(PREFAULT_PAGES)(
    _In_ PVOID Address,
    _In_ ULONG NumberOfPages
    );
typedef PREFAULT_PAGES *PPREFAULT_PAGES, **PPPREFAULT_PAGES;
RTL_API PREFAULT_PAGES PrefaultPages;

#define PrefaultPage(Address) (*(volatile *)(PCHAR)(Address))

#define PrefaultNextPage(Address)                          \
    (*(volatile *)(PCHAR)((ULONG_PTR)Address + PAGE_SIZE))

#define _RTLEXFUNCTIONS_HEAD                                                   \
    PDESTROY_RTL DestroyRtl;                                                   \
    PPREFAULT_PAGES PrefaultPages;                                             \
    PRTL_CHECK_BIT RtlCheckBit;                                                \
    PRTL_INITIALIZE_SPLAY_LINKS RtlInitializeSplayLinks;                       \
    PRTL_PARENT RtlParent;                                                     \
    PRTL_LEFT_CHILD RtlLeftChild;                                              \
    PRTL_RIGHT_CHILD RtlRightChild;                                            \
    PRTL_IS_ROOT RtlIsRoot;                                                    \
    PRTL_IS_LEFT_CHILD RtlIsLeftChild;                                         \
    PRTL_IS_RIGHT_CHILD RtlIsRightChild;                                       \
    PRTL_INSERT_AS_LEFT_CHILD RtlInsertAsLeftChild;                            \
    PRTL_INSERT_AS_RIGHT_CHILD RtlInsertAsRightChild;                          \
    PCOPY_TO_MEMORY_MAPPED_MEMORY CopyToMemoryMappedMemory;                    \
    PFIND_CHARS_IN_UNICODE_STRING FindCharsInUnicodeString;                    \
    PCREATE_BITMAP_INDEX_FOR_UNICODE_STRING CreateBitmapIndexForUnicodeString; \
    PFIND_CHARS_IN_STRING FindCharsInString;                                   \
    PCREATE_BITMAP_INDEX_FOR_STRING CreateBitmapIndexForString;                \
    PFILES_EXISTW FilesExistW;                                                 \
    PFILES_EXISTA FilesExistA;                                                 \
    PTEST_EXCEPTION_HANDLER TestExceptionHandler;                              \
    PARGVW_TO_ARGVA ArgvWToArgvA;                                              \
    PUNICODE_STRING_TO_PATH UnicodeStringToPath;                               \
    PDESTROY_PATH DestroyPath;                                                 \
    PGET_MODULE_PATH GetModulePath;                                            \
    PCURRENT_DIRECTORY_TO_UNICODE_STRING CurrentDirectoryToUnicodeString;      \
    PCURRENT_DIRECTORY_TO_PATH CurrentDirectoryToPath;                         \
    PLOAD_PATH_ENVIRONMENT_VARIABLE LoadPathEnvironmentVariable;               \
    PDESTROY_PATH_ENVIRONMENT_VARIABLE DestroyPathEnvironmentVariable;         \
    PLOAD_SHLWAPI LoadShlwapi;

typedef struct _RTLEXFUNCTIONS {
    _RTLEXFUNCTIONS_HEAD
} RTLEXFUNCTIONS, *PRTLEXFUNCTIONS, **PPRTLEXFUNCTIONS;

#define _SHLWAPIFUNCTIONS_HEAD             \
    PPATH_CANONICALIZEA PathCanonicalizeA;

typedef struct _SHLWAPI_FUNCTIONS {
    _SHLWAPIFUNCTIONS_HEAD
} SHLWAPI_FUNCTIONS, *PSHLWAPI_FUNCTIONS, **PPSHLWAPI_FUNCTIONS;

typedef struct _RTL {
    ULONG       Size;
    HMODULE     NtdllModule;
    HMODULE     Kernel32Module;
    HMODULE     NtosKrnlModule;
    HMODULE     ShlwapiModule;

    P__C_SPECIFIC_HANDLER __C_specific_handler;
    P__SECURITY_INIT_COOKIE __security_init_cookie;
    PVOID MaximumFileSectionSize;

    HANDLE      HeapHandle;

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

} RTL, *PRTL, **PPRTL;

#define RtlUpcaseChar(C)                                         \
    (CHAR)(((C) >= 'a' && (C) <= 'z' ? (C) - ('a' - 'A') : (C)))

#define RtlUpcaseUnicodeChar(C)                                   \
    (WCHAR)(((C) >= 'a' && (C) <= 'z' ? (C) - ('a' - 'A') : (C)))

#define RtlOffsetToPointer(B,O)    ((PCHAR)(((PCHAR)(B)) + ((ULONG_PTR)(O))))
#define RtlOffsetFromPointer(B,O)  ((PCHAR)(((PCHAR)(B)) - ((ULONG_PTR)(O))))
#define RtlPointerToOffset(B,P)    ((ULONG_PTR)(((PCHAR)(P)) - ((PCHAR)(B))))

#define ALIGN_DOWN(Address, Alignment)                     \
    ((ULONG_PTR)(Address) & (~((ULONG_PTR)(Alignment)-1)))

#define ALIGN_UP(Address, Alignment) (                        \
    (((ULONG_PTR)(Address)) + (((ULONG_PTR)(Alignment))-1)) & \
    ~(((ULONG_PTR)(Alignment))-1)                             \
)

#define ALIGN_UP_POINTER(Address) (ALIGN_UP(Address, sizeof(ULONG_PTR)))

#define ALIGN_DOWN_POINTER(Address) (ALIGN_DOWN(Address, sizeof(ULONG_PTR)))

#define ALIGN_DOWN_USHORT_TO_POINTER_SIZE(Value)                   \
    (USHORT)(ALIGN_DOWN((USHORT)Value, (USHORT)sizeof(ULONG_PTR)))

#define ALIGN_UP_USHORT_TO_POINTER_SIZE(Value)                   \
    (USHORT)(ALIGN_UP((USHORT)Value, (USHORT)sizeof(ULONG_PTR)))

#define BITMAP_ALIGNMENT 128
#define ALIGN_UP_BITMAP(Address)                  \
    (USHORT)(ALIGN_UP(Address, BITMAP_ALIGNMENT))

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
USHORT
GetAddressAlignment(_In_ PVOID Address)
{
    ULONGLONG Integer = (ULONGLONG)Address;
    ULONGLONG NumTrailingZeros = TrailingZeros64(Integer);
    return (1 << NumTrailingZeros);
}

FORCEINLINE
BOOL
PointerToOffsetCrossesPageBoundary(
    _In_ PVOID Pointer,
    _In_ LONG Offset
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
AssertTrue(
    _In_ PSTR Expression,
    _In_ BOOL Value
    )
{
    if (!Value) {
#ifdef _DEBUG
        __debugbreak();
#endif
        OutputDebugStringA(Expression);
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
#ifdef _DEBUG
        __debugbreak();
#endif
        OutputDebugStringA(Expression);
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

#define TRY_AVX __try
#define TRY_AVX_ALIGNED __try
#define TRY_AVX_UNALIGNED __try

#define TRY_SSE42 __try
#define TRY_SSE42_ALIGNED __try
#define TRY_SSE42_UNALIGNED __try

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

////////////////////////////////////////////////////////////////////////////////
// SIMD Utilities
////////////////////////////////////////////////////////////////////////////////

typedef __m128i DECLSPEC_ALIGN(16) XMMWORD, *PXMMWORD, **PPXMMWORD;
typedef __m256i DECLSPEC_ALIGN(32) YMMWORD, *PYMMWORD, **PPYMMWORD;
//typedef __m512i DECLSPEC_ALIGN(64) ZMMWORD, *PZMMWORD, **PPZMMWORD;

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

RTL_API
VOID
Debugbreak();

typedef
_Success_(return != 0)
_Check_return_
BOOL
(INITIALIZE_RTL)(
    _Out_bytecap_(*SizeOfRtl) PRTL   Rtl,
    _Inout_                   PULONG SizeOfRtl
    );
typedef INITIALIZE_RTL *PINITIALIZE_RTL, **PPINTIALIZE_RTL;
RTL_API INITIALIZE_RTL InitializeRtl;

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
BOOL
IsValidNullTerminatedUnicodeStringWithMinimumLengthInChars(
    _In_ PUNICODE_STRING String,
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
IsValidUnicodeStringWithMinimumLengthInChars(
    _In_ PUNICODE_STRING String,
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
IsValidMinimumDirectoryUnicodeString(
    _In_ PUNICODE_STRING String
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
    _In_ PUNICODE_STRING String
    )
{
    return IsValidNullTerminatedUnicodeStringWithMinimumLengthInChars(
        String,
        1
    );
}

FORCEINLINE
BOOL
IsValidMinimumDirectoryNullTerminatedUnicodeString(
    _In_ PUNICODE_STRING String
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
    _In_ PUNICODE_STRING String
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
    _Inout_ PSTRING Destination,
    _In_    PSTRING String,
    _In_    CHAR    Char
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
    _Inout_ PSTRING Destination,
    _In_    CHAR    Char,
    _In_    PSTRING String
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
    _In_ PUNICODE_STRING SourceString,
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
HashUnicodeToAtom(_In_ PWSTR String)
{
    PWCH Buffer;
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
HashUnicodeStringToAtom(_In_ PUNICODE_STRING String)
{
    PWCH Buffer;
    WCHAR Char;
    ULONG Hash;
    USHORT Index;

    Hash = 0;
    Buffer = String->Buffer;

    for (Index = 0; Index < String->Length; Index++) {
        Char = RtlUpcaseUnicodeChar(Buffer[Index]);
        Hash = Hash + (Char << 1) + (Char >> 1) + Char;
    }

    return Hash;
}

FORCEINLINE
ULONG
HashAnsiToAtom(_In_ PSTR String)
{
    PCH Pointer;
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
HashAnsiStringToAtom(_In_ PSTRING String)
{
    PCH Buffer;
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


FORCEINLINE
VOID
InlineFindCharsInString(
    _In_     PSTRING      String,
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
    _In_ PUNICODE_STRING String,
    _In_ WCHAR           CharToFind,
    _In_ PRTL_BITMAP     Bitmap
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
    _In_ PUNICODE_STRING String,
    _In_ WCHAR           Char1ToFind,
    _In_ WCHAR           Char2ToFind,
    _In_ PRTL_BITMAP     Bitmap1,
    _In_ PRTL_BITMAP     Bitmap2
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


RTL_API
BOOL
InitializeRtlManually(PRTL Rtl, PULONG SizeOfRtl);

FORCEINLINE
BOOL
ConvertUtf16StringToUtf8StringSlow(
    _In_ PUNICODE_STRING Utf16,
    _Out_ PPSTRING Utf8Pointer,
    _In_ PALLOCATOR Allocator
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
        Allocator->Calloc(
            Allocator->Context,
            1,
            AlignedBufferSizeInBytes.LowPart
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
    _In_ PUNICODE_STRING Utf16,
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
USHORT
CountNumberOfDigits(_In_ ULONG Value)
{
    USHORT Count = 0;

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

    ActualNumberOfDigits = CountNumberOfDigits(Integer);

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

#ifdef __cplusplus
} // extern "C"
#endif

// vim:set ts=8 sw=4 sts=4 tw=80 expandtab                                     :
