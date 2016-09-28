/*++

Copyright (c) 2016 Trent Nelson <trent@trent.me>

Module Name:

    TraceStore.h

Abstract:

    This is the main header file for the TraceStore component.  It defines
    structures and functions related to all aspects of TraceStore functionality.

--*/

#pragma once

#ifdef __cplusplus
extern "C" {
#endif

#ifdef _TRACE_STORE_INTERNAL_BUILD

//
// This is an internal build of the TraceStore component.
//

#define TRACE_STORE_API __declspec(dllexport)
#define TRACE_STORE_DATA extern __declspec(dllexport)

#include "stdafx.h"

#else

//
// We're being included by an external component.
//

#define TRACE_STORE_API __declspec(dllimport)
#define TRACE_STORE_DATA extern __declspec(dllimport)

#include <Windows.h>
#include <sal.h>
#include <Strsafe.h>
#include "../Rtl/Rtl.h"
#include "../TracerConfig/TracerConfig.h"

#endif

#define TIMESTAMP_TO_SECONDS    1000000
#define SECONDS_TO_MICROSECONDS 1000000

#define MAX_UNICODE_STRING 255
#define _OUR_MAX_PATH MAX_UNICODE_STRING

////////////////////////////////////////////////////////////////////////////////
// Structures
////////////////////////////////////////////////////////////////////////////////

typedef struct _TRACE_STORE_ALLOCATION {
    union {
        ULARGE_INTEGER  NumberOfRecords;
        ULARGE_INTEGER  NumberOfAllocations;
    };
    union {
        LARGE_INTEGER   RecordSize;
        ULARGE_INTEGER  AllocationSize;
    };
} TRACE_STORE_ALLOCATION, *PTRACE_STORE_ALLOCATION;

C_ASSERT(sizeof(TRACE_STORE_ALLOCATION) == 16);

typedef struct _TRACE_STORE_ADDRESS {
    PVOID         PreferredBaseAddress;                     // 8    0   8
    PVOID         BaseAddress;                              // 8    8   16
    LARGE_INTEGER FileOffset;                               // 8    16  24
    LARGE_INTEGER MappedSize;                               // 8    24  32
    DWORD         ProcessId;                                // 4    32  36
    DWORD         RequestingThreadId;                       // 4    36  40

    //
    // Timestamps are kept at each stage of the memory map's lifecycle.  They
    // are used to calculate the elapsed times (in microseconds) below.  The
    // LARGE_INTEGER structs are passed to QueryPerformanceCounter(&Counter).
    //

    struct {

        //
        // The time where the new memory map was pushed onto the "prepare
        // memory map" threadpool work queue.
        //

        LARGE_INTEGER Requested;                            // 8    40  48

        //
        // The time the memory map was pushed onto the "next memory map
        // available" list.
        //

        LARGE_INTEGER Prepared;                             // 8    48  56

        //
        // The time the memory map was popped off the available list and put
        // into active use.
        //

        LARGE_INTEGER Consumed;                             // 8    56  64

        //
        // The time the memory map was pushed onto the "release memory map"
        // queue.
        //

        LARGE_INTEGER Retired;                              // 8    64  72

        //
        // The time the memory map had been released; this will be after the
        // view has been flushed and the memory map handle has been closed.
        //

        LARGE_INTEGER Released;                             // 8    72  80

    } Timestamp;

    //
    // Elapsed timestamps that capture the time the memory map spent in various
    // states.
    //

    struct {

        //
        // Time between Requested and Prepared.
        //

        LARGE_INTEGER AwaitingPreparation;                  // 8    80  88


        //
        // Time between Prepared and Consumed.
        //

        LARGE_INTEGER AwaitingConsumption;                  // 8    88  96

        //
        // Time between Consumed and Retired.
        //

        LARGE_INTEGER Active;                               // 8    96  104

        //
        // Time between Retired and Released.
        //

        LARGE_INTEGER AwaitingRelease;                      // 8    104 112

    } Elapsed;

    LONG MappedSequenceId;                                  // 4    112 116

    union {
        PROCESSOR_NUMBER RequestingProcessor;               // 4    116 120
        struct {
            WORD RequestingProcGroup;
            BYTE RequestingProcNumber;
            union {
                BYTE RequestingProcReserved;
                UCHAR RequestingNumaNode;
            };
        };
    };

    union {
        PROCESSOR_NUMBER FulfillingProcessor;               // 4    120 124
        struct {
            WORD FulfillingProcGroup;
            BYTE FulfillingProcNumber;
            union {
                BYTE FulfillingProcReserved;
                UCHAR FulfillingNumaNode;
            };
        };
    };

    DWORD FulfillingThreadId;                               // 4    124 128

} TRACE_STORE_ADDRESS, *PTRACE_STORE_ADDRESS, **PPTRACE_STORE_ADDRESS;

C_ASSERT(sizeof(TRACE_STORE_ADDRESS) == 128);

typedef struct _TRACE_STORE_EOF {
    LARGE_INTEGER EndOfFile;
} TRACE_STORE_EOF, *PTRACE_STORE_EOF, **PPTRACE_STORE_EOF;

typedef struct _TRACE_STORE_START_TIME {
    FILETIME        FileTimeUtc;
    FILETIME        FileTimeLocal;
    SYSTEMTIME      SystemTimeUtc;
    SYSTEMTIME      SystemTimeLocal;
    ULARGE_INTEGER  SecondsSince1970;
    ULARGE_INTEGER  MicrosecondsSince1970;
    LARGE_INTEGER   PerformanceCounter;
} TRACE_STORE_START_TIME, *PTRACE_STORE_START_TIME;

typedef struct _TRACE_STORE_TIME {

    //
    // The value of QueryPerformanceFrequency().
    //

    LARGE_INTEGER   Frequency;

    //
    // The multiplicand used when calculating elapsed time from the performance
    // counter and frequency using the MSDN-recommended approach:
    //
    //      Elapsed.QuadPart *= Multiplicand.QuadPart;
    //      Elapsed.QuadPart /= Frequency.QuadPart;
    //
    //

    LARGE_INTEGER           Multiplicand;

    TRACE_STORE_START_TIME  StartTime;

} TRACE_STORE_TIME, *PTRACE_STORE_TIME, **PPTRACE_STORE_TIME;

//
// The TRACE_STORE_STATS structure tracks statistics about the trace store.
// It is currently used to track performance metrics that indicate the trace
// store mechanisms are falling behind the current system load (i.e. memory
// maps can't be prepared quickly enough, allocations are outpacing preparation,
// etc).
//

typedef struct _TRACE_STORE_STATS {

    //
    // Tracks the total number of records that had to be dropped because a
    // trace store allocation couldn't be satisfied (for one of the reasons
    // tracked separately in counters below).
    //

    ULONG DroppedRecords;

    //
    // Tracks how many times allocation failed because no free memory maps
    // were available to prepare the next trace store (or retire old stores).
    // (A lookaside-list of free memory map structures is kept because they
    // are allocated and retired relatively frequently.  A high value for this
    // counter would indicate an anomalous condition as the number of possible
    // memory map structures kept "in flight" is relatively predictable for
    // a given set of trace stores: prev prev, prev, active, and then next.
    // The solution would be to increase the default number of free memory
    // maps.)
    //

    ULONG ExhaustedFreeMemoryMaps;

    //
    // This counter tracks how many times an allocation failed because the
    // next memory map hadn't been prepared yet in the asynchronous thread
    // pool.  When a new memory map is "consumed", a threadpool work item is
    // submitted to prepare the next memory map for the trace store, such that
    // there's always a ready memory map as soon as the current one has been
    // exhausted.  A high value for this counter means that an entire memory
    // map section was consumed before the next memory map could be prepared;
    // this typically happens when there's a confluence of three things in
    // particular: a fast CPU, a tight loop being traced (i.e. a very high rate
    // of trace store allocations), and a memory map size being set too small.
    // Bumping the size of each individual memory map should fix this because
    // the time required to prepare a memory map isn't directly proportional
    // to the size of the memory map -- that is, the kernel doesn't have to
    // do that much more work to prepare a 10MB mapping versus a 2MB mapping,
    // but a trace store consumer will take a lot longer to consume a 10MB
    // chunk versus a 2MB chunk.
    //

    ULONG AllocationsOutpacingNextMemoryMapPreparation;

    //
    // Tracks how many times the desired base address for a trace store memory
    // mapping was unavailable.  That is, the number of times MapViewOfFileEx()
    // failed to map a view when called with a preferred (explicit) base
    // address.  This has performance implications for trace store readers that
    // need to remap trace stores at the same address but are unable, as they'll
    // have to do expensive on-the-fly remappings of any embedded pointers
    // before the memory map can be used.  (Similar to the cost incurred by
    // the loader when a DLL can't be mapped at its preferred address.)
    //

    ULONG PreferredAddressUnavailable;

} TRACE_STORE_STATS, *PTRACE_STORE_STATS, **PPTRACE_STORE_STATS;

typedef struct _TRACE_SESSION {
    PRTL                Rtl;
    LARGE_INTEGER       SessionId;
    GUID                MachineGuid;
    PISID               Sid;
    PCWSTR              UserName;
    PCWSTR              ComputerName;
    PCWSTR              DomainName;
    FILETIME            SystemTime;
} TRACE_SESSION, *PTRACE_SESSION;

//
// TRACE_STORE_INFO is intended for storage of single-instance structs of
// various tracing-related information.  (Single-instance as in there's only
// ever one global instance of the given record, i.e. the EndOfFile.  This is
// in contrast to things like the allocation and address records, which by
// nature, will usually have multiple occurrences/allocations.)
//

typedef struct _TRACE_STORE_INFO {
    TRACE_STORE_EOF     Eof;
    TRACE_STORE_TIME    Time;
    TRACE_STORE_STATS   Stats;
} TRACE_STORE_INFO, *PTRACE_STORE_INFO, **PPTRACE_STORE_INFO;

typedef
VOID
(WINAPI GET_SYSTEM_TIME_PRECISE_AS_FILETIME)(
    _Out_ LPFILETIME lpSystemTimeAsFileTime
    );
typedef GET_SYSTEM_TIME_PRECISE_AS_FILETIME \
    *PGET_SYSTEM_TIME_PRECISE_AS_FILETIME;

typedef
NTSTATUS
(WINAPI NT_QUERY_SYSTEM_TIME)(
    _Out_ PLARGE_INTEGER SystemTime
    );
typedef NT_QUERY_SYSTEM_TIME *PNT_QUERY_SYSTEM_TIME;

typedef struct _TIMER_FUNCTION {
    PGET_SYSTEM_TIME_PRECISE_AS_FILETIME GetSystemTimePreciseAsFileTime;
    PNT_QUERY_SYSTEM_TIME NtQuerySystemTime;
} TIMER_FUNCTION;

typedef TIMER_FUNCTION *PTIMER_FUNCTION;
typedef TIMER_FUNCTION **PPTIMER_FUNCTION;

//
// Forward definitions.
//

typedef struct _TRACE_STORE TRACE_STORE, *PTRACE_STORE;
typedef struct _TRACE_STORES TRACE_STORES, *PTRACE_STORES;
typedef struct _TRACE_SESSION TRACE_SESSION, *PTRACE_SESSION;
typedef struct _TRACE_CONTEXT TRACE_CONTEXT, *PTRACE_CONTEXT;

typedef struct _TRACE_FLAGS {
    union {
        ULONG AsLong;
        struct {
            ULONG Readonly:1;
            ULONG Compress:1;
            ULONG DisablePrefaultPages:1;

            //
            // The following flags relate to the dwFlagsAndAttributes parameter
            // passed to CreateFile() when opening a trace store.
            //

            //
            // When set, do not set the FILE_FLAG_OVERLAPPED flag.
            //

            ULONG DisableFileFlagOverlapped:1;

            //
            // When set, do not set the FILE_FLAG_SEQUENTIAL_SCAN flag.  This
            // will automatically be set if EnableFileFlagRandomAccess is set.
            //

            ULONG DisableFileFlagSequentialScan:1;

            //
            // When set, sets the FILE_FLAG_RANDOM_ACCESS flag.  (This implies
            // DisableFileFlagSequentialScan.)
            //

            ULONG EnableFileFlagRandomAccess:1;

            //
            // When set, sets the FILE_FLAG_WRITE_THROUGH flag.
            //

            ULONG EnableFileFlagWriteThrough:1;

        };
    };
    ULONG Unused1;
} TRACE_FLAGS, *PTRACE_FLAGS;

typedef struct _TRACE_CONTEXT {
    ULONG                       Size;
    ULONG                       SequenceId;
    PRTL                        Rtl;
    PTRACE_SESSION              TraceSession;
    PTRACE_STORES               TraceStores;
    PTIMER_FUNCTION             TimerFunction;
    PVOID                       UserData;
    PTP_CALLBACK_ENVIRON        ThreadpoolCallbackEnvironment;
    HANDLE                      HeapHandle;
    PSTRING                     BaseDirectory;
    TRACE_STORE_TIME            Time;
} TRACE_CONTEXT, *PTRACE_CONTEXT;

typedef struct _TRACE_STORE_THREADPOOL {
    PTP_POOL Threadpool;
    TP_CALLBACK_ENVIRON CallbackEnvironment;
    PTP_WORK ExtendTraceStoreCallback;
    HANDLE ExtendTraceStoreEvent;
} TRACE_STORE_THREADPOOL, *PTRACE_STORE_THREADPOOL;

typedef struct _TRACE_STORES TRACE_STORES, *PTRACE_STORES;

//
// This is the workhorse of the trace store machinery: the 64-byte (one cache
// line) memory map struct that captures details about a given trace store
// memory mapping section.  These are pushed and popped atomically off the
// interlocked singly-linked list heads for each TRACE_STORE struct.
//

typedef struct DECLSPEC_ALIGN(16) _TRACE_STORE_MEMORY_MAP {
    union {
        DECLSPEC_ALIGN(16) SLIST_ENTRY              ListEntry;   // 8       8
        struct {
            DECLSPEC_ALIGN(8) PVOID                 PrevAddress; // 8       8
            DECLSPEC_ALIGN(8) PTRACE_STORE_ADDRESS  pAddress;    // 8       16
        };
    };
    DECLSPEC_ALIGN(8)  HANDLE        FileHandle;                // 8        24
    DECLSPEC_ALIGN(8)  HANDLE        MappingHandle;             // 8        32
    DECLSPEC_ALIGN(8)  LARGE_INTEGER FileOffset;                // 8        40
    DECLSPEC_ALIGN(8)  LARGE_INTEGER MappingSize;               // 8        48
    DECLSPEC_ALIGN(8)  PVOID         BaseAddress;               // 8        56
    DECLSPEC_ALIGN(8)
    union {
        PVOID PreferredBaseAddress;                             // 8        64
        PVOID NextAddress;                                      // 8        64
    };
} TRACE_STORE_MEMORY_MAP, *PTRACE_STORE_MEMORY_MAP, **PPTRACE_STORE_MEMORY_MAP;

typedef volatile PTRACE_STORE_MEMORY_MAP VPTRACE_STORE_MEMORY_MAP;

C_ASSERT(sizeof(TRACE_STORE_MEMORY_MAP) == 64);

typedef
_Check_return_
_Success_(return != 0)
PVOID
(ALLOCATE_RECORDS)(
    _In_    PTRACE_CONTEXT  TraceContext,
    _In_    PTRACE_STORE    TraceStore,
    _In_    PULARGE_INTEGER RecordSize,
    _In_    PULARGE_INTEGER NumberOfRecords
    );
typedef ALLOCATE_RECORDS *PALLOCATE_RECORDS;

typedef struct _TRACE_STORE {
    SLIST_HEADER            CloseMemoryMaps;
    SLIST_HEADER            PrepareMemoryMaps;
    SLIST_HEADER            NextMemoryMaps;
    SLIST_HEADER            FreeMemoryMaps;
    SLIST_HEADER            PrefaultMemoryMaps;

    PRTL                    Rtl;
    PTRACE_CONTEXT          TraceContext;
    LARGE_INTEGER           InitialSize;
    LARGE_INTEGER           ExtensionSize;
    LARGE_INTEGER           MappingSize;
    PTP_WORK                PrefaultFuturePageWork;
    PTP_WORK                PrepareNextMemoryMapWork;
    PTP_WORK                CloseMemoryMapWork;
    HANDLE                  NextMemoryMapAvailableEvent;
    HANDLE                  AllMemoryMapsAreFreeEvent;

    PTRACE_STORE_MEMORY_MAP PrevMemoryMap;
    PTRACE_STORE_MEMORY_MAP MemoryMap;

    volatile ULONG  NumberOfActiveMemoryMaps;

    volatile LONG   MappedSequenceId;

    //
    // Each trace store, when initialized, is assigned a unique sequence
    // ID, starting at 1.  The trace store and all metadata stores share
    // the same sequence ID.
    //

    ULONG SequenceId;

    LARGE_INTEGER TotalNumberOfAllocations;
    LARGE_INTEGER TotalAllocationSize;

    TRACE_FLAGS Flags;

    union {
        struct {
            ULONG NoRetire:1;
            ULONG NoPrefaulting:1;
            ULONG RecordSimpleAllocation:1;
            ULONG NoPreferredAddressReuse:1;
            ULONG IsReadonly:1;
            ULONG SetEndOfFileOnClose:1;
            ULONG IsMetadata:1;
        };
    };

    DWORD CreateFileDesiredAccess;
    DWORD CreateFileMappingProtectionFlags;
    DWORD CreateFileFlagsAndAttributes;
    DWORD MapViewOfFileDesiredAccess;

    HANDLE FileHandle;
    PVOID PrevAddress;

    PTRACE_STORE            AllocationStore;
    PTRACE_STORE            AddressStore;
    PTRACE_STORE            InfoStore;

    PALLOCATE_RECORDS       AllocateRecords;

    //
    // Inline TRACE_STORE_ALLOCATION.
    //

    union {
        TRACE_STORE_ALLOCATION Allocation;
        struct {
            union {
                ULARGE_INTEGER  NumberOfRecords;
                ULARGE_INTEGER  NumberOfAllocations;
            };
            union {
                LARGE_INTEGER   RecordSize;
                ULARGE_INTEGER  AllocationSize;
            };
        };
    };
    PTRACE_STORE_ALLOCATION pAllocation;

    //
    // Inline TRACE_STORE_INFO.
    //

    union {

        TRACE_STORE_INFO Info;

        struct {

            //
            // Inline TRACE_STORE_EOF.
            //

            union {

                TRACE_STORE_EOF Eof;

                struct {
                    LARGE_INTEGER EndOfFile;
                };

            };


            //
            // Inline TRACE_STORE_TIME.
            //

            union {

                TRACE_STORE_TIME Time;

                struct {
                    LARGE_INTEGER   Frequency;
                    LARGE_INTEGER   Multiplicand;
                    FILETIME        StartTime;
                    LARGE_INTEGER   StartCounter;
                };

            };

            //
            // Inline TRACE_STORE_STATS.
            //

            union {

                TRACE_STORE_STATS Stats;

                struct {
                    ULONG DroppedRecords;
                    ULONG ExhaustedFreeMemoryMaps;
                    ULONG AllocationsOutpacingNextMemoryMapPreparation;
                    ULONG PreferredAddressUnavailable;
                };

            };
        };
    };

    //
    // The metadata stores for a given trace store are mapped into the pointers
    // below.  This is a somewhat quirky but convenient way of accessing the
    // metadata struct from the trace store but also having any changes made
    // to the struct be reflected in the backing file -- by virtue of the fact
    // NT's virtual memory, file system and cache manager all play together
    // nicely to give a single coherent view of memory/file contents.
    //

    PTRACE_STORE_EOF    pEof;
    PTRACE_STORE_TIME   pTime;
    PTRACE_STORE_STATS  pStats;
    PTRACE_STORE_INFO   pInfo;

    UNICODE_STRING Path;
    WCHAR PathBuffer[_OUR_MAX_PATH];

} TRACE_STORE, *PTRACE_STORE;

typedef enum _TRACE_STORE_ID {
    TraceStoreNullId = 0,
    TraceStoreEventId = 1,
    TraceStoreStringId,
    TraceStoreStringBufferId,
    TraceStoreHashedStringId,
    TraceStoreHashedStringBufferId,
    TraceStoreBufferId,
    TraceStoreFunctionTableId,
    TraceStoreFunctionTableEntryId,
    TraceStorePathTableId,
    TraceStorePathTableEntryId,
    TraceStoreSessionId,
    TraceStoreFilenameStringId,
    TraceStoreFilenameStringBufferId,
    TraceStoreDirectoryStringId,
    TraceStoreDirectoryStringBufferId,
    TraceStoreStringArrayId,
    TraceStoreStringTableId,
    TraceStoreInvalidId
} TRACE_STORE_ID, *PTRACE_STORE_ID;

typedef enum _TRACE_STORE_INDEX {
    TraceStoreEventIndex = 0,
    TraceStoreEventAllocationIndex,
    TraceStoreEventAddressIndex,
    TraceStoreEventInfoIndex,
    TraceStoreStringIndex,
    TraceStoreStringAllocationIndex,
    TraceStoreStringAddressIndex,
    TraceStoreStringInfoIndex,
    TraceStoreStringBufferIndex,
    TraceStoreStringBufferAllocationIndex,
    TraceStoreStringBufferAddressIndex,
    TraceStoreStringBufferInfoIndex,
    TraceStoreHashedStringIndex,
    TraceStoreHashedStringAllocationIndex,
    TraceStoreHashedStringAddressIndex,
    TraceStoreHashedStringInfoIndex,
    TraceStoreHashedStringBufferIndex,
    TraceStoreHashedStringBufferAllocationIndex,
    TraceStoreHashedStringBufferAddressIndex,
    TraceStoreHashedStringBufferInfoIndex,
    TraceStoreBufferIndex,
    TraceStoreBufferAllocationIndex,
    TraceStoreBufferAddressIndex,
    TraceStoreBufferInfoIndex,
    TraceStoreFunctionTableIndex,
    TraceStoreFunctionTableAllocationIndex,
    TraceStoreFunctionTableAddressIndex,
    TraceStoreFunctionTableInfoIndex,
    TraceStoreFunctionTableEntryIndex,
    TraceStoreFunctionTableEntryAllocationIndex,
    TraceStoreFunctionTableEntryAddressIndex,
    TraceStoreFunctionTableEntryInfoIndex,
    TraceStorePathTableIndex,
    TraceStorePathTableAllocationIndex,
    TraceStorePathTableAddressIndex,
    TraceStorePathTableInfoIndex,
    TraceStorePathTableEntryIndex,
    TraceStorePathTableEntryAllocationIndex,
    TraceStorePathTableEntryAddressIndex,
    TraceStorePathTableEntryInfoIndex,
    TraceStoreSessionIndex,
    TraceStoreSessionAllocationIndex,
    TraceStoreSessionAddressIndex,
    TraceStoreSessionInfoIndex,
    TraceStoreFilenameStringIndex,
    TraceStoreFilenameStringAllocationIndex,
    TraceStoreFilenameStringAddressIndex,
    TraceStoreFilenameStringInfoIndex,
    TraceStoreFilenameStringBufferIndex,
    TraceStoreFilenameStringBufferAllocationIndex,
    TraceStoreFilenameStringBufferAddressIndex,
    TraceStoreFilenameStringBufferInfoIndex,
    TraceStoreDirectoryStringIndex,
    TraceStoreDirectoryStringAllocationIndex,
    TraceStoreDirectoryStringAddressIndex,
    TraceStoreDirectoryStringInfoIndex,
    TraceStoreDirectoryStringBufferIndex,
    TraceStoreDirectoryStringBufferAllocationIndex,
    TraceStoreDirectoryStringBufferAddressIndex,
    TraceStoreDirectoryStringBufferInfoIndex,
    TraceStoreStringArrayIndex,
    TraceStoreStringArrayAllocationIndex,
    TraceStoreStringArrayAddressIndex,
    TraceStoreStringArrayInfoIndex,
    TraceStoreStringTableIndex,
    TraceStoreStringTableAllocationIndex,
    TraceStoreStringTableAddressIndex,
    TraceStoreStringTableInfoIndex,
    TraceStoreInvalidIndex
} TRACE_STORE_INDEX, *PTRACE_STORE_INDEX;

#define MAX_TRACE_STORES TraceStoreStringTableInfoIndex + 1

#define FOR_EACH_TRACE_STORE(TraceStores, Index, StoreIndex)        \
    for (Index = 0, StoreIndex = 0;                                 \
         Index < TraceStores->NumberOfTraceStores;                  \
         Index++, StoreIndex += TraceStores->ElementsPerTraceStore)

typedef struct _Struct_size_bytes_(SizeOfStruct) _TRACE_STORES {
    USHORT      SizeOfStruct;
    USHORT      SizeOfAllocation;
    USHORT      NumberOfTraceStores;
    USHORT      ElementsPerTraceStore;
    TRACE_FLAGS Flags;
    ULONG       Reserved1;
    PRTL        Rtl;
    STRING      BaseDirectory;
    TRACE_STORE Stores[MAX_TRACE_STORES];
} TRACE_STORES, *PTRACE_STORES;


typedef struct _TRACE_STORE_DESCRIPTOR {
    TRACE_STORE_ID      TraceStoreId;
    TRACE_STORE_INDEX   TraceStoreIndex;
    LPCWSTR             TraceStoreName;
} TRACE_STORE_DESCRIPTOR, *PTRACE_STORE_DESCRIPTOR;

typedef struct _TRACE_STORE_FIELD_RELOC {

    //
    // Offset from the start of the struct in bytes.
    //

    ULONG Offset;

    //
    // Id of the trace store this field refers to.
    //

    TRACE_STORE_ID TraceStoreId;

} TRACE_STORE_FIELD_RELOC, *PTRACE_STORE_FIELD_RELOC;
typedef CONST TRACE_STORE_FIELD_RELOC *PCTRACE_STORE_FIELD_RELOC;
typedef CONST TRACE_STORE_FIELD_RELOC **PPCTRACE_STORE_FIELD_RELOC;

C_ASSERT(sizeof(TRACE_STORE_FIELD_RELOC) == 8);

#define LAST_TRACE_STORE_FIELD_RELOC { 0, 0 }


typedef struct _TRACE_STORE_FIELD_RELOCS {

    //
    // Id of the trace store this relocation information applies to.
    //

    TRACE_STORE_ID TraceStoreId;

    //
    // Pointer to an array of field relocation structures.
    //

    PTRACE_STORE_FIELD_RELOC Relocations;

} TRACE_STORE_FIELD_RELOCS, *PTRACE_STORE_FIELD_RELOCS;
typedef CONST TRACE_STORE_FIELD_RELOCS *PCTRACE_STORE_FIELD_RELOCS;
typedef CONST TRACE_STORE_FIELD_RELOCS **PPCTRACE_STORE_FIELD_RELOCS;

#define LAST_TRACE_STORE_FIELD_RELOCS { 0, NULL }

////////////////////////////////////////////////////////////////////////////////
// Function Type Definitions
////////////////////////////////////////////////////////////////////////////////

typedef
_Success_(return != 0)
BOOL
(INITIALIZE_TRACE_SESSION)(
    _In_                                 PRTL           Rtl,
    _Inout_bytecap_(*SizeOfTraceSession) PTRACE_SESSION TraceSession,
    _In_                                 PULONG         SizeOfTraceSession
    );
typedef INITIALIZE_TRACE_SESSION *PINITIALIZE_TRACE_SESSION;
TRACE_STORE_API INITIALIZE_TRACE_SESSION InitializeTraceSession;

typedef
_Success_(return != 0)
BOOL
(INITIALIZE_TRACE_STORES)(
    _In_        PRTL            Rtl,
    _In_        PWSTR           BaseDirectory,
    _Inout_opt_ PTRACE_STORES   TraceStores,
    _Inout_     PULONG          SizeOfTraceStores,
    _In_opt_    PULONG          InitialFileSizes,
    _In_        PTRACE_FLAGS    TraceFlags
    );
typedef INITIALIZE_TRACE_STORES *PINITIALIZE_TRACE_STORES;
TRACE_STORE_API INITIALIZE_TRACE_STORES InitializeTraceStores;

typedef
_Success_(return != 0)
BOOL
(INITIALIZE_TRACE_CONTEXT)(
    _In_     PRTL            Rtl,
    _Inout_bytecap_(*SizeOfTraceContext)
             PTRACE_CONTEXT  TraceContext,
    _In_     PULONG          SizeOfTraceContext,
    _In_     PTRACE_SESSION  TraceSession,
    _In_     PTRACE_STORES   TraceStores,
    _In_     PTP_CALLBACK_ENVIRON  ThreadpoolCallbackEnvironment,
    _In_opt_ PVOID UserData
    );
typedef INITIALIZE_TRACE_CONTEXT *PINITIALIZE_TRACE_CONTEXT;
TRACE_STORE_API INITIALIZE_TRACE_CONTEXT InitializeTraceContext;

typedef
VOID
(CLOSE_TRACE_STORES)(
    _In_ PTRACE_STORES TraceStores
    );
typedef CLOSE_TRACE_STORES *PCLOSE_TRACE_STORES;
TRACE_STORE_API CLOSE_TRACE_STORES CloseTraceStores;


////////////////////////////////////////////////////////////////////////////////
// Inline Functions
////////////////////////////////////////////////////////////////////////////////

FORCEINLINE
VOID
TraceTimeQueryPerformanceCounter(
    _In_    PTRACE_STORE_TIME   Time,
    _Out_   PLARGE_INTEGER      ElapsedPointer
    )
/*++

Routine Description:

    This routine calculates the elapsed microseconds relative to the trace
    store's start time, using the performance counter frequency information
    saved when the trace store was started.

Arguments:

    Time - Supplies a pointer to a TRACE_STORE_TIME struct that the elapsed
        time is calculated against.

    ElapsedPointer - Supplies a pointer to a variable that receives the
        elapsed time in microseconds relative to the start time of the
        TRACE_STORE_TIME struct.

Return Value:

    None.

--*/
{
    LARGE_INTEGER Elapsed;

    //
    // Query the performance counter.
    //

    QueryPerformanceCounter(&Elapsed);

    //
    // Get the elapsed time since the start of the trace.
    //

    Elapsed.QuadPart -= Time->StartTime.PerformanceCounter.QuadPart;

    //
    // Convert to microseconds.
    //

    Elapsed.QuadPart *= Time->Multiplicand.QuadPart;
    Elapsed.QuadPart /= Time->Frequency.QuadPart;

    //
    // Update relative to 1970 C UNIX time.
    //

    Elapsed.QuadPart += Time->StartTime.MicrosecondsSince1970.QuadPart;

    //
    // Update the caller's pointer.
    //

    ElapsedPointer->QuadPart = Elapsed.QuadPart;

}

//
// The following two routines are helper routines that simplify calling the
// TraceTimeQueryPerformanceCounter() routine above from a TraceContext or
// TraceStore pointer.
//

FORCEINLINE
VOID
TraceContextQueryPerformanceCounter(
    _In_    PTRACE_CONTEXT  TraceContext,
    _Out_   PLARGE_INTEGER  ElapsedPointer
    )
{
    TraceTimeQueryPerformanceCounter(&TraceContext->Time, ElapsedPointer);
}

FORCEINLINE
VOID
TraceStoreQueryPerformanceCounter(
    _In_    PTRACE_STORE    TraceStore,
    _Out_   PLARGE_INTEGER  ElapsedPointer
    )
{
    TraceTimeQueryPerformanceCounter(&TraceStore->TraceContext->Time,
                                     ElapsedPointer);
}

//
// Helper routines for determining if a trace store is a metadata trace store,
// and whether or not a trace store has varying record sizes.
//

FORCEINLINE
BOOL
IsMetadataTraceStore(_In_ PTRACE_STORE TraceStore)
{
    return TraceStore->IsMetadata;
}

FORCEINLINE
BOOL
HasVaryingRecordSizes(
    _In_    PTRACE_STORE    TraceStore
    )
/*--

Routine Description:

    This routine indicates whether or not a trace store has varying record
    sizes.  That is, whether or not AllocateRecords() has been called with
    identical record count + record size parameters every time.  This is
    determined by simply comparing the trace store's end of file to the
    current trace store allocation record's record size * number of records.
    If they don't match, the trace store has varying record sizes.

Arguments:

    TraceStore - Supplies a pointer to a TRACE_STORE struct.

Return Value:

    TRUE if the trace store has varying record sizes, FALSE if not.

--*/
{
    return (
        TraceStore->pEof->EndOfFile.QuadPart != (
            TraceStore->pAllocation->RecordSize.QuadPart *
            TraceStore->pAllocation->NumberOfRecords.QuadPart
        )
    );
}

#ifdef __cplusplus
}; // extern "C"
#endif

// vim:set ts=8 sw=4 sts=4 tw=80 expandtab                                     :
