/*++

Copyright (c) 2016 Trent Nelson <trent@trent.me>

Module Name:

    TraceStoreConstants.c

Abstract:

    This module defines constants used by the TraceStore component.

--*/

#include "stdafx.h"

LPCWSTR TraceStoreFileNames[] = {
    L"TraceEvent.dat",
    L"TraceString.dat",
    L"TraceStringBuffer.dat",
    L"TraceHashedString.dat",
    L"TraceHashedStringBuffer.dat",
    L"TraceBuffer.dat",
    L"TraceFunctionTable.dat",
    L"TraceFunctionTableEntry.dat",
    L"TracePathTable.dat",
    L"TracePathTableEntry.dat",
    L"TraceSession.dat",
    L"TraceFilenameString.dat",
    L"TraceFilenameStringBuffer.dat",
    L"TraceDirectoryString.dat",
    L"TraceDirectoryStringBuffer.dat",
    L"TraceStringArray.dat",
    L"TraceStringTable.dat",
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

WCHAR TraceStoreBitmapSuffix[] = L":Bitmap";
DWORD TraceStoreBitmapSuffixLength = (
    sizeof(TraceStoreBitmapSuffix) /
    sizeof(WCHAR)
);

WCHAR TraceStoreInfoSuffix[] = L":Info";
DWORD TraceStoreInfoSuffixLength = (
    sizeof(TraceStoreInfoSuffix) /
    sizeof(WCHAR)
);

USHORT LongestTraceStoreSuffixLength = (
    sizeof(TraceStoreMetadataInfoSuffix) /
    sizeof(WCHAR)
);

USHORT NumberOfTraceStores = (
    sizeof(TraceStoreFileNames) /
    sizeof(LPCWSTR)
);

USHORT ElementsPerTraceStore = 7;
USHORT NumberOfMetadataStores = 6;

//
// The Event trace store gets an initial file size of 16MB, everything else
// gets 4MB.
//

ULONG InitialTraceStoreFileSizes[] = {
    10 << 23,   // Event
    10 << 20,   // String
    10 << 20,   // StringBuffer
    10 << 20,   // HashedString
    10 << 20,   // HashedStringBuffer
    10 << 20,   // Buffer
    10 << 20,   // FunctionTable
    10 << 20,   // FunctionTableEntry
    10 << 20,   // PathTable
    10 << 20,   // PathTableEntry
    10 << 20,   // TraceSession
    10 << 20,   // TraceFilenameString
    10 << 20,   // TraceFilenameStringBuffer
    10 << 20,   // TraceDirectoryString
    10 << 20,   // TraceDirectoryStringBuffer
    10 << 20,   // StringArray
    10 << 20    // StringTable
};


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
        0   // Unused
    },

    //
    // String
    //

    {
        0,  // VaryingRecordSize
        0,  // RecordSizeIsAlwaysPowerOf2
        1,  // MultipleRecords
        0,  // StreamingWrite
        0,  // StreamingRead
        0,  // FrequentAllocations
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
        0   // Unused
    },

    //
    // HashedString
    //

    {
        0,  // VaryingRecordSize
        0,  // RecordSizeIsAlwaysPowerOf2
        1,  // MultipleRecords
        0,  // StreamingWrite
        0,  // StreamingRead
        0,  // FrequentAllocations
        0   // Unused
    },

    //
    // HashedStringBuffer
    //

    {
        1,  // VaryingRecordSize
        0,  // RecordSizeIsAlwaysPowerOf2
        1,  // MultipleRecords
        0,  // StreamingWrite
        0,  // StreamingRead
        0,  // FrequentAllocations
        0   // Unused
    },

    //
    // Buffer
    //

    {
        1,  // VaryingRecordSize
        0,  // RecordSizeIsAlwaysPowerOf2
        1,  // MultipleRecords
        0,  // StreamingWrite
        0,  // StreamingRead
        0,  // FrequentAllocations
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
        0   // Unused
    },

    //
    // TraceFilenameString
    //

    {
        0,  // VaryingRecordSize
        0,  // RecordSizeIsAlwaysPowerOf2
        1,  // MultipleRecords
        0,  // StreamingWrite
        0,  // StreamingRead
        0,  // FrequentAllocations
        0   // Unused
    },

    //
    // TraceFilenameStringBuffer
    //

    {
        1,  // VaryingRecordSize
        0,  // RecordSizeIsAlwaysPowerOf2
        1,  // MultipleRecords
        0,  // StreamingWrite
        0,  // StreamingRead
        0,  // FrequentAllocations
        0   // Unused
    },

    //
    // TraceDirectoryString
    //

    {
        0,  // VaryingRecordSize
        0,  // RecordSizeIsAlwaysPowerOf2
        1,  // MultipleRecords
        0,  // StreamingWrite
        0,  // StreamingRead
        0,  // FrequentAllocations
        0   // Unused
    },

    //
    // TraceDirectoryStringBuffer
    //

    {
        1,  // VaryingRecordSize
        0,  // RecordSizeIsAlwaysPowerOf2
        1,  // MultipleRecords
        0,  // StreamingWrite
        0,  // StreamingRead
        0,  // FrequentAllocations
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
        0   // Unused
    },

    //
    // StringTable
    //

    {
        0,  // VaryingRecordSize
        0,  // RecordSizeIsAlwaysPowerOf2
        1,  // MultipleRecords
        0,  // StreamingWrite
        0,  // StreamingRead
        0,  // FrequentAllocations
        0   // Unused
    }
};

