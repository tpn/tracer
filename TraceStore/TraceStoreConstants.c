/*++

Copyright (c) 2016 Trent Nelson <trent@trent.me>

Module Name:

    TraceStoreConstants.c

Abstract:

    This module defines constants used by the TraceStore component.

--*/

#include "stdafx.h"

volatile BOOL PauseBeforeThreadpoolWorkEnabled = FALSE;

volatile BOOL PauseBeforeBindMetadataInfo = TRUE;
volatile BOOL PauseBeforeBindRemainingMetadata = TRUE;
volatile BOOL PauseBeforeBindTraceStore = TRUE;
volatile BOOL PauseBeforePrepareReadonlyNonStreamingMap = TRUE;
volatile BOOL PauseBeforeReadonlyNonStreamingBindComplete = TRUE;
volatile BOOL PauseBeforeRelocate = TRUE;

LPCWSTR TraceStoreFileNames[] = {
    L"TraceEvent.dat",
    L"TraceStringBuffer.dat",
    L"TraceFunctionTable.dat",
    L"TraceFunctionTableEntry.dat",
    L"TracePathTable.dat",
    L"TracePathTableEntry.dat",
    L"TraceSession.dat",
    L"TraceStringArray.dat",
    L"TraceStringTable.dat",
    L"TraceEventTraitsEx.dat",
    L"TraceWsWatchInfoEx.dat",
    L"TraceWsWorkingSetExInfo.dat",
    L"TraceStoreCCallStackTable.dat",
    L"TraceStoreCCallStackTableEntry.dat",
    L"TraceStoreCModuleTable.dat",
    L"TraceStoreCModuleTableEntry.dat",
    L"TraceStorePythonCallStackTable.dat",
    L"TraceStorePythonCallStackTableEntry.dat",
    L"TraceStorePythonModuleTable.dat",
    L"TraceStorePythonModuleTableEntry.dat",
    L"TraceStoreLineTable.dat",
    L"TraceStoreLineTableEntry.dat",
    L"TraceStoreLineStringBuffer.dat",
    L"TraceStoreCallStack.dat",
    L"TraceStorePerformance.dat",
    L"TraceStorePerformanceDelta.dat"
};

WCHAR TraceStoreMetadataInfoSuffix[] = L":MetadataInfo";
DWORD TraceStoreMetadataInfoSuffixLength = (
    sizeof(TraceStoreMetadataInfoSuffix) /
    sizeof(WCHAR)
);

WCHAR TraceStoreAllocationSuffix[] = L":Allocation";
DWORD TraceStoreAllocationSuffixLength = (
    sizeof(TraceStoreAllocationSuffix) /
    sizeof(WCHAR)
);

WCHAR TraceStoreRelocationSuffix[] = L":Relocation";
DWORD TraceStoreRelocationSuffixLength = (
    sizeof(TraceStoreRelocationSuffix) /
    sizeof(WCHAR)
);

WCHAR TraceStoreAddressSuffix[] = L":Address";
DWORD TraceStoreAddressSuffixLength = (
    sizeof(TraceStoreAddressSuffix) /
    sizeof(WCHAR)
);

WCHAR TraceStoreAddressRangeSuffix[] = L":AddressRange";
DWORD TraceStoreAddressRangeSuffixLength = (
    sizeof(TraceStoreAddressRangeSuffix) /
    sizeof(WCHAR)
);

WCHAR TraceStoreAllocationTimestampSuffix[] = L":AllocationTimestamp";
DWORD TraceStoreAllocationTimestampSuffixLength = (
    sizeof(TraceStoreAllocationTimestampSuffix) /
    sizeof(WCHAR)
);

WCHAR TraceStoreAllocationTimestampDeltaSuffix[] = (
    L":AllocationTimestampDelta"
);

DWORD TraceStoreAllocationTimestampDeltaSuffixLength = (
    sizeof(TraceStoreAllocationTimestampDeltaSuffix) /
    sizeof(WCHAR)
);

