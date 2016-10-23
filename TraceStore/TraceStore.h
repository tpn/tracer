/*++

Copyright (c) 2016 Trent Nelson <trent@trent.me>

Module Name:

    TraceStore.h

Abstract:

    This is the main header file for the TraceStore component.  It defines
    structures and functions related to all aspects of TraceStore functionality.

--*/

#pragma once

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
#include "TraceStoreIndex.h"

#endif

#ifdef __cplusplus
extern "C" {
#endif

#define TIMESTAMP_TO_SECONDS    1000000
#define SECONDS_TO_MICROSECONDS 1000000

#define MAX_UNICODE_STRING 255
#define _OUR_MAX_PATH MAX_UNICODE_STRING

////////////////////////////////////////////////////////////////////////////////
// Structures
////////////////////////////////////////////////////////////////////////////////

typedef struct _TRACE_STORE_ALLOCATION {
    ULARGE_INTEGER  NumberOfRecords;
    LARGE_INTEGER   RecordSize;
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
        // The time the new memory map was pushed onto the "prepare memory
        // map" threadpool work queue.
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

//
// Track total number of allocations and total record size.
//

typedef struct _TRACE_STORE_TOTALS {
    ULARGE_INTEGER NumberOfAllocations;
    ULARGE_INTEGER AllocationSize;
} TRACE_STORE_TOTALS, *PTRACE_STORE_TOTALS;

//
// This structure contains bitfields that capture various traits of the trace
// store.  It is used by the trace store machinery to pick optimal behavior
// when working with a trace store.  It is used for both normal and metadata
// trace stores.
//

typedef struct _Struct_size_bytes_(sizeof(ULONG)) _TRACE_STORE_TRAITS {

    //
    // When set, indicates records of different sizes may be allocated.  When
    // clear, indicates records will always be of a fixed size.  That is, the
    // AllocateRecords() routine will always be called with the same RecordSize
    // parameter.
    //
    // Invariants:
    //
    //   - If VaryingRecordSize == TRUE:
    //          Assert MultipleRecords == TRUE
    //

    ULONG VaryingRecordSize:1;

    //
    // When set, indicates that the record size will always be a power of two.
    //

    ULONG RecordSizeIsAlwaysPowerOf2:1;

    //
    // When set, indicates the trace store may contain multiple records.  When
    // clear, indicates there will always only ever be a single record.
    //
    // Invariants:
    //
    //   - If MultipleRecords == FALSE:
    //          Assert StreamingWrite == FALSE
    //          Assert StreamingRead == FALSE
    //

    ULONG MultipleRecords:1;

    //
    // When set, indicates that the trace store should "stream" pages when
    // writing data.  This results in pages being retired once they have been
    // consumed (that is, they are unmapped from memory).  This is used for
    // trace stores that contain event data, which can result in hundreds of
    // millions of records.
    //
    // When clear, indicates memory maps should not be retired once they have
    // been filled up.  This is used for reference data that needs to stay
    // memory resident for the duration of the trace session.
    //
    // StreamingWrite usually implies StreamingRead, however, this not always
    // the case.  The :address and :allocation metadata stores are configured
    // to do streaming writes, but have streaming reads disabled.  This is
    // because the trace store memory map reader algorithm needs to have all
    // allocation and address information available in order to determine
    // how to map/relocate data.
    //

    ULONG StreamingWrite:1;

    //
    // When set, indicates that streaming should be used when reading the trace
    // store's contents.  This is identical to StreamingWrite in that pages
    // are retired (unmapped from memory) once they have been read.
    //
    // When clear, the entire contents of the trace store will be mapped
    // up-front when the trace store is bound to a trace context.
    //

    ULONG StreamingRead:1;

    //
    // Mark the remaining bits as unused.
    //

    ULONG Unused:27;

} TRACE_STORE_TRAITS, *PTRACE_STORE_TRAITS;
typedef const TRACE_STORE_TRAITS CTRACE_STORE_TRAITS, *PCTRACE_STORE_TRAITS;

C_ASSERT(sizeof(TRACE_STORE_TRAITS) == sizeof(ULONG));

typedef
_Success_(return != 0)
BOOL
(VALIDATE_TRACE_STORE_TRAITS_INVARIANTS)(
    _In_ TRACE_STORE_TRAITS Traits
    );
typedef VALIDATE_TRACE_STORE_TRAITS_INVARIANTS \
      *PVALIDATE_TRACE_STORE_TRAITS_INVARIANTS;
TRACE_STORE_API VALIDATE_TRACE_STORE_TRAITS_INVARIANTS \
                ValidateTraceStoreTraitsInvariants;

//
// This enum should be kept in sync with the TRACE_STORE_TRAITS bitflags struct.
//

typedef enum _Enum_is_bitflag_ _TRACE_STORE_TRAIT_ID {
    VaryingRecordSizeTrait              =       1,
    RecordSizeIsAlwaysPowerOf2Trait     =  1 << 1,
    MultipleRecordsTrait                =  1 << 2,
    StreamingWriteTrait                 =  1 << 3,
    StreamingReadTrait                  =  1 << 4,
    InvalidTrait                        = (1 << 4) + 1
} TRACE_STORE_TRAIT_ID, *PTRACE_STORE_TRAIT_ID;

//
// The following macros provide a convenient way to work with a trait and a
// trait's semantic inverse, e.g. instead of having to write:
//
//      Traits = TraceStore->pTraits;
//      if (!Traits.MultipleRecords) {
//
//          //
//          // Do some work specific to trace stores that only have a single
//          // record.
//          //
//
//          ...
//
//      }
//
// One can write:
//
//      if (IsSingleRecord(Traits)) {
//
//

#define HasVaryingRecords(Traits) ((Traits).VaryingRecordSize)
#define IsFixedRecordSize(Traits) (!(Traits).VaryingRecordSize)

#define IsRecordSizeAlwaysPowerOf2(Traits) ((Traits).RecordSizeIsAlwaysPowerOf2)

#define HasMultipleRecords(Traits) ((Traits).MultipleRecords)
#define IsSingleRecord(Traits) (!(Traits).MultipleRecords)

#define IsStreamingWrite(Traits) ((Traits).StreamingWrite)
#define IsStreamingRead(Traits) ((Traits).StreamingRead)
#define IsStreaming(Traits) ((Traits).StreamingRead || (Traits).StreamingWrite)

//
// TRACE_STORE_INFO is intended for storage of single-instance structs of
// various tracing-related information.  (Single-instance as in there's only
// ever one global instance of the given record, i.e. the EndOfFile.  This is
// in contrast to things like the allocation and address records, which by
// nature, will usually have multiple occurrences/allocations.)
//

typedef struct DECLSPEC_ALIGN(128) _TRACE_STORE_INFO {
    TRACE_STORE_EOF     Eof;
    TRACE_STORE_TIME    Time;
    TRACE_STORE_STATS   Stats;
    TRACE_STORE_TOTALS  Totals;

    //
    // We're at 128 bytes here and consume two cache lines.  Traits pushes us
    // over into a third cache line, and we want our size to be a power of 2
    // (because these structures are packed successively in the struct below
    // and we don't want two different metadata stores sharing a cache line,
    // nor do we want to risk crossing a page boundary), so we pad out to a
    // forth cache line.
    //

    TRACE_STORE_TRAITS  Traits;

    //
    // We're at 132 bytes.  256 - 132 = 124 bytes of padding.
    //

    CHAR Unused[124];

} TRACE_STORE_INFO, *PTRACE_STORE_INFO, **PPTRACE_STORE_INFO;

C_ASSERT(sizeof(TRACE_STORE_INFO) == 256);
C_ASSERT(FIELD_OFFSET(TRACE_STORE_INFO, Traits) == 128);

typedef struct _TRACE_STORE_METADATA_INFO {
    TRACE_STORE_INFO MetadataInfo;
    TRACE_STORE_INFO Allocation;
    TRACE_STORE_INFO Relocation;
    TRACE_STORE_INFO Address;
    TRACE_STORE_INFO Bitmap;
    TRACE_STORE_INFO Info;
} TRACE_STORE_METADATA_INFO, *PTRACE_STORE_METADATA_INFO;

C_ASSERT(FIELD_OFFSET(TRACE_STORE_METADATA_INFO, Allocation) == 256);
C_ASSERT(sizeof(TRACE_STORE_METADATA_INFO) == 1536);

//
// For trace stores that record instances of structures from a running program,
// a relocation facility is provided to adjust embedded pointers when loading a
// store in read-only mode.  This is exposed via the TRACE_STORE_FIELD_RELOC
// structure below.
//
// See PythonTraceTraceStoreRelocations.c module for an example of how an array
// of field relocations can be specified statically.
//

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

FORCEINLINE
BOOL
IsLastTraceStoreFieldRelocElement(
    _In_ PTRACE_STORE_FIELD_RELOC FieldReloc
    )
{
    return (
        FieldReloc &&
        FieldReloc->Offset == 0 &&
        FieldReloc->TraceStoreId == 0
    );
}

//
// Multiple trace store to relocation arrays can be defined by the following
// structure.  Again, see the PythonTracer module for examples.
//

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

FORCEINLINE
BOOL
IsLastTraceStoreFieldRelocsElement(
    _In_ PTRACE_STORE_FIELD_RELOCS FieldRelocs
    )
{
    return (
        FieldRelocs &&
        FieldRelocs->TraceStoreId == TraceStoreNullId &&
        FieldRelocs->Relocations == 0
    );
}

#define TRACE_STORE_FIELD_RELOC_ARRAY_ALIGNMENT 128
FORCEINLINE
BOOL
IsAlignedTraceStoreFieldReloc(
    _In_ PTRACE_STORE_FIELD_RELOC FieldReloc
    )
{
    return IsAligned(FieldReloc, TRACE_STORE_FIELD_RELOC_ARRAY_ALIGNMENT);
}

FORCEINLINE
BOOL
AssertAlignedTraceStoreFieldReloc(
    _In_ PTRACE_STORE_FIELD_RELOC FieldReloc
    )
{
    return AssertAligned(FieldReloc, TRACE_STORE_FIELD_RELOC_ARRAY_ALIGNMENT);
}

#define TRACE_STORE_FIELD_RELOCS_ARRAY_ALIGNMENT 128
FORCEINLINE
BOOL
IsAlignedTraceStoreFieldRelocs(
    _In_ PTRACE_STORE_FIELD_RELOCS FieldRelocs
    )
{
    return IsAligned(FieldRelocs, TRACE_STORE_FIELD_RELOCS_ARRAY_ALIGNMENT);
}

FORCEINLINE
BOOL
AssertAlignedTraceStoreFieldRelocs(
    _In_ PTRACE_STORE_FIELD_RELOCS FieldRelocs
    )
{
    return AssertAligned(FieldRelocs, TRACE_STORE_FIELD_RELOCS_ARRAY_ALIGNMENT);
}

//
// The following two structures, TRACE_STORE_RELOC and TRACE_STORE_RELOCS,
// are used to reflect in-memory relocation information for a given trace store.
// This is done because TRACE_STORE_FIELD_RELOC and TRACE_STORE_FIELD_RELOCS
// are optimized for ease of statically declaring in C (via FIELD_OFFSET()),
// but at runtime, we'd like to track a little more information, such as number
// of relocations instead of relying on the NULL element at the end of each
// array, etc.
//

typedef _Struct_size_bytes_(SizeOfStruct) struct _TRACE_STORE_RELOC {

    //
    // Size of the structure, in bytes.
    //

    _Field_range_(==, sizeof(struct _TRACE_STORE_RELOC)) USHORT SizeOfStruct;

    //
    // Number of fields in the relocation array.
    //

    USHORT NumberOfRelocations;

    //
    // Padding out to 8-bytes.
    //

    ULONG Unused1;

    //
    // Pointer to the array of TRACE_STORE_FIELD_RELOC structures.
    //

    PTRACE_STORE_FIELD_RELOC Relocations;

} TRACE_STORE_RELOC, *PTRACE_STORE_RELOC;


//
// Trace store free space is tracked in TRACE_STORE_BITMAP structure, which
// mirrors the 64-bit memory layout of an RTL_BITMAP structure, such that
// it can be cast to said structure and used with Rtl bitmap routines.
//

typedef struct _TRACE_STORE_BITMAP {

    ULONG SizeOfBitMap;

    //
    // Leverage the fact that we get a free ULONG slot between 'SizeOfBitMap'
    // and 'Buffer' on x64 (due to the latter needing to be 8-byte aligned)
    // and stash two additional values:
    //
    //  Granularity - Represents the number of bytes each bit in the bitmap
    //                represents.
    //
    //  Shift - How many bits to shift left to translate a bit position to the
    //          corresponding byte offset.  That is, the base 2 exponent to
    //          derive granularity.
    //
    // E.g. if Granularity were 8, Shift would be 3.
    //
    //      2 ** 3 = 8
    //      1 << 3 = 8
    //
    // (Thus, Granularity will always be a power of 2.)
    //

    USHORT Granularity;
    USHORT Shift;

    PULONG Buffer;

} TRACE_STORE_BITMAP, *PTRACE_STORE_BITMAP;

C_ASSERT(FIELD_OFFSET(TRACE_STORE_BITMAP, Buffer) == 8);

//
// End of trace store metadata-related structures.
//

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
            // When set, sets the FILE_FLAG_WRITE_THROUGH flag.  Ignored when
            // readonly.
            //

            ULONG EnableFileFlagWriteThrough:1;

            //
            // (End of flags related to CreateFile().)
            //

            //
            // When set, indicates the caller does not want to be automatically
            // added to the global trace store rundown list.  Not applicable
            // when readonly.
            //

            ULONG NoGlobalRundown:1;

            //
            // When set, the trace store will not be truncated as part of
            // close or rundown.  This is set internally by metadata stores
            // that need it and should not be modified.  Not applicable when
            // readonly.
            //

            ULONG NoTruncate:1;

        };
    };
} TRACE_FLAGS, *PTRACE_FLAGS;

