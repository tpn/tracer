#pragma once

#ifdef __cpplus
extern "C" {
#endif

#include <Windows.h>

#ifndef PAGE_SIZE
#define PAGE_SIZE 4096
#endif

#ifndef PAGE_ALIGN
#define PAGE_ALIGN(Va) ((PVOID)((ULONG_PTR)(Va) & ~(PAGE_SIZE - 1)))
#endif

typedef const LONG CLONG;
typedef PVOID *PPVOID;

typedef const SHORT CSHORT;

typedef struct _STRING {
    USHORT Length;
    USHORT MaximumLength;
    PCHAR  Buffer;
} STRING, *PSTRING, **PPSTRING;

typedef struct _UNICODE_STRING {
    USHORT Length;
    USHORT MaximumLength;
    PWSTR  Buffer;
} UNICODE_STRING, *PUNICODE_STRING, **PPUNICODE_STRING;
typedef const UNICODE_STRING *PCUNICODE_STRING;
#define UNICODE_NULL ((WCHAR)0)

typedef VOID (RTL_COPY_UNICODE_STRING)(
    _Inout_  PUNICODE_STRING  DestinationString,
    _In_opt_ PCUNICODE_STRING SourceString
    );
typedef RTL_COPY_UNICODE_STRING *PRTL_COPY_UNICODE_STRING;

// 65536
#define MAX_STRING  (sizeof(CHAR)  * ((1 << (sizeof(USHORT) * 8)) / sizeof(CHAR)))
#define MAX_USTRING (sizeof(WCHAR) * ((1 << (sizeof(USHORT) * 8)) / sizeof(WCHAR)))

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

typedef VOID (WINAPI *PGETSYSTEMTIMEPRECISEASFILETIME)(_Out_ LPFILETIME lpSystemTimeAsFileTime);
typedef LONG (WINAPI *PNTQUERYSYSTEMTIME)(_Out_ PLARGE_INTEGER SystemTime);

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

typedef EXCEPTION_DISPOSITION (__cdecl *PCSPECIFICHANDLER)(
    PEXCEPTION_RECORD ExceptionRecord,
    ULONG_PTR Frame,
    PCONTEXT Context,
    struct _DISPATCHER_CONTEXT *Dispatch
);

//
// Prefix Helpers
//
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

    PRTL_INITIALIZE_GENERIC_TABLE_AVL RtlInitializeGenericTableAvl;

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
} PREFIX_TABLE_ENTRY;
typedef PREFIX_TABLE_ENTRY *PPREFIX_TABLE_ENTRY;

typedef struct _PREFIX_TABLE {
    CSHORT NodeTypeCode;
    CSHORT NameLength;
    PPREFIX_TABLE_ENTRY NextPrefixTree;
} PREFIX_TABLE;
typedef PREFIX_TABLE *PPREFIX_TABLE;

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
} RTL_BITMAP, *PRTL_BITMAP;

typedef struct _RTL_BITMAP_RUN {
    ULONG StartingIndex;
    ULONG NumberOfBits;
} RTL_BITMAP_RUN, *PRTL_BITMAP_RUN;

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
    PRTL_SPLAY_LINKS RtlSplayLinks;                                                                    \
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
    PRTL_COPY_UNICODE_STRING RtlCopyUnicodeString;

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
    PRTL_FIND_FIRST_RUN_CLEAR RtlFindFirstRunClear;

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
// Our functions
//
typedef PVOID (*PCOPYTOMEMORYMAPPEDMEMORY)(
    PVOID Destination,
    LPCVOID Source,
    SIZE_T Size
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


#define _RTLEXFUNCTIONS_HEAD                             \
    PRTL_CHECK_BIT RtlCheckBit;                          \
    PRTL_INITIALIZE_SPLAY_LINKS RtlInitializeSplayLinks; \
    PRTL_PARENT RtlParent;                               \
    PRTL_LEFT_CHILD RtlLeftChild;                        \
    PRTL_RIGHT_CHILD RtlRightChild;                      \
    PRTL_IS_ROOT RtlIsRoot;                              \
    PRTL_IS_LEFT_CHILD RtlIsLeftChild;                   \
    PRTL_IS_RIGHT_CHILD RtlIsRightChild;                 \
    PRTL_INSERT_AS_LEFT_CHILD RtlInsertAsLeftChild;      \
    PRTL_INSERT_AS_RIGHT_CHILD RtlInsertAsRightChild;    \
    PCOPYTOMEMORYMAPPEDMEMORY CopyToMemoryMappedMemory;

typedef struct _RTLEXFUNCTIONS {
    _RTLEXFUNCTIONS_HEAD
} RTLEXFUNCTIONS, *PRTLEXFUNCTIONS, **PPRTLEXFUNCTIONS;

typedef struct _RTL {
    ULONG       Size;
    HMODULE     NtdllModule;
    HMODULE     Kernel32Module;
    HMODULE     NtosKrnlModule;

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

} RTL, *PRTL, **PPRTL;

#define RtlOffsetToPointer(B,O)    ((PCHAR)(     ((PCHAR)(B)) + ((ULONG_PTR)(O))  ))
#define RtlOffsetFromPointer(B,O)  ((PCHAR)(     ((PCHAR)(B)) - ((ULONG_PTR)(O))  ))
#define RtlPointerToOffset(B,P)    ((ULONG_PTR)( ((PCHAR)(P)) - ((PCHAR)(B))      ))

#define PrefaultPage(Address) (*(volatile *)(PCHAR)(Address))
#define PrefaultNextPage(Address) (*(volatile *)(PCHAR)((ULONG_PTR)Address + PAGE_SIZE))

#define ALIGN_DOWN(Address, Alignment) ((ULONG_PTR)(Address) & ~((ULONG_PTR)(Alignment)-1))
#define ALIGN_UP(Address, Alignment) (ALIGN_DOWN((Address) + (Alignment) - 1), (Alignment))

#define RTL_API __declspec(dllexport)

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
    PVOID Destination,
    LPCVOID Source,
    SIZE_T Size
);

#ifdef __cpp
} // extern "C"
#endif
