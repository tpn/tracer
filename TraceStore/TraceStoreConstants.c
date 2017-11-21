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
    L"PythonTracer_TraceEventTraitsEx.dat",
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

CONST LPCSTR TraceStoreSqlite3VirtualTableNames[] = {
    "PythonTracer_TraceEvent",
    "PythonTracer_TraceEvent_MetadataInfo",
    "PythonTracer_TraceEvent_Allocation",
    "PythonTracer_TraceEvent_Relocation",
    "PythonTracer_TraceEvent_Address",
    "PythonTracer_TraceEvent_AddressRange",
    "PythonTracer_TraceEvent_AllocationTimestamp",
    "PythonTracer_TraceEvent_AllocationTimestampDelta",
    "PythonTracer_TraceEvent_Synchronization",
    "PythonTracer_TraceEvent_Info",

    "TraceStore_StringBuffer",
    "TraceStore_StringBuffer_MetadataInfo",
    "TraceStore_StringBuffer_Allocation",
    "TraceStore_StringBuffer_Relocation",
    "TraceStore_StringBuffer_Address",
    "TraceStore_StringBuffer_AddressRange",
    "TraceStore_StringBuffer_AllocationTimestamp",
    "TraceStore_StringBuffer_AllocationTimestampDelta",
    "TraceStore_StringBuffer_Synchronization",
    "TraceStore_StringBuffer_Info",

    "Python_PythonFunctionTable",
    "Python_PythonFunctionTable_MetadataInfo",
    "Python_PythonFunctionTable_Allocation",
    "Python_PythonFunctionTable_Relocation",
    "Python_PythonFunctionTable_Address",
    "Python_PythonFunctionTable_AddressRange",
    "Python_PythonFunctionTable_AllocationTimestamp",
    "Python_PythonFunctionTable_AllocationTimestampDelta",
    "Python_PythonFunctionTable_Synchronization",
    "Python_PythonFunctionTable_Info",

    "Python_PythonFunctionTableEntry",
    "Python_PythonFunctionTableEntry_MetadataInfo",
    "Python_PythonFunctionTableEntry_Allocation",
    "Python_PythonFunctionTableEntry_Relocation",
    "Python_PythonFunctionTableEntry_Address",
    "Python_PythonFunctionTableEntry_AddressRange",
    "Python_PythonFunctionTableEntry_AllocationTimestamp",
    "Python_PythonFunctionTableEntry_AllocationTimestampDelta",
    "Python_PythonFunctionTableEntry_Synchronization",
    "Python_PythonFunctionTableEntry_Info",

    "Python_PythonPathTable",
    "Python_PythonPathTable_MetadataInfo",
    "Python_PythonPathTable_Allocation",
    "Python_PythonPathTable_Relocation",
    "Python_PythonPathTable_Address",
    "Python_PythonPathTable_AddressRange",
    "Python_PythonPathTable_AllocationTimestamp",
    "Python_PythonPathTable_AllocationTimestampDelta",
    "Python_PythonPathTable_Synchronization",
    "Python_PythonPathTable_Info",

    "Python_PythonPathTableEntry",
    "Python_PythonPathTableEntry_MetadataInfo",
    "Python_PythonPathTableEntry_Allocation",
    "Python_PythonPathTableEntry_Relocation",
    "Python_PythonPathTableEntry_Address",
    "Python_PythonPathTableEntry_AddressRange",
    "Python_PythonPathTableEntry_AllocationTimestamp",
    "Python_PythonPathTableEntry_AllocationTimestampDelta",
    "Python_PythonPathTableEntry_Synchronization",
    "Python_PythonPathTableEntry_Info",

    "TraceStore_PageFault",
    "TraceStore_PageFault_MetadataInfo",
    "TraceStore_PageFault_Allocation",
    "TraceStore_PageFault_Relocation",
    "TraceStore_PageFault_Address",
    "TraceStore_PageFault_AddressRange",
    "TraceStore_PageFault_AllocationTimestamp",
    "TraceStore_PageFault_AllocationTimestampDelta",
    "TraceStore_PageFault_Synchronization",
    "TraceStore_PageFault_Info",

    "StringTable_StringArray",
    "StringTable_StringArray_MetadataInfo",
    "StringTable_StringArray_Allocation",
    "StringTable_StringArray_Relocation",
    "StringTable_StringArray_Address",
    "StringTable_StringArray_AddressRange",
    "StringTable_StringArray_AllocationTimestamp",
    "StringTable_StringArray_AllocationTimestampDelta",
    "StringTable_StringArray_Synchronization",
    "StringTable_StringArray_Info",

    "StringTable_StringTable",
    "StringTable_StringTable_MetadataInfo",
    "StringTable_StringTable_Allocation",
    "StringTable_StringTable_Relocation",
    "StringTable_StringTable_Address",
    "StringTable_StringTable_AddressRange",
    "StringTable_StringTable_AllocationTimestamp",
    "StringTable_StringTable_AllocationTimestampDelta",
    "StringTable_StringTable_Synchronization",
    "StringTable_StringTable_Info",

    "PythonTracer_EventTraitsEx",
    "PythonTracer_EventTraitsEx_MetadataInfo",
    "PythonTracer_EventTraitsEx_Allocation",
    "PythonTracer_EventTraitsEx_Relocation",
    "PythonTracer_EventTraitsEx_Address",
    "PythonTracer_EventTraitsEx_AddressRange",
    "PythonTracer_EventTraitsEx_AllocationTimestamp",
    "PythonTracer_EventTraitsEx_AllocationTimestampDelta",
    "PythonTracer_EventTraitsEx_Synchronization",
    "PythonTracer_EventTraitsEx_Info",

    "TraceStore_WsWatchInfoEx",
    "TraceStore_WsWatchInfoEx_MetadataInfo",
    "TraceStore_WsWatchInfoEx_Allocation",
    "TraceStore_WsWatchInfoEx_Relocation",
    "TraceStore_WsWatchInfoEx_Address",
    "TraceStore_WsWatchInfoEx_AddressRange",
    "TraceStore_WsWatchInfoEx_AllocationTimestamp",
    "TraceStore_WsWatchInfoEx_AllocationTimestampDelta",
    "TraceStore_WsWatchInfoEx_Synchronization",
    "TraceStore_WsWatchInfoEx_Info",

    "TraceStore_WsWorkingSetExInfo",
    "TraceStore_WsWorkingSetExInfo_MetadataInfo",
    "TraceStore_WsWorkingSetExInfo_Allocation",
    "TraceStore_WsWorkingSetExInfo_Relocation",
    "TraceStore_WsWorkingSetExInfo_Address",
    "TraceStore_WsWorkingSetExInfo_AddressRange",
    "TraceStore_WsWorkingSetExInfo_AllocationTimestamp",
    "TraceStore_WsWorkingSetExInfo_AllocationTimestampDelta",
    "TraceStore_WsWorkingSetExInfo_Synchronization",
    "TraceStore_WsWorkingSetExInfo_Info",

    "TraceStore_CCallStackTable",
    "TraceStore_CCallStackTable_MetadataInfo",
    "TraceStore_CCallStackTable_Allocation",
    "TraceStore_CCallStackTable_Relocation",
    "TraceStore_CCallStackTable_Address",
    "TraceStore_CCallStackTable_AddressRange",
    "TraceStore_CCallStackTable_AllocationTimestamp",
    "TraceStore_CCallStackTable_AllocationTimestampDelta",
    "TraceStore_CCallStackTable_Synchronization",
    "TraceStore_CCallStackTable_Info",

    "TraceStore_CCallStackTableEntry",
    "TraceStore_CCallStackTableEntry_MetadataInfo",
    "TraceStore_CCallStackTableEntry_Allocation",
    "TraceStore_CCallStackTableEntry_Relocation",
    "TraceStore_CCallStackTableEntry_Address",
    "TraceStore_CCallStackTableEntry_AddressRange",
    "TraceStore_CCallStackTableEntry_AllocationTimestamp",
    "TraceStore_CCallStackTableEntry_AllocationTimestampDelta",
    "TraceStore_CCallStackTableEntry_Synchronization",
    "TraceStore_CCallStackTableEntry_Info",

    "TraceStore_ModuleTable",
    "TraceStore_ModuleTable_MetadataInfo",
    "TraceStore_ModuleTable_Allocation",
    "TraceStore_ModuleTable_Relocation",
    "TraceStore_ModuleTable_Address",
    "TraceStore_ModuleTable_AddressRange",
    "TraceStore_ModuleTable_AllocationTimestamp",
    "TraceStore_ModuleTable_AllocationTimestampDelta",
    "TraceStore_ModuleTable_Synchronization",
    "TraceStore_ModuleTable_Info",

    "TraceStore_ModuleTableEntry",
    "TraceStore_ModuleTableEntry_MetadataInfo",
    "TraceStore_ModuleTableEntry_Allocation",
    "TraceStore_ModuleTableEntry_Relocation",
    "TraceStore_ModuleTableEntry_Address",
    "TraceStore_ModuleTableEntry_AddressRange",
    "TraceStore_ModuleTableEntry_AllocationTimestamp",
    "TraceStore_ModuleTableEntry_AllocationTimestampDelta",
    "TraceStore_ModuleTableEntry_Synchronization",
    "TraceStore_ModuleTableEntry_Info",

    "Python_PythonCallStackTable",
    "Python_PythonCallStackTable_MetadataInfo",
    "Python_PythonCallStackTable_Allocation",
    "Python_PythonCallStackTable_Relocation",
    "Python_PythonCallStackTable_Address",
    "Python_PythonCallStackTable_AddressRange",
    "Python_PythonCallStackTable_AllocationTimestamp",
    "Python_PythonCallStackTable_AllocationTimestampDelta",
    "Python_PythonCallStackTable_Synchronization",
    "Python_PythonCallStackTable_Info",

    "Python_PythonCallStackTableEntry",
    "Python_PythonCallStackTableEntry_MetadataInfo",
    "Python_PythonCallStackTableEntry_Allocation",
    "Python_PythonCallStackTableEntry_Relocation",
    "Python_PythonCallStackTableEntry_Address",
    "Python_PythonCallStackTableEntry_AddressRange",
    "Python_PythonCallStackTableEntry_AllocationTimestamp",
    "Python_PythonCallStackTableEntry_AllocationTimestampDelta",
    "Python_PythonCallStackTableEntry_Synchronization",
    "Python_PythonCallStackTableEntry_Info",

    "Python_PythonModuleTable",
    "Python_PythonModuleTable_MetadataInfo",
    "Python_PythonModuleTable_Allocation",
    "Python_PythonModuleTable_Relocation",
    "Python_PythonModuleTable_Address",
    "Python_PythonModuleTable_AddressRange",
    "Python_PythonModuleTable_AllocationTimestamp",
    "Python_PythonModuleTable_AllocationTimestampDelta",
    "Python_PythonModuleTable_Synchronization",
    "Python_PythonModuleTable_Info",

    "Python_PythonModuleTableEntry",
    "Python_PythonModuleTableEntry_MetadataInfo",
    "Python_PythonModuleTableEntry_Allocation",
    "Python_PythonModuleTableEntry_Relocation",
    "Python_PythonModuleTableEntry_Address",
    "Python_PythonModuleTableEntry_AddressRange",
    "Python_PythonModuleTableEntry_AllocationTimestamp",
    "Python_PythonModuleTableEntry_AllocationTimestampDelta",
    "Python_PythonModuleTableEntry_Synchronization",
    "Python_PythonModuleTableEntry_Info",

    "TraceStore_LineTable",
    "TraceStore_LineTable_MetadataInfo",
    "TraceStore_LineTable_Allocation",
    "TraceStore_LineTable_Relocation",
    "TraceStore_LineTable_Address",
    "TraceStore_LineTable_AddressRange",
    "TraceStore_LineTable_AllocationTimestamp",
    "TraceStore_LineTable_AllocationTimestampDelta",
    "TraceStore_LineTable_Synchronization",
    "TraceStore_LineTable_Info",

    "TraceStore_LineTableEntry",
    "TraceStore_LineTableEntry_MetadataInfo",
    "TraceStore_LineTableEntry_Allocation",
    "TraceStore_LineTableEntry_Relocation",
    "TraceStore_LineTableEntry_Address",
    "TraceStore_LineTableEntry_AddressRange",
    "TraceStore_LineTableEntry_AllocationTimestamp",
    "TraceStore_LineTableEntry_AllocationTimestampDelta",
    "TraceStore_LineTableEntry_Synchronization",
    "TraceStore_LineTableEntry_Info",

    "TraceStore_LineStringBuffer",
    "TraceStore_LineStringBuffer_MetadataInfo",
    "TraceStore_LineStringBuffer_Allocation",
    "TraceStore_LineStringBuffer_Relocation",
    "TraceStore_LineStringBuffer_Address",
    "TraceStore_LineStringBuffer_AddressRange",
    "TraceStore_LineStringBuffer_AllocationTimestamp",
    "TraceStore_LineStringBuffer_AllocationTimestampDelta",
    "TraceStore_LineStringBuffer_Synchronization",
    "TraceStore_LineStringBuffer_Info",

    "TraceStore_CallStack",
    "TraceStore_CallStack_MetadataInfo",
    "TraceStore_CallStack_Allocation",
    "TraceStore_CallStack_Relocation",
    "TraceStore_CallStack_Address",
    "TraceStore_CallStack_AddressRange",
    "TraceStore_CallStack_AllocationTimestamp",
    "TraceStore_CallStack_AllocationTimestampDelta",
    "TraceStore_CallStack_Synchronization",
    "TraceStore_CallStack_Info",

    "TraceStore_Performance",
    "TraceStore_Performance_MetadataInfo",
    "TraceStore_Performance_Allocation",
    "TraceStore_Performance_Relocation",
    "TraceStore_Performance_Address",
    "TraceStore_Performance_AddressRange",
    "TraceStore_Performance_AllocationTimestamp",
    "TraceStore_Performance_AllocationTimestampDelta",
    "TraceStore_Performance_Synchronization",
    "TraceStore_Performance_Info",

    "TraceStore_PerformanceDelta",
    "TraceStore_PerformanceDelta_MetadataInfo",
    "TraceStore_PerformanceDelta_Allocation",
    "TraceStore_PerformanceDelta_Relocation",
    "TraceStore_PerformanceDelta_Address",
    "TraceStore_PerformanceDelta_AddressRange",
    "TraceStore_PerformanceDelta_AllocationTimestamp",
    "TraceStore_PerformanceDelta_AllocationTimestampDelta",
    "TraceStore_PerformanceDelta_Synchronization",
    "TraceStore_PerformanceDelta_Info",

    "TraceStore_SourceCode",
    "TraceStore_SourceCode_MetadataInfo",
    "TraceStore_SourceCode_Allocation",
    "TraceStore_SourceCode_Relocation",
    "TraceStore_SourceCode_Address",
    "TraceStore_SourceCode_AddressRange",
    "TraceStore_SourceCode_AllocationTimestamp",
    "TraceStore_SourceCode_AllocationTimestampDelta",
    "TraceStore_SourceCode_Synchronization",
    "TraceStore_SourceCode_Info",

    "TraceStore_Bitmap",
    "TraceStore_Bitmap_MetadataInfo",
    "TraceStore_Bitmap_Allocation",
    "TraceStore_Bitmap_Relocation",
    "TraceStore_Bitmap_Address",
    "TraceStore_Bitmap_AddressRange",
    "TraceStore_Bitmap_AllocationTimestamp",
    "TraceStore_Bitmap_AllocationTimestampDelta",
    "TraceStore_Bitmap_Synchronization",
    "TraceStore_Bitmap_Info",

    "TraceStore_ImageFile",
    "TraceStore_ImageFile_MetadataInfo",
    "TraceStore_ImageFile_Allocation",
    "TraceStore_ImageFile_Relocation",
    "TraceStore_ImageFile_Address",
    "TraceStore_ImageFile_AddressRange",
    "TraceStore_ImageFile_AllocationTimestamp",
    "TraceStore_ImageFile_AllocationTimestampDelta",
    "TraceStore_ImageFile_Synchronization",
    "TraceStore_ImageFile_Info",

    "TraceStore_UnicodeStringBuffer",
    "TraceStore_UnicodeStringBuffer_MetadataInfo",
    "TraceStore_UnicodeStringBuffer_Allocation",
    "TraceStore_UnicodeStringBuffer_Relocation",
    "TraceStore_UnicodeStringBuffer_Address",
    "TraceStore_UnicodeStringBuffer_AddressRange",
    "TraceStore_UnicodeStringBuffer_AllocationTimestamp",
    "TraceStore_UnicodeStringBuffer_AllocationTimestampDelta",
    "TraceStore_UnicodeStringBuffer_Synchronization",
    "TraceStore_UnicodeStringBuffer_Info",

    "TraceStore_Line",
    "TraceStore_Line_MetadataInfo",
    "TraceStore_Line_Allocation",
    "TraceStore_Line_Relocation",
    "TraceStore_Line_Address",
    "TraceStore_Line_AddressRange",
    "TraceStore_Line_AllocationTimestamp",
    "TraceStore_Line_AllocationTimestampDelta",
    "TraceStore_Line_Synchronization",
    "TraceStore_Line_Info",

    "TraceStore_Object",
    "TraceStore_Object_MetadataInfo",
    "TraceStore_Object_Allocation",
    "TraceStore_Object_Relocation",
    "TraceStore_Object_Address",
    "TraceStore_Object_AddressRange",
    "TraceStore_Object_AllocationTimestamp",
    "TraceStore_Object_AllocationTimestampDelta",
    "TraceStore_Object_Synchronization",
    "TraceStore_Object_Info",

    "TraceStore_ModuleLoadEvent",
    "TraceStore_ModuleLoadEvent_MetadataInfo",
    "TraceStore_ModuleLoadEvent_Allocation",
    "TraceStore_ModuleLoadEvent_Relocation",
    "TraceStore_ModuleLoadEvent_Address",
    "TraceStore_ModuleLoadEvent_AddressRange",
    "TraceStore_ModuleLoadEvent_AllocationTimestamp",
    "TraceStore_ModuleLoadEvent_AllocationTimestampDelta",
    "TraceStore_ModuleLoadEvent_Synchronization",
    "TraceStore_ModuleLoadEvent_Info",

    "TraceStore_SymbolTable",
    "TraceStore_SymbolTable_MetadataInfo",
    "TraceStore_SymbolTable_Allocation",
    "TraceStore_SymbolTable_Relocation",
    "TraceStore_SymbolTable_Address",
    "TraceStore_SymbolTable_AddressRange",
    "TraceStore_SymbolTable_AllocationTimestamp",
    "TraceStore_SymbolTable_AllocationTimestampDelta",
    "TraceStore_SymbolTable_Synchronization",
    "TraceStore_SymbolTable_Info",

    "TraceStore_SymbolTableEntry",
    "TraceStore_SymbolTableEntry_MetadataInfo",
    "TraceStore_SymbolTableEntry_Allocation",
    "TraceStore_SymbolTableEntry_Relocation",
    "TraceStore_SymbolTableEntry_Address",
    "TraceStore_SymbolTableEntry_AddressRange",
    "TraceStore_SymbolTableEntry_AllocationTimestamp",
    "TraceStore_SymbolTableEntry_AllocationTimestampDelta",
    "TraceStore_SymbolTableEntry_Synchronization",
    "TraceStore_SymbolTableEntry_Info",

    "TraceStore_SymbolModuleInfo",
    "TraceStore_SymbolModuleInfo_MetadataInfo",
    "TraceStore_SymbolModuleInfo_Allocation",
    "TraceStore_SymbolModuleInfo_Relocation",
    "TraceStore_SymbolModuleInfo_Address",
    "TraceStore_SymbolModuleInfo_AddressRange",
    "TraceStore_SymbolModuleInfo_AllocationTimestamp",
    "TraceStore_SymbolModuleInfo_AllocationTimestampDelta",
    "TraceStore_SymbolModuleInfo_Synchronization",
    "TraceStore_SymbolModuleInfo_Info",

    "TraceStore_SymbolFile",
    "TraceStore_SymbolFile_MetadataInfo",
    "TraceStore_SymbolFile_Allocation",
    "TraceStore_SymbolFile_Relocation",
    "TraceStore_SymbolFile_Address",
    "TraceStore_SymbolFile_AddressRange",
    "TraceStore_SymbolFile_AllocationTimestamp",
    "TraceStore_SymbolFile_AllocationTimestampDelta",
    "TraceStore_SymbolFile_Synchronization",
    "TraceStore_SymbolFile_Info",

    "TraceStore_SymbolInfo",
    "TraceStore_SymbolInfo_MetadataInfo",
    "TraceStore_SymbolInfo_Allocation",
    "TraceStore_SymbolInfo_Relocation",
    "TraceStore_SymbolInfo_Address",
    "TraceStore_SymbolInfo_AddressRange",
    "TraceStore_SymbolInfo_AllocationTimestamp",
    "TraceStore_SymbolInfo_AllocationTimestampDelta",
    "TraceStore_SymbolInfo_Synchronization",
    "TraceStore_SymbolInfo_Info",

    "TraceStore_SymbolLine",
    "TraceStore_SymbolLine_MetadataInfo",
    "TraceStore_SymbolLine_Allocation",
    "TraceStore_SymbolLine_Relocation",
    "TraceStore_SymbolLine_Address",
    "TraceStore_SymbolLine_AddressRange",
    "TraceStore_SymbolLine_AllocationTimestamp",
    "TraceStore_SymbolLine_AllocationTimestampDelta",
    "TraceStore_SymbolLine_Synchronization",
    "TraceStore_SymbolLine_Info",

    "TraceStore_SymbolType",
    "TraceStore_SymbolType_MetadataInfo",
    "TraceStore_SymbolType_Allocation",
    "TraceStore_SymbolType_Relocation",
    "TraceStore_SymbolType_Address",
    "TraceStore_SymbolType_AddressRange",
    "TraceStore_SymbolType_AllocationTimestamp",
    "TraceStore_SymbolType_AllocationTimestampDelta",
    "TraceStore_SymbolType_Synchronization",
    "TraceStore_SymbolType_Info",

    "TraceStore_StackFrame",
    "TraceStore_StackFrame_MetadataInfo",
    "TraceStore_StackFrame_Allocation",
    "TraceStore_StackFrame_Relocation",
    "TraceStore_StackFrame_Address",
    "TraceStore_StackFrame_AddressRange",
    "TraceStore_StackFrame_AllocationTimestamp",
    "TraceStore_StackFrame_AllocationTimestampDelta",
    "TraceStore_StackFrame_Synchronization",
    "TraceStore_StackFrame_Info",

    "TraceStore_TypeInfoTable",
    "TraceStore_TypeInfoTable_MetadataInfo",
    "TraceStore_TypeInfoTable_Allocation",
    "TraceStore_TypeInfoTable_Relocation",
    "TraceStore_TypeInfoTable_Address",
    "TraceStore_TypeInfoTable_AddressRange",
    "TraceStore_TypeInfoTable_AllocationTimestamp",
    "TraceStore_TypeInfoTable_AllocationTimestampDelta",
    "TraceStore_TypeInfoTable_Synchronization",
    "TraceStore_TypeInfoTable_Info",

    "TraceStore_TypeInfoTableEntry",
    "TraceStore_TypeInfoTableEntry_MetadataInfo",
    "TraceStore_TypeInfoTableEntry_Allocation",
    "TraceStore_TypeInfoTableEntry_Relocation",
    "TraceStore_TypeInfoTableEntry_Address",
    "TraceStore_TypeInfoTableEntry_AddressRange",
    "TraceStore_TypeInfoTableEntry_AllocationTimestamp",
    "TraceStore_TypeInfoTableEntry_AllocationTimestampDelta",
    "TraceStore_TypeInfoTableEntry_Synchronization",
    "TraceStore_TypeInfoTableEntry_Info",

    "TraceStore_TypeInfoStringBuffer",
    "TraceStore_TypeInfoStringBuffer_MetadataInfo",
    "TraceStore_TypeInfoStringBuffer_Allocation",
    "TraceStore_TypeInfoStringBuffer_Relocation",
    "TraceStore_TypeInfoStringBuffer_Address",
    "TraceStore_TypeInfoStringBuffer_AddressRange",
    "TraceStore_TypeInfoStringBuffer_AllocationTimestamp",
    "TraceStore_TypeInfoStringBuffer_AllocationTimestampDelta",
    "TraceStore_TypeInfoStringBuffer_Synchronization",
    "TraceStore_TypeInfoStringBuffer_Info",

    "TraceStore_FunctionTable",
    "TraceStore_FunctionTable_MetadataInfo",
    "TraceStore_FunctionTable_Allocation",
    "TraceStore_FunctionTable_Relocation",
    "TraceStore_FunctionTable_Address",
    "TraceStore_FunctionTable_AddressRange",
    "TraceStore_FunctionTable_AllocationTimestamp",
    "TraceStore_FunctionTable_AllocationTimestampDelta",
    "TraceStore_FunctionTable_Synchronization",
    "TraceStore_FunctionTable_Info",

    "TraceStore_FunctionTableEntry",
    "TraceStore_FunctionTableEntry_MetadataInfo",
    "TraceStore_FunctionTableEntry_Allocation",
    "TraceStore_FunctionTableEntry_Relocation",
    "TraceStore_FunctionTableEntry_Address",
    "TraceStore_FunctionTableEntry_AddressRange",
    "TraceStore_FunctionTableEntry_AllocationTimestamp",
    "TraceStore_FunctionTableEntry_AllocationTimestampDelta",
    "TraceStore_FunctionTableEntry_Synchronization",
    "TraceStore_FunctionTableEntry_Info",

    "TraceStore_FunctionAssembly",
    "TraceStore_FunctionAssembly_MetadataInfo",
    "TraceStore_FunctionAssembly_Allocation",
    "TraceStore_FunctionAssembly_Relocation",
    "TraceStore_FunctionAssembly_Address",
    "TraceStore_FunctionAssembly_AddressRange",
    "TraceStore_FunctionAssembly_AllocationTimestamp",
    "TraceStore_FunctionAssembly_AllocationTimestampDelta",
    "TraceStore_FunctionAssembly_Synchronization",
    "TraceStore_FunctionAssembly_Info",

    "TraceStore_FunctionSourceCode",
    "TraceStore_FunctionSourceCode_MetadataInfo",
    "TraceStore_FunctionSourceCode_Allocation",
    "TraceStore_FunctionSourceCode_Relocation",
    "TraceStore_FunctionSourceCode_Address",
    "TraceStore_FunctionSourceCode_AddressRange",
    "TraceStore_FunctionSourceCode_AllocationTimestamp",
    "TraceStore_FunctionSourceCode_AllocationTimestampDelta",
    "TraceStore_FunctionSourceCode_Synchronization",
    "TraceStore_FunctionSourceCode_Info",

    "TraceStore_ExamineSymbolsLine",
    "TraceStore_ExamineSymbolsLine_MetadataInfo",
    "TraceStore_ExamineSymbolsLine_Allocation",
    "TraceStore_ExamineSymbolsLine_Relocation",
    "TraceStore_ExamineSymbolsLine_Address",
    "TraceStore_ExamineSymbolsLine_AddressRange",
    "TraceStore_ExamineSymbolsLine_AllocationTimestamp",
    "TraceStore_ExamineSymbolsLine_AllocationTimestampDelta",
    "TraceStore_ExamineSymbolsLine_Synchronization",
    "TraceStore_ExamineSymbolsLine_Info",

    "TraceStore_ExamineSymbolsText",
    "TraceStore_ExamineSymbolsText_MetadataInfo",
    "TraceStore_ExamineSymbolsText_Allocation",
    "TraceStore_ExamineSymbolsText_Relocation",
    "TraceStore_ExamineSymbolsText_Address",
    "TraceStore_ExamineSymbolsText_AddressRange",
    "TraceStore_ExamineSymbolsText_AllocationTimestamp",
    "TraceStore_ExamineSymbolsText_AllocationTimestampDelta",
    "TraceStore_ExamineSymbolsText_Synchronization",
    "TraceStore_ExamineSymbolsText_Info",

    "TraceStore_ExaminedSymbol",
    "TraceStore_ExaminedSymbol_MetadataInfo",
    "TraceStore_ExaminedSymbol_Allocation",
    "TraceStore_ExaminedSymbol_Relocation",
    "TraceStore_ExaminedSymbol_Address",
    "TraceStore_ExaminedSymbol_AddressRange",
    "TraceStore_ExaminedSymbol_AllocationTimestamp",
    "TraceStore_ExaminedSymbol_AllocationTimestampDelta",
    "TraceStore_ExaminedSymbol_Synchronization",
    "TraceStore_ExaminedSymbol_Info",

    "TraceStore_ExaminedSymbolSecondary",
    "TraceStore_ExaminedSymbolSecondary_MetadataInfo",
    "TraceStore_ExaminedSymbolSecondary_Allocation",
    "TraceStore_ExaminedSymbolSecondary_Relocation",
    "TraceStore_ExaminedSymbolSecondary_Address",
    "TraceStore_ExaminedSymbolSecondary_AddressRange",
    "TraceStore_ExaminedSymbolSecondary_AllocationTimestamp",
    "TraceStore_ExaminedSymbolSecondary_AllocationTimestampDelta",
    "TraceStore_ExaminedSymbolSecondary_Synchronization",
    "TraceStore_ExaminedSymbolSecondary_Info",

    "TraceStore_UnassembleFunctionLine",
    "TraceStore_UnassembleFunctionLine_MetadataInfo",
    "TraceStore_UnassembleFunctionLine_Allocation",
    "TraceStore_UnassembleFunctionLine_Relocation",
    "TraceStore_UnassembleFunctionLine_Address",
    "TraceStore_UnassembleFunctionLine_AddressRange",
    "TraceStore_UnassembleFunctionLine_AllocationTimestamp",
    "TraceStore_UnassembleFunctionLine_AllocationTimestampDelta",
    "TraceStore_UnassembleFunctionLine_Synchronization",
    "TraceStore_UnassembleFunctionLine_Info",

    "TraceStore_UnassembleFunctionText",
    "TraceStore_UnassembleFunctionText_MetadataInfo",
    "TraceStore_UnassembleFunctionText_Allocation",
    "TraceStore_UnassembleFunctionText_Relocation",
    "TraceStore_UnassembleFunctionText_Address",
    "TraceStore_UnassembleFunctionText_AddressRange",
    "TraceStore_UnassembleFunctionText_AllocationTimestamp",
    "TraceStore_UnassembleFunctionText_AllocationTimestampDelta",
    "TraceStore_UnassembleFunctionText_Synchronization",
    "TraceStore_UnassembleFunctionText_Info",

    "TraceStore_UnassembledFunction",
    "TraceStore_UnassembledFunction_MetadataInfo",
    "TraceStore_UnassembledFunction_Allocation",
    "TraceStore_UnassembledFunction_Relocation",
    "TraceStore_UnassembledFunction_Address",
    "TraceStore_UnassembledFunction_AddressRange",
    "TraceStore_UnassembledFunction_AllocationTimestamp",
    "TraceStore_UnassembledFunction_AllocationTimestampDelta",
    "TraceStore_UnassembledFunction_Synchronization",
    "TraceStore_UnassembledFunction_Info",

    "TraceStore_UnassembledFunctionSecondary",
    "TraceStore_UnassembledFunctionSecondary_MetadataInfo",
    "TraceStore_UnassembledFunctionSecondary_Allocation",
    "TraceStore_UnassembledFunctionSecondary_Relocation",
    "TraceStore_UnassembledFunctionSecondary_Address",
    "TraceStore_UnassembledFunctionSecondary_AddressRange",
    "TraceStore_UnassembledFunctionSecondary_AllocationTimestamp",
    "TraceStore_UnassembledFunctionSecondary_AllocationTimestampDelta",
    "TraceStore_UnassembledFunctionSecondary_Synchronization",
    "TraceStore_UnassembledFunctionSecondary_Info",

    "TraceStore_DisplayTypeLine",
    "TraceStore_DisplayTypeLine_MetadataInfo",
    "TraceStore_DisplayTypeLine_Allocation",
    "TraceStore_DisplayTypeLine_Relocation",
    "TraceStore_DisplayTypeLine_Address",
    "TraceStore_DisplayTypeLine_AddressRange",
    "TraceStore_DisplayTypeLine_AllocationTimestamp",
    "TraceStore_DisplayTypeLine_AllocationTimestampDelta",
    "TraceStore_DisplayTypeLine_Synchronization",
    "TraceStore_DisplayTypeLine_Info",

    "TraceStore_DisplayTypeText",
    "TraceStore_DisplayTypeText_MetadataInfo",
    "TraceStore_DisplayTypeText_Allocation",
    "TraceStore_DisplayTypeText_Relocation",
    "TraceStore_DisplayTypeText_Address",
    "TraceStore_DisplayTypeText_AddressRange",
    "TraceStore_DisplayTypeText_AllocationTimestamp",
    "TraceStore_DisplayTypeText_AllocationTimestampDelta",
    "TraceStore_DisplayTypeText_Synchronization",
    "TraceStore_DisplayTypeText_Info",

    "TraceStore_DisplayedType",
    "TraceStore_DisplayedType_MetadataInfo",
    "TraceStore_DisplayedType_Allocation",
    "TraceStore_DisplayedType_Relocation",
    "TraceStore_DisplayedType_Address",
    "TraceStore_DisplayedType_AddressRange",
    "TraceStore_DisplayedType_AllocationTimestamp",
    "TraceStore_DisplayedType_AllocationTimestampDelta",
    "TraceStore_DisplayedType_Synchronization",
    "TraceStore_DisplayedType_Info",

    "TraceStore_DisplayedTypeSecondary",
    "TraceStore_DisplayedTypeSecondary_MetadataInfo",
    "TraceStore_DisplayedTypeSecondary_Allocation",
    "TraceStore_DisplayedTypeSecondary_Relocation",
    "TraceStore_DisplayedTypeSecondary_Address",
    "TraceStore_DisplayedTypeSecondary_AddressRange",
    "TraceStore_DisplayedTypeSecondary_AllocationTimestamp",
    "TraceStore_DisplayedTypeSecondary_AllocationTimestampDelta",
    "TraceStore_DisplayedTypeSecondary_Synchronization",
    "TraceStore_DisplayedTypeSecondary_Info",
};