typedef struct _Struct_size_bytes_(sizeof(USHORT)) _TRACE_CONTEXT_FLAGS {
    USHORT Valid:1;
    USHORT Readonly:1;
    USHORT InitializeAsync:1;
    USHORT Unused:13;
} TRACE_CONTEXT_FLAGS, *PTRACE_CONTEXT_FLAGS;

C_ASSERT(sizeof(TRACE_CONTEXT_FLAGS) == sizeof(USHORT));

typedef struct DECLSPEC_ALIGN(16) _TRACE_STORE_WORK {
    SLIST_HEADER ListHead;
    PTP_WORK ThreadpoolWork;
    HANDLE WorkCompleteEvent;
    PVOID Unused1;

    volatile ULONG NumberOfActiveItems;
    volatile ULONG NumberOfFailedItems;
    ULONG TotalNumberOfItems;
    ULONG Unused2;

    ULONGLONG Unused3;

} TRACE_STORE_WORK, *PTRACE_STORE_WORK;

C_ASSERT(FIELD_OFFSET(TRACE_STORE_WORK, ThreadpoolWork) == 16);
C_ASSERT(FIELD_OFFSET(TRACE_STORE_WORK, WorkCompleteEvent) == 24);
C_ASSERT(FIELD_OFFSET(TRACE_STORE_WORK, Unused1) == 32);
C_ASSERT(FIELD_OFFSET(TRACE_STORE_WORK, NumberOfActiveItems) == 40);
C_ASSERT(FIELD_OFFSET(TRACE_STORE_WORK, NumberOfFailedItems) == 44);
C_ASSERT(FIELD_OFFSET(TRACE_STORE_WORK, Unused3) == 56);
C_ASSERT(sizeof(TRACE_STORE_WORK) == 64);

