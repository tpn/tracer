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

//
// Fully-qualified paths.
//

CONST UNICODE_STRING AsmDllPath = \
    RTL_CONSTANT_STRING(L"Asm.dll");

CONST UNICODE_STRING RtlDllPath = \
    RTL_CONSTANT_STRING(L"Rtl.dll");

CONST UNICODE_STRING TracerCoreDllPath = \
    RTL_CONSTANT_STRING(L"TracerCore.dll");

CONST UNICODE_STRING PythonDllPath = \
    RTL_CONSTANT_STRING(L"Python.dll");

CONST UNICODE_STRING TracerHeapDllPath = \
    RTL_CONSTANT_STRING(L"TracerHeap.dll");

CONST UNICODE_STRING TraceStoreDllPath = \
    RTL_CONSTANT_STRING(L"TraceStore.dll");

CONST UNICODE_STRING DebugEngineDllPath = \
    RTL_CONSTANT_STRING(L"DebugEngine.dll");

CONST UNICODE_STRING StringTableDllPath = \
    RTL_CONSTANT_STRING(L"StringTable.dll");

CONST UNICODE_STRING PythonTracerDllPath = \
    RTL_CONSTANT_STRING(L"PythonTracer.dll");

CONST UNICODE_STRING TlsTracerHeapDllPath = \
    RTL_CONSTANT_STRING(L"TlsTracerHeap.dll");

CONST UNICODE_STRING InjectionThunkDllPath = \
    RTL_CONSTANT_STRING(L"InjectionThunk.dll");

CONST UNICODE_STRING TracedPythonSessionDllPath = \
    RTL_CONSTANT_STRING(L"TracedPythonSession.dll");

CONST UNICODE_STRING TraceStoreSqlite3ExtDllPath = \
    RTL_CONSTANT_STRING(L"TraceStoreSqlite3Ext.dll");

CONST UNICODE_STRING PythonTracerInjectionDllPath = \
    RTL_CONSTANT_STRING(L"PythonTracerInjection.dll");

CONST PUNICODE_STRING IntermediatePaths[] = {
    (CONST PUNICODE_STRING)&x64_Release,
    (CONST PUNICODE_STRING)&x64_Debug,
    (CONST PUNICODE_STRING)&x64_PGInstrument,
    (CONST PUNICODE_STRING)&x64_PGOptimize
};

CONST TRACER_DLL_OFFSET_TO_DLL_PATH_ENTRY DllPathOffsets[] = {
    { FIELD_OFFSET(TRACER_PATHS, AsmDllPath),           &AsmDllPath           },
    { FIELD_OFFSET(TRACER_PATHS, RtlDllPath),           &RtlDllPath           },
    { FIELD_OFFSET(TRACER_PATHS, PythonDllPath),        &PythonDllPath        },
    { FIELD_OFFSET(TRACER_PATHS, TracerCoreDllPath),    &TracerCoreDllPath    },
    { FIELD_OFFSET(TRACER_PATHS, TracerHeapDllPath),    &TracerHeapDllPath    },
    { FIELD_OFFSET(TRACER_PATHS, TraceStoreDllPath),    &TraceStoreDllPath    },
    { FIELD_OFFSET(TRACER_PATHS, DebugEngineDllPath),   &DebugEngineDllPath   },
    { FIELD_OFFSET(TRACER_PATHS, StringTableDllPath),   &StringTableDllPath   },
    { FIELD_OFFSET(TRACER_PATHS, PythonTracerDllPath),  &PythonTracerDllPath  },
    { FIELD_OFFSET(TRACER_PATHS, TlsTracerHeapDllPath), &TlsTracerHeapDllPath },
    {
        FIELD_OFFSET(TRACER_PATHS, InjectionThunkDllPath),
        &InjectionThunkDllPath,
    },
    {
        FIELD_OFFSET(TRACER_PATHS, TracedPythonSessionDllPath),
        &TracedPythonSessionDllPath
    },
    {
        FIELD_OFFSET(TRACER_PATHS, TraceStoreSqlite3ExtDllPath),
        &TraceStoreSqlite3ExtDllPath
    },
    {
        FIELD_OFFSET(TRACER_PATHS, PythonTracerInjectionDllPath),
        &PythonTracerInjectionDllPath
    },
    LAST_DLL_OFFSET_ENTRY
};

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
// Subtract 1 to account for the trailing NULL element.
//

CONST USHORT NumberOfDllPathOffsets = (
    (sizeof(DllPathOffsets) /
     sizeof(DllPathOffsets[0])) - 1
);

_Use_decl_annotations_
BOOL
IsLastPathOffset(
    PCTRACER_DLL_OFFSET_TO_DLL_PATH_ENTRY Entry
    )
{
    return IsLastPathOffsetInline(Entry);
}

// vim:set ts=8 sw=4 sts=4 tw=80 expandtab                                     :
