/*++

Copyright (c) 2016 Trent Nelson <trent@trent.me>

Module Name:

    TraceStoreIndex.h

Abstract:

    This module defines trace store enumerations.  Each top-level trace store
    (i.e. non-metadata trace stores) is allocated an ID, which is captured in
    the TRACE_STORE_ID structure.  TRACE_STORE_INDEX, on the other hand, is an
    enumeration that supplies the index offset of a trace store -- normal or
    metadata, within the TRACE_STORES struct.

    Thus, if one wanted to obtain a pointer to the TRACE_STORE structure for
    the event store, this would be done as follows:

        PTRACE_STORE EventStore;
        EventStore = TraceStores->Stores[TraceStoreEventIndex];

--*/

#pragma once

#ifdef __cplusplus
extern "C" {
#endif

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

#define MAX_TRACE_STORE_IDS TraceStoreInvalidId

FORCEINLINE
TRACE_STORE_ID
ArrayIndexToTraceStoreId(
    _In_ USHORT Index
    )
{
    return (TRACE_STORE_ID)Index + 1;
}

//
// N.B. To add a new index for each store to the following enum in vim, mark
//      the enum boundaries with ma and mb, then run the command:
//
//          :'a,'b s/TraceStore\(.\+\)AllocationIndex,$/TraceStore\1AllocationIndex,\r    TraceStore\1RelocationIndex,/
//
//      Replacing 'RelocationIndex' with the name of the new index you want
//      to add.
//

typedef enum _TRACE_STORE_INDEX {
    TraceStoreEventIndex = 0,
    TraceStoreEventAllocationIndex,
    TraceStoreEventRelocationIndex,
    TraceStoreEventAddressIndex,
    TraceStoreEventBitmapIndex,
    TraceStoreEventInfoIndex,
    TraceStoreStringIndex,
    TraceStoreStringAllocationIndex,
    TraceStoreStringRelocationIndex,
    TraceStoreStringAddressIndex,
    TraceStoreStringBitmapIndex,
    TraceStoreStringInfoIndex,
    TraceStoreStringBufferIndex,
    TraceStoreStringBufferAllocationIndex,
    TraceStoreStringBufferRelocationIndex,
    TraceStoreStringBufferAddressIndex,
    TraceStoreStringBufferBitmapIndex,
    TraceStoreStringBufferInfoIndex,
    TraceStoreHashedStringIndex,
    TraceStoreHashedStringAllocationIndex,
    TraceStoreHashedStringRelocationIndex,
    TraceStoreHashedStringAddressIndex,
    TraceStoreHashedStringBitmapIndex,
    TraceStoreHashedStringInfoIndex,
    TraceStoreHashedStringBufferIndex,
    TraceStoreHashedStringBufferAllocationIndex,
    TraceStoreHashedStringBufferRelocationIndex,
    TraceStoreHashedStringBufferAddressIndex,
    TraceStoreHashedStringBufferBitmapIndex,
    TraceStoreHashedStringBufferInfoIndex,
    TraceStoreBufferIndex,
    TraceStoreBufferAllocationIndex,
    TraceStoreBufferRelocationIndex,
    TraceStoreBufferAddressIndex,
    TraceStoreBufferBitmapIndex,
    TraceStoreBufferInfoIndex,
    TraceStoreFunctionTableIndex,
    TraceStoreFunctionTableAllocationIndex,
    TraceStoreFunctionTableRelocationIndex,
    TraceStoreFunctionTableAddressIndex,
    TraceStoreFunctionTableBitmapIndex,
    TraceStoreFunctionTableInfoIndex,
    TraceStoreFunctionTableEntryIndex,
    TraceStoreFunctionTableEntryAllocationIndex,
    TraceStoreFunctionTableEntryRelocationIndex,
    TraceStoreFunctionTableEntryAddressIndex,
    TraceStoreFunctionTableEntryBitmapIndex,
    TraceStoreFunctionTableEntryInfoIndex,
    TraceStorePathTableIndex,
    TraceStorePathTableAllocationIndex,
    TraceStorePathTableRelocationIndex,
    TraceStorePathTableAddressIndex,
    TraceStorePathTableBitmapIndex,
    TraceStorePathTableInfoIndex,
    TraceStorePathTableEntryIndex,
    TraceStorePathTableEntryAllocationIndex,
    TraceStorePathTableEntryRelocationIndex,
    TraceStorePathTableEntryAddressIndex,
    TraceStorePathTableEntryBitmapIndex,
    TraceStorePathTableEntryInfoIndex,
    TraceStoreSessionIndex,
    TraceStoreSessionAllocationIndex,
    TraceStoreSessionRelocationIndex,
    TraceStoreSessionAddressIndex,
    TraceStoreSessionBitmapIndex,
    TraceStoreSessionInfoIndex,
    TraceStoreFilenameStringIndex,
    TraceStoreFilenameStringAllocationIndex,
    TraceStoreFilenameStringRelocationIndex,
    TraceStoreFilenameStringAddressIndex,
    TraceStoreFilenameStringBitmapIndex,
    TraceStoreFilenameStringInfoIndex,
    TraceStoreFilenameStringBufferIndex,
    TraceStoreFilenameStringBufferAllocationIndex,
    TraceStoreFilenameStringBufferRelocationIndex,
    TraceStoreFilenameStringBufferAddressIndex,
    TraceStoreFilenameStringBufferBitmapIndex,
    TraceStoreFilenameStringBufferInfoIndex,
    TraceStoreDirectoryStringIndex,
    TraceStoreDirectoryStringAllocationIndex,
    TraceStoreDirectoryStringRelocationIndex,
    TraceStoreDirectoryStringAddressIndex,
    TraceStoreDirectoryStringBitmapIndex,
    TraceStoreDirectoryStringInfoIndex,
    TraceStoreDirectoryStringBufferIndex,
    TraceStoreDirectoryStringBufferAllocationIndex,
    TraceStoreDirectoryStringBufferRelocationIndex,
    TraceStoreDirectoryStringBufferAddressIndex,
    TraceStoreDirectoryStringBufferBitmapIndex,
    TraceStoreDirectoryStringBufferInfoIndex,
    TraceStoreStringArrayIndex,
    TraceStoreStringArrayAllocationIndex,
    TraceStoreStringArrayRelocationIndex,
    TraceStoreStringArrayAddressIndex,
    TraceStoreStringArrayBitmapIndex,
    TraceStoreStringArrayInfoIndex,
    TraceStoreStringTableIndex,
    TraceStoreStringTableAllocationIndex,
    TraceStoreStringTableRelocationIndex,
    TraceStoreStringTableAddressIndex,
    TraceStoreStringTableBitmapIndex,
    TraceStoreStringTableInfoIndex,
    TraceStoreInvalidIndex
} TRACE_STORE_INDEX, *PTRACE_STORE_INDEX;

#define MAX_TRACE_STORES TraceStoreStringTableInfoIndex + 1

#ifdef __cplusplus
}; // extern "C"
#endif

// vim:set ts=8 sw=4 sts=4 tw=80 expandtab                                     :
