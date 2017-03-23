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

CONST LPCWSTR TraceStoreFileNames[] = {
    L"PythonTracer_TraceEvent.dat",
    L"TraceStore_StringBuffer.dat",
    L"Python_PythonFunctionTable.dat",
    L"Python_PythonFunctionTableEntry.dat",
    L"Python_PythonPathTable.dat",
    L"Python_PythonPathTableEntry.dat",
    L"TraceStore_PageFault.dat",
    L"StringTable_StringArray.dat",
    L"StringTable_StringTable.dat",
    L"Python_TraceEventTraitsEx.dat",
    L"TraceStore_WsWatchInfoEx.dat",
    L"TraceStore_WsWorkingSetExInfo.dat",
    L"TraceStore_CCallStackTable.dat",
    L"TraceStore_CCallStackTableEntry.dat",
    L"TraceStore_ModuleTable.dat",
    L"TraceStore_ModuleTableEntry.dat",
    L"Python_PythonCallStackTable.dat",
    L"Python_PythonCallStackTableEntry.dat",
    L"Python_PythonModuleTable.dat",
    L"Python_PythonModuleTableEntry.dat",
    L"TraceStore_LineTable.dat",
    L"TraceStore_LineTableEntry.dat",
    L"TraceStore_LineStringBuffer.dat",
    L"TraceStore_CallStack.dat",
    L"TraceStore_Performance.dat",
    L"TraceStore_PerformanceDelta.dat",
    L"TraceStore_SourceCode.dat",
    L"TraceStore_Bitmap.dat",
    L"TraceStore_ImageFile.dat",
    L"TraceStore_UnicodeStringBuffer.dat",
    L"TraceStore_Line.dat",
    L"TraceStore_Object.dat",
    L"TraceStore_ModuleLoadEvent.dat",
    L"TraceStore_SymbolTable.dat",
    L"TraceStore_SymbolTableEntry.dat",
    L"TraceStore_SymbolModuleInfo.dat",
    L"TraceStore_SymbolFile.dat",
    L"TraceStore_SymbolInfo.dat",
    L"TraceStore_SymbolLine.dat",
    L"TraceStore_SymbolType.dat",
    L"TraceStore_StackFrame.dat",
    L"TraceStore_TypeInfoTable.dat",
    L"TraceStore_TypeInfoTableEntry.dat",
    L"TraceStore_TypeStringBuffer.dat",
    L"TraceStore_FunctionTable.dat",
    L"TraceStore_FunctionTableEntry.dat",
    L"TraceStore_FunctionAssembly.dat",
    L"TraceStore_FunctionSourceCode.dat",
    L"TraceStore_ExamineSymbolsLines.dat",
    L"TraceStore_ExamineSymbolsText.txt",
    L"TraceStore_ExaminedSymbol.dat",
    L"TraceStore_ExaminedSymbolSecondary.dat",
    L"TraceStore_UnassembleFunctionLines.dat",
    L"TraceStore_UnassembleFunctionText.txt",
    L"TraceStore_UnassembledFunction.dat",
    L"TraceStore_UnassembledFunctionSecondary.dat",
    L"TraceStore_DisplayTypeLines.dat",
    L"TraceStore_DisplayTypeText.txt",
    L"TraceStore_DisplayedType.dat",
    L"TraceStore_DisplayedTypeSecondary.dat",
};

CONST TRACE_STORE_ID TraceStoreIds[] = {
    TraceStoreNullId,
    TraceStoreEventId,
    TraceStoreStringBufferId,
    TraceStorePythonFunctionTableId,
    TraceStorePythonFunctionTableEntryId,
    TraceStorePythonPathTableId,
    TraceStorePythonPathTableEntryId,
    TraceStorePageFaultId,
    TraceStoreStringArrayId,
    TraceStoreStringTableId,
    TraceStoreEventTraitsExId,
    TraceStoreWsWatchInfoExId,
    TraceStoreWsWorkingSetExInfoId,
    TraceStoreCCallStackTableId,
    TraceStoreCCallStackTableEntryId,
    TraceStoreModuleTableId,
    TraceStoreModuleTableEntryId,
    TraceStorePythonCallStackTableId,
    TraceStorePythonCallStackTableEntryId,
    TraceStorePythonModuleTableId,
    TraceStorePythonModuleTableEntryId,
    TraceStoreLineTableId,
    TraceStoreLineTableEntryId,
    TraceStoreLineStringBufferId,
    TraceStoreCallStackId,
    TraceStorePerformanceId,
    TraceStorePerformanceDeltaId,
    TraceStoreSourceCodeId,
    TraceStoreBitmapId,
    TraceStoreImageFileId,
    TraceStoreUnicodeStringBufferId,
    TraceStoreLineId,
    TraceStoreObjectId,
    TraceStoreModuleLoadEventId,
    TraceStoreSymbolTableId,
    TraceStoreSymbolTableEntryId,
    TraceStoreSymbolModuleInfoId,
    TraceStoreSymbolFileId,
    TraceStoreSymbolInfoId,
    TraceStoreSymbolLineId,
    TraceStoreSymbolTypeId,
    TraceStoreStackFrameId,
    TraceStoreTypeInfoTableId,
    TraceStoreTypeInfoTableEntryId,
    TraceStoreTypeInfoStringBufferId,
    TraceStoreFunctionTableId,
    TraceStoreFunctionTableEntryId,
    TraceStoreFunctionAssemblyId,
    TraceStoreFunctionSourceCodeId,
    TraceStoreExamineSymbolsLineId,
    TraceStoreExamineSymbolsTextId,
    TraceStoreExaminedSymbolId,
    TraceStoreExaminedSymbolSecondaryId,
    TraceStoreUnassembleFunctionLineId,
    TraceStoreUnassembleFunctionTextId,
    TraceStoreUnassembledFunctionId,
    TraceStoreUnassembledFunctionSecondaryId,
    TraceStoreDisplayTypeLineId,
    TraceStoreDisplayTypeTextId,
    TraceStoreDisplayedTypeId,
    TraceStoreDisplayedTypeSecondaryId,
    TraceStoreInvalidId,
};