typedef struct _Struct_size_bytes_(SizeOfStruct) _TRACE_CONTEXT {

    //
    // Size of the structure, in bytes.
    //

    _Field_range_(==, sizeof(struct _TRACE_CONTEXT)) USHORT SizeOfStruct;

    //
    // Flags.
    //

    TRACE_CONTEXT_FLAGS Flags;

    //
    // If any threadpool work items fail, the affected trace store is pushed
    // to the FailedListHead below, and the following counter is incremented
    // atomically.
    //

    volatile ULONG FailedCount;

    PRTL Rtl;
    PALLOCATOR Allocator;
    PTRACE_SESSION TraceSession;
    PTRACE_STORES TraceStores;
    PTIMER_FUNCTION TimerFunction;
    PVOID UserData;
    PTP_CALLBACK_ENVIRON ThreadpoolCallbackEnvironment;

    //
    // 64 bytes.  End of first cache line.
    //

    //
    // Second cache line.
    //

    //
    // This event is set when all trace stores have been initialized or an
    // error has occurred during loading.
    //

    HANDLE LoadingCompleteEvent;

    PTP_CLEANUP_GROUP ThreadpoolCleanupGroup;

    TRACE_STORE_WORK BindMetadataInfoStoreWork;
    TRACE_STORE_WORK BindRemainingMetadataStoresWork;
    TRACE_STORE_WORK BindTraceStoreWork;

    SLIST_HEADER FailedListHead;

    volatile ULONG ActiveWorkItems;

    //
    // This represents the number of binds for main (not metadata) trace stores.
    // When it hits zero, the LoadingCompleteEvent will be set.
    //

    volatile ULONG BindsInProgress;

    //
    // Stash Time at the end as it's large and doesn't have any alignment
    // requirements.
    //

    TRACE_STORE_TIME Time;

} TRACE_CONTEXT, *PTRACE_CONTEXT;

