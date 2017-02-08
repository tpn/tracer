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
    L"TracePythonFunctionTable.dat",
    L"TracePythonFunctionTableEntry.dat",
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
    L"TraceStoreModuleTable.dat",
    L"TraceStoreModuleTableEntry.dat",
    L"TraceStorePythonCallStackTable.dat",
    L"TraceStorePythonCallStackTableEntry.dat",
    L"TraceStorePythonModuleTable.dat",
    L"TraceStorePythonModuleTableEntry.dat",
    L"TraceStoreLineTable.dat",
    L"TraceStoreLineTableEntry.dat",
    L"TraceStoreLineStringBuffer.dat",
    L"TraceStoreCallStack.dat",
    L"TraceStorePerformance.dat",
    L"TraceStorePerformanceDelta.dat",
    L"TraceStoreSourceCode.dat",
    L"TraceStoreBitmap.dat",
    L"TraceStoreImageFile.dat",
    L"TraceStoreUnicodeStringBuffer.dat",
    L"TraceStoreLine.dat",
    L"TraceStoreObject.dat",
    L"TraceStoreModuleLoadEvent.dat",
    L"TraceStoreSymbolTable.dat",
    L"TraceStoreSymbolTableEntry.dat",
    L"TraceStoreSymbolModuleInfo.dat",
    L"TraceStoreSymbolFile.dat",
    L"TraceStoreSymbolInfo.dat",
    L"TraceStoreSymbolLine.dat",
    L"TraceStoreSymbolType.dat",
    L"TraceStoreStackFrame.dat",
    L"TraceStoreTypeInfoTable.dat",
    L"TraceStoreTypeInfoTableEntry.dat",
    L"TraceStoreTypeStringBuffer.dat",
    L"TraceStoreFunctionTable.dat",
    L"TraceStoreFunctionTableEntry.dat",
    L"TraceStoreFunctionAssembly.dat",
    L"TraceStoreFunctionSourceCode.dat",
    L"TraceStoreExamineSymbolsLines.dat",
    L"TraceStoreExamineSymbolsText.txt",
    L"TraceStoreExaminedSymbol.dat",
    L"TraceStoreExaminedSymbolSecondary.dat",
    L"TraceStoreUnassembleFunctionLines.dat",
    L"TraceStoreUnassembleFunctionText.txt",
    L"TraceStoreUnassembledFunction.dat",
    L"TraceStoreUnassembledFunctionSecondary.dat",
    L"TraceStoreDisplayTypeLines.dat",
    L"TraceStoreDisplayTypeText.txt",
    L"TraceStoreDisplayedType.dat",
    L"TraceStoreDisplayedTypeSecondary.dat",
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

WCHAR TraceStoreSynchronizationSuffix[] = L":Synchronization";
DWORD TraceStoreSynchronizationSuffixLength = (
    sizeof(TraceStoreSynchronizationSuffix) /
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

PWCHAR TraceStoreMetadataSuffixes[] = {
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

#define NUMBER_OF_METADATA_STORES (       \
    sizeof(TraceStoreMetadataSuffixes) /  \
    sizeof(TraceStoreMetadataSuffixes[0]) \
)

USHORT NumberOfMetadataStores = NUMBER_OF_METADATA_STORES;
USHORT ElementsPerTraceStore = NUMBER_OF_METADATA_STORES + 1;

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
     1 << 18,   // TraceSession
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
     1 << 18,   // TraceSession
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
        0,  // PageAligned
        0,  // Periodic
        0,  // ConcurrentDataStructure
        0,  // NoAllocationAlignment
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
        0,  // PageAligned
        0,  // Periodic
        0,  // ConcurrentDataStructure
        0,  // NoAllocationAlignment
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

USHORT InitialFreeMemoryMapsForNonStreamingReaders = 64;
USHORT InitialFreeMemoryMapsForNonStreamingMetadataReaders = 64;
USHORT InitialFreeMemoryMapsForNonStreamingWriters = 64;
USHORT InitialFreeMemoryMapsForNonStreamingMetadataWriters = 64;
USHORT InitialFreeMemoryMapsForStreamingReaders = 64;
USHORT InitialFreeMemoryMapsForStreamingWriters = 64;
USHORT InitialFreeMemoryMapMultiplierForFrequentAllocators = 1;

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

LARGE_INTEGER DefaultSynchronizationTraceStoreSize = {
    sizeof(TRACE_STORE_SYNC)
};

LARGE_INTEGER DefaultSynchronizationTraceStoreMappingSize = {
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
    0,  // PageAligned
    0,  // Periodic
    0,  // ConcurrentDataStructure
    0,  // NoAllocationAlignment
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
    0,  // PageAligned
    0,  // Periodic
    0,  // ConcurrentDataStructure
    0,  // NoAllocationAlignment
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
    0,  // PageAligned
    0,  // Periodic
    0,  // ConcurrentDataStructure
    0,  // NoAllocationAlignment
    0   // Unused
};

TRACE_STORE_TRAITS AddressStoreTraits = {
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

TRACE_STORE_TRAITS AddressRangeStoreTraits = {
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
    0,  // PageAligned
    0,  // Periodic
    0,  // ConcurrentDataStructure
    0,  // NoAllocationAlignment
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
    0,  // PageAligned
    0,  // Periodic
    0,  // ConcurrentDataStructure
    0,  // NoAllocationAlignment
    0   // Unused
};

TRACE_STORE_TRAITS SynchronizationStoreTraits = {
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
    0,  // PageAligned
    0,  // Periodic
    0,  // ConcurrentDataStructure
    0,  // NoAllocationAlignment
    0   // Unused
};

// vim:set ts=8 sw=4 sts=4 tw=80 expandtab                                     :