CONST WCHAR TraceStoreMetadataInfoSuffix[] = L":MetadataInfo";
CONST DWORD TraceStoreMetadataInfoSuffixLength = (
    sizeof(TraceStoreMetadataInfoSuffix) /
    sizeof(WCHAR)
);

CONST WCHAR TraceStoreAllocationSuffix[] = L":Allocation";
CONST DWORD TraceStoreAllocationSuffixLength = (
    sizeof(TraceStoreAllocationSuffix) /
    sizeof(WCHAR)
);

CONST WCHAR TraceStoreRelocationSuffix[] = L":Relocation";
CONST DWORD TraceStoreRelocationSuffixLength = (
    sizeof(TraceStoreRelocationSuffix) /
    sizeof(WCHAR)
);

CONST WCHAR TraceStoreAddressSuffix[] = L":Address";
CONST DWORD TraceStoreAddressSuffixLength = (
    sizeof(TraceStoreAddressSuffix) /
    sizeof(WCHAR)
);

CONST WCHAR TraceStoreAddressRangeSuffix[] = L":AddressRange";
CONST DWORD TraceStoreAddressRangeSuffixLength = (
    sizeof(TraceStoreAddressRangeSuffix) /
    sizeof(WCHAR)
);

CONST WCHAR TraceStoreAllocationTimestampSuffix[] = L":AllocationTimestamp";
CONST DWORD TraceStoreAllocationTimestampSuffixLength = (
    sizeof(TraceStoreAllocationTimestampSuffix) /
    sizeof(WCHAR)
);

CONST WCHAR TraceStoreAllocationTimestampDeltaSuffix[] = (
    L":AllocationTimestampDelta"
);

CONST DWORD TraceStoreAllocationTimestampDeltaSuffixLength = (
    sizeof(TraceStoreAllocationTimestampDeltaSuffix) /
    sizeof(WCHAR)
);

CONST WCHAR TraceStoreSynchronizationSuffix[] = L":Synchronization";
CONST DWORD TraceStoreSynchronizationSuffixLength = (
    sizeof(TraceStoreSynchronizationSuffix) /
    sizeof(WCHAR)
);

CONST WCHAR TraceStoreInfoSuffix[] = L":Info";
CONST DWORD TraceStoreInfoSuffixLength = (
    sizeof(TraceStoreInfoSuffix) /
    sizeof(WCHAR)
);

CONST USHORT LongestTraceStoreSuffixLength = (
    sizeof(TraceStoreAllocationTimestampDeltaSuffixLength) /
    sizeof(WCHAR)
);

CONST USHORT NumberOfTraceStores = (
    sizeof(TraceStoreFileNames) /
    sizeof(LPCWSTR)
);

CONST LPCWSTR TraceStoreMetadataSuffixes[] = {
    TraceStoreMetadataInfoSuffix,
    TraceStoreAllocationSuffix,
    TraceStoreRelocationSuffix,
    TraceStoreAddressSuffix,
    TraceStoreAddressRangeSuffix,
    TraceStoreAllocationTimestampSuffix,
    TraceStoreAllocationTimestampDeltaSuffix,
    TraceStoreSynchronizationSuffix,
    TraceStoreInfoSuffix,
};

CONST TRACE_STORE_METADATA_ID TraceStoreMetadataIds[] = {
    TraceStoreMetadataNullId,
    TraceStoreMetadataMetadataInfoId,
    TraceStoreMetadataAllocationId,
    TraceStoreMetadataRelocationId,
    TraceStoreMetadataAddressId,
    TraceStoreMetadataAddressRangeId,
    TraceStoreMetadataAllocationTimestampId,
    TraceStoreMetadataAllocationTimestampDeltaId,
    TraceStoreMetadataSynchronizationId,
    TraceStoreMetadataInfoId,
    TraceStoreMetadataInvalidId,
};

#define NUMBER_OF_METADATA_STORES (       \
    sizeof(TraceStoreMetadataSuffixes) /  \
    sizeof(TraceStoreMetadataSuffixes[0]) \
)

CONST USHORT NumberOfMetadataStores = NUMBER_OF_METADATA_STORES;
CONST USHORT ElementsPerTraceStore = NUMBER_OF_METADATA_STORES + 1;

//
// Size key:
//      1 << 16 == 64KB
//      1 << 21 == 2MB
//      1 << 22 == 4MB
//      1 << 23 == 8MB
//      1 << 24 == 16MB
//      1 << 25 == 32MB
//      1 << 26 == 64MB
//      1 << 27 == 128MB
//      1 << 28 == 256MB
//      1 << 29 == 512MB
//      1 << 30 == 1024MB
//      1 << 31 == 2048MB
//      1 << 32 == 4096MB
//
// N.B.: the trace store size should always be greater than or equal to the
//       mapping size.
//


#ifdef _DEBUG

//
// Use smaller versions of files when debugging.
//

#define TRACE_STORE_USE_SMALLER_INITIAL_FILE_SIZES

#endif

#ifdef TRACE_STORE_USE_SMALLER_INITIAL_FILE_SIZES