WCHAR TraceStoreInfoSuffix[] = L":Info";
DWORD TraceStoreInfoSuffixLength = (
    sizeof(TraceStoreInfoSuffix) /
    sizeof(WCHAR)
);

USHORT LongestTraceStoreSuffixLength = (
    sizeof(TraceStoreAllocationTimestampDeltaSuffixLength) /
    sizeof(WCHAR)
);

USHORT NumberOfTraceStores = (
    sizeof(TraceStoreFileNames) /
    sizeof(LPCWSTR)
);

USHORT ElementsPerTraceStore = 9;
USHORT NumberOfMetadataStores = 8;

LONGLONG InitialTraceStoreFileSizesAsLongLong[] = {
     1 << 30,   // Event
     1 << 25,   // StringBuffer
     1 << 16,   // FunctionTable
     1 << 26,   // FunctionTableEntry
     1 << 16,   // PathTable
     1 << 26,   // PathTableEntry
     1 << 18,   // TraceSession
     1 << 21,   // StringArray
     1 << 21,   // StringTable
     1 << 30,   // EventTraitsEx
     1 << 27,   // WsWatchInfoEx
     1 << 26,   // WorkingSetExInfo
     1 << 16,   // CCallStackTable
     1 << 25,   // CCallStackTableEntry
     1 << 16,   // CModuleTable
     1 << 25,   // CModuleTableEntry
     1 << 16,   // PythonCallStackTable
     1 << 25,   // PythonCallStackTableEntry
     1 << 16,   // PythonModuleTable
     1 << 25,   // PythonModuleTableEntry
     1 << 16,   // LineTable
     1 << 25,   // LineTableEntry
     1 << 25,   // LineStringBuffer
     1 << 27,   // CallStack
     1 << 25,   // Performance
     1 << 25    // PerformanceDelta
};

CONST PLARGE_INTEGER InitialTraceStoreFileSizes = (PLARGE_INTEGER)(
    InitialTraceStoreFileSizesAsLongLong
);

//
// N.B. To add a new flag to each store in vim, mark the brace boundaries with
//      ma and mb, and then run the command:
//
//          :'a,'b s:0   // Unused$:0,  // NewFlag\r        0   // Unused:
//
//      Where NewFlag is the name of the new flag to add.
//

