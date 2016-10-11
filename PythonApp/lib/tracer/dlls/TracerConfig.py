#===============================================================================
# Imports
#===============================================================================

from ctypes import (
    Union,
    Structure,

    POINTER,
    CFUNCTYPE,
)

from ..wintypes import (
    BOOL,
    USHORT,
    ULONG,
    PVOID,
    SIZE_T,
    HMODULE,
    SRWLOCK,
    LIST_ENTRY,
    UNICODE_STRING,
    PUNICODE_STRING,
)

from .Allocator import (
    ALLOCATOR,
    PALLOCATOR,
    PPALLOCATOR,
    GET_OR_CREATE_GLOBAL_ALLOCATOR
)

#===============================================================================
# Globals
#===============================================================================

#===============================================================================
# Aliases
#===============================================================================

#===============================================================================
# Helpers
#===============================================================================

#===============================================================================
# Structures
#===============================================================================

class TRACER_PATHS(Structure):
    _fields_ = [
        ('Size', USHORT),
        ('Padding', USHORT * 3),
        ('InstallationDirectory', UNICODE_STRING),
        ('BaseTraceDirectory', UNICODE_STRING),
        ('DefaultPythonDirectory', UNICODE_STRING),
        ('RtlDllPath', UNICODE_STRING),
        ('PythonDllPath', UNICODE_STRING),
        ('TracerHeapDllPath', UNICODE_STRING),
        ('TraceStoreDllPath', UNICODE_STRING),
        ('StringTableDllPath', UNICODE_STRING),
        ('PythonTracerDllPath', UNICODE_STRING),
        ('TlsTracerHeapDllPath', UNICODE_STRING),
        ('TracedPythonSessionDllPath', UNICODE_STRING),
    ]
PTRACER_PATHS = POINTER(TRACER_PATHS)

class TRACER_FLAGS(Structure):
    _fields_ = [
        ('DebugBreakOnEntry', ULONG, 1),
        ('LoadDebugLibraries', ULONG, 1),
        ('DisableTraceSessionDirectoryCompression', ULONG, 1),
        ('DisablePrefaultPages', ULONG, 1),
        ('EnableMemoryTracing', ULONG, 1),
        ('EnableIoCounterTracing', ULONG, 1),
        ('EnableHandleCountTracing', ULONG, 1),
        ('DisableFileFlagOverlapped', ULONG, 1),
        ('DisableFileFlagSequentialScan', ULONG, 1),
        ('EnableFileFlagRandomAccess', ULONG, 1),
        ('EnableFileFlagWriteThrough', ULONG, 1),
    ]
PTRACER_FLAGS = POINTER(TRACER_FLAGS)

class TRACE_SESSION_DIRECTORY(Structure):
    _fields_ = [
        ('ListEntry', LIST_ENTRY),
        ('Directory', UNICODE_STRING),
        ('Context', PVOID),
    ]
PTRACE_SESSION_DIRECTORY = POINTER(TRACE_SESSION_DIRECTORY)

class TRACE_SESSION_DIRECTORIES(Structure):
    _fields_ = [
        ('ListHead', LIST_ENTRY),
        ('Lock', SRWLOCK),
        ('Count', ULONG),
    ]
PTRACE_SESSION_DIRECTORIES = POINTER(TRACE_SESSION_DIRECTORIES)

class TRACER_CONFIG(Structure):
    _fields_ = [
        ('Size', USHORT),
        ('Padding1', USHORT),
        ('Flags', TRACER_FLAGS),
        ('Allocator', PALLOCATOR),
        ('TlsTracerHeapModule', HMODULE),
        ('ListEntry', LIST_ENTRY),
        ('Paths', TRACER_PATHS),
        ('TraceSessionDirectories', TRACE_SESSION_DIRECTORIES),
    ]
PTRACER_CONFIG = POINTER(TRACER_CONFIG)

#===============================================================================
# Function Prototypes
#===============================================================================

INITIALIZE_TRACER_CONFIG = CFUNCTYPE(
    PTRACER_CONFIG,
    PALLOCATOR,
    PUNICODE_STRING
)

# vim:set ts=8 sw=4 sts=4 tw=80 ai et                                          :
