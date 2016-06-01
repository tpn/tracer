#ifdef __cpplus
extern "C" {
#endif

#pragma once

#include <Windows.h>

#define RTL_API __declspec(dllexport)

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

typedef const LONG CLONG;
typedef PVOID *PPVOID;
typedef const PVOID PCVOID;
typedef const PVOID CPVOID;
typedef CHAR **PPCHAR;

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
} UNICODE_STRING, *PUNICODE_STRING, **PPUNICODE_STRING;
typedef const UNICODE_STRING *PCUNICODE_STRING;
#define UNICODE_NULL ((WCHAR)0)

typedef CONST char *PCSZ;

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


typedef CHAR *PSZ;
typedef const CHAR *PCSZ;

typedef struct _PROCESS_MEMORY_COUNTERS {
    DWORD  cb;
    DWORD  PageFaultCount;
    SIZE_T PeakWorkingSetSize;
    SIZE_T WorkingSetSize;
    SIZE_T QuotaPeakPagedPoolUsage;
    SIZE_T QuotaPagedPoolUsage;
    SIZE_T QuotaPeakNonPagedPoolUsage;
    SIZE_T QuotaNonPagedPoolUsage;
    SIZE_T PagefileUsage;
    SIZE_T PeakPagefileUsage;
} PROCESS_MEMORY_COUNTERS, *PPROCESS_MEMORY_COUNTERS;

typedef struct _PROCESS_MEMORY_COUNTERS_EX {
    DWORD  cb;
    DWORD  PageFaultCount;
    SIZE_T PeakWorkingSetSize;
    SIZE_T WorkingSetSize;
    SIZE_T QuotaPeakPagedPoolUsage;
    SIZE_T QuotaPagedPoolUsage;
    SIZE_T QuotaPeakNonPagedPoolUsage;
    SIZE_T QuotaNonPagedPoolUsage;
    SIZE_T PagefileUsage;
    SIZE_T PeakPagefileUsage;
    SIZE_T PrivateUsage;
} PROCESS_MEMORY_COUNTERS_EX, *PPROCESS_MEMORY_COUNTERS_EX;

typedef BOOL (WINAPI *PGET_PROCESS_MEMORY_INFO)(
    _In_    HANDLE                      Process,
    _In_    PPROCESS_MEMORY_COUNTERS    ppsmemCounters,
    _In_    DWORD                       cb
    );

typedef BOOL (WINAPI *PGET_PROCESS_IO_COUNTERS)(
    _In_    HANDLE          Process,
    _Out_   PIO_COUNTERS    lpIoCounters
    );

typedef BOOL (WINAPI *PGET_PROCESS_HANDLE_COUNT)(
    _In_        HANDLE      Process,
    _Inout_     PDWORD      pdwHandleCount
    );

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

typedef NTSYSAPI SIZE_T (NTAPI RTL_COMPARE_MEMORY)(
    _In_ const VOID * Source1,
    _In_ const VOID * Source2,
    _In_ SIZE_T Length
    );

typedef RTL_COMPARE_MEMORY *PRTL_COMPARE_MEMORY;

typedef EXCEPTION_DISPOSITION (__cdecl *P__C_SPECIFIC_HANDLER)(
    PEXCEPTION_RECORD ExceptionRecord,
    ULONG_PTR Frame,
    PCONTEXT Context,
    struct _DISPATCHER_CONTEXT *Dispatch
);

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

typedef BOOLEAN (NTAPI *PRTL_CREATE_HASH_TABLE)(
    _Inout_ PRTL_DYNAMIC_HASH_TABLE *HashTable,
    _In_ ULONG Shift,
    _In_ ULONG Flags
    );

typedef BOOLEAN (NTAPI *PRTL_CREATE_HASH_TABLE_EX)(
    _Inout_ PRTL_DYNAMIC_HASH_TABLE *HashTable,
    _In_ ULONG InitialSize,
    _In_ ULONG Shift,
    _In_ ULONG Flags
    );

typedef VOID (NTAPI *PRTL_DELETE_HASH_TABLE)(
    _In_ PRTL_DYNAMIC_HASH_TABLE HashTable
    );