LONGLONG InitialTraceStoreFileSizesAsLongLong[] = {
     1 << 30,   // Event
     1 << 25,   // StringBuffer
     1 << 16,   // PythonFunctionTable
     1 << 25,   // PythonFunctionTableEntry
     1 << 16,   // PathTable
     1 << 25,   // PathTableEntry
     1 << 26,   // TracePageFault
     1 << 21,   // StringArray
     1 << 21,   // StringTable
     1 << 25,   // EventTraitsEx
     1 << 25,   // WsWatchInfoEx
     1 << 25,   // WorkingSetExInfo
     1 << 16,   // CCallStackTable
     1 << 25,   // CCallStackTableEntry
     1 << 16,   // ModuleTable
     1 << 25,   // ModuleTableEntry
     1 << 16,   // PythonCallStackTable
     1 << 25,   // PythonCallStackTableEntry
     1 << 16,   // PythonModuleTable
     1 << 25,   // PythonModuleTableEntry
     1 << 16,   // LineTable
     1 << 25,   // LineTableEntry
     1 << 25,   // LineStringBuffer
     1 << 25,   // CallStack
     1 << 25,   // Performance
     1 << 25,   // PerformanceDelta
     1 << 25,   // SourceCode
     1 << 24,   // Bitmap
     1 << 27,   // ImageFile
     1 << 23,   // UnicodeStringBuffer
     1 << 24,   // Line
     1 << 23,   // Object
     1 << 23,   // ModuleLoadEvent
     1 << 16,   // SymbolTable
     1 << 25,   // SymbolTableEntry
     1 << 25,   // SymbolModuleInfo
     1 << 27,   // SymbolFile
     1 << 24,   // SymbolInfo
     1 << 24,   // SymbolLine
     1 << 24,   // SymbolType
     1 << 25,   // StackFrame
     1 << 16,   // TypeInfoTable
     1 << 25,   // TypeInfoTableEntry
     1 << 25,   // TypeInfoStringBuffer
     1 << 16,   // FunctionTable
     1 << 26,   // FunctionTableEntry
     1 << 25,   // FunctionAssembly
     1 << 25,   // FunctionSourceCode
     1 << 23,   // ExamineSymbolsLines
     1 << 25,   // ExamineSymbolsText
     1 << 25,   // ExaminedSymbol
     1 << 23,   // ExaminedSymbolSecondary
     1 << 23,   // UnassembleFunctionLines
     1 << 25,   // UnassembleFunctionText
     1 << 25,   // UnassembledFunction
     1 << 23,   // UnassembledFunctionSecondary
     1 << 23,   // DisplayTypeLines
     1 << 25,   // DisplayTypeText
     1 << 25,   // DisplayedType
     1 << 23,   // DisplayedTypeSecondary
};

#else

LONGLONG InitialTraceStoreFileSizesAsLongLong[] = {
     1 << 30,   // Event
     1 << 25,   // StringBuffer
     1 << 16,   // PythonFunctionTable
     1 << 26,   // PythonFunctionTableEntry
     1 << 16,   // PathTable
     1 << 26,   // PathTableEntry
     1 << 26,   // TracePageFault
     1 << 21,   // StringArray
     1 << 21,   // StringTable
     1 << 30,   // EventTraitsEx
     1 << 27,   // WsWatchInfoEx
     1 << 26,   // WorkingSetExInfo
     1 << 16,   // CCallStackTable
     1 << 25,   // CCallStackTableEntry
     1 << 16,   // ModuleTable
     1 << 25,   // ModuleTableEntry
     1 << 16,   // PythonCallStackTable
     1 << 25,   // PythonCallStackTableEntry
     1 << 16,   // PythonModuleTable
     1 << 25,   // PythonModuleTableEntry
     1 << 16,   // LineTable
     1 << 25,   // LineTableEntry
     1 << 25,   // LineStringBuffer
     1 << 27,   // CallStack
     1 << 25,   // Performance
     1 << 25,   // PerformanceDelta
     1 << 27,   // SourceCode
     1 << 24,   // Bitmap
     1 << 29,   // ImageFile
     1 << 23,   // UnicodeStringBuffer
     1 << 24,   // Line
     1 << 23,   // Object
     1 << 23,   // ModuleLoadEvent
     1 << 16,   // SymbolTable
     1 << 28,   // SymbolTableEntry
     1 << 27,   // SymbolModuleInfo
     1 << 29,   // SymbolFile
     1 << 24,   // SymbolInfo
     1 << 24,   // SymbolLine
     1 << 24,   // SymbolType
     1 << 25,   // StackFrame
     1 << 16,   // TypeInfoTable
     1 << 27,   // TypeInfoTableEntry
     1 << 26,   // TypeInfoStringBuffer
     1 << 16,   // FunctionTable
     1 << 26,   // FunctionTableEntry
     1 << 27,   // FunctionAssembly
     1 << 27,   // FunctionSourceCode
     1 << 23,   // ExamineSymbolsLines
     1 << 25,   // ExamineSymbolsText
     1 << 25,   // ExaminedSymbol
     1 << 23,   // ExaminedSymbolSecondary
     1 << 23,   // UnassembleFunctionLines
     1 << 25,   // UnassembleFunctionText
     1 << 25,   // UnassembledFunction
     1 << 23,   // UnassembledFunctionSecondary
     1 << 23,   // DisplayTypeLines
     1 << 25,   // DisplayTypeText
     1 << 25,   // DisplayedType
     1 << 23,   // DisplayedTypeSecondary
};

#endif

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

