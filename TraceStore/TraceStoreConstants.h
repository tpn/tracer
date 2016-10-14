/*++

Copyright (c) 2016 Trent Nelson <trent@trent.me>

Module Name:

    TraceStoreConstants.h

Abstract:

    This module defines constants used by the TraceStore component.

--*/

#pragma once

#include "TraceStore.h"

#ifdef __cplusplus
extern "C" {
#endif

static const LPCWSTR TraceStoreFileNames[] = {
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

static const WCHAR TraceStoreMetadataInfoSuffix[] = L":metadatainfo";
static const DWORD TraceStoreMetadataInfoSuffixLength = (
    sizeof(TraceStoreMetadataInfoSuffix) /
    sizeof(WCHAR)
);

static const WCHAR TraceStoreAllocationSuffix[] = L":allocation";
static const DWORD TraceStoreAllocationSuffixLength = (
    sizeof(TraceStoreAllocationSuffix) /
    sizeof(WCHAR)
);

static const WCHAR TraceStoreRelocationSuffix[] = L":relocation";
static const DWORD TraceStoreRelocationSuffixLength = (
    sizeof(TraceStoreRelocationSuffix) /
    sizeof(WCHAR)
);

static const WCHAR TraceStoreAddressSuffix[] = L":address";
static const DWORD TraceStoreAddressSuffixLength = (
    sizeof(TraceStoreAddressSuffix) /
    sizeof(WCHAR)
);

static const WCHAR TraceStoreBitmapSuffix[] = L":bitmap";
static const DWORD TraceStoreBitmapSuffixLength = (
    sizeof(TraceStoreBitmapSuffix) /
    sizeof(WCHAR)
);

static const WCHAR TraceStoreInfoSuffix[] = L":info";
static const DWORD TraceStoreInfoSuffixLength = (
    sizeof(TraceStoreInfoSuffix) /
    sizeof(WCHAR)
);

static const USHORT LongestTraceStoreSuffixLength = (
    sizeof(TraceStoreMetadataInfoSuffix) /
    sizeof(WCHAR)
);

static const USHORT NumberOfTraceStores = (
    sizeof(TraceStoreFileNames) /
    sizeof(LPCWSTR)
);

static const USHORT ElementsPerTraceStore = 7;

//
// The Event trace store gets an initial file size of 16MB, everything else
// gets 4MB.
//

static const ULONG InitialTraceStoreFileSizes[] = {
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

static const TRACE_STORE_TRAITS TraceStoreTraits[] = {

    //
    // Event
    //

    {
        0,  // VaryingRecordSize
        1,  // RecordSizeIsAlwaysPowerOf2
        1,  // MultipleRecords
        1,  // StreamingWrite
        1,  // StreamingRead
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
        0   // Unused
    }

};

static const LARGE_INTEGER MaximumMappingSize = { 1 << 31 }; // 2GB

static const SIZE_T InitialTraceContextHeapSize = (1 << 21); // 2MB
static const SIZE_T MaximumTraceContextHeapSize = (1 << 26); // 64MB

static const USHORT InitialFreeMemoryMaps = 32;

static const USHORT TraceStoreMetadataInfoStructSize = (
    sizeof(TRACE_STORE_METADATA_INFO)
);
static const USHORT TraceStoreAllocationStructSize = (
    sizeof(TRACE_STORE_ALLOCATION)
);
static const USHORT TraceStoreRelocationStructSize = (
    sizeof(TRACE_STORE_FIELD_RELOC)
);
static const USHORT TraceStoreAddressStructSize = sizeof(TRACE_STORE_ADDRESS);
static const USHORT TraceStoreBitmapStructSize = sizeof(TRACE_STORE_BITMAP);
static const USHORT TraceStoreInfoStructSize = sizeof(TRACE_STORE_INFO);

static const ULONG DefaultTraceStoreMappingSize = (1 << 21);            //  4MB
static const ULONG DefaultTraceStoreEventMappingSize = (1 << 23);       // 16MB

static const ULONG DefaultAllocationTraceStoreSize = (1 << 21);         //  4MB
static const ULONG DefaultAllocationTraceStoreMappingSize = (1 << 16);  // 64KB

static const ULONG DefaultRelocationTraceStoreSize = (1 << 16);         // 64KB
static const ULONG DefaultRelocationTraceStoreMappingSize = (1 << 16);  // 64KB

static const ULONG DefaultAddressTraceStoreSize = (1 << 21);            //  4MB
static const ULONG DefaultAddressTraceStoreMappingSize = (1 << 16);     // 64KB

static const ULONG DefaultBitmapTraceStoreSize = (1 << 16);             // 64KB
static const ULONG DefaultBitmapTraceStoreMappingSize = (1 << 16);      // 64KB

static const ULONG DefaultMetadataInfoTraceStoreSize = (
    sizeof(TRACE_STORE_METADATA_INFO)
);
static const ULONG DefaultMetadataInfoTraceStoreMappingSize = (
    sizeof(TRACE_STORE_METADATA_INFO)
);

static const ULONG DefaultInfoTraceStoreSize = (
    sizeof(TRACE_STORE_INFO)
);
static const ULONG DefaultInfoTraceStoreMappingSize = (
    sizeof(TRACE_STORE_INFO)
);

static const TRACE_STORE_TRAITS AllocationStoreTraits = {
    0,  // VaryingRecordSize
    1,  // RecordSizeIsAlwaysPowerOf2
    1,  // MultipleRecords
    1,  // StreamingWrite
    0,  // StreamingRead
    0   // Unused
};

static const TRACE_STORE_TRAITS RelocationStoreTraits = {
    0,  // VaryingRecordSize
    0,  // RecordSizeIsAlwaysPowerOf2
    0,  // MultipleRecords
    0,  // StreamingWrite
    0,  // StreamingRead
    0   // Unused
};

static const TRACE_STORE_TRAITS AddressStoreTraits = {
    0,  // VaryingRecordSize
    1,  // RecordSizeIsAlwaysPowerOf2
    1,  // MultipleRecords
    1,  // StreamingWrite
    0,  // StreamingRead
    0   // Unused
};

static const TRACE_STORE_TRAITS BitmapStoreTraits = {
    1,  // VaryingRecordSize
    0,  // RecordSizeIsAlwaysPowerOf2
    1,  // MultipleRecords
    0,  // StreamingWrite
    0,  // StreamingRead
    0   // Unused
};

static const TRACE_STORE_TRAITS InfoStoreTraits = {
    0,  // VaryingRecordSize
    1,  // RecordSizeIsAlwaysPowerOf2
    0,  // MultipleRecords
    0,  // StreamingWrite
    0,  // StreamingRead
    0   // Unused
};

static const TRACE_STORE_TRAITS MetadataInfoStoreTraits = {
    0,  // VaryingRecordSize
    1,  // RecordSizeIsAlwaysPowerOf2
    0,  // MultipleRecords
    0,  // StreamingWrite
    0,  // StreamingRead
    0   // Unused
};

#ifdef __cplusplus
}; // extern "C"
#endif

// vim:set ts=8 sw=4 sts=4 tw=80 expandtab                                     :
