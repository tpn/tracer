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
// Fully-qualified paths.
//

TRACER_CONFIG_DATA CONST UNICODE_STRING RtlDllPath;
TRACER_CONFIG_DATA CONST UNICODE_STRING TrlDllPath;
TRACER_CONFIG_DATA CONST UNICODE_STRING PythonDllPath;
TRACER_CONFIG_DATA CONST UNICODE_STRING TracerHeapDllPath;
TRACER_CONFIG_DATA CONST UNICODE_STRING TraceStoreDllPath;
TRACER_CONFIG_DATA CONST UNICODE_STRING DebugEngineDllPath;
TRACER_CONFIG_DATA CONST UNICODE_STRING StringTableDllPath;
TRACER_CONFIG_DATA CONST UNICODE_STRING PythonTracerDllPath;
TRACER_CONFIG_DATA CONST UNICODE_STRING TlsTracerHeapDllPath;
TRACER_CONFIG_DATA CONST UNICODE_STRING TracedPythonSessionDllPath;

//
// This array can be indexed by the TRACER_BINARY_TYPE_INDEX enum.
//

TRACER_CONFIG_DATA CONST PUNICODE_STRING IntermediatePaths[];

//
// A helper offset table that can be enumerated over in order to ease
// initialization of the various TRACER_PATHS DllPath strings.
//

typedef struct _TRACER_DLL_OFFSET_TO_DLL_PATH_ENTRY {
    SHORT Offset;
    PCUNICODE_STRING DllPath;
} TRACER_DLL_OFFSET_TO_DLL_PATH_ENTRY, *PTRACER_DLL_OFFSET_TO_DLL_PATH_ENTRY;
typedef TRACER_DLL_OFFSET_TO_DLL_PATH_ENTRY CONST \
     *PCTRACER_DLL_OFFSET_TO_DLL_PATH_ENTRY;

TRACER_CONFIG_DATA CONST TRACER_DLL_OFFSET_TO_DLL_PATH_ENTRY \
                         DllPathOffsets[];

TRACER_CONFIG_DATA CONST USHORT NumberOfDllPathOffsets;

#define LAST_DLL_OFFSET_ENTRY { -1, NULL }

typedef
BOOL
(IS_LAST_PATH_OFFSET)(
    _In_ PCTRACER_DLL_OFFSET_TO_DLL_PATH_ENTRY Entry
    );
typedef IS_LAST_PATH_OFFSET *PIS_LAST_PATH_OFFSET;
TRACER_CONFIG_API IS_LAST_PATH_OFFSET IsLastPathOffset;

FORCEINLINE
BOOL
IsLastPathOffsetInline(
    _In_ PCTRACER_DLL_OFFSET_TO_DLL_PATH_ENTRY Entry
    )
{
    return (
        Entry &&
        Entry->Offset == -1 &&
        Entry->DllPath == NULL
    );
}

#ifdef __cplusplus
}; // extern "C" {
#endif

// vim:set ts=8 sw=4 sts=4 tw=80 expandtab                                     :
