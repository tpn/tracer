/*++

Copyright (c) 2016 Trent Nelson <trent@trent.me>

Module Name:

    TracerConfigConstants.h

Abstract:

    This module defines constants related to the TracerConfig component.

--*/

#include "stdafx.h"

////////////////////////////////////////////////////////////////////////////////
// TracerConfig Paths
////////////////////////////////////////////////////////////////////////////////

//
// Intermediate paths.
//

CONST UNICODE_STRING x64_Release = \
    RTL_CONSTANT_STRING(L"\\x64\\Release\\");

CONST UNICODE_STRING x64_Debug = \
    RTL_CONSTANT_STRING(L"\\x64\\Debug\\");

CONST UNICODE_STRING x64_PGInstrument = \
    RTL_CONSTANT_STRING(L"\\x64\\PGInstrument\\");

CONST UNICODE_STRING x64_PGOptimize = \
    RTL_CONSTANT_STRING(L"\\x64\\PGOptimize\\");

CONST PUNICODE_STRING IntermediatePaths[] = {
    (CONST PUNICODE_STRING)&x64_Release,
    (CONST PUNICODE_STRING)&x64_Debug,
    (CONST PUNICODE_STRING)&x64_PGInstrument,
    (CONST PUNICODE_STRING)&x64_PGOptimize
};

//
// DLL filenames.
//

CONST UNICODE_STRING AsmDllFilename =
    RTL_CONSTANT_STRING(L"Asm.dll");

CONST UNICODE_STRING RtlDllFilename =
    RTL_CONSTANT_STRING(L"Rtl.dll");

CONST UNICODE_STRING TracerCoreDllFilename =
    RTL_CONSTANT_STRING(L"TracerCore.dll");

CONST UNICODE_STRING PythonDllFilename =
    RTL_CONSTANT_STRING(L"Python.dll");

CONST UNICODE_STRING TracerHeapDllFilename =
    RTL_CONSTANT_STRING(L"TracerHeap.dll");

CONST UNICODE_STRING TraceStoreDllFilename =
    RTL_CONSTANT_STRING(L"TraceStore.dll");

CONST UNICODE_STRING DebugEngineDllFilename =
    RTL_CONSTANT_STRING(L"DebugEngine.dll");

CONST UNICODE_STRING StringTableDllFilename =
    RTL_CONSTANT_STRING(L"StringTable.dll");

CONST UNICODE_STRING PythonTracerDllFilename =
    RTL_CONSTANT_STRING(L"PythonTracer.dll");

CONST UNICODE_STRING TlsTracerHeapDllFilename =
    RTL_CONSTANT_STRING(L"TlsTracerHeap.dll");

CONST UNICODE_STRING InjectionThunkDllFilename =
    RTL_CONSTANT_STRING(L"InjectionThunk.dll");

CONST UNICODE_STRING TracedPythonSessionDllFilename =
    RTL_CONSTANT_STRING(L"TracedPythonSession.dll");

CONST UNICODE_STRING TraceStoreSqlite3ExtDllFilename =
    RTL_CONSTANT_STRING(L"TraceStoreSqlite3Ext.dll");

CONST UNICODE_STRING PythonTracerInjectionDllFilename =
    RTL_CONSTANT_STRING(L"PythonTracerInjection.dll");

#define OFFSET_ENTRY(Name)                      \
    {                                           \
        FIELD_OFFSET(TRACER_PATHS, Name##Path), \
        &##Name##Filename,                      \
    }

CONST TRACER_OFFSET_TO_PATH_ENTRY DllPathOffsets[] = {
    OFFSET_ENTRY(AsmDll),
    OFFSET_ENTRY(RtlDll),
    OFFSET_ENTRY(TracerCoreDll),
    OFFSET_ENTRY(PythonDll),
    OFFSET_ENTRY(TracerHeapDll),
    OFFSET_ENTRY(TraceStoreDll),
    OFFSET_ENTRY(DebugEngineDll),
    OFFSET_ENTRY(StringTableDll),
    OFFSET_ENTRY(PythonTracerDll),
    OFFSET_ENTRY(TlsTracerHeapDll),
    OFFSET_ENTRY(InjectionThunkDll),
    OFFSET_ENTRY(TracedPythonSessionDll),
    OFFSET_ENTRY(TraceStoreSqlite3ExtDll),
    OFFSET_ENTRY(PythonTracerInjectionDll),
    LAST_OFFSET_ENTRY,
};

//
// Subtract 1 to account for the trailing NULL element (LAST_OFFSET_ENTRY).
//
//

CONST USHORT NumberOfDllPathOffsets = ARRAYSIZE(DllPathOffsets) - 1;

CONST TRACER_DLL_PATH_TYPE TracerInjectionDllsBitmap = {
    0, // AsmDllPath
    0, // RtlDllPath
    0, // TracerCoreDllPath
    0, // PythonDllPath
    0, // TracerHeapDllPath
    0, // TraceStoreDllPath
    0, // DebugEngineDllPath
    0, // StringTableDllPath
    0, // PythonTracerDllPath
    0, // TlsTracerHeapDllPath
    0, // InjectionThunkDllPath
    0, // TracedPythonSessionDllPath
    0, // TraceStoreSqlite3ExtDllPath
    1, // PythonTracerInjectionDllPath
    0, // Remaining
};

//
// PTX filenames.
//

CONST UNICODE_STRING TraceStoreKernelsPtxFilename =
    RTL_CONSTANT_STRING(L"TraceStoreKernels.ptx");

CONST TRACER_OFFSET_TO_PATH_ENTRY PtxPathOffsets[] = {
    OFFSET_ENTRY(TraceStoreKernelsPtx),
    LAST_OFFSET_ENTRY
};

//
// Subtract 1 to account for the trailing NULL element (LAST_OFFSET_ENTRY).
//

CONST USHORT NumberOfPtxPathOffsets = ARRAYSIZE(PtxPathOffsets) - 1;

//
// This array can be indexed by a valid TRACER_PATH_TYPE enum value.
//

#define PATH_TYPE_OFFSETS(Name)                             \
    {                                                       \
        ARRAYSIZE(##Name##PathOffsets) - 1,                 \
        (PCTRACER_OFFSET_TO_PATH_ENTRY)##Name##PathOffsets, \
    }

CONST TRACER_PATH_OFFSETS TracerPathOffsets[] = {
    PATH_TYPE_OFFSETS(Dll),
    PATH_TYPE_OFFSETS(Ptx),
};

//
// Non-inline entry points of inline helpers.
//

_Use_decl_annotations_
BOOL
IsLastPathOffset(
    PCTRACER_OFFSET_TO_PATH_ENTRY Entry
    )
{
    return IsLastPathOffsetInline(Entry);
}

_Use_decl_annotations_
BOOL
IsValidTracerPathType(
    TRACER_PATH_TYPE Type
    )
{
    return IsValidTracerPathTypeInline(Type);
}

_Use_decl_annotations_
BOOL
IsValidTracerPathTypeAndIndex(
    TRACER_PATH_TYPE PathType,
    USHORT Index
    )
{
    USHORT Offsets;

    if (!IsValidTracerPathTypeInline(PathType)) {
        return FALSE;
    }

    Offsets = TracerPathOffsets[PathType].NumberOfOffsets;
    return (Index < Offsets);
}

// vim:set ts=8 sw=4 sts=4 tw=80 expandtab                                     :