TRACE_STORE_TRAITS TraceStoreTraits[] = {

    //
    // Event
    //

    {
        0,  // VaryingRecordSize
        1,  // RecordSizeIsAlwaysPowerOf2
        1,  // MultipleRecords
        1,  // StreamingWrite
        1,  // StreamingRead
        1,  // FrequentAllocations
        0,  // BlockingAllocations
        0,  // LinkedStore
        1,  // CoalescedAllocations
        0,  // ConcurrentAllocations
        0,  // AllowPageSpill
        0   // Unused
    },

    //
    // StringBuffer
    //

    {
        1,  // VaryingRecordSize
        0,  // RecordSizeIsAlwaysPowerOf2
        1,  // MultipleRecords
        0,  // StreamingWrite
        0,  // StreamingRead
        0,  // FrequentAllocations
        0,  // BlockingAllocations
        0,  // LinkedStore
        0,  // CoalescedAllocations
        0,  // ConcurrentAllocations
        0,  // AllowPageSpill
        0   // Unused
    },

    //
    // FunctionTable
    //

    {
        0,  // VaryingRecordSize
        1,  // RecordSizeIsAlwaysPowerOf2
        0,  // MultipleRecords
        0,  // StreamingWrite
        0,  // StreamingRead
        0,  // FrequentAllocations
        0,  // BlockingAllocations
        0,  // LinkedStore
        0,  // CoalescedAllocations
        0,  // ConcurrentAllocations
        0,  // AllowPageSpill
        0   // Unused
    },

    //
    // FunctionTableEntry
    //

    {
        0,  // VaryingRecordSize
        1,  // RecordSizeIsAlwaysPowerOf2
        1,  // MultipleRecords
        0,  // StreamingWrite
        0,  // StreamingRead
        0,  // FrequentAllocations
        0,  // BlockingAllocations
        0,  // LinkedStore
        0,  // CoalescedAllocations
        0,  // ConcurrentAllocations
        0,  // AllowPageSpill
        0   // Unused
    },

    //
    // PathTable
    //

    {
        0,  // VaryingRecordSize
        1,  // RecordSizeIsAlwaysPowerOf2
        0,  // MultipleRecords
        0,  // StreamingWrite
        0,  // StreamingRead
        0,  // FrequentAllocations
        0,  // BlockingAllocations
        0,  // LinkedStore
        0,  // CoalescedAllocations
        0,  // ConcurrentAllocations
        0,  // AllowPageSpill
        0   // Unused
    },

    //
    // PathTableEntry
    //

    {
        0,  // VaryingRecordSize
        1,  // RecordSizeIsAlwaysPowerOf2
        1,  // MultipleRecords
        0,  // StreamingWrite
        0,  // StreamingRead
        0,  // FrequentAllocations
        0,  // BlockingAllocations
        0,  // LinkedStore
        0,  // CoalescedAllocations
        0,  // ConcurrentAllocations
        0,  // AllowPageSpill
        0   // Unused
    },

    //
    // TraceSession
    //

    {
        0,  // VaryingRecordSize
        0,  // RecordSizeIsAlwaysPowerOf2
        0,  // MultipleRecords
        0,  // StreamingWrite
        0,  // StreamingRead
        0,  // FrequentAllocations
        0,  // BlockingAllocations
        0,  // LinkedStore
        0,  // CoalescedAllocations
        0,  // ConcurrentAllocations
        0,  // AllowPageSpill
        0   // Unused
    },

    //
    // StringArray
    //

    {
        1,  // VaryingRecordSize
        0,  // RecordSizeIsAlwaysPowerOf2
        1,  // MultipleRecords
        0,  // StreamingWrite
        0,  // StreamingRead
        0,  // FrequentAllocations
        0,  // BlockingAllocations
        0,  // LinkedStore
        1,  // CoalescedAllocations
        0,  // ConcurrentAllocations
        0,  // AllowPageSpill
        0   // Unused
    },

    //
    // StringTable
    //

    {
        0,  // VaryingRecordSize
        1,  // RecordSizeIsAlwaysPowerOf2
        1,  // MultipleRecords
        0,  // StreamingWrite
        0,  // StreamingRead
        0,  // FrequentAllocations
        0,  // BlockingAllocations
        0,  // LinkedStore
        1,  // CoalescedAllocations
        0,  // ConcurrentAllocations
        0,  // AllowPageSpill
        0   // Unused
    },

    //
    // EventTraitsEx
    //

    {
        0,  // VaryingRecordSize
        1,  // RecordSizeIsAlwaysPowerOf2
        1,  // MultipleRecords
        1,  // StreamingWrite
        1,  // StreamingRead
        1,  // FrequentAllocations
        1,  // BlockingAllocations
        1,  // LinkedStore
        1,  // CoalescedAllocations
        0,  // ConcurrentAllocations
        0,  // AllowPageSpill
        0   // Unused
    },

    //
    // WsWatchInfoEx
    //

    {
        0,  // VaryingRecordSize
        1,  // RecordSizeIsAlwaysPowerOf2
        1,  // MultipleRecords
        1,  // StreamingWrite
        1,  // StreamingRead
        1,  // FrequentAllocations
        0,  // BlockingAllocations
        0,  // LinkedStore
        0,  // CoalescedAllocations
        0,  // ConcurrentAllocations
        0,  // AllowPageSpill
        0   // Unused
    },

    //
    // WsWorkingSetEx
    //

    {
        0,  // VaryingRecordSize
        1,  // RecordSizeIsAlwaysPowerOf2
        1,  // MultipleRecords
        1,  // StreamingWrite
        1,  // StreamingRead
        1,  // FrequentAllocations
        1,  // BlockingAllocations
        1,  // LinkedStore (linked to WsWatchInfoEx)
        0,  // CoalescedAllocations
        0,  // ConcurrentAllocations
        0,  // AllowPageSpill
        0   // Unused
    },

    //
    // CCallStackTable
    //

    {
        0,  // VaryingRecordSize
        1,  // RecordSizeIsAlwaysPowerOf2
        0,  // MultipleRecords
        0,  // StreamingWrite
        0,  // StreamingRead
        0,  // FrequentAllocations
        0,  // BlockingAllocations
        0,  // LinkedStore
        0,  // CoalescedAllocations
        0,  // ConcurrentAllocations
        0,  // AllowPageSpill
        0   // Unused
    },

    //
    // CCallStackTableEntry
    //

    {
        0,  // VaryingRecordSize
        1,  // RecordSizeIsAlwaysPowerOf2
        1,  // MultipleRecords
        0,  // StreamingWrite
        0,  // StreamingRead
        0,  // FrequentAllocations
        0,  // BlockingAllocations
        0,  // LinkedStore
        0,  // CoalescedAllocations
        1,  // ConcurrentAllocations
        0,  // AllowPageSpill
        0   // Unused
    },

    //
    // CModuleTable
    //

    {
        0,  // VaryingRecordSize
        1,  // RecordSizeIsAlwaysPowerOf2
        0,  // MultipleRecords
        0,  // StreamingWrite
        0,  // StreamingRead
        0,  // FrequentAllocations
        0,  // BlockingAllocations
        0,  // LinkedStore
        0,  // CoalescedAllocations
        0,  // ConcurrentAllocations
        0,  // AllowPageSpill
        0   // Unused
    },

    //
    // CModuleTableEntry
    //

    {
        0,  // VaryingRecordSize
        1,  // RecordSizeIsAlwaysPowerOf2
        1,  // MultipleRecords
        0,  // StreamingWrite
        0,  // StreamingRead
        0,  // FrequentAllocations
        0,  // BlockingAllocations
        0,  // LinkedStore
        0,  // CoalescedAllocations
        0,  // ConcurrentAllocations
        0,  // AllowPageSpill
        0   // Unused
    },

    //
    // PythonCallStackTable
    //

    {
        0,  // VaryingRecordSize
        1,  // RecordSizeIsAlwaysPowerOf2
        0,  // MultipleRecords
        0,  // StreamingWrite
        0,  // StreamingRead
        0,  // FrequentAllocations
        0,  // BlockingAllocations
        0,  // LinkedStore
        0,  // CoalescedAllocations
        0,  // ConcurrentAllocations
        0,  // AllowPageSpill
        0   // Unused
    },

    //
    // PythonCallStackTableEntry
    //

    {
        0,  // VaryingRecordSize
        1,  // RecordSizeIsAlwaysPowerOf2
        1,  // MultipleRecords
        0,  // StreamingWrite
        0,  // StreamingRead
        0,  // FrequentAllocations
        0,  // BlockingAllocations
        0,  // LinkedStore
        0,  // PythonoalescedAllocations
        0,  // ConcurrentAllocations
        0,  // AllowPageSpill
        0   // Unused
    },

    //
    // PythonModuleTable
    //

    {
        0,  // VaryingRecordSize
        1,  // RecordSizeIsAlwaysPowerOf2
        0,  // MultipleRecords
        0,  // StreamingWrite
        0,  // StreamingRead
        0,  // FrequentAllocations
        0,  // BlockingAllocations
        0,  // LinkedStore
        0,  // CoalescedAllocations
        0,  // ConcurrentAllocations
        0,  // AllowPageSpill
        0   // Unused
    },

    //
    // PythonModuleTableEntry
    //

    {
        0,  // VaryingRecordSize
        1,  // RecordSizeIsAlwaysPowerOf2
        1,  // MultipleRecords
        0,  // StreamingWrite
        0,  // StreamingRead
        0,  // FrequentAllocations
        0,  // BlockingAllocations
        0,  // LinkedStore
        0,  // CoalescedAllocations
        0,  // ConcurrentAllocations
        0,  // AllowPageSpill
        0   // Unused
    },

    //
    // LineTable
    //

    {
        0,  // VaryingRecordSize
        1,  // RecordSizeIsAlwaysPowerOf2
        0,  // MultipleRecords
        0,  // StreamingWrite
        0,  // StreamingRead
        0,  // FrequentAllocations
        0,  // BlockingAllocations
        0,  // LinkedStore
        0,  // CoalescedAllocations
        0,  // ConcurrentAllocations
        0,  // AllowPageSpill
        0   // Unused
    },

    //
    // LineTableEntry
    //

    {
        0,  // VaryingRecordSize
        1,  // RecordSizeIsAlwaysPowerOf2
        1,  // MultipleRecords
        0,  // StreamingWrite
        0,  // StreamingRead
        0,  // FrequentAllocations
        1,  // BlockingAllocations
        0,  // LinkedStore
        0,  // CoalescedAllocations
        1,  // ConcurrentAllocations
        0,  // AllowPageSpill
        0   // Unused
    },

    //
    // LineStringBuffer
    //

    {
        1,  // VaryingRecordSize
        1,  // RecordSizeIsAlwaysPowerOf2
        1,  // MultipleRecords
        0,  // StreamingWrite
        0,  // StreamingRead
        0,  // FrequentAllocations
        0,  // BlockingAllocations
        0,  // LinkedStore
        0,  // CoalescedAllocations
        1,  // ConcurrentAllocations
        0,  // AllowPageSpill
        0   // Unused
    },

    //
    // CallStack
    //

    {
        0,  // VaryingRecordSize
        1,  // RecordSizeIsAlwaysPowerOf2
        1,  // MultipleRecords
        1,  // StreamingWrite
        1,  // StreamingRead
        1,  // FrequentAllocations
        0,  // BlockingAllocations
        0,  // LinkedStore
        1,  // CoalescedAllocations
        0,  // ConcurrentAllocations
        0,  // AllowPageSpill
        0   // Unused
    },

    //
    // Performance
    //

    {
        0,  // VaryingRecordSize
        1,  // RecordSizeIsAlwaysPowerOf2
        1,  // MultipleRecords
        1,  // StreamingWrite
        1,  // StreamingRead
        0,  // FrequentAllocations
        1,  // BlockingAllocations
        0,  // LinkedStore
        0,  // CoalescedAllocations
        0,  // ConcurrentAllocations
        0,  // AllowPageSpill
        0   // Unused
    },

    //
    // PerformanceDelta
    //

    {
        0,  // VaryingRecordSize
        1,  // RecordSizeIsAlwaysPowerOf2
        1,  // MultipleRecords
        1,  // StreamingWrite
        1,  // StreamingRead
        0,  // FrequentAllocations
        1,  // BlockingAllocations
        1,  // LinkedStore
        1,  // CoalescedAllocations
        0,  // ConcurrentAllocations
        0,  // AllowPageSpill
        0   // Unused
    }
};