typedef BOOLEAN (NTAPI *PRTL_INSERT_ENTRY_HASH_TABLE)(
    _In_ PRTL_DYNAMIC_HASH_TABLE HashTable,
    _In_ PRTL_DYNAMIC_HASH_TABLE_ENTRY Entry,
    _In_ ULONG_PTR Signature,
    _Inout_opt_ PRTL_DYNAMIC_HASH_TABLE_CONTEXT Context
    );

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

typedef PRTL_DYNAMIC_HASH_TABLE_ENTRY (NTAPI *PRTL_WEAKLY_ENUMERATE_ENTRY_HASH_TABLE)(
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

typedef PRTL_DYNAMIC_HASH_TABLE_ENTRY (NTAPI *PRTL_STRONGLY_ENUMERATE_ENTRY_HASH_TABLE)(
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
// Our extended PREFIX_TABLE and STRING_TABLE functionality.
//

#define INITIAL_STRING_TABLE_NUM_ELEMENTS 7

typedef struct _STRING_TABLE {
    PRTL Rtl;
    HANDLE HeapHandle;
    USHORT NumberOfElements;
    USHORT Available;
    ULONG Unused1;

    //
    // Function pointers needed for searching and sorting.
    //

    PCRTCOMPARE Compare;
    PBSEARCH BinarySearch;
    PQSORT QuickSort;

    //
    // Base address of the underlying PSTRING array.
    //

    PPSTRING Table;

} STRING_TABLE, *PSTRING_TABLE, **PPSTRING_TABLE;

typedef BOOL (*PCREATE_STRING_TABLE)(
    _In_        HANDLE          HeapHandle,
    _In_opt_    USHORT          NumberOfElements,
    _Out_       PPSTRING_TABLE  StringTablePointer
    );

typedef VOID (*PDESTROY_STRING_TABLE)(
    _In_    PPSTRING_TABLE  StringTable
    );

RTL_API
_Success_(return != 0)
BOOL
CreateStringTable(
    _In_        PRTL            Rtl,
    _In_        HANDLE          HeapHandle,
    _In_opt_    PUSHORT         NumberOfElements,
    _Out_       PPSTRING_TABLE  StringTablePointer
    );

typedef BOOL (*PINSERT_STRING_TABLE)(
    _In_        PSTRING_TABLE   StringTable,
    _In_        PSTRING         String,
    _Out_opt_   PBOOL           NewElement
    );

RTL_API
_Success_(return != 0)
BOOL
InsertStringTable(
    _In_        PSTRING_TABLE   StringTable,
    _In_        PSTRING         String,
    _Out_opt_   PBOOL           NewElement
    );

typedef struct _PREFIX_TABLE_EX {

    //
    // Inline the PREFIX_TABLE struct.
    //

    union {
        PREFIX_TABLE PrefixTable;
        struct {
            CSHORT NodeTypeCode;
            CSHORT NameLength;
            PPREFIX_TABLE_ENTRY NextPrefixTree;
        };
    };

    //
    // Inline the STRING_TABLE struct.
    //

    union {
        STRING_TABLE Strings;
        struct {
            HANDLE HeapHandle;
            USHORT NumberOfElements;
            USHORT Available;
            PPSTRING Array;
        };
    };

} PREFIX_TABLE_EX, *PPREFIX_TABLE_EX, **PPPREFIX_TABLE_EX;

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
    _Out_ PUNICODE_PREFIX_TABLE PrefixTable
    );

typedef BOOLEAN (NTAPI *PRTL_INSERT_UNICODE_PREFIX)(
    _In_ PUNICODE_PREFIX_TABLE PrefixTable,
    _In_ PUNICODE_STRING Prefix,
    _Out_ PUNICODE_PREFIX_TABLE_ENTRY PrefixTableEntry
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
    PGET_PROCESS_MEMORY_INFO K32GetProcessMemoryInfo;                                                  \
    PGET_PROCESS_IO_COUNTERS GetProcessIoCounters;                                                     \
    PGET_PROCESS_HANDLE_COUNT GetProcessHandleCount;                                                   \
    PCREATE_TOOLHELP32_SNAPSHOT CreateToolhelp32Snapshot;                                              \
    PTHREAD32_FIRST Thread32First;                                                                     \
    PTHREAD32_NEXT Thread32Next;

typedef struct _RTLFUNCTIONSE {
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
typedef PVOID (*PCOPY_TO_MEMORY_MAPPED_MEMORY)(
    PRTL Rtl,
    PVOID Destination,
    LPCVOID Source,
    SIZE_T Size
);

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

typedef PVOID (ALLOCATION_ROUTINE)(
    _In_opt_ PVOID AllocationContext,
    _In_ const ULONG ByteSize
    );

typedef ALLOCATION_ROUTINE *PALLOCATION_ROUTINE;

typedef VOID (FREE_ROUTINE)(
    _In_opt_ PVOID Context,
    _In_ PVOID Buffer
    );

typedef FREE_ROUTINE *PFREE_ROUTINE;

typedef BOOL (FILES_EXISTW)(
    _In_      PRTL             Rtl,
    _In_      PUNICODE_STRING  Directory,
    _In_      USHORT           NumberOfFilenames,
    _In_      PPUNICODE_STRING Filenames,
    _Out_     PBOOL            Exists,
    _Out_opt_ PUSHORT          WhichIndex,
    _Out_opt_ PPUNICODE_STRING WhichFilename
    );

typedef FILES_EXISTW *PFILES_EXISTW;

typedef BOOL (FILES_EXISTA)(
    _In_      PRTL     Rtl,
    _In_      PSTRING  Directory,
    _In_      USHORT   NumberOfFilenames,
    _In_      PPSTRING Filenames,
    _Out_     PBOOL    Exists,
    _Out_opt_ PUSHORT  WhichIndex,
    _Out_opt_ PPSTRING WhichFilename
    );

typedef FILES_EXISTA *PFILES_EXISTA;

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

#define _RTLEXFUNCTIONS_HEAD                                                   \
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

typedef BOOL (*PINITIALIZE_RTL)(
    _Out_bytecap_(*SizeOfRtl) PRTL   Rtl,
    _Inout_                   PULONG SizeOfRtl
    );

#define RtlUpcaseChar(C)         (CHAR )(((C) >= 'a' && (C) <= 'z' ? (C) - ('a' - 'A') : (C)))
#define RtlUpcaseUnicodeChar(C) (WCHAR )(((C) >= 'a' && (C) <= 'z' ? (C) - ('a' - 'A') : (C)))

#define RtlOffsetToPointer(B,O)    ((PCHAR)(     ((PCHAR)(B)) + ((ULONG_PTR)(O))  ))
#define RtlOffsetFromPointer(B,O)  ((PCHAR)(     ((PCHAR)(B)) - ((ULONG_PTR)(O))  ))
#define RtlPointerToOffset(B,P)    ((ULONG_PTR)( ((PCHAR)(P)) - ((PCHAR)(B))      ))

#define PrefaultPage(Address) (*(volatile *)(PCHAR)(Address))
#define PrefaultNextPage(Address) (*(volatile *)(PCHAR)((ULONG_PTR)Address + PAGE_SIZE))

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

RTL_API
VOID
Debugbreak();

RTL_API
BOOL
InitializeRtl(
    _Out_bytecap_(*SizeOfRtl) PRTL   Rtl,
    _Inout_                   PULONG SizeOfRtl
);

RTL_API
PVOID
CopyToMemoryMappedMemory(
    PRTL Rtl,
    PVOID Destination,
    LPCVOID Source,
    SIZE_T Size
    );

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

FORCEINLINE
VOID
CopyString(
    _Out_    PSTRING Destination,
    _In_opt_ PCSTRING Source
    )
{
    USHORT Length;

    if (!Source) {
        Destination->Length = 0;
        return;
    }

    Length = min(Destination->MaximumLength, Source->Length);

    __movsb((PBYTE)Destination->Buffer,
            (PBYTE)Source->Buffer,
            Length);

    Destination->Length = Length;

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

RTL_API
BOOL
InitializeRtlManually(PRTL Rtl, PULONG SizeOfRtl);

RTL_API
BOOL
LoadShlwapi(PRTL Rtl);

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

#ifdef __cpp
} // extern "C"
#endif