CONST TRACE_STORE_TRAITS TraceStoreTraits[] = {

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
        0,  // PageAligned
        0,  // Periodic
        0,  // ConcurrentDataStructure
        0,  // NoAllocationAlignment
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
        0,  // PageAligned
        0,  // Periodic
        0,  // ConcurrentDataStructure
        0,  // NoAllocationAlignment
        0   // Unused
    },

    //
    // PythonFunctionTable
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
        0,  // PageAligned
        0,  // Periodic
        0,  // ConcurrentDataStructure
        0,  // NoAllocationAlignment
        0   // Unused
    },

    //
    // PythonFunctionTableEntry
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
        0,  // PageAligned
        0,  // Periodic
        0,  // ConcurrentDataStructure
        0,  // NoAllocationAlignment
        0   // Unused
    },

    //
    // PythonPathTable
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
        0,  // PageAligned
        0,  // Periodic
        0,  // ConcurrentDataStructure
        0,  // NoAllocationAlignment
        0   // Unused
    },

    //
    // PythonPathTableEntry
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
        0,  // PageAligned
        0,  // Periodic
        0,  // ConcurrentDataStructure
        0,  // NoAllocationAlignment
        0   // Unused
    },

    //
    // TracePageFault
    //

    {
        0,  // VaryingRecordSize
        1,  // RecordSizeIsAlwaysPowerOf2
        1,  // MultipleRecords
        1,  // StreamingWrite
        0,  // StreamingRead
        0,  // FrequentAllocations
        1,  // BlockingAllocations
        0,  // LinkedStore
        1,  // CoalescedAllocations
        0,  // ConcurrentAllocations
        0,  // AllowPageSpill
        0,  // PageAligned
        0,  // Periodic
        0,  // ConcurrentDataStructure
        0,  // NoAllocationAlignment
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
        1,  // BlockingAllocations
        0,  // LinkedStore
        1,  // CoalescedAllocations
        1,  // ConcurrentAllocations
        0,  // AllowPageSpill
        0,  // PageAligned
        0,  // Periodic
        0,  // ConcurrentDataStructure
        0,  // NoAllocationAlignment
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
        1,  // BlockingAllocations
        0,  // LinkedStore
        1,  // CoalescedAllocations
        1,  // ConcurrentAllocations
        0,  // AllowPageSpill
        0,  // PageAligned
        0,  // Periodic
        0,  // ConcurrentDataStructure
        0,  // NoAllocationAlignment
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
        0,  // PageAligned
        0,  // Periodic
        0,  // ConcurrentDataStructure
        0,  // NoAllocationAlignment
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
        0,  // PageAligned
        1,  // Periodic
        0,  // ConcurrentDataStructure
        0,  // NoAllocationAlignment
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
        0,  // PageAligned
        1,  // Periodic
        0,  // ConcurrentDataStructure
        0,  // NoAllocationAlignment
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
        0,  // PageAligned
        0,  // Periodic
        0,  // ConcurrentDataStructure
        0,  // NoAllocationAlignment
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
        0,  // PageAligned
        0,  // Periodic
        0,  // ConcurrentDataStructure
        0,  // NoAllocationAlignment
        0   // Unused
    },

    //
    // ModuleTable
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
        0,  // PageAligned
        0,  // Periodic
        1,  // ConcurrentDataStructure
        0,  // NoAllocationAlignment
        0   // Unused
    },

    //
    // ModuleTableEntry
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
        0,  // PageAligned
        0,  // Periodic
        0,  // ConcurrentDataStructure
        0,  // NoAllocationAlignment
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
        0,  // PageAligned
        0,  // Periodic
        0,  // ConcurrentDataStructure
        0,  // NoAllocationAlignment
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
        0,  // PageAligned
        0,  // Periodic
        0,  // ConcurrentDataStructure
        0,  // NoAllocationAlignment
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
        0,  // PageAligned
        0,  // Periodic
        0,  // ConcurrentDataStructure
        0,  // NoAllocationAlignment
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
        0,  // PageAligned
        0,  // Periodic
        0,  // ConcurrentDataStructure
        0,  // NoAllocationAlignment
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
        0,  // PageAligned
        0,  // Periodic
        0,  // ConcurrentDataStructure
        0,  // NoAllocationAlignment
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
        0,  // PageAligned
        0,  // Periodic
        0,  // ConcurrentDataStructure
        0,  // NoAllocationAlignment
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
        0,  // PageAligned
        0,  // Periodic
        0,  // ConcurrentDataStructure
        0,  // NoAllocationAlignment
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
        0,  // PageAligned
        0,  // Periodic
        0,  // ConcurrentDataStructure
        0,  // NoAllocationAlignment
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
        0,  // PageAligned
        1,  // Periodic
        0,  // ConcurrentDataStructure
        0,  // NoAllocationAlignment
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
        0,  // PageAligned
        1,  // Periodic
        0,  // ConcurrentDataStructure
        0,  // NoAllocationAlignment
        0   // Unused
    },

    //
    // SourceCode
    //

    {
        1,  // VaryingRecordSize
        0,  // RecordSizeIsAlwaysPowerOf2
        1,  // MultipleRecords
        0,  // StreamingWrite
        0,  // StreamingRead
        0,  // FrequentAllocations
        1,  // BlockingAllocations
        0,  // LinkedStore
        0,  // CoalescedAllocations
        1,  // ConcurrentAllocations
        0,  // AllowPageSpill
        1,  // PageAligned
        0,  // Periodic
        0,  // ConcurrentDataStructure
        0,  // NoAllocationAlignment
        0   // Unused
    },

    //
    // Bitmap
    //

    {
        1,  // VaryingRecordSize
        0,  // RecordSizeIsAlwaysPowerOf2
        1,  // MultipleRecords
        0,  // StreamingWrite
        0,  // StreamingRead
        0,  // FrequentAllocations
        1,  // BlockingAllocations
        0,  // LinkedStore
        0,  // CoalescedAllocations
        1,  // ConcurrentAllocations
        1,  // AllowPageSpill
        0,  // PageAligned
        0,  // Periodic
        0,  // ConcurrentDataStructure
        0,  // NoAllocationAlignment
        0   // Unused
    },

    //
    // ImageFile
    //

    {
        1,  // VaryingRecordSize
        0,  // RecordSizeIsAlwaysPowerOf2
        1,  // MultipleRecords
        0,  // StreamingWrite
        0,  // StreamingRead
        0,  // FrequentAllocations
        1,  // BlockingAllocations
        0,  // LinkedStore
        0,  // CoalescedAllocations
        1,  // ConcurrentAllocations
        0,  // AllowPageSpill
        1,  // PageAligned
        0,  // Periodic
        0,  // ConcurrentDataStructure
        0,  // NoAllocationAlignment
        0   // Unused
    },

    //
    // UnicodeStringBuffer
    //

    {
        1,  // VaryingRecordSize
        0,  // RecordSizeIsAlwaysPowerOf2
        1,  // MultipleRecords
        0,  // StreamingWrite
        0,  // StreamingRead
        0,  // FrequentAllocations
        1,  // BlockingAllocations
        0,  // LinkedStore
        0,  // CoalescedAllocations
        1,  // ConcurrentAllocations
        0,  // AllowPageSpill
        0,  // PageAligned
        0,  // Periodic
        0,  // ConcurrentDataStructure
        0,  // NoAllocationAlignment
        0   // Unused
    },

    //
    // Line
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
        0,  // PageAligned
        0,  // Periodic
        0,  // ConcurrentDataStructure
        0,  // NoAllocationAlignment
        0   // Unused
    },

    //
    // Object
    //

    {
        1,  // VaryingRecordSize
        0,  // RecordSizeIsAlwaysPowerOf2
        1,  // MultipleRecords
        0,  // StreamingWrite
        0,  // StreamingRead
        0,  // FrequentAllocations
        1,  // BlockingAllocations
        0,  // LinkedStore
        0,  // CoalescedAllocations
        1,  // ConcurrentAllocations
        0,  // AllowPageSpill
        0,  // PageAligned
        0,  // Periodic
        0,  // ConcurrentDataStructure
        0,  // NoAllocationAlignment
        0   // Unused
    },

    //
    // ModuleLoadEvent
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
        0,  // PageAligned
        0,  // Periodic
        0,  // ConcurrentDataStructure
        0,  // NoAllocationAlignment
        0   // Unused
    },

    //
    // SymbolTable
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
        0,  // PageAligned
        0,  // Periodic
        1,  // ConcurrentDataStructure
        0,  // NoAllocationAlignment
        0   // Unused
    },

    //
    // SymbolTableEntry
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
        0,  // ConcurrentAllocations
        0,  // AllowPageSpill
        0,  // PageAligned
        0,  // Periodic
        0,  // ConcurrentDataStructure
        0,  // NoAllocationAlignment
        0   // Unused
    },


    //
    // SymbolModuleInfo
    //

    {
        0,  // VaryingRecordSize
        0,  // RecordSizeIsAlwaysPowerOf2
        1,  // MultipleRecords
        0,  // StreamingWrite
        0,  // StreamingRead
        0,  // FrequentAllocations
        1,  // BlockingAllocations
        0,  // LinkedStore
        0,  // CoalescedAllocations
        0,  // ConcurrentAllocations
        0,  // AllowPageSpill
        0,  // PageAligned
        0,  // Periodic
        0,  // ConcurrentDataStructure
        0,  // NoAllocationAlignment
        0   // Unused
    },

    //
    // SymbolFile
    //

    {
        1,  // VaryingRecordSize
        0,  // RecordSizeIsAlwaysPowerOf2
        1,  // MultipleRecords
        0,  // StreamingWrite
        0,  // StreamingRead
        0,  // FrequentAllocations
        1,  // BlockingAllocations
        0,  // LinkedStore
        0,  // CoalescedAllocations
        1,  // ConcurrentAllocations
        0,  // AllowPageSpill
        1,  // PageAligned
        0,  // Periodic
        0,  // ConcurrentDataStructure
        0,  // NoAllocationAlignment
        0   // Unused
    },

    //
    // SymbolInfo
    //

    {
        1,  // VaryingRecordSize
        0,  // RecordSizeIsAlwaysPowerOf2
        1,  // MultipleRecords
        0,  // StreamingWrite
        0,  // StreamingRead
        0,  // FrequentAllocations
        1,  // BlockingAllocations
        0,  // LinkedStore
        0,  // CoalescedAllocations
        1,  // ConcurrentAllocations
        0,  // AllowPageSpill
        1,  // PageAligned
        0,  // Periodic
        0,  // ConcurrentDataStructure
        0,  // NoAllocationAlignment
        0   // Unused
    },

    //
    // SymbolLine
    //

    {
        1,  // VaryingRecordSize
        0,  // RecordSizeIsAlwaysPowerOf2
        1,  // MultipleRecords
        0,  // StreamingWrite
        0,  // StreamingRead
        0,  // FrequentAllocations
        1,  // BlockingAllocations
        0,  // LinkedStore
        0,  // CoalescedAllocations
        0,  // ConcurrentAllocations
        0,  // AllowPageSpill
        1,  // PageAligned
        0,  // Periodic
        0,  // ConcurrentDataStructure
        0,  // NoAllocationAlignment
        0   // Unused
    },

    //
    // SymbolType
    //

    {
        1,  // VaryingRecordSize
        0,  // RecordSizeIsAlwaysPowerOf2
        1,  // MultipleRecords
        0,  // StreamingWrite
        0,  // StreamingRead
        0,  // FrequentAllocations
        1,  // BlockingAllocations
        0,  // LinkedStore
        0,  // CoalescedAllocations
        0,  // ConcurrentAllocations
        0,  // AllowPageSpill
        1,  // PageAligned
        0,  // Periodic
        0,  // ConcurrentDataStructure
        0,  // NoAllocationAlignment
        0   // Unused
    },

    //
    // StackFrame
    //

    {
        1,  // VaryingRecordSize
        0,  // RecordSizeIsAlwaysPowerOf2
        1,  // MultipleRecords
        0,  // StreamingWrite
        0,  // StreamingRead
        0,  // FrequentAllocations
        1,  // BlockingAllocations
        0,  // LinkedStore
        0,  // CoalescedAllocations
        1,  // ConcurrentAllocations
        0,  // AllowPageSpill
        1,  // PageAligned
        0,  // Periodic
        0,  // ConcurrentDataStructure
        0,  // NoAllocationAlignment
        0   // Unused
    },

    //
    // TypeInfoTable
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
        0,  // PageAligned
        0,  // Periodic
        1,  // ConcurrentDataStructure
        0,  // NoAllocationAlignment
        0   // Unused
    },

    //
    // TypeInfoTableEntry
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
        0,  // PageAligned
        0,  // Periodic
        1,  // ConcurrentDataStructure
        0,  // NoAllocationAlignment
        0   // Unused
    },

    //
    // TypeInfoStringBuffer
    //

    {
        1,  // VaryingRecordSize
        0,  // RecordSizeIsAlwaysPowerOf2
        1,  // MultipleRecords
        0,  // StreamingWrite
        0,  // StreamingRead
        0,  // FrequentAllocations
        1,  // BlockingAllocations
        0,  // LinkedStore
        0,  // CoalescedAllocations
        1,  // ConcurrentAllocations
        1,  // AllowPageSpill
        0,  // PageAligned
        0,  // Periodic
        0,  // ConcurrentDataStructure
        0,  // NoAllocationAlignment
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
        0,  // PageAligned
        0,  // Periodic
        1,  // ConcurrentDataStructure
        0,  // NoAllocationAlignment
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
        1,  // BlockingAllocations
        0,  // LinkedStore
        0,  // CoalescedAllocations
        1,  // ConcurrentAllocations
        0,  // AllowPageSpill
        0,  // PageAligned
        0,  // Periodic
        0,  // ConcurrentDataStructure
        0,  // NoAllocationAlignment
        0   // Unused
    },

    //
    // FunctionAssembly
    //

    {
        1,  // VaryingRecordSize
        0,  // RecordSizeIsAlwaysPowerOf2
        1,  // MultipleRecords
        0,  // StreamingWrite
        0,  // StreamingRead
        0,  // FrequentAllocations
        1,  // BlockingAllocations
        0,  // LinkedStore
        0,  // CoalescedAllocations
        1,  // ConcurrentAllocations
        0,  // AllowPageSpill
        0,  // PageAligned
        0,  // Periodic
        0,  // ConcurrentDataStructure
        0,  // NoAllocationAlignment
        0   // Unused
    },

    //
    // FunctionSourceCode
    //

    {
        1,  // VaryingRecordSize
        0,  // RecordSizeIsAlwaysPowerOf2
        1,  // MultipleRecords
        0,  // StreamingWrite
        0,  // StreamingRead
        0,  // FrequentAllocations
        1,  // BlockingAllocations
        0,  // LinkedStore
        0,  // CoalescedAllocations
        1,  // ConcurrentAllocations
        0,  // AllowPageSpill
        0,  // PageAligned
        0,  // Periodic
        0,  // ConcurrentDataStructure
        0,  // NoAllocationAlignment
        0   // Unused
    },

    //
    // ExamineSymbolsLines
    //

    {
        0,  // VaryingRecordSize
        1,  // RecordSizeIsAlwaysPowerOf2
        1,  // MultipleRecords
        1,  // StreamingWrite
        0,  // StreamingRead
        0,  // FrequentAllocations
        1,  // BlockingAllocations
        0,  // LinkedStore
        0,  // CoalescedAllocations
        0,  // ConcurrentAllocations
        0,  // AllowPageSpill
        0,  // PageAligned
        0,  // Periodic
        0,  // ConcurrentDataStructure
        0,  // NoAllocationAlignment
        0   // Unused
    },

    //
    // ExamineSymbolsText
    //

    {
        1,  // VaryingRecordSize
        0,  // RecordSizeIsAlwaysPowerOf2
        1,  // MultipleRecords
        0,  // StreamingWrite
        0,  // StreamingRead
        0,  // FrequentAllocations
        1,  // BlockingAllocations
        1,  // LinkedStore
        0,  // CoalescedAllocations
        0,  // ConcurrentAllocations
        1,  // AllowPageSpill
        0,  // PageAligned
        0,  // Periodic
        0,  // ConcurrentDataStructure
        1,  // NoAllocationAlignment
        0   // Unused
    },

    //
    // ExaminedSymbol
    //

    {
        0,  // VaryingRecordSize
        0,  // RecordSizeIsAlwaysPowerOf2
        1,  // MultipleRecords
        0,  // StreamingWrite
        0,  // StreamingRead
        0,  // FrequentAllocations
        1,  // BlockingAllocations
        0,  // LinkedStore
        0,  // CoalescedAllocations
        0,  // ConcurrentAllocations
        0,  // AllowPageSpill
        0,  // PageAligned
        0,  // Periodic
        0,  // ConcurrentDataStructure
        0,  // NoAllocationAlignment
        0   // Unused
    },

    //
    // ExaminedSymbolSecondary
    //

    {
        1,  // VaryingRecordSize
        0,  // RecordSizeIsAlwaysPowerOf2
        1,  // MultipleRecords
        0,  // StreamingWrite
        0,  // StreamingRead
        0,  // FrequentAllocations
        1,  // BlockingAllocations
        0,  // LinkedStore
        0,  // CoalescedAllocations
        0,  // ConcurrentAllocations
        0,  // AllowPageSpill
        0,  // PageAligned
        0,  // Periodic
        0,  // ConcurrentDataStructure
        0,  // NoAllocationAlignment
        0   // Unused
    },

    //
    // UnassembleFunctionLines
    //

    {
        0,  // VaryingRecordSize
        1,  // RecordSizeIsAlwaysPowerOf2
        1,  // MultipleRecords
        1,  // StreamingWrite
        0,  // StreamingRead
        0,  // FrequentAllocations
        1,  // BlockingAllocations
        0,  // LinkedStore
        0,  // CoalescedAllocations
        0,  // ConcurrentAllocations
        0,  // AllowPageSpill
        0,  // PageAligned
        0,  // Periodic
        0,  // ConcurrentDataStructure
        0,  // NoAllocationAlignment
        0   // Unused
    },

    //
    // UnassembleFunctionText
    //

    {
        1,  // VaryingRecordSize
        0,  // RecordSizeIsAlwaysPowerOf2
        1,  // MultipleRecords
        0,  // StreamingWrite
        0,  // StreamingRead
        0,  // FrequentAllocations
        1,  // BlockingAllocations
        1,  // LinkedStore
        0,  // CoalescedAllocations
        0,  // ConcurrentAllocations
        1,  // AllowPageSpill
        0,  // PageAligned
        0,  // Periodic
        0,  // ConcurrentDataStructure
        1,  // NoAllocationAlignment
        0   // Unused
    },

    //
    // UnassembledFunction
    //

    {
        0,  // VaryingRecordSize
        0,  // RecordSizeIsAlwaysPowerOf2
        1,  // MultipleRecords
        0,  // StreamingWrite
        0,  // StreamingRead
        0,  // FrequentAllocations
        1,  // BlockingAllocations
        0,  // LinkedStore
        0,  // CoalescedAllocations
        0,  // ConcurrentAllocations
        0,  // AllowPageSpill
        0,  // PageAligned
        0,  // Periodic
        0,  // ConcurrentDataStructure
        0,  // NoAllocationAlignment
        0   // Unused
    },

    //
    // UnassembledFunctionSecondary
    //

    {
        1,  // VaryingRecordSize
        0,  // RecordSizeIsAlwaysPowerOf2
        1,  // MultipleRecords
        0,  // StreamingWrite
        0,  // StreamingRead
        0,  // FrequentAllocations
        1,  // BlockingAllocations
        0,  // LinkedStore
        0,  // CoalescedAllocations
        0,  // ConcurrentAllocations
        0,  // AllowPageSpill
        0,  // PageAligned
        0,  // Periodic
        0,  // ConcurrentDataStructure
        0,  // NoAllocationAlignment
        0   // Unused
    },

    //
    // DisplayTypeLines
    //

    {
        0,  // VaryingRecordSize
        1,  // RecordSizeIsAlwaysPowerOf2
        1,  // MultipleRecords
        1,  // StreamingWrite
        0,  // StreamingRead
        0,  // FrequentAllocations
        1,  // BlockingAllocations
        0,  // LinkedStore
        0,  // CoalescedAllocations
        0,  // ConcurrentAllocations
        0,  // AllowPageSpill
        0,  // PageAligned
        0,  // Periodic
        0,  // ConcurrentDataStructure
        0,  // NoAllocationAlignment
        0   // Unused
    },

    //
    // DisplayTypeText
    //

    {
        1,  // VaryingRecordSize
        0,  // RecordSizeIsAlwaysPowerOf2
        1,  // MultipleRecords
        0,  // StreamingWrite
        0,  // StreamingRead
        0,  // FrequentAllocations
        1,  // BlockingAllocations
        1,  // LinkedStore
        0,  // CoalescedAllocations
        0,  // ConcurrentAllocations
        1,  // AllowPageSpill
        0,  // PageAligned
        0,  // Periodic
        0,  // ConcurrentDataStructure
        1,  // NoAllocationAlignment
        0   // Unused
    },

    //
    // DisplayedType
    //

    {
        0,  // VaryingRecordSize
        0,  // RecordSizeIsAlwaysPowerOf2
        1,  // MultipleRecords
        0,  // StreamingWrite
        0,  // StreamingRead
        0,  // FrequentAllocations
        1,  // BlockingAllocations
        0,  // LinkedStore
        0,  // CoalescedAllocations
        0,  // ConcurrentAllocations
        0,  // AllowPageSpill
        0,  // PageAligned
        0,  // Periodic
        0,  // ConcurrentDataStructure
        0,  // NoAllocationAlignment
        0   // Unused
    },

    //
    // DisplayedTypeSecondary
    //

    {
        1,  // VaryingRecordSize
        0,  // RecordSizeIsAlwaysPowerOf2
        1,  // MultipleRecords
        0,  // StreamingWrite
        0,  // StreamingRead
        0,  // FrequentAllocations
        1,  // BlockingAllocations
        0,  // LinkedStore
        0,  // CoalescedAllocations
        0,  // ConcurrentAllocations
        0,  // AllowPageSpill
        0,  // PageAligned
        0,  // Periodic
        0,  // ConcurrentDataStructure
        0,  // NoAllocationAlignment
        0   // Unused
    },
};