TRACE_STORE_STRUCTURE_SIZES TraceStoreStructureSizes = {
    sizeof(TRACE_STORE),
    sizeof(TRACE_STORES),
    sizeof(TRACE_CONTEXT),
    sizeof(TRACE_STORE_START_TIME),
    sizeof(TRACE_STORE_INFO),
    sizeof(TRACE_STORE_METADATA_INFO),
    sizeof(TRACE_STORE_RELOC),
    sizeof(TRACE_STORE_ADDRESS),
    sizeof(TRACE_STORE_ADDRESS_RANGE),
    sizeof(ADDRESS_BIT_COUNTS),
};

LARGE_INTEGER MinimumMappingSize = { 1 << 16 }; // 64KB
LARGE_INTEGER MaximumMappingSize = { 1 << 31 }; // 2GB

USHORT InitialFreeMemoryMapsForNonStreamingReaders = 16;
USHORT InitialFreeMemoryMapsForNonStreamingMetadataReaders = 8;
USHORT InitialFreeMemoryMapsForNonStreamingWriters = 16;
USHORT InitialFreeMemoryMapsForNonStreamingMetadataWriters = 8;
USHORT InitialFreeMemoryMapsForStreamingReaders = 32;
USHORT InitialFreeMemoryMapsForStreamingWriters = 32;
USHORT InitialFreeMemoryMapMultiplierForFrequentAllocators = 8;