LARGE_INTEGER MaximumMappingSize = { 1 << 31 }; // 2GB

USHORT InitialFreeMemoryMapsForNonStreamingReaders = 128;
USHORT InitialFreeMemoryMapsForNonStreamingMetadataReaders = 64;
USHORT InitialFreeMemoryMapsForNonStreamingWriters = 256;
USHORT InitialFreeMemoryMapsForNonStreamingMetadataWriters = 128;
USHORT InitialFreeMemoryMapsForStreamingReaders = 64;
USHORT InitialFreeMemoryMapsForStreamingWriters = 64;
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
USHORT TraceStoreBitmapStructSize = sizeof(TRACE_STORE_BITMAP);
USHORT TraceStoreInfoStructSize = sizeof(TRACE_STORE_INFO);

ULONG DefaultTraceStoreMappingSize = (1 << 21);             //  4MB
ULONG DefaultTraceStoreEventMappingSize = (1 << 23);        // 16MB

ULONG DefaultAllocationTraceStoreSize = (1 << 21);          //  4MB
ULONG DefaultAllocationTraceStoreMappingSize = (1 << 16);   // 64KB

ULONG DefaultRelocationTraceStoreSize = (1 << 16);          // 64KB
ULONG DefaultRelocationTraceStoreMappingSize = (1 << 16);   // 64KB

ULONG DefaultAddressTraceStoreSize = (1 << 21);             //  4MB
ULONG DefaultAddressTraceStoreMappingSize = (1 << 16);      // 64KB

ULONG DefaultAddressRangeTraceStoreSize = (1 << 16);        // 64KB
ULONG DefaultAddressRangeTraceStoreMappingSize = (1 << 16); // 64KB

ULONG DefaultBitmapTraceStoreSize = (1 << 16);              // 64KB
ULONG DefaultBitmapTraceStoreMappingSize = (1 << 16);       // 64KB

ULONG DefaultMetadataInfoTraceStoreSize = (
    sizeof(TRACE_STORE_METADATA_INFO)
);
ULONG DefaultMetadataInfoTraceStoreMappingSize = (
    sizeof(TRACE_STORE_METADATA_INFO)
);

ULONG DefaultInfoTraceStoreSize = (
    sizeof(TRACE_STORE_INFO)
);
ULONG DefaultInfoTraceStoreMappingSize = (
    sizeof(TRACE_STORE_INFO)
);

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
    0   // Unused
};

TRACE_STORE_TRAITS AllocationStoreTraits = {
    0,  // VaryingRecordSize
    1,  // RecordSizeIsAlwaysPowerOf2
    1,  // MultipleRecords
    1,  // StreamingWrite
    0,  // StreamingRead
    0,  // FrequentAllocations
    0   // Unused
};

TRACE_STORE_TRAITS RelocationStoreTraits = {
    1,  // VaryingRecordSize
    0,  // RecordSizeIsAlwaysPowerOf2
    1,  // MultipleRecords
    0,  // StreamingWrite
    0,  // StreamingRead
    0,  // FrequentAllocations
    0   // Unused
};

TRACE_STORE_TRAITS AddressStoreTraits = {
    0,  // VaryingRecordSize
    1,  // RecordSizeIsAlwaysPowerOf2
    1,  // MultipleRecords
    1,  // StreamingWrite
    0,  // StreamingRead
    0,  // FrequentAllocations
    0   // Unused
};

TRACE_STORE_TRAITS AddressRangeStoreTraits = {
    0,  // VaryingRecordSize
    1,  // RecordSizeIsAlwaysPowerOf2
    1,  // MultipleRecords
    1,  // StreamingWrite
    0,  // StreamingRead
    0,  // FrequentAllocations
    0   // Unused
};

TRACE_STORE_TRAITS BitmapStoreTraits = {
    1,  // VaryingRecordSize
    0,  // RecordSizeIsAlwaysPowerOf2
    1,  // MultipleRecords
    0,  // StreamingWrite
    0,  // StreamingRead
    0,  // FrequentAllocations
    0   // Unused
};

TRACE_STORE_TRAITS InfoStoreTraits = {
    0,  // VaryingRecordSize
    1,  // RecordSizeIsAlwaysPowerOf2
    0,  // MultipleRecords
    0,  // StreamingWrite
    0,  // StreamingRead
    0,  // FrequentAllocations
    0   // Unused
};

// vim:set ts=8 sw=4 sts=4 tw=80 expandtab                                     :