C_ASSERT(FIELD_OFFSET(TRACE_CONTEXT, Rtl) == 8);
C_ASSERT(FIELD_OFFSET(TRACE_CONTEXT, UserData) == 48);
C_ASSERT(FIELD_OFFSET(TRACE_CONTEXT, LoadingCompleteEvent) == 64);

typedef
VOID
(CALLBACK TRACE_STORES_LOADING_COMPLETE_CALLBACK)(
    _In_ struct _READONLY_TRACE_CONTEXT *TraceContextReadonly,
    _In_ PVOID UserContext
    );
typedef TRACE_STORES_LOADING_COMPLETE_CALLBACK \
      *PTRACE_STORES_LOADING_COMPLETE_CALLBACK;

typedef struct
_Struct_size_bytes_(sizeof(ULONG))
_READONLY_TRACE_CONTEXT_FLAGS {
    ULONG Valid:1;
    ULONG Unused:31;
} READONLY_TRACE_CONTEXT_FLAGS, *PREADONLY_TRACE_CONTEXT_FLAGS;

typedef struct _Struct_size_bytes_(SizeOfStruct) _READONLY_TRACE_CONTEXT {

    //
    // Size of the structure, in bytes.
    //

    _Field_range_(==, sizeof(struct _READONLY_TRACE_CONTEXT))
        USHORT SizeOfStruct;

    //
    // Pad out to 4 bytes.
    //

    USHORT Padding1;                                            // 4    0     4

    READONLY_TRACE_CONTEXT_FLAGS Flags;                         // 4    4     8

    PRTL Rtl;                                                   // 8    8    16
    PALLOCATOR Allocator;                                       // 8   16    24
    PUNICODE_STRING Directory;                                  // 8   24    32
    PVOID UserData;                                             // 8   32    40
    PTP_CALLBACK_ENVIRON ThreadpoolCallbackEnvironment;         // 8   40    48
    PTRACE_STORES TraceStores;                                  // 8   48    56

    //
    // This event is signaled when all trace stores have finished loading.
    //

    HANDLE LoadingCompleteEvent;                                // 8   56    64

    //
    // Second cache line.
    //

    //
    // An optional callback that will be invoked when the trace stores have
    // finished loading.
    //
                                                                // 8   64    72
    PTRACE_STORES_LOADING_COMPLETE_CALLBACK TraceStoresLoadingCompleteCallback;

    //
    // Threadpool work items.
    //

    //
    // This work item simply calls the TraceStoresLoadingCompleteCallback if
    // set when the loading has completed.
    //

    PTP_WORK TraceStoresLoadingCompleteWork;                    // 8   72    80

    //
    // Our SLIST_HEADER for trace store load work items.
    //

    SLIST_HEADER LoadTraceStoreMaps;                            // 16  80    96

    //
    // This work item is used for loading all trace stores.
    //

    PTP_WORK LoadTraceStoreWork;                                // 8   96   104

    //
    // Pad out the rest of the second cache line.
    //

    CHAR Padding2[24];                                          // 24  104  128

    //
    // Third cache line.
    //

    //
    // Make sure the volatile counter is on a separate cache line.  This will
    // be incremented to match the number of concurrent loads in progress.  As
    // each load completes, the counter will be decremented.  Once it reaches
    // zero, the AllTraceStoresAreReadyEvent will be signaled and, if set, the
    // TraceStoresLoadingCompleteCallback will be dispatched to the threadpool.
    //

    volatile ULONG ConcurrentLoadsInProgress;                   // 4 128    132

    //
    // Pad out the remaining third cache line.
    //

    CHAR Padding3[60];                                          // 60 132   192

    //
    // And reserve a forth cache line in order to ensure we're a power-of-2
    // size.
    //

    ULONGLONG Padding4[8];                                      // 64 192   256

} READONLY_TRACE_CONTEXT, *PREADONLY_TRACE_CONTEXT;

//
// Verify field alignments.  The FIELD_OFFSETS() are mainly there to provide
// an easier way to track down why the final C_ASSERT(sizeof()) fails.  There
// only field that has a strict alignment is the LoadsInProgress.
//

C_ASSERT(FIELD_OFFSET(READONLY_TRACE_CONTEXT,
                      TraceStoresLoadingCompleteCallback) == 64);
C_ASSERT(FIELD_OFFSET(READONLY_TRACE_CONTEXT, LoadTraceStoreWork) == 96);
C_ASSERT(FIELD_OFFSET(READONLY_TRACE_CONTEXT, Padding2) == 104);
C_ASSERT(FIELD_OFFSET(READONLY_TRACE_CONTEXT,
                      ConcurrentLoadsInProgress) == 128);
C_ASSERT(FIELD_OFFSET(READONLY_TRACE_CONTEXT, Padding4) == 192);
C_ASSERT(sizeof(READONLY_TRACE_CONTEXT) == 256);

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

typedef
_Check_return_
_Success_(return != 0)
PVOID
(ALLOCATE_ALIGNED_RECORDS)(
    _In_    PTRACE_CONTEXT  TraceContext,
    _In_    PTRACE_STORE    TraceStore,
    _In_    PULARGE_INTEGER RecordSize,
    _In_    PULARGE_INTEGER NumberOfRecords,
    _In_    USHORT          Alignment
    );
typedef ALLOCATE_ALIGNED_RECORDS *PALLOCATE_ALIGNED_RECORDS;

