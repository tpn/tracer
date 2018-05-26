/*++

Copyright (c) 2016 Trent Nelson <trent@trent.me>

Module Name:

    TracerConfigConstants.h

Abstract:

    This module declares constants related to the TracerConfig component.  It
    contains intermediate path strings (e.g. "x64\\Release", "x64\\Debug"),
    DLL filename strings (e.g. "Rtl.dll"), and a path offset table that maps
    an index offset of a field in the TRACER_CONFIG struct that represents a
    path, to the corresponding path string variable.  This is used to simplify
    loading paths during the creation and initialization of the TRACER_CONFIG
    structure.

--*/

#pragma once

#ifdef __cplusplus
extern "C" {
#endif

#include "stdafx.h"

////////////////////////////////////////////////////////////////////////////////
// TracerConfig Paths
////////////////////////////////////////////////////////////////////////////////

//
// Intermediate paths.
//

TRACER_CONFIG_DATA CONST UNICODE_STRING x64_Release;
TRACER_CONFIG_DATA CONST UNICODE_STRING x64_Debug;
TRACER_CONFIG_DATA CONST UNICODE_STRING x64_PGInstrument;
TRACER_CONFIG_DATA CONST UNICODE_STRING x64_PGOptimize;

//
// DLL filenames.
//

TRACER_CONFIG_DATA CONST UNICODE_STRING AsmDllFilename;
TRACER_CONFIG_DATA CONST UNICODE_STRING RtlDllFilename;
TRACER_CONFIG_DATA CONST UNICODE_STRING PythonDllFilename;
TRACER_CONFIG_DATA CONST UNICODE_STRING TracerCoreDllFilename;
TRACER_CONFIG_DATA CONST UNICODE_STRING TracerHeapDllFilename;
TRACER_CONFIG_DATA CONST UNICODE_STRING TraceStoreDllFilename;
TRACER_CONFIG_DATA CONST UNICODE_STRING DebugEngineDllFilename;
TRACER_CONFIG_DATA CONST UNICODE_STRING StringTableDllFilename;
TRACER_CONFIG_DATA CONST UNICODE_STRING PythonTracerDllFilename;
TRACER_CONFIG_DATA CONST UNICODE_STRING TlsTracerHeapDllFilename;
TRACER_CONFIG_DATA CONST UNICODE_STRING InjectionThunkDllFilename;
TRACER_CONFIG_DATA CONST UNICODE_STRING TracedPythonSessionDllFilename;
TRACER_CONFIG_DATA CONST UNICODE_STRING PythonTracerInjectionDllFilename;
TRACER_CONFIG_DATA CONST UNICODE_STRING PerfectHashTableDllFilename;

//
// PTX filenames.
//

TRACER_CONFIG_DATA CONST UNICODE_STRING TraceStoreKernelsPtxFilename;

//
// A bitmap where each set bit corresponds to the TRACER_DLL_PATH_ID of the same
// value, indicating which DLLs support tracer injection.
//

TRACER_CONFIG_DATA CONST TRACER_DLL_PATH_TYPE TracerInjectionDllsBitmap;

//
// This array can be indexed by the TRACER_BINARY_TYPE_INDEX enum.
//

TRACER_CONFIG_DATA CONST PUNICODE_STRING IntermediatePaths[];

//
// A helper offset table that can be enumerated over in order to ease
// initialization of the various DLL and PTX paths in the TRACER_PATHS struct.
//

typedef struct _TRACER_OFFSET_TO_PATH_ENTRY {
    SHORT Offset;
    PCUNICODE_STRING Filename;
} TRACER_OFFSET_TO_PATH_ENTRY, *PTRACER_OFFSET_TO_PATH_ENTRY;
typedef TRACER_OFFSET_TO_PATH_ENTRY CONST *PCTRACER_OFFSET_TO_PATH_ENTRY;

#define LAST_OFFSET_ENTRY { -1, NULL }

typedef
BOOL
(IS_LAST_PATH_OFFSET)(
    _In_ PCTRACER_OFFSET_TO_PATH_ENTRY Entry
    );
typedef IS_LAST_PATH_OFFSET *PIS_LAST_PATH_OFFSET;
TRACER_CONFIG_API IS_LAST_PATH_OFFSET IsLastPathOffset;

FORCEINLINE
BOOL
IsLastPathOffsetInline(
    _In_ PCTRACER_OFFSET_TO_PATH_ENTRY Entry
    )
{
    return (
        Entry &&
        Entry->Offset == -1 &&
        Entry->Filename == NULL
    );
}

TRACER_CONFIG_DATA CONST TRACER_OFFSET_TO_PATH_ENTRY DllPathOffsets[];
TRACER_CONFIG_DATA CONST TRACER_OFFSET_TO_PATH_ENTRY PtxPathOffsets[];

TRACER_CONFIG_DATA CONST USHORT NumberOfDllPathOffsets;
TRACER_CONFIG_DATA CONST USHORT NumberOfPtxPathOffsets;

//
// Define a structure + array that can be indexed by the path type to derive
// the number of offsets and the corresponding offset+filename entry.
//

typedef struct _TRACER_PATH_OFFSETS {
    USHORT NumberOfOffsets;
    CONST TRACER_OFFSET_TO_PATH_ENTRY *Entries;
} TRACER_PATH_OFFSETS;

TRACER_CONFIG_DATA CONST TRACER_PATH_OFFSETS TracerPathOffsets[];

#ifdef __cplusplus
}; // extern "C" {
#endif

// vim:set ts=8 sw=4 sts=4 tw=80 expandtab                                     :
