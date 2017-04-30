/*++

Copyright (c) 2017 Trent Nelson <trent@trent.me>

Module Name:

    TraceStoreTypes.c

Abstract:

    This module exposes trace store and metadata store types.

--*/

#include "stdafx.h"

//
// XXX: I have no idea where _PREFAST_ was defined by the time we hit this
//      point (StringTable and Python don't experience this complication),
//      nevertheless, if it's defined, UNREFERENCED_PARAMETER() bombs out
//      with linking errors.  Explicitly undefine then redefine it here.
//

#ifdef UNREFERENCED_PARAMETER
#undef UNREFERENCED_PARAMETER
#endif

#define UNREFERENCED_PARAMETER(P) (P)

//
// Define the trace store types owned by this module.
//

//
// N.B. Keep these sorted alphabetically.  In vim, mark the start and end brace
//      with ma and mb, then issue the command:
//
//          :'a,'b !sort -b -k 2
//

typedef struct _TRACE_STORE_TYPES {
    PRTL_BITMAP Bitmap;
    PDEBUG_ENGINE_DISPLAYED_TYPE DisplayedType;
    PLINKED_LINE DisplayTypeLines;
    PCHAR DisplayTypeText;
    PDEBUG_ENGINE_EXAMINED_SYMBOL ExaminedSymbol;
    PLINKED_LINE ExamineSymbolsLines;
    PCHAR ExamineSymbolsText;
    PTRACE_FUNCTION_TABLE FunctionTable;
    PTRACE_FUNCTION_TABLE_ENTRY FunctionTableEntry;
    PBYTE ImageFile;
    PSTRING Line;
    PTRACE_MODULE_LOAD_EVENT ModuleLoadEvent;
    PTRACE_MODULE_TABLE ModuleTable;
    PTRACE_MODULE_TABLE_ENTRY ModuleTableEntry;
    PPVOID Object;
    PTRACE_PAGE_FAULT PageFault;
    PTRACE_PERFORMANCE Performance;
    PTRACE_PERFORMANCE PerformanceDelta;
    PCHAR SourceCode;
    PSRCCODEINFOW SourceCodeInfo;
    PSOURCEFILEW SourceFile;
    PCHAR StringBuffer;
    PSYMBOL_INFO SymbolInfo;
    PTRACE_SYMBOL_TABLE SymbolTable;
    PTRACE_SYMBOL_TABLE_ENTRY SymbolTableEntry;
    PDEBUG_ENGINE_UNASSEMBLED_FUNCTION UnassembledFunction;
    PLINKED_LINE UnassembleFunctionLines;
    PCHAR UnassembleFunctionText;
    PWCHAR UnicodeStringBuffer;
    PTRACE_WS_WATCH_INFORMATION_EX WsWatchInfoEx;
    PPSAPI_WORKING_SET_EX_INFORMATION WsWorkingSetExInfo;
} TRACE_STORE_TYPES;
typedef TRACE_STORE_TYPES *PTRACE_STORE_TYPES;

//
// Define the trace store metadata types.
//

typedef struct _METADATA_STORE_TYPES {
    PTRACE_STORE_ADDRESS Address;
    PTRACE_STORE_ADDRESS_RANGE AddressRange;
    PTRACE_STORE_ALLOCATION Allocation;
    PTRACE_STORE_ALLOCATION_TIMESTAMP AllocationTimestamp;
    PTRACE_STORE_ALLOCATION_TIMESTAMP_DELTA AllocationTimestampDelta;
    PTRACE_STORE_INFO Info;
    PTRACE_STORE_METADATA_INFO MetadataInfo;
    PTRACE_STORE_RELOC Relocation;
    PTRACE_STORE_SYNC Synchronization;
} METADATA_STORE_TYPES;
typedef METADATA_STORE_TYPES *PMETADATA_STORE_TYPES;

//
// Define other notable module types that are useful to export.
//

typedef struct _MODULE_TYPES {
    PTRACE_CONTEXT TraceContext;
    PTRACE_DEBUG_CONTEXT TraceDebugContext;
    PTRACE_STORE TraceStore;
    PTRACE_STORE_MEMORY_MAP TraceStoreMemoryMap;
    PTRACE_STORE_METADATA_STORES TraceStoreMetadataStores;
    PTRACE_STORE TraceStores;
    PTRACE_SYMBOL_CONTEXT TraceSymbolContext;
} MODULE_TYPES;
typedef MODULE_TYPES *PMODULE_TYPES;

//
// Define notable function pointer types.
//

typedef struct _FUNCTION_TYPES {
    PALLOCATE_RECORDS AllocateRecords;
    PALLOCATE_RECORDS_WITH_TIMESTAMP AllocateRecordsWithTimestamp;
    PBIND_COMPLETE BindComplete;
    PTRY_ALLOCATE_RECORDS TryAllocateRecords;
    PTRY_ALLOCATE_RECORDS_WITH_TIMESTAMP TryAllocateRecordsWithTimestamp;
} FUNCTION_TYPES;
typedef FUNCTION_TYPES *PFUNCTION_TYPES;

//
// Type exposure functions.
//

DECLSPEC_DLLEXPORT
VOID
TraceStoreTypes(
    PTRACE_STORE_TYPES Types
    )
{
    UNREFERENCED_PARAMETER(Types);
}

DECLSPEC_DLLEXPORT
VOID
MetadataStoreTypes(
    PMETADATA_STORE_TYPES Types
    )
{
    UNREFERENCED_PARAMETER(Types);
}

DECLSPEC_DLLEXPORT
VOID
ModuleTypes(
    PMODULE_TYPES Types
    )
{
    UNREFERENCED_PARAMETER(Types);
}

DECLSPEC_DLLEXPORT
VOID
FunctionTypes(
    PFUNCTION_TYPES Types
    )
{
    UNREFERENCED_PARAMETER(Types);
}

// vim:set ts=8 sw=4 sts=4 tw=80 expandtab                                     :
