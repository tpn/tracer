#include "TracerConfigPrivate.h"

#include "../Rtl/Rtl.h"

////////////////////////////////////////////////////////////////////////////////
// TracerConfig Paths
////////////////////////////////////////////////////////////////////////////////

//
// Static UNICODE_STRING instances for path constants.
//

static CONST UNICODE_STRING x64_Release = \
    RTL_CONSTANT_STRING(L"\\x64\\Release\\");

static CONST UNICODE_STRING x64_Debug = \
    RTL_CONSTANT_STRING(L"\\x64\\Debug\\");

static CONST UNICODE_STRING RtlDllPath = \
    RTL_CONSTANT_STRING(L"Rtl.dll");

static CONST UNICODE_STRING TracerDllPath = \
    RTL_CONSTANT_STRING(L"Tracer.dll");

static CONST UNICODE_STRING PythonDllPath = \
    RTL_CONSTANT_STRING(L"Python.dll");

static CONST UNICODE_STRING PythonTracerDllPath = \
    RTL_CONSTANT_STRING(L"PythonTracer.dll");

static CONST UNICODE_STRING TlsTracerHeapDllPath = \
    RTL_CONSTANT_STRING(L"TlsTracerHeap.dll");

//
// This array can be indexed by TracerConfig.Flags.LoadDebugLibraries
// to obtain the appropriate intermediate path string for the given
// setting.
//

static CONST PUNICODE_STRING IntermediatePaths[] = {
    (CONST PUNICODE_STRING)&x64_Release,
    (CONST PUNICODE_STRING)&x64_Debug
};

//
// A helper offset table that can be enumerated over in order to ease
// initialization of the various TRACER_PATHS DllPath strings.
//

static CONST struct {
    USHORT Offset;
    PCUNICODE_STRING DllPath;
} PathOffsets[] = {
    { FIELD_OFFSET(TRACER_PATHS, RtlDllPath),           &RtlDllPath           },
    { FIELD_OFFSET(TRACER_PATHS, TracerDllPath),        &TracerDllPath        },
    { FIELD_OFFSET(TRACER_PATHS, PythonDllPath),        &PythonDllPath        },
    { FIELD_OFFSET(TRACER_PATHS, PythonTracerDllPath),  &PythonTracerDllPath  },
    { FIELD_OFFSET(TRACER_PATHS, TlsTracerHeapDllPath), &TlsTracerHeapDllPath }
};

static CONST USHORT NumberOfPathOffsets = (
    sizeof(PathOffsets) /
    sizeof(PathOffsets[0])
);

// vim:set ts=8 sw=4 sts=4 tw=80 expandtab                                     :