CONST LPCSTR TraceStoreSqlite3IntervalVirtualTableNames[] = {
    "PythonTracer_TraceEvent_Interval",
    "TraceStore_StringBuffer_Interval",
    "Python_PythonFunctionTable_Interval",
    "Python_PythonFunctionTableEntry_Interval",
    "Python_PythonPathTable_Interval",
    "Python_PythonPathTableEntry_Interval",
    "TraceStore_PageFault_Interval",
    "StringTable_StringArray_Interval",
    "StringTable_StringTable_Interval",
    "PythonTracer_TraceEventTraitsEx_Interval",
    "TraceStore_WsWatchInfoEx_Interval",
    "TraceStore_WsWorkingSetExInfo_Interval",
    "TraceStore_CCallStackTable_Interval",
    "TraceStore_CCallStackTableEntry_Interval",
    "TraceStore_ModuleTable_Interval",
    "TraceStore_ModuleTableEntry_Interval",
    "Python_PythonCallStackTable_Interval",
    "Python_PythonCallStackTableEntry_Interval",
    "Python_PythonModuleTable_Interval",
    "Python_PythonModuleTableEntry_Interval",
    "TraceStore_LineTable_Interval",
    "TraceStore_LineTableEntry_Interval",
    "TraceStore_LineStringBuffer_Interval",
    "TraceStore_CallStack_Interval",
    "TraceStore_Performance_Interval",
    "TraceStore_PerformanceDelta_Interval",
    "TraceStore_SourceCode_Interval",
    "TraceStore_Bitmap_Interval",
    "TraceStore_ImageFile_Interval",
    "TraceStore_UnicodeStringBuffer_Interval",
    "TraceStore_Line_Interval",
    "TraceStore_Object_Interval",
    "TraceStore_ModuleLoadEvent_Interval",
    "TraceStore_SymbolTable_Interval",
    "TraceStore_SymbolTableEntry_Interval",
    "TraceStore_SymbolModuleInfo_Interval",
    "TraceStore_SymbolFile_Interval",
    "TraceStore_SymbolInfo_Interval",
    "TraceStore_SymbolLine_Interval",
    "TraceStore_SymbolType_Interval",
    "TraceStore_StackFrame_Interval",
    "TraceStore_TypeInfoTable_Interval",
    "TraceStore_TypeInfoTableEntry_Interval",
    "TraceStore_TypeStringBuffer_Interval",
    "TraceStore_FunctionTable_Interval",
    "TraceStore_FunctionTableEntry_Interval",
    "TraceStore_FunctionAssembly_Interval",
    "TraceStore_FunctionSourceCode_Interval",
    "TraceStore_ExamineSymbolsLines_Interval",
    "TraceStore_ExamineSymbolsText_Interval",
    "TraceStore_ExaminedSymbol_Interval",
    "TraceStore_ExaminedSymbolSecondary_Interval",
    "TraceStore_UnassembleFunctionLines_Interval",
    "TraceStore_UnassembleFunctionText_Interval",
    "TraceStore_UnassembledFunction_Interval",
    "TraceStore_UnassembledFunctionSecondary_Interval",
    "TraceStore_DisplayTypeLines_Interval",
    "TraceStore_DisplayTypeText_Interval",
    "TraceStore_DisplayedType_Interval",
    "TraceStore_DisplayedTypeSecondary_Interval",
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
        1,  // Compress
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
        1,  // Compress
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
        0,  // Compress
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
        0,  // Compress
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
        0,  // Compress
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
        0,  // Compress
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
        1,  // Compress
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
        0,  // Compress
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
        0,  // Compress
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
        1,  // Compress
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
        1,  // Compress
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
        1,  // Compress
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
        0,  // Compress
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
        0,  // Compress
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
        0,  // Compress
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
        0,  // Compress
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
        0,  // Compress
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
        0,  // Compress
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
        0,  // Compress
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
        0,  // Compress
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
        0,  // Compress
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
        0,  // Compress
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
        0,  // Compress
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
        1,  // Compress
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
        1,  // Compress
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
        1,  // Compress
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
        1,  // Compress
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
        1,  // Compress
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
        1,  // Compress
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
        1,  // Compress
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
        0,  // Compress
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
        0,  // Compress
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
        0,  // Compress
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
        0,  // Compress
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
        0,  // Compress
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
        0,  // Compress
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
        0,  // Compress
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
        0,  // PageAligned
        0,  // Periodic
        0,  // ConcurrentDataStructure
        0,  // NoAllocationAlignment
        1,  // Compress
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
        0,  // PageAligned
        0,  // Periodic
        0,  // ConcurrentDataStructure
        0,  // NoAllocationAlignment
        0,  // Compress
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
        0,  // PageAligned
        0,  // Periodic
        0,  // ConcurrentDataStructure
        0,  // NoAllocationAlignment
        0,  // Compress
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
        0,  // PageAligned
        0,  // Periodic
        0,  // ConcurrentDataStructure
        0,  // NoAllocationAlignment
        0,  // Compress
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
        0,  // Compress
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
        0,  // Compress
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
        0,  // Compress
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
        0,  // Compress
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
        0,  // Compress
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
        0,  // Compress
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
        1,  // Compress
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
        0,  // Compress
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
        0,  // Compress
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
        0,  // Compress
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
        0,  // Compress
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
        0,  // Compress
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
        0,  // Compress
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
        0,  // Compress
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
        0,  // Compress
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
        0,  // Compress
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
        0,  // Compress
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
        0,  // Compress
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
        0,  // Compress
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
    0,  // Compress
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
    1,  // Compress
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
    0,  // Compress
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
    0,  // Compress
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
    1,  // Compress
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
    0,  // Compress
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
    0,  // Compress
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
    0,  // Compress
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
    0,  // Compress
    0   // Unused
};