USHORT TraceStoreMetadataInfoStructSize = (
    sizeof(TRACE_STORE_METADATA_INFO)
);
USHORT TraceStoreAllocationStructSize = (
    sizeof(TRACE_STORE_ALLOCATION)
);
USHORT TraceStoreRelocationStructSize = (
    sizeof(TRACE_STORE_FIELD_RELOC)
);
USHORT TraceStoreAddressStructSize = sizeof(TRACE_STORE_ADDRESS);
USHORT TraceStoreAddressRangeStructSize = sizeof(TRACE_STORE_ADDRESS_RANGE);
USHORT TraceStoreAllocationTimestampStructSize = (
    sizeof(TRACE_STORE_ALLOCATION_TIMESTAMP)
);
USHORT TraceStoreAllocationTimestampDeltaStructSize = (
    sizeof(TRACE_STORE_ALLOCATION_TIMESTAMP_DELTA)
);
USHORT TraceStoreInfoStructSize = sizeof(TRACE_STORE_INFO);

//
// Size key:
//      1 << 16 == 64KB
//      1 << 21 == 2MB
//      1 << 22 == 4MB
//      1 << 23 == 8MB
//      1 << 25 ==
//
// N.B.: the trace store size should always be greater than or equal to the
//       mapping size.
//

LARGE_INTEGER DefaultTraceStoreMappingSize = { 1 << 26 };
LARGE_INTEGER DefaultTraceStoreEventMappingSize = { 1 << 30 };

