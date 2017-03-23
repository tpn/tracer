/*++

Copyright (c) 2016 Trent Nelson <trent@trent.me>

Module Name:

    TraceStoreConstants.h

Abstract:

    This module declares constants used by the TraceStore component.

--*/

#pragma once

#ifdef __cplusplus
extern "C" {
#endif

#include "stdafx.h"

TRACE_STORE_DATA CONST LPCWSTR TraceStoreFileNames[];

TRACE_STORE_DATA CONST WCHAR TraceStoreMetadataInfoSuffix[];
TRACE_STORE_DATA CONST DWORD TraceStoreMetadataInfoSuffixLength;

TRACE_STORE_DATA CONST WCHAR TraceStoreAllocationSuffix[];
TRACE_STORE_DATA CONST DWORD TraceStoreAllocationSuffixLength;

TRACE_STORE_DATA CONST WCHAR TraceStoreRelocationSuffix[];
TRACE_STORE_DATA CONST DWORD TraceStoreRelocationSuffixLength;

TRACE_STORE_DATA CONST WCHAR TraceStoreAddressSuffix[];
TRACE_STORE_DATA CONST DWORD TraceStoreAddressSuffixLength;

TRACE_STORE_DATA CONST WCHAR TraceStoreAddressRangeSuffix[];
TRACE_STORE_DATA CONST DWORD TraceStoreAddressRangeSuffixLength;

TRACE_STORE_DATA CONST WCHAR TraceStoreAllocationTimestampSuffix[];
TRACE_STORE_DATA CONST DWORD TraceStoreAllocationTimestampSuffixLength;

TRACE_STORE_DATA CONST WCHAR TraceStoreAllocationTimestampDeltaSuffix[];
TRACE_STORE_DATA CONST DWORD TraceStoreAllocationTimestampDeltaSuffixLength;

TRACE_STORE_DATA CONST WCHAR TraceStoreSynchronizationSuffix[];
TRACE_STORE_DATA CONST DWORD TraceStoreSynchronizationSuffixLength;

TRACE_STORE_DATA CONST WCHAR TraceStoreInfoSuffix[];
TRACE_STORE_DATA CONST DWORD TraceStoreInfoSuffixLength;

TRACE_STORE_DATA CONST LPCWSTR TraceStoreMetadataSuffixes[];

TRACE_STORE_DATA CONST USHORT LongestTraceStoreSuffixLength;
TRACE_STORE_DATA CONST USHORT NumberOfTraceStores;
TRACE_STORE_DATA CONST USHORT NumberOfMetadataStores;
TRACE_STORE_DATA CONST USHORT ElementsPerTraceStore;

TRACE_STORE_DATA CONST TRACE_STORE_STRUCTURE_SIZES TraceStoreStructureSizes;

TRACE_STORE_DATA CONST PLARGE_INTEGER InitialTraceStoreFileSizes;

CONST TRACE_STORE_TRAITS TraceStoreTraits[];

TRACE_STORE_DATA CONST LARGE_INTEGER MinimumMappingSize;
TRACE_STORE_DATA CONST LARGE_INTEGER MaximumMappingSize;

TRACE_STORE_DATA CONST USHORT InitialFreeMemoryMapsForNonStreamingReaders;
TRACE_STORE_DATA CONST USHORT InitialFreeMemoryMapsForNonStreamingMetadataReaders;
TRACE_STORE_DATA CONST USHORT InitialFreeMemoryMapsForNonStreamingWriters;
TRACE_STORE_DATA CONST USHORT InitialFreeMemoryMapsForNonStreamingMetadataWriters;
TRACE_STORE_DATA CONST USHORT InitialFreeMemoryMapsForStreamingReaders;
TRACE_STORE_DATA CONST USHORT InitialFreeMemoryMapsForStreamingWriters;
TRACE_STORE_DATA CONST USHORT InitialFreeMemoryMapMultiplierForFrequentAllocators;

TRACE_STORE_DATA CONST USHORT TraceStoreMetadataInfoStructSize;
TRACE_STORE_DATA CONST USHORT TraceStoreAllocationStructSize;
TRACE_STORE_DATA CONST USHORT TraceStoreRelocationStructSize;
TRACE_STORE_DATA CONST USHORT TraceStoreAddressStructSize;
TRACE_STORE_DATA CONST USHORT TraceStoreAddressRangeStructSize;
TRACE_STORE_DATA CONST USHORT TraceStoreBitmapStructSize;
TRACE_STORE_DATA CONST USHORT TraceStoreSynchronizationStructSize;
TRACE_STORE_DATA CONST USHORT TraceStoreInfoStructSize;

TRACE_STORE_DATA CONST LARGE_INTEGER DefaultTraceStoreMappingSize;
TRACE_STORE_DATA CONST LARGE_INTEGER DefaultTraceStoreEventMappingSize;

TRACE_STORE_DATA CONST LARGE_INTEGER DefaultAllocationTraceStoreSize;
TRACE_STORE_DATA CONST LARGE_INTEGER DefaultAllocationTraceStoreMappingSize;

TRACE_STORE_DATA CONST LARGE_INTEGER DefaultRelocationTraceStoreSize;
TRACE_STORE_DATA CONST LARGE_INTEGER DefaultRelocationTraceStoreMappingSize;

TRACE_STORE_DATA CONST LARGE_INTEGER DefaultAddressTraceStoreSize;
TRACE_STORE_DATA CONST LARGE_INTEGER DefaultAddressTraceStoreMappingSize;

TRACE_STORE_DATA CONST LARGE_INTEGER DefaultAddressRangeTraceStoreSize;
TRACE_STORE_DATA CONST LARGE_INTEGER DefaultAddressRangeTraceStoreMappingSize;

TRACE_STORE_DATA CONST LARGE_INTEGER DefaultAllocationTimestampTraceStoreSize;
TRACE_STORE_DATA CONST LARGE_INTEGER DefaultAllocationTimestampTraceStoreMappingSize;

TRACE_STORE_DATA CONST LARGE_INTEGER DefaultAllocationTimestampDeltaTraceStoreSize;
TRACE_STORE_DATA CONST LARGE_INTEGER DefaultAllocationTimestampDeltaTraceStoreMappingSize;

TRACE_STORE_DATA CONST LARGE_INTEGER DefaultSynchronizationTraceStoreSize;
TRACE_STORE_DATA CONST LARGE_INTEGER DefaultSynchronizationTraceStoreMappingSize;

TRACE_STORE_DATA CONST LARGE_INTEGER DefaultMetadataInfoTraceStoreSize;
TRACE_STORE_DATA CONST LARGE_INTEGER DefaultMetadataInfoTraceStoreMappingSize;

TRACE_STORE_DATA CONST LARGE_INTEGER DefaultInfoTraceStoreSize;
TRACE_STORE_DATA CONST LARGE_INTEGER DefaultInfoTraceStoreMappingSize;

TRACE_STORE_DATA CONST TRACE_STORE_TRAITS AllocationStoreTraits;
TRACE_STORE_DATA CONST TRACE_STORE_TRAITS RelocationStoreTraits;
TRACE_STORE_DATA CONST TRACE_STORE_TRAITS AddressStoreTraits;
TRACE_STORE_DATA CONST TRACE_STORE_TRAITS AddressRangeStoreTraits;
TRACE_STORE_DATA CONST TRACE_STORE_TRAITS AllocationTimestampStoreTraits;
TRACE_STORE_DATA CONST TRACE_STORE_TRAITS AllocationTimestampDeltaStoreTraits;
TRACE_STORE_DATA CONST TRACE_STORE_TRAITS SynchronizationStoreTraits;
TRACE_STORE_DATA CONST TRACE_STORE_TRAITS InfoStoreTraits;
TRACE_STORE_DATA CONST TRACE_STORE_TRAITS MetadataInfoStoreTraits;

TRACE_STORE_DATA volatile BOOL PauseBeforeThreadpoolWorkEnabled;

TRACE_STORE_DATA volatile BOOL PauseBeforeBindMetadataInfo;
TRACE_STORE_DATA volatile BOOL PauseBeforeBindRemainingMetadata;
TRACE_STORE_DATA volatile BOOL PauseBeforeBindTraceStore;
TRACE_STORE_DATA volatile BOOL PauseBeforePrepareReadonlyNonStreamingMap;
TRACE_STORE_DATA volatile BOOL PauseBeforeReadonlyNonStreamingBindComplete;
TRACE_STORE_DATA volatile BOOL PauseBeforeRelocate;

TRACE_STORE_DATA CONST TRACE_STORE_ID TraceStoreIds[];

#ifdef __cplusplus
}; // extern "C"
#endif

// vim:set ts=8 sw=4 sts=4 tw=80 expandtab nowrap                              :
