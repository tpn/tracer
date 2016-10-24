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

TRACE_STORE_DATA WCHAR TraceStoreBitmapSuffix[];
TRACE_STORE_DATA DWORD TraceStoreBitmapSuffixLength;

TRACE_STORE_DATA WCHAR TraceStoreInfoSuffix[];
TRACE_STORE_DATA DWORD TraceStoreInfoSuffixLength;

TRACE_STORE_DATA USHORT LongestTraceStoreSuffixLength;
TRACE_STORE_DATA USHORT NumberOfTraceStores;
TRACE_STORE_DATA USHORT ElementsPerTraceStore;

TRACE_STORE_DATA ULONG InitialTraceStoreFileSizes[];

TRACE_STORE_TRAITS TraceStoreTraits[];

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
TRACE_STORE_DATA USHORT TraceStoreBitmapStructSize;
TRACE_STORE_DATA USHORT TraceStoreInfoStructSize;

TRACE_STORE_DATA ULONG DefaultTraceStoreMappingSize;
TRACE_STORE_DATA ULONG DefaultTraceStoreEventMappingSize;

TRACE_STORE_DATA ULONG DefaultAllocationTraceStoreSize;
TRACE_STORE_DATA ULONG DefaultAllocationTraceStoreMappingSize;

TRACE_STORE_DATA ULONG DefaultRelocationTraceStoreSize;
TRACE_STORE_DATA ULONG DefaultRelocationTraceStoreMappingSize;

TRACE_STORE_DATA ULONG DefaultAddressTraceStoreSize;
TRACE_STORE_DATA ULONG DefaultAddressTraceStoreMappingSize;

TRACE_STORE_DATA ULONG DefaultBitmapTraceStoreSize;
TRACE_STORE_DATA ULONG DefaultBitmapTraceStoreMappingSize;

TRACE_STORE_DATA ULONG DefaultMetadataInfoTraceStoreSize;
TRACE_STORE_DATA ULONG DefaultMetadataInfoTraceStoreMappingSize;

TRACE_STORE_DATA ULONG DefaultInfoTraceStoreSize;
TRACE_STORE_DATA ULONG DefaultInfoTraceStoreMappingSize;

TRACE_STORE_DATA TRACE_STORE_TRAITS AllocationStoreTraits;
TRACE_STORE_DATA TRACE_STORE_TRAITS RelocationStoreTraits;
TRACE_STORE_DATA TRACE_STORE_TRAITS AddressStoreTraits;
TRACE_STORE_DATA TRACE_STORE_TRAITS BitmapStoreTraits;
TRACE_STORE_DATA TRACE_STORE_TRAITS InfoStoreTraits;
TRACE_STORE_DATA TRACE_STORE_TRAITS MetadataInfoStoreTraits;

#ifdef __cplusplus
}; // extern "C"
#endif

// vim:set ts=8 sw=4 sts=4 tw=80 expandtab                                     :