typedef
_Check_return_
_Success_(return != 0)
PVOID
(ALLOCATE_ALIGNED_OFFSET_RECORDS)(
    _In_    PTRACE_CONTEXT  TraceContext,
    _In_    PTRACE_STORE    TraceStore,
    _In_    PULARGE_INTEGER RecordSize,
    _In_    PULARGE_INTEGER NumberOfRecords,
    _In_    USHORT          Alignment,
    _In_    USHORT          Offset
    );
typedef ALLOCATE_ALIGNED_OFFSET_RECORDS *PALLOCATE_ALIGNED_OFFSET_RECORDS;

typedef
_Check_return_
_Success_(return != 0)
BOOL
(BIND_COMPLETE)(
    _In_ PTRACE_CONTEXT TraceContext,
    _In_ PTRACE_STORE TraceStore,
    _In_ PTRACE_STORE_MEMORY_MAP FirstMemoryMap
    );
typedef BIND_COMPLETE *PBIND_COMPLETE;

typedef struct _TRACE_STORE {
    SLIST_HEADER            CloseMemoryMaps;
    SLIST_HEADER            PrepareMemoryMaps;
    SLIST_HEADER            NextMemoryMaps;
    SLIST_HEADER            FreeMemoryMaps;
    SLIST_HEADER            PrefaultMemoryMaps;

    SLIST_ENTRY             ListEntry;
    PRTL                    Rtl;
    PALLOCATOR              Allocator;
    PTRACE_CONTEXT          TraceContext;
    PREADONLY_TRACE_CONTEXT ReadonlyTraceContext;
    LARGE_INTEGER           InitialSize;
    LARGE_INTEGER           ExtensionSize;
    LARGE_INTEGER           MappingSize;
    PTP_WORK                FinalizeFirstMemoryMapWork;
    PTP_WORK                PrefaultFuturePageWork;
    PTP_WORK                PrepareNextMemoryMapWork;
    PTP_WORK                CloseMemoryMapWork;
    HANDLE                  NextMemoryMapAvailableEvent;
    HANDLE                  AllMemoryMapsAreFreeEvent;

    PTRACE_STORE_MEMORY_MAP PrevMemoryMap;
    PTRACE_STORE_MEMORY_MAP MemoryMap;

    volatile ULONG  TotalNumberOfMemoryMaps;
    volatile ULONG  NumberOfActiveMemoryMaps;

    volatile LONG   MappedSequenceId;

    volatile LONG   MetadataBindsInProgress;

    TRACE_STORE_MEMORY_MAP SingleMemoryMap;

    //
    // Each trace store, when initialized, is assigned a unique sequence
    // ID, starting at 1.  The trace store and all metadata stores share
    // the same sequence ID.
    //

    ULONG SequenceId;

    //
    // NUMA node for this trace store.  This is passed to the various file
    // and memory map system calls like CreateFileMappingNuma() and
    // MapViewOfFileExNuma().
    //

    ULONG NumaNode;

    //
    // ID and Index values for the store and metadata, with the latter only
    // being set if the store is actually a metadata store.
    //

    TRACE_STORE_ID TraceStoreId;
    TRACE_STORE_METADATA_ID TraceStoreMetadataId;
    TRACE_STORE_INDEX TraceStoreIndex;

    TRACE_FLAGS TraceFlags;

    union {
        struct {
            ULONG NoRetire:1;
            ULONG NoPrefaulting:1;
            ULONG NoPreferredAddressReuse:1;
            ULONG IsReadonly:1;
            ULONG SetEndOfFileOnClose:1;
            ULONG IsMetadata:1;
            ULONG HasRelocations:1;
            ULONG NoTruncate:1;
        };
    };

    //
    // This may be set if any system calls fail.
    //

    DWORD LastError;

    DWORD CreateFileDesiredAccess;
    DWORD CreateFileCreationDisposition;
    DWORD CreateFileMappingProtectionFlags;
    DWORD CreateFileFlagsAndAttributes;
    DWORD MapViewOfFileDesiredAccess;

    HANDLE FileHandle;
    PVOID PrevAddress;

    //
    // The trace store pointers below will be valid for all trace and metadata
    // stores, even if a store is pointing to itself.
    //

    PTRACE_STORE TraceStore;

    PTRACE_STORE MetadataInfoStore;
    PTRACE_STORE AllocationStore;
    PTRACE_STORE RelocationStore;
    PTRACE_STORE AddressStore;
    PTRACE_STORE BitmapStore;
    PTRACE_STORE InfoStore;

    //
    // Allocator functions.  (N.B.: only AllocateRecords() is currently
    // implemented.)
    //

    PALLOCATE_RECORDS AllocateRecords;
    PALLOCATE_ALIGNED_RECORDS AllocateAlignedRecords;
    PALLOCATE_ALIGNED_OFFSET_RECORDS AllocateAlignedOffsetRecords;

    //
    // Bind complete callback.  This is called as the final step by BindStore()
    // if the binding was successful.  It is an internal function and should
    // not be overridden.  The metadata stores use it to finalize their state
    // once the first memory map is available.
    //

    PBIND_COMPLETE BindComplete;

    //
    // InitializeTraceStores() will point pReloc at the caller's relocation
    // information if applicable.  The trace store is responsible for writing
    // this information into the :relocation metadata store when being bound
    // to a trace context.
    //

    PTRACE_STORE_RELOC pReloc;

    //
    // Likewise for pTraits.
    //

    PTRACE_STORE_TRAITS pTraits;

    //
    // The actual underlying field relocations array is pointed to by the
    // pReloc->Relocations field; however, this pointer will not be valid
    // when re-loading persisted :relocation metadata from the metadata store.
    // As we can't write to the underlying memory map to adjust the pointer,
    // we keep a separate pointer here to point to the field relocations array.
    // This will only have a value when TraceStore->IsReadonly == TRUE and
    // relocation information has been loaded (TraceStore->HasRelocations).
    // The number of elements is governed by pReloc->NumberOfRelocations.
    //

    PTRACE_STORE_FIELD_RELOC BaseFieldRelocations;

    //
    // For trace stores, the pointers below will point to the metadata trace
    // store memory maps.  Eof, Time, Stats, Totals and Traits are convenience
    // pointers into the Info struct.  For metadata stores, Info will be pointed
    // to the relevant offset into the :metadatainfo store.
    //

    PTRACE_STORE_EOF    Eof;
    PTRACE_STORE_TIME   Time;
    PTRACE_STORE_STATS  Stats;
    PTRACE_STORE_TOTALS Totals;
    PTRACE_STORE_TRAITS Traits;
    PTRACE_STORE_INFO   Info;

    //
    // These will be NULL for metadata trace stores.  For trace stores, they
    // will point to the base address of the metadata store's memory map (i.e.
    // the metadata file content).
    //

    PTRACE_STORE_RELOC Reloc;
    PTRACE_STORE_BITMAP Bitmap;
    PTRACE_STORE_ALLOCATION Allocation;

#ifdef _TRACE_STORE_EMBED_PATH
    UNICODE_STRING Path;
    WCHAR PathBuffer[_OUR_MAX_PATH];
#endif

} TRACE_STORE, *PTRACE_STORE, **PPTRACE_STORE;