LARGE_INTEGER DefaultAllocationTraceStoreSize = { 1 << 21 };
LARGE_INTEGER DefaultAllocationTraceStoreMappingSize = { 1 << 21 };

LARGE_INTEGER DefaultRelocationTraceStoreSize = { 1 << 16 };
LARGE_INTEGER DefaultRelocationTraceStoreMappingSize = { 1 << 16 };

LARGE_INTEGER DefaultAddressTraceStoreSize = { 1 << 21 };
LARGE_INTEGER DefaultAddressTraceStoreMappingSize = { 1 << 21 };

LARGE_INTEGER DefaultAddressRangeTraceStoreSize = { 1 << 20 };
LARGE_INTEGER DefaultAddressRangeTraceStoreMappingSize = { 1 << 20 };

LARGE_INTEGER DefaultAllocationTimestampTraceStoreSize = { 1 << 26 };
LARGE_INTEGER DefaultAllocationTimestampTraceStoreMappingSize = { 1 << 26 };

LARGE_INTEGER DefaultAllocationTimestampDeltaTraceStoreSize = { 1 << 25 };
LARGE_INTEGER DefaultAllocationTimestampDeltaTraceStoreMappingSize = {
    1 << 25
};

LARGE_INTEGER DefaultMetadataInfoTraceStoreSize = {
    sizeof(TRACE_STORE_METADATA_INFO)
};

LARGE_INTEGER DefaultMetadataInfoTraceStoreMappingSize = {
    sizeof(TRACE_STORE_METADATA_INFO)
};

LARGE_INTEGER DefaultInfoTraceStoreSize = {
    sizeof(TRACE_STORE_INFO)
};

LARGE_INTEGER DefaultInfoTraceStoreMappingSize = {
    sizeof(TRACE_STORE_INFO)
};

//
// N.B. To add a new flag to each store in vim, mark the brace boundaries with
//      ma and mb, and then run the command:
//
//          'a,'b s:0   // Unused$:0,  // NewFlag\r    0   // Unused:
//
//      Where NewFlag is the name of the new flag to add.
//

