/*++

Copyright (c) 2017 Trent Nelson <trent@trent.me>

Module Name:

    TraceStoreTypes.c

Abstract:

    Each trace store file name follows the following convention:

        <ModuleName>_<RecordType>.dat

    As part of that convention, every module also exports a global pointer to
    the structure of the named record under the same name if this is the sole
    structure that is written to the trace store.

    For example, the "TraceStore_ModuleTableEntry.dat" file would translate
    to `dt TraceStore!ModuleTableEntry` in Windbg nomenclature, which would
    resolve to this DLL export:

        PTRACE_MODULE_TABLE_ENTRY ModuleTableEntry = 0;

    The purpose of this file is to contain all relevant type exports.

--*/

#include "stdafx.h"

//
// The following types map directly to types written to trace stores.
//

TRACE_STORE_DATA CONST PCHAR StringBuffer = 0;
TRACE_STORE_DATA CONST PTRACE_PAGE_FAULT PageFault = 0;
TRACE_STORE_DATA CONST PTRACE_WS_WATCH_INFORMATION_EX WsWatchInfoEx = 0;
TRACE_STORE_DATA CONST PPSAPI_WORKING_SET_EX_INFORMATION WsWorkingSetExInfo = 0;
TRACE_STORE_DATA CONST PTRACE_MODULE_TABLE ModuleTable = 0;
TRACE_STORE_DATA CONST PTRACE_MODULE_TABLE_ENTRY ModuleTableEntry = 0;
TRACE_STORE_DATA CONST PTRACE_PERFORMANCE Performance = 0;
TRACE_STORE_DATA CONST PTRACE_PERFORMANCE PerformanceDelta = 0;
TRACE_STORE_DATA CONST PCHAR SourceCode = 0;
TRACE_STORE_DATA CONST PRTL_BITMAP Bitmap = 0;
TRACE_STORE_DATA CONST PBYTE ImageFile = 0;
TRACE_STORE_DATA CONST PWCHAR UnicodeStringBuffer = 0;
TRACE_STORE_DATA CONST PSTRING Line = 0;
TRACE_STORE_DATA CONST PPVOID Object = 0;
TRACE_STORE_DATA CONST PTRACE_MODULE_LOAD_EVENT ModuleLoadEvent = 0;
TRACE_STORE_DATA CONST PTRACE_SYMBOL_TABLE SymbolTable = 0;
TRACE_STORE_DATA CONST PTRACE_SYMBOL_TABLE_ENTRY SymbolTableEntry = 0;
TRACE_STORE_DATA CONST PLINKED_LINE ExamineSymbolsLines = 0;
TRACE_STORE_DATA CONST PCHAR ExamineSymbolsText = 0;
TRACE_STORE_DATA CONST PDEBUG_ENGINE_EXAMINED_SYMBOL ExaminedSymbol = 0;
TRACE_STORE_DATA CONST PLINKED_LINE UnassembleFunctionLines = 0;
TRACE_STORE_DATA CONST PCHAR UnassembleFunctionText = 0;
TRACE_STORE_DATA CONST PDEBUG_ENGINE_UNASSEMBLED_FUNCTION UnassembledFunction = 0;
TRACE_STORE_DATA CONST PLINKED_LINE DisplayTypeLines = 0;
TRACE_STORE_DATA CONST PCHAR DisplayTypeText = 0;
TRACE_STORE_DATA CONST PDEBUG_ENGINE_DISPLAYED_TYPE DisplayedType = 0;

//
// Miscellaneous types that are also useful to export in order to be picked up
// by the pdb file.
//

TRACE_STORE_DATA CONST PTRACE_STORE_MEMORY_MAP TraceStoreMemoryMap = 0;
TRACE_STORE_DATA CONST PTRACE_STORE TraceStore = 0;
TRACE_STORE_DATA CONST PTRACE_CONTEXT TraceContext = 0;

// vim:set ts=8 sw=4 sts=4 tw=80 expandtab nowrap                              :