#define FOR_EACH_TRACE_STORE(TraceStores, Index, StoreIndex)        \
    for (Index = 0, StoreIndex = 0;                                 \
         Index < TraceStores->NumberOfTraceStores;                  \
         Index++, StoreIndex += TraceStores->ElementsPerTraceStore)

#define TRACE_STORE_METADATA_DECL(Name)                                  \
    PTRACE_STORE Name##Store = TraceStore + TraceStoreMetadata##Name##Id

#define TRACE_STORE_DECLS()                                         \
        PTRACE_STORE TraceStore = &TraceStores->Stores[StoreIndex]; \
        TRACE_STORE_METADATA_DECL(MetadataInfo);                    \
        TRACE_STORE_METADATA_DECL(Allocation);                      \
        TRACE_STORE_METADATA_DECL(Relocation);                      \
        TRACE_STORE_METADATA_DECL(Address);                         \
        TRACE_STORE_METADATA_DECL(Bitmap);                          \
        TRACE_STORE_METADATA_DECL(Info);

typedef struct _Struct_size_bytes_(SizeOfStruct) _TRACE_STORES {
    USHORT            SizeOfStruct;
    USHORT            SizeOfAllocation;
    USHORT            NumberOfTraceStores;
    USHORT            ElementsPerTraceStore;
    USHORT            NumberOfFieldRelocationsElements;
    USHORT            Padding1;
    ULONG             Padding2;
    TRACE_FLAGS       Flags;
    UNICODE_STRING    BaseDirectory;
    PRTL              Rtl;
    PALLOCATOR        Allocator;
    LIST_ENTRY        RundownListEntry;
    struct _TRACE_STORES_RUNDOWN *Rundown;
    TRACE_STORE_RELOC Relocations[MAX_TRACE_STORE_IDS];
    TRACE_STORE       Stores[MAX_TRACE_STORES];
} TRACE_STORES, *PTRACE_STORES;

typedef struct _TRACE_STORE_METADATA_STORES {
    PTRACE_STORE MetadataInfoStore;
    PTRACE_STORE AllocationStore;
    PTRACE_STORE RelocationStore;
    PTRACE_STORE AddressStore;
    PTRACE_STORE BitmapStore;
    PTRACE_STORE InfoStore;
} TRACE_STORE_METADATA_STORES, *PTRACE_STORE_METADATA_STORES;

////////////////////////////////////////////////////////////////////////////////
// Function Type Definitions
////////////////////////////////////////////////////////////////////////////////

//
// TraceStoreSession-related functions.
//

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

//
// TraceStores-related functions.
//

typedef
ULONG
(GET_TRACE_STORES_ALLOCATION_SIZE)(
    _In_ USHORT NumberOfTraceStores
    );
typedef GET_TRACE_STORES_ALLOCATION_SIZE *PGET_TRACE_STORES_ALLOCATION_SIZE;
TRACE_STORE_API GET_TRACE_STORES_ALLOCATION_SIZE GetTraceStoresAllocationSize;

typedef
_Success_(return != 0)
BOOL
(INITIALIZE_TRACE_STORES)(
    _In_opt_    PRTL            Rtl,
    _In_opt_    PALLOCATOR      Allocator,
    _In_opt_    PWSTR           BaseDirectory,
    _Inout_opt_ PTRACE_STORES   TraceStores,
    _Inout_     PULONG          SizeOfTraceStores,
    _In_opt_    PULONG          InitialFileSizes,
    _In_opt_    PTRACE_FLAGS    TraceFlags,
    _In_opt_    PTRACE_STORE_FIELD_RELOCS FieldRelocations
    );
typedef INITIALIZE_TRACE_STORES *PINITIALIZE_TRACE_STORES;
TRACE_STORE_API INITIALIZE_TRACE_STORES InitializeTraceStores;

typedef
_Success_(return != 0)
BOOL
(INITIALIZE_READONLY_TRACE_STORES)(
    _In_opt_    PRTL            Rtl,
    _In_opt_    PALLOCATOR      Allocator,
    _In_opt_    PWSTR           BaseDirectory,
    _Inout_opt_ PTRACE_STORES   TraceStores,
    _Inout_     PULONG          SizeOfTraceStores,
    _In_opt_    PTRACE_FLAGS    TraceFlags
    );
typedef INITIALIZE_READONLY_TRACE_STORES *PINITIALIZE_READONLY_TRACE_STORES;
TRACE_STORE_API INITIALIZE_READONLY_TRACE_STORES InitializeReadonlyTraceStores;

typedef
_Success_(return != 0)
BOOL
(CLOSE_TRACE_STORES)(
    _In_ PTRACE_STORES TraceStores
    );