TRACE_STORE_TRAITS MetadataInfoStoreTraits = {
    0,  // VaryingRecordSize
    0,  // RecordSizeIsAlwaysPowerOf2
    0,  // MultipleRecords
    0,  // StreamingWrite
    0,  // StreamingRead
    0,  // FrequentAllocations
    0,  // BlockingAllocations
    0,  // LinkedStore
    0,  // CoalescedAllocations
    0,  // ConcurrentAllocations
    0,  // AllowPageSpill
    0   // Unused
};

TRACE_STORE_TRAITS AllocationStoreTraits = {
    0,  // VaryingRecordSize
    1,  // RecordSizeIsAlwaysPowerOf2
    1,  // MultipleRecords
    1,  // StreamingWrite
    0,  // StreamingRead
    0,  // FrequentAllocations
    0,  // BlockingAllocations
    0,  // LinkedStore
    0,  // CoalescedAllocations
    0,  // ConcurrentAllocations
    0,  // AllowPageSpill
    0   // Unused
};

TRACE_STORE_TRAITS RelocationStoreTraits = {
    1,  // VaryingRecordSize
    0,  // RecordSizeIsAlwaysPowerOf2
    1,  // MultipleRecords
    0,  // StreamingWrite
    0,  // StreamingRead
    0,  // FrequentAllocations
    0,  // BlockingAllocations
    0,  // LinkedStore
    0,  // CoalescedAllocations
    0,  // ConcurrentAllocations
    0,  // AllowPageSpill
    0   // Unused
};

TRACE_STORE_TRAITS AddressStoreTraits = {
    0,  // VaryingRecordSize
    1,  // RecordSizeIsAlwaysPowerOf2
    1,  // MultipleRecords
    1,  // StreamingWrite
    0,  // StreamingRead
    0,  // FrequentAllocations
    0,  // BlockingAllocations
    0,  // LinkedStore
    0,  // CoalescedAllocations
    0,  // ConcurrentAllocations
    0,  // AllowPageSpill
    0   // Unused
};

TRACE_STORE_TRAITS AddressRangeStoreTraits = {
    0,  // VaryingRecordSize
    1,  // RecordSizeIsAlwaysPowerOf2
    1,  // MultipleRecords
    1,  // StreamingWrite
    0,  // StreamingRead
    0,  // FrequentAllocations
    0,  // BlockingAllocations
    0,  // LinkedStore
    0,  // CoalescedAllocations
    0,  // ConcurrentAllocations
    0,  // AllowPageSpill
    0   // Unused
};

TRACE_STORE_TRAITS AllocationTimestampStoreTraits = {
    0,  // VaryingRecordSize
    1,  // RecordSizeIsAlwaysPowerOf2
    1,  // MultipleRecords
    1,  // StreamingWrite
    1,  // StreamingRead
    1,  // FrequentAllocations
    1,  // BlockingAllocations
    0,  // LinkedStore
    0,  // CoalescedAllocations
    0,  // ConcurrentAllocations
    0,  // AllowPageSpill
    0   // Unused
};

TRACE_STORE_TRAITS AllocationTimestampDeltaStoreTraits = {
    0,  // VaryingRecordSize
    1,  // RecordSizeIsAlwaysPowerOf2
    1,  // MultipleRecords
    1,  // StreamingWrite
    1,  // StreamingRead
    1,  // FrequentAllocations
    1,  // BlockingAllocations
    0,  // LinkedStore
    0,  // CoalescedAllocations
    0,  // ConcurrentAllocations
    0,  // AllowPageSpill
    0   // Unused
};

TRACE_STORE_TRAITS InfoStoreTraits = {
    0,  // VaryingRecordSize
    1,  // RecordSizeIsAlwaysPowerOf2
    0,  // MultipleRecords
    0,  // StreamingWrite
    0,  // StreamingRead
    0,  // FrequentAllocations
    0,  // BlockingAllocations
    0,  // LinkedStore
    0,  // CoalescedAllocations
    0,  // ConcurrentAllocations
    0,  // AllowPageSpill
    0   // Unused
};

// vim:set ts=8 sw=4 sts=4 tw=80 expandtab                                     :