//
// StringTable-related constants.  Each table consists of a maximum of 16
// strings, as this is the limit of an individual string table.
//

#undef DSTR
#define DSTR(String) String ";"

CONST CHAR TraceStoreSqlite3StringTableDelimiter = ';';

CONST STRING TraceStoreSqlite3FunctionsString = RTL_CONSTANT_STRING(
    DSTR("tscount")
    DSTR("tsavg")
    DSTR("tssum")
    DSTR("tsmin")
    DSTR("tsmax")
);

CONST TRACE_STORE_SQLITE3_FUNCTION TraceStoreSqlite3Functions[] = {

    //
    // tscount, 0 arguments.
    //

    {
        //
        // Id
        //

        CountFunctionId,

        //
        // Flags
        //

        {
            0,  // Unused
        },

        //
        // Name
        //

        "tscount",

        //
        // NumberOfArguments
        //

        0,

        //
        // TextEncodingAndDeterministicFlag
        //

        SQLITE_UTF8 | SQLITE_DETERMINISTIC,

        //
        // ScalarFunction
        //

        NULL,

        //
        // AggregateStepFunction
        //

        TraceStoreSqlite3CountStep,

        //
        // AggregateFinalFunction
        //

        TraceStoreSqlite3CountFinal,

        //
        // DestroyFunction
        //

        TraceStoreSqlite3CountDestroy,

    },

    //
    // tscount, 1 argument.
    //

    {
        //
        // Id
        //

        CountFunctionId,

        //
        // Flags
        //

        {
            0,  // Unused
        },

        //
        // Name
        //

        "tscount",

        //
        // NumberOfArguments
        //

        1,

        //
        // TextEncodingAndDeterministicFlag
        //

        SQLITE_UTF8 | SQLITE_DETERMINISTIC,

        //
        // ScalarFunction
        //

        NULL,

        //
        // AggregateStepFunction
        //

        TraceStoreSqlite3CountStep,

        //
        // AggregateFinalFunction
        //

        TraceStoreSqlite3CountFinal,

        //
        // DestroyFunction
        //

        TraceStoreSqlite3CountDestroy,

    },

    //
    // count, 0 arguments.
    //

    {
        //
        // Id
        //

        CountFunctionId,

        //
        // Flags
        //

        {
            0,  // Unused
        },

        //
        // Name
        //

        "count",

        //
        // NumberOfArguments
        //

        0,

        //
        // TextEncodingAndDeterministicFlag
        //

        SQLITE_UTF8 | SQLITE_DETERMINISTIC,

        //
        // ScalarFunction
        //

        NULL,

        //
        // AggregateStepFunction
        //

        TraceStoreSqlite3CountStep,

        //
        // AggregateFinalFunction
        //

        TraceStoreSqlite3CountFinal,

        //
        // DestroyFunction
        //

        TraceStoreSqlite3CountDestroy,

    },

    //
    // count, 1 argument.
    //

    {
        //
        // Id
        //

        CountFunctionId,

        //
        // Flags
        //

        {
            0,  // Unused
        },

        //
        // Name
        //

        "count",

        //
        // NumberOfArguments
        //

        1,

        //
        // TextEncodingAndDeterministicFlag
        //

        SQLITE_UTF8 | SQLITE_DETERMINISTIC,

        //
        // ScalarFunction
        //

        NULL,

        //
        // AggregateStepFunction
        //

        TraceStoreSqlite3CountStep,

        //
        // AggregateFinalFunction
        //

        TraceStoreSqlite3CountFinal,

        //
        // DestroyFunction
        //

        TraceStoreSqlite3CountDestroy,

    },


    LAST_FUNCTION_ENTRY,

};

// vim:set ts=8 sw=4 sts=4 tw=80 expandtab nowrap                              :