typedef CLOSE_TRACE_STORES *PCLOSE_TRACE_STORES;
TRACE_STORE_API CLOSE_TRACE_STORES CloseTraceStores;

//
// TraceStoreGlobalRundown-related functions.
//

typedef
_Success_(return != 0)
_Check_return_
BOOL
(INITIALIZE_GLOBAL_TRACE_STORES_RUNDOWN)(
    VOID
    );
typedef INITIALIZE_GLOBAL_TRACE_STORES_RUNDOWN \
      *PINITIALIZE_GLOBAL_TRACE_STORES_RUNDOWN;
TRACE_STORE_API INITIALIZE_GLOBAL_TRACE_STORES_RUNDOWN \
                InitializeGlobalTraceStoresRundown;

typedef
VOID
(DESTROY_GLOBAL_TRACE_STORES_RUNDOWN)(
    VOID
    );
typedef DESTROY_GLOBAL_TRACE_STORES_RUNDOWN \
      *PDESTROY_GLOBAL_TRACE_STORES_RUNDOWN;
TRACE_STORE_API DESTROY_GLOBAL_TRACE_STORES_RUNDOWN \
                DestroyGlobalTraceStoresRundown;

typedef
VOID
(RUNDOWN_GLOBAL_TRACE_STORES)(
    VOID
    );
typedef RUNDOWN_GLOBAL_TRACE_STORES *PRUNDOWN_GLOBAL_TRACE_STORES;
TRACE_STORE_API RUNDOWN_GLOBAL_TRACE_STORES RundownGlobalTraceStores;

typedef
BOOL
(IS_GLOBAL_TRACE_STORES_RUNDOWN_ACTIVE)(
    VOID
    );
typedef IS_GLOBAL_TRACE_STORES_RUNDOWN_ACTIVE \
      *PIS_GLOBAL_TRACE_STORES_RUNDOWN_ACTIVE;
TRACE_STORE_API IS_GLOBAL_TRACE_STORES_RUNDOWN_ACTIVE \
                IsGlobalTraceStoresRundownActive;

typedef
_Success_(return != 0)
BOOL
(UPDATE_TRACER_CONFIG_WITH_TRACE_STORE_INFO)(
    _In_ PTRACER_CONFIG TracerConfig
    );
typedef UPDATE_TRACER_CONFIG_WITH_TRACE_STORE_INFO \
      *PUPDATE_TRACER_CONFIG_WITH_TRACE_STORE_INFO;

TRACE_STORE_API UPDATE_TRACER_CONFIG_WITH_TRACE_STORE_INFO \
                UpdateTracerConfigWithTraceStoreInfo;

//
// TraceStoreContext-related functions.
//

typedef
_Success_(return != 0)
BOOL
(INITIALIZE_TRACE_CONTEXT)(
    _In_opt_ PRTL                  Rtl,
    _In_opt_ PALLOCATOR            Allocator,
    _Inout_bytecap_(*SizeOfTraceContext)
             PTRACE_CONTEXT        TraceContext,
    _In_     PULONG                SizeOfTraceContext,
    _In_opt_ PTRACE_SESSION        TraceSession,
    _In_opt_ PTRACE_STORES         TraceStores,
    _In_opt_ PTP_CALLBACK_ENVIRON  ThreadpoolCallbackEnvironment,
    _In_opt_ PTRACE_CONTEXT_FLAGS  TraceContextFlags,
    _In_opt_ PVOID                 UserData
    );
typedef INITIALIZE_TRACE_CONTEXT *PINITIALIZE_TRACE_CONTEXT;
TRACE_STORE_API INITIALIZE_TRACE_CONTEXT InitializeTraceContext;

//
// TraceStoreReadonlyContext-related functions.
//

typedef
_Success_(return != 0)
BOOL
(INITIALIZE_READONLY_TRACE_CONTEXT)(
    _In_opt_ PRTL                    Rtl,
    _In_opt_ PALLOCATOR              Allocator,
    _Inout_bytecap_(*SizeOfReadonlyTraceContext)
             PREADONLY_TRACE_CONTEXT ReadonlyTraceContext,
    _In_     PULONG                  SizeOfReadonlyTraceContext,
    _In_opt_ PTRACE_STORES           TraceStores,
    _In_opt_ PTP_CALLBACK_ENVIRON    ThreadpoolCallbackEnvironment,
    _In_opt_ PVOID                   UserData
    );
typedef INITIALIZE_READONLY_TRACE_CONTEXT \
      *PINITIALIZE_READONLY_TRACE_CONTEXT;
TRACE_STORE_API INITIALIZE_READONLY_TRACE_CONTEXT \
                InitializeReadonlyTraceContext;

////////////////////////////////////////////////////////////////////////////////
// Inline Functions
////////////////////////////////////////////////////////////////////////////////

FORCEINLINE
PTRACE_STORE
TraceStoreToMetadataStore(
    _In_ PTRACE_STORE TraceStore,
    _In_ TRACE_STORE_METADATA_ID MetadataId
    )
{
    USHORT Index;
    Index = TraceStoreMetadataIdToArrayIndex(MetadataId);
    return TraceStore + Index + 1;
}

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
// whether a trace store is readonly, and whether or not a trace store has
// varying record sizes.
//

FORCEINLINE
BOOL
IsMetadataTraceStore(
    _In_ PTRACE_STORE TraceStore
    )
{
    return TraceStore->IsMetadata;
}

FORCEINLINE
BOOL
IsReadonlyTraceStore(
    _In_ PTRACE_STORE TraceStore
    )
{
    return TraceStore->IsReadonly;
}

FORCEINLINE
BOOL
TraceStoreHasRelocations(
    _In_ PTRACE_STORE TraceStore
    )
{
    return TraceStore->HasRelocations;
}

