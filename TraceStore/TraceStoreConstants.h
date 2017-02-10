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

TRACE_STORE_DATA LPCWSTR TraceStoreFileNames[];

TRACE_STORE_DATA WCHAR TraceStoreMetadataInfoSuffix[];
TRACE_STORE_DATA DWORD TraceStoreMetadataInfoSuffixLength;

TRACE_STORE_DATA WCHAR TraceStoreAllocationSuffix[];
TRACE_STORE_DATA DWORD TraceStoreAllocationSuffixLength;

TRACE_STORE_DATA WCHAR TraceStoreRelocationSuffix[];
TRACE_STORE_DATA DWORD TraceStoreRelocationSuffixLength;

TRACE_STORE_DATA WCHAR TraceStoreAddressSuffix[];
TRACE_STORE_DATA DWORD TraceStoreAddressSuffixLength;

TRACE_STORE_DATA WCHAR TraceStoreAddressRangeSuffix[];
TRACE_STORE_DATA DWORD TraceStoreAddressRangeSuffixLength;

TRACE_STORE_DATA WCHAR TraceStoreAllocationTimestampSuffix[];
TRACE_STORE_DATA DWORD TraceStoreAllocationTimestampSuffixLength;

TRACE_STORE_DATA WCHAR TraceStoreAllocationTimestampDeltaSuffix[];
TRACE_STORE_DATA DWORD TraceStoreAllocationTimestampDeltaSuffixLength;

TRACE_STORE_DATA WCHAR TraceStoreSynchronizationSuffix[];
TRACE_STORE_DATA DWORD TraceStoreSynchronizationSuffixLength;

TRACE_STORE_DATA WCHAR TraceStoreInfoSuffix[];
TRACE_STORE_DATA DWORD TraceStoreInfoSuffixLength;

TRACE_STORE_DATA PWCHAR TraceStoreMetadataSuffixes[];

TRACE_STORE_DATA USHORT LongestTraceStoreSuffixLength;
TRACE_STORE_DATA USHORT NumberOfTraceStores;
TRACE_STORE_DATA USHORT NumberOfMetadataStores;
TRACE_STORE_DATA USHORT ElementsPerTraceStore;

TRACE_STORE_DATA TRACE_STORE_STRUCTURE_SIZES TraceStoreStructureSizes;

TRACE_STORE_DATA CONST PLARGE_INTEGER InitialTraceStoreFileSizes;

TRACE_STORE_TRAITS TraceStoreTraits[];

TRACE_STORE_DATA LARGE_INTEGER MinimumMappingSize;
TRACE_STORE_DATA LARGE_INTEGER MaximumMappingSize;

TRACE_STORE_DATA USHORT InitialFreeMemoryMapsForNonStreamingReaders;
TRACE_STORE_DATA USHORT InitialFreeMemoryMapsForNonStreamingMetadataReaders;
TRACE_STORE_DATA USHORT InitialFreeMemoryMapsForNonStreamingWriters;
TRACE_STORE_DATA USHORT InitialFreeMemoryMapsForNonStreamingMetadataWriters;
TRACE_STORE_DATA USHORT InitialFreeMemoryMapsForStreamingReaders;
TRACE_STORE_DATA USHORT InitialFreeMemoryMapsForStreamingWriters;
TRACE_STORE_DATA USHORT InitialFreeMemoryMapMultiplierForFrequentAllocators;

TRACE_STORE_DATA USHORT TraceStoreMetadataInfoStructSize;
TRACE_STORE_DATA USHORT TraceStoreAllocationStructSize;
TRACE_STORE_DATA USHORT TraceStoreRelocationStructSize;
TRACE_STORE_DATA USHORT TraceStoreAddressStructSize;
TRACE_STORE_DATA USHORT TraceStoreAddressRangeStructSize;
TRACE_STORE_DATA USHORT TraceStoreBitmapStructSize;
TRACE_STORE_DATA USHORT TraceStoreSynchronizationStructSize;
TRACE_STORE_DATA USHORT TraceStoreInfoStructSize;

TRACE_STORE_DATA LARGE_INTEGER DefaultTraceStoreMappingSize;
TRACE_STORE_DATA LARGE_INTEGER DefaultTraceStoreEventMappingSize;

TRACE_STORE_DATA LARGE_INTEGER DefaultAllocationTraceStoreSize;
TRACE_STORE_DATA LARGE_INTEGER DefaultAllocationTraceStoreMappingSize;

TRACE_STORE_DATA LARGE_INTEGER DefaultRelocationTraceStoreSize;
TRACE_STORE_DATA LARGE_INTEGER DefaultRelocationTraceStoreMappingSize;

TRACE_STORE_DATA LARGE_INTEGER DefaultAddressTraceStoreSize;
TRACE_STORE_DATA LARGE_INTEGER DefaultAddressTraceStoreMappingSize;

TRACE_STORE_DATA LARGE_INTEGER DefaultAddressRangeTraceStoreSize;
TRACE_STORE_DATA LARGE_INTEGER DefaultAddressRangeTraceStoreMappingSize;

TRACE_STORE_DATA LARGE_INTEGER DefaultAllocationTimestampTraceStoreSize;
TRACE_STORE_DATA LARGE_INTEGER DefaultAllocationTimestampTraceStoreMappingSize;

TRACE_STORE_DATA LARGE_INTEGER DefaultAllocationTimestampDeltaTraceStoreSize;
TRACE_STORE_DATA LARGE_INTEGER DefaultAllocationTimestampDeltaTraceStoreMappingSize;

TRACE_STORE_DATA LARGE_INTEGER DefaultSynchronizationTraceStoreSize;
TRACE_STORE_DATA LARGE_INTEGER DefaultSynchronizationTraceStoreMappingSize;

TRACE_STORE_DATA LARGE_INTEGER DefaultMetadataInfoTraceStoreSize;
TRACE_STORE_DATA LARGE_INTEGER DefaultMetadataInfoTraceStoreMappingSize;

TRACE_STORE_DATA LARGE_INTEGER DefaultInfoTraceStoreSize;
TRACE_STORE_DATA LARGE_INTEGER DefaultInfoTraceStoreMappingSize;

TRACE_STORE_DATA TRACE_STORE_TRAITS AllocationStoreTraits;
TRACE_STORE_DATA TRACE_STORE_TRAITS RelocationStoreTraits;
TRACE_STORE_DATA TRACE_STORE_TRAITS AddressStoreTraits;
TRACE_STORE_DATA TRACE_STORE_TRAITS AddressRangeStoreTraits;
TRACE_STORE_DATA TRACE_STORE_TRAITS AllocationTimestampStoreTraits;
TRACE_STORE_DATA TRACE_STORE_TRAITS AllocationTimestampDeltaStoreTraits;
TRACE_STORE_DATA TRACE_STORE_TRAITS SynchronizationStoreTraits;
TRACE_STORE_DATA TRACE_STORE_TRAITS InfoStoreTraits;
TRACE_STORE_DATA TRACE_STORE_TRAITS MetadataInfoStoreTraits;

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

// vim:set ts=8 sw=4 sts=4 tw=80 expandtab                                     :
