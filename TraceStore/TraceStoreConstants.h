/*++

Copyright (c) 2016 Trent Nelson <trent@trent.me>

Module Name:

    TraceStoreConstants.h

Abstract:

    This module defines constants used by the TraceStore component.

--*/

#pragma once

#ifdef __cpplus
extern "C" {
#endif

#include "stdafx.h"

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
};

static const WCHAR TraceStoreAllocationSuffix[] = L":allocation";
static const DWORD TraceStoreAllocationSuffixLength = (
    sizeof(TraceStoreAllocationSuffix) /
    sizeof(WCHAR)
);

static const WCHAR TraceStoreAddressSuffix[] = L":address";
static const DWORD TraceStoreAddressSuffixLength = (
    sizeof(TraceStoreAddressSuffix) /
    sizeof(WCHAR)
);

static const WCHAR TraceStoreInfoSuffix[] = L":info";
static const DWORD TraceStoreInfoSuffixLength = (
    sizeof(TraceStoreInfoSuffix) /
    sizeof(WCHAR)
);

static const USHORT LongestTraceStoreSuffixLength = (
    sizeof(TraceStoreAllocationSuffix) /
    sizeof(WCHAR)
);

static const USHORT NumberOfTraceStores = (
    sizeof(TraceStoreFileNames) /
    sizeof(LPCWSTR)
);

static const USHORT ElementsPerTraceStore = 4;

//
// The Event trace store gets an initial file size of 80MB,
// everything else gets 10MB.
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
    10 << 20    // TraceDirectoryStringBuffer
};

static const LARGE_INTEGER MaximumMappingSize = { 1 << 31 }; // 2GB

static const SIZE_T InitialTraceContextHeapSize = (1 << 21); // 2MB
static const SIZE_T MaximumTraceContextHeapSize = (1 << 26); // 64MB

static const USHORT InitialFreeMemoryMaps = 32;

static const USHORT TraceStoreAllocationStructSize = (
    sizeof(TRACE_STORE_ALLOCATION)
);
static const USHORT TraceStoreAddressStructSize = sizeof(TRACE_STORE_ADDRESS);
static const USHORT TraceStoreInfoStructSize = sizeof(TRACE_STORE_INFO);

static const ULONG DefaultTraceStoreMappingSize = (1 << 21); // 2MB
static const ULONG DefaultTraceStoreEventMappingSize = (1 << 23); // 8MB

static const ULONG DefaultAllocationTraceStoreSize = (1 << 21); // 2MB
static const ULONG DefaultAllocationTraceStoreMappingSize = (1 << 16); // 64KB

static const ULONG DefaultAddressTraceStoreSize = (1 << 21); // 2MB
static const ULONG DefaultAddressTraceStoreMappingSize = (1 << 16); // 64KB

static const ULONG DefaultInfoTraceStoreSize = (1 << 16); // 64KB
static const ULONG DefaultInfoTraceStoreMappingSize = (1 << 16); // 64KB

#ifdef __cpp
} // extern "C"
#endif

// vim:set ts=8 sw=4 sts=4 tw=80 expandtab                                     :