FORCEINLINE
BOOL
HasVaryingRecordSizes(
    _In_    PTRACE_STORE    TraceStore
    )
/*++

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
        TraceStore->Eof->EndOfFile.QuadPart != (
            TraceStore->Allocation->RecordSize.QuadPart *
            TraceStore->Allocation->NumberOfRecords.QuadPart
        )
    );
}

FORCEINLINE
_Success_(return != 0)
BOOL
ValidateFieldRelocationsArray(
    _In_  PTRACE_STORE_FIELD_RELOCS FieldRelocations,
    _Out_ PUSHORT NumberOfFieldRelocationsElements,
    _Out_ PUSHORT MaximumNumberOfInnerFieldRelocationElements
    )
/*++

Routine Description:

    This routine validates a TRACE_STORE_FIELD_RELOCS structure and all
    containing TRACE_STORE_FIELD_RELOC structures embedded with it.

Arguments:

    FieldRelocations - Supplies a pointer to the first element of an array of
        TRACE_STORE_FIELD_RELOCS structures.  An empty element denotes the end
        of the array.

    NumberOfFieldRelocationsElements - Supplies a pointer to an address of a
        variable that will receive the number of elements in the
        FieldRelocations array if it was successfully validated.

    MaximumNumberOfInnerFieldRelocationElements - Supplies a pointer to an
        address of a variable that will receive the maximum number of inner
        TRACE_STORE_FIELD_RELOC elements seen whilst performing validation.
        This allows a caller to subsequently cap loop indices to a more
        sensible upper bound other than MAX_USHORT.

Return Value:

    TRUE if the structure is valid, FALSE if not.

--*/
{
    BOOL Valid;
    BOOL FoundLastOuter;
    BOOL FoundLastInner;
    USHORT Outer;
    USHORT Inner;
    USHORT MaxId;
    USHORT Offset;
    USHORT MaxRelocs;
    USHORT LastOffset;
    USHORT MinimumOffset;
    USHORT MaximumOffset;
    USHORT MaxInnerElements;
    ULONG_INTEGER LongOffset;
    TRACE_STORE_ID Id;
    PTRACE_STORE_FIELD_RELOC Reloc;
    PTRACE_STORE_FIELD_RELOC FirstReloc;
    PTRACE_STORE_FIELD_RELOCS Relocs;

    //
    // Validate arguments.
    //

    if (!ARGUMENT_PRESENT(FieldRelocations)) {
        return FALSE;
    }

    if (!ARGUMENT_PRESENT(NumberOfFieldRelocationsElements)) {
        return FALSE;
    }

    if (!ARGUMENT_PRESENT(MaximumNumberOfInnerFieldRelocationElements)) {
        return FALSE;
    }

    //
    // Clear the caller's pointer up-front.
    //

    *NumberOfFieldRelocationsElements = 0;
    *MaximumNumberOfInnerFieldRelocationElements = 0;

    //
    // Verify alignment.
    //

    if (!IsAlignedTraceStoreFieldRelocs(FieldRelocations)) {
        return FALSE;
    }

    //
    // Initialize variables.
    //

    MaxId = (USHORT)MAX_TRACE_STORE_IDS;
    MaxRelocs = (USHORT)-1;
    MaxInnerElements = 0;

    FoundLastOuter = FALSE;
    FoundLastInner = FALSE;

    //
    // Enclose all of logic in a __try/__except block that catches general
    // protection faults, which can easily occur if the caller passes in
    // malformed data.
    //

    __try {

        //
        // Loop through the outer field relocations array, then loop through
        // the inner field relocation array referenced by the outer element.
        //

        for (Outer = 0; Outer < MaxId; Outer++) {
            Relocs = FieldRelocations + Outer;

            if (IsLastTraceStoreFieldRelocsElement(Relocs)) {
                FoundLastOuter = TRUE;
                break;
            }

            LastOffset = 0;
            MaximumOffset = 0;
            MinimumOffset = (USHORT)-1;

            FirstReloc = Relocs->Relocations;

            if (!IsAlignedTraceStoreFieldReloc(FirstReloc)) {
                return FALSE;
            }

            for (Inner = 0; Inner < MaxRelocs; Inner++) {
                Reloc = FirstReloc + Inner;
                LongOffset.LongPart = Reloc->Offset;
                if (LongOffset.HighPart) {
                    return FALSE;
                }
                Offset = LongOffset.LowPart;
                Id = Reloc->TraceStoreId;

                if (IsLastTraceStoreFieldRelocElement(Reloc)) {
                    FoundLastInner = TRUE;
                    break;
                }

                //
                // Ensure offsets are sequential and that the trace store ID
                // isn't larger than the known maximum ID.
                //

                Valid = (
                    Id <= MaxId && (
                        Offset == 0 ||
                        Offset >= LastOffset
                    )
                );

                if (!Valid) {
                    return FALSE;
                }

                LastOffset = Offset;
            }

            if (!FoundLastInner) {
                return FALSE;
            }

            //
            // Did this array have the most elements of all the ones we've seen
            // so far?  If so, update our counter.
            //

            if (Inner > MaxInnerElements) {
                MaxInnerElements = Inner;
            }

        }

        if (!FoundLastOuter) {
            return FALSE;
        }

        //
        // Update the caller's pointers.
        //

        *NumberOfFieldRelocationsElements = Outer;
        *MaximumNumberOfInnerFieldRelocationElements = MaxInnerElements;

        //
        // Return success.
        //

        return TRUE;

    } __except (GetExceptionCode() == EXCEPTION_ACCESS_VIOLATION ?
                EXCEPTION_EXECUTE_HANDLER :
                EXCEPTION_CONTINUE_SEARCH) {

        return FALSE;
    }
}

#ifdef __cplusplus
}; // extern "C"
#endif

// vim:set ts=8 sw=4 sts=4 tw=80 expandtab                                     :