CONST TRACE_STORE_STRUCTURE_SIZES TraceStoreStructureSizes = {
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

CONST LARGE_INTEGER MinimumMappingSize = { 1 << 16 }; // 64KB
CONST LARGE_INTEGER MaximumMappingSize = { 1 << 31 }; // 2GB

CONST USHORT InitialFreeMemoryMapsForNonStreamingReaders = 64;
CONST USHORT InitialFreeMemoryMapsForNonStreamingMetadataReaders = 64;
CONST USHORT InitialFreeMemoryMapsForNonStreamingWriters = 64;
CONST USHORT InitialFreeMemoryMapsForNonStreamingMetadataWriters = 64;
CONST USHORT InitialFreeMemoryMapsForStreamingReaders = 64;
CONST USHORT InitialFreeMemoryMapsForStreamingWriters = 64;
CONST USHORT InitialFreeMemoryMapMultiplierForFrequentAllocators = 1;

CONST USHORT TraceStoreMetadataInfoStructSize = (
    sizeof(TRACE_STORE_METADATA_INFO)
);
CONST USHORT TraceStoreAllocationStructSize = (
    sizeof(TRACE_STORE_ALLOCATION)
);
CONST USHORT TraceStoreRelocationStructSize = (
    sizeof(TRACE_STORE_FIELD_RELOC)
);
CONST USHORT TraceStoreAddressStructSize = sizeof(TRACE_STORE_ADDRESS);
CONST USHORT TraceStoreAddressRangeStructSize = sizeof(TRACE_STORE_ADDRESS_RANGE);
CONST USHORT TraceStoreAllocationTimestampStructSize = (
    sizeof(TRACE_STORE_ALLOCATION_TIMESTAMP)
);
CONST USHORT TraceStoreAllocationTimestampDeltaStructSize = (
    sizeof(TRACE_STORE_ALLOCATION_TIMESTAMP_DELTA)
);
CONST USHORT TraceStoreInfoStructSize = sizeof(TRACE_STORE_INFO);

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

CONST LARGE_INTEGER DefaultTraceStoreMappingSize = { 1 << 26 };
CONST LARGE_INTEGER DefaultTraceStoreEventMappingSize = { 1 << 30 };

CONST LARGE_INTEGER DefaultAllocationTraceStoreSize = { 1 << 21 };
CONST LARGE_INTEGER DefaultAllocationTraceStoreMappingSize = { 1 << 21 };

CONST LARGE_INTEGER DefaultRelocationTraceStoreSize = { 1 << 16 };
CONST LARGE_INTEGER DefaultRelocationTraceStoreMappingSize = { 1 << 16 };

CONST LARGE_INTEGER DefaultAddressTraceStoreSize = { 1 << 21 };
CONST LARGE_INTEGER DefaultAddressTraceStoreMappingSize = { 1 << 21 };

CONST LARGE_INTEGER DefaultAddressRangeTraceStoreSize = { 1 << 20 };
CONST LARGE_INTEGER DefaultAddressRangeTraceStoreMappingSize = { 1 << 20 };

CONST LARGE_INTEGER DefaultAllocationTimestampTraceStoreSize = { 1 << 26 };
CONST LARGE_INTEGER DefaultAllocationTimestampTraceStoreMappingSize = { 1 << 26 };

CONST LARGE_INTEGER DefaultAllocationTimestampDeltaTraceStoreSize = { 1 << 25 };
CONST LARGE_INTEGER DefaultAllocationTimestampDeltaTraceStoreMappingSize = {
    1 << 25
};

CONST LARGE_INTEGER DefaultMetadataInfoTraceStoreSize = {
    sizeof(TRACE_STORE_METADATA_INFO)
};

CONST LARGE_INTEGER DefaultMetadataInfoTraceStoreMappingSize = {
    sizeof(TRACE_STORE_METADATA_INFO)
};

CONST LARGE_INTEGER DefaultInfoTraceStoreSize = {
    sizeof(TRACE_STORE_INFO)
};

CONST LARGE_INTEGER DefaultInfoTraceStoreMappingSize = {
    sizeof(TRACE_STORE_INFO)
};

CONST LARGE_INTEGER DefaultSynchronizationTraceStoreSize = {
    sizeof(TRACE_STORE_SYNC)
};

CONST LARGE_INTEGER DefaultSynchronizationTraceStoreMappingSize = {
    sizeof(TRACE_STORE_SYNC)
};


//
// N.B. To add a new flag to each store in vim, mark the brace boundaries with
//      ma and mb, and then run the command:
//
//          'a,'b s:0   // Unused$:0,  // NewFlag\r    0   // Unused:
//
//      Where NewFlag is the name of the new flag to add.
//

CONST TRACE_STORE_TRAITS MetadataInfoStoreTraits = {
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
    0,  // PageAligned
    0,  // Periodic
    0,  // ConcurrentDataStructure
    0,  // NoAllocationAlignment
    0   // Unused
};

CONST TRACE_STORE_TRAITS AllocationStoreTraits = {
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
    0,  // PageAligned
    0,  // Periodic
    0,  // ConcurrentDataStructure
    0,  // NoAllocationAlignment
    0   // Unused
};

CONST TRACE_STORE_TRAITS RelocationStoreTraits = {
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
    0,  // PageAligned
    0,  // Periodic
    0,  // ConcurrentDataStructure
    0,  // NoAllocationAlignment
    0   // Unused
};

CONST TRACE_STORE_TRAITS AddressStoreTraits = {
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
    0,  // PageAligned
    0,  // Periodic
    0,  // ConcurrentDataStructure
    0,  // NoAllocationAlignment
    0   // Unused
};

CONST TRACE_STORE_TRAITS AddressRangeStoreTraits = {
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
    0,  // PageAligned
    0,  // Periodic
    0,  // ConcurrentDataStructure
    0,  // NoAllocationAlignment
    0   // Unused
};

CONST TRACE_STORE_TRAITS AllocationTimestampStoreTraits = {
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
    0,  // PageAligned
    0,  // Periodic
    0,  // ConcurrentDataStructure
    0,  // NoAllocationAlignment
    0   // Unused
};

CONST TRACE_STORE_TRAITS AllocationTimestampDeltaStoreTraits = {
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
    0,  // PageAligned
    0,  // Periodic
    0,  // ConcurrentDataStructure
    0,  // NoAllocationAlignment
    0   // Unused
};

CONST TRACE_STORE_TRAITS SynchronizationStoreTraits = {
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
    0,  // PageAligned
    0,  // Periodic
    0,  // ConcurrentDataStructure
    0,  // NoAllocationAlignment
    0   // Unused
};

CONST TRACE_STORE_TRAITS InfoStoreTraits = {
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
    0,  // PageAligned
    0,  // Periodic
    0,  // ConcurrentDataStructure
    0,  // NoAllocationAlignment
    0   // Unused
};

// vim:set ts=8 sw=4 sts=4 tw=80 expandtab nowrap                              :
