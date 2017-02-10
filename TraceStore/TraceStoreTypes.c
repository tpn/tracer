/*++

Copyright (c) 2017 Trent Nelson <trent@trent.me>

Module Name:

    TraceStoreTypes.c

Abstract:

    Each trace store file name follows the following convention:

        <ModuleName>_<RecordType>.dat

    As part of that convention, every module also exports a global structure
    of the named record under the same name if this is the sole structure that
    is written to the trace store.

    For example, the "TraceStore_ModuleTableEntry.dat" file would translate
    to `dt TraceStore!ModuleTableEntry` in Windbg nomenclature, which would
    resolve to this DLL export:

        TRACE_MODULE_TABLE_ENTRY ModuleTableEntry = { 0 };

    The purpose of this file is to contain all relevant type exports.

--*/

#include "stdafx.h"

//
// The following types map directly to types written to trace stores.
//

TRACE_STORE_DATA CONST CHAR StringBuffer = { 0 };
TRACE_STORE_DATA CONST TRACE_PAGE_FAULT PageFault = { 0 };
TRACE_STORE_DATA CONST TRACE_WS_WATCH_INFORMATION_EX WsWatchInfoEx = { 0 };
TRACE_STORE_DATA CONST PSAPI_WORKING_SET_EX_INFORMATION WsWorkingSetExInfo = { 0 };
TRACE_STORE_DATA CONST TRACE_MODULE_TABLE ModuleTable = { 0 };
TRACE_STORE_DATA CONST TRACE_MODULE_TABLE_ENTRY ModuleTableEntry = { 0 };
TRACE_STORE_DATA CONST TRACE_PERFORMANCE Performance = { 0 };
TRACE_STORE_DATA CONST TRACE_PERFORMANCE PerformanceDelta = { 0 };
TRACE_STORE_DATA CONST CHAR SourceCode = { 0 };
TRACE_STORE_DATA CONST RTL_BITMAP Bitmap = { 0 };
TRACE_STORE_DATA CONST BYTE ImageFile = { 0 };
TRACE_STORE_DATA CONST WCHAR UnicodeStringBuffer = { 0 };
TRACE_STORE_DATA CONST STRING Line = { 0 };
TRACE_STORE_DATA CONST PVOID Object = { 0 };
TRACE_STORE_DATA CONST TRACE_MODULE_LOAD_EVENT ModuleLoadEvent = { 0 };
TRACE_STORE_DATA CONST TRACE_SYMBOL_TABLE SymbolTable = { 0 };
TRACE_STORE_DATA CONST TRACE_SYMBOL_TABLE_ENTRY SymbolTableEntry = { 0 };
TRACE_STORE_DATA CONST LINKED_LINE ExamineSymbolsLines = { 0 };
TRACE_STORE_DATA CONST CHAR ExamineSymbolsText = { 0 };
TRACE_STORE_DATA CONST DEBUG_ENGINE_EXAMINED_SYMBOL ExaminedSymbol = { 0 };
TRACE_STORE_DATA CONST LINKED_LINE UnassembleFunctionLines = { 0 };
TRACE_STORE_DATA CONST CHAR UnassembleFunctionText = { 0 };
TRACE_STORE_DATA CONST DEBUG_ENGINE_UNASSEMBLED_FUNCTION UnassembledFunction = { 0 };
TRACE_STORE_DATA CONST LINKED_LINE DisplayTypeLines = { 0 };
TRACE_STORE_DATA CONST CHAR DisplayTypeText = { 0 };
TRACE_STORE_DATA CONST DEBUG_ENGINE_DISPLAYED_TYPE DisplayedType = { 0 };

//
// Miscellaneous types that are also useful to export in order to be picked up
// by the pdb file.
//

TRACE_STORE_DATA CONST TRACE_STORE_MEMORY_MAP TraceStoreMemoryMap = { 0 };
TRACE_STORE_DATA CONST TRACE_STORE TraceStore = { 0 };
TRACE_STORE_DATA CONST TRACE_CONTEXT TraceContext = { 0 };

// vim:set ts=8 sw=4 sts=4 tw=80 expandtab                                     :
