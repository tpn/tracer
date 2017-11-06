#===============================================================================
# Imports
#===============================================================================

from ..util import (
    prompt_for_directory,
    list_directories_by_latest,
)

from ..wintypes import (
    cast,
    byref,
    errcheck,
    create_unicode_string,

    Union,
    Structure,

    CDLL,
    BOOL,
    USHORT,
    ULONG,
    PVOID,
    SIZE_T,
    HMODULE,
    SRWLOCK,
    POINTER,
    CFUNCTYPE,
    LIST_ENTRY,
    RTL_BITMAP,
    UNICODE_STRING,
    PUNICODE_STRING,
)

from .Allocator import (
    ALLOCATOR,
    PALLOCATOR,
    PPALLOCATOR,
    GET_OR_CREATE_GLOBAL_ALLOCATOR
)

from .Rtl import (
    PPRTL,
)

#===============================================================================
# Enums
#===============================================================================

TracerReleaseBinaries = 0
TracerDebugBinaries = 1
TracerPGInstrumentedBinaries = 3
TracerPGOptimizedBinaries = 4

#===============================================================================
# Globals
#===============================================================================
TRACER_REGISTRY_KEY = 'Software\\Tracer'

INTERMEDIATE_PATH = {
    TracerReleaseBinaries: 'x64\\Release',
    TracerDebugBinaries: 'x64\\Debug',
    TracerPGInstrumentedBinaries: 'x64\\PGInstrument',
    TracerPGOptimizedBinaries: 'x64\\PGOptimize',
}

#===============================================================================
# Aliases
#===============================================================================

#===============================================================================
# Helpers
#===============================================================================
def get_intermediate_path(winreg, key):

    load_debug = bool(winreg.QueryValueEx(key, 'LoadDebugLibraries')[0])
    load_pgi = bool(winreg.QueryValueEx(key, 'LoadPGInstrumentedLibraries')[0])
    load_pgo = bool(winreg.QueryValueEx(key, 'LoadPGOptimizedLibraries')[0])

    if load_pgo:
        index = TracerPGOptimizedBinaries
    elif load_pgi:
        index = TracerPGInstrumentedBinaries
    elif load_debug:
        index = TracerDebugBinaries
    else:
        index = TracerReleaseBinaries

    return INTERMEDIATE_PATH[index]

def get_tracer_config_dll_path():
    from ..path import join_path
    from ..util import import_winreg

    winreg = import_winreg()
    registry = winreg.ConnectRegistry(None, winreg.HKEY_CURRENT_USER)
    key = winreg.OpenKey(registry, TRACER_REGISTRY_KEY)

    intermediate_path = get_intermediate_path(winreg, key)

    installation_dir = winreg.QueryValueEx(key, 'InstallationDirectory')[0]
    winreg.CloseKey(key)

    dll_base_directory = join_path(installation_dir, intermediate_path)

    path = join_path(dll_base_directory, 'TracerConfig.dll')
    return path

def load_tracer_config_dll():
    path = get_tracer_config_dll_path()
    dll = CDLL(path)
    return dll

#===============================================================================
# Structures
#===============================================================================

class TRACER_PATHS(Structure):
    _fields_ = [
        ('SizeOfStruct', USHORT),
        ('NumberOfDllPaths', USHORT),
        ('NumberOfPtxPaths', USHORT),
        ('Padding', USHORT),
        ('InstallationDirectory', UNICODE_STRING),
        ('BaseTraceDirectory', UNICODE_STRING),
        ('DebuggerSettingsXmlPath', UNICODE_STRING),
        ('AsmDllPath', UNICODE_STRING),
        ('RtlDllPath', UNICODE_STRING),
        ('TracerCoreDllPath', UNICODE_STRING),
        ('PythonDllPath', UNICODE_STRING),
        ('TracerHeapDllPath', UNICODE_STRING),
        ('TraceStoreDllPath', UNICODE_STRING),
        ('DebugEngineDllPath', UNICODE_STRING),
        ('StringTableDllPath', UNICODE_STRING),
        ('PythonTracerDllPath', UNICODE_STRING),
        ('TlsTracerHeapDllPath', UNICODE_STRING),
        ('InjectionThunkDllPath', UNICODE_STRING),
        ('TracedPythonSessionDllPath', UNICODE_STRING),
        ('TraceStoreSqlite3ExtDllPath', UNICODE_STRING),
        ('PythonTracerInjectionDllPath', UNICODE_STRING),
        ('TraceStoreKernelsPtxPath', UNICODE_STRING),
        ('TracerInjectionDllsBitmap', RTL_BITMAP),
        ('TracerInjectionDlls', ULONG),
    ]
PTRACER_PATHS = POINTER(TRACER_PATHS)

class TRACER_FLAGS(Structure):
    _fields_ = [
        ('DebugBreakOnEntry', ULONG, 1),
        ('LoadDebugLibraries', ULONG, 1),
        ('LoadPGInstrumentedLibraries', ULONG, 1),
        ('LoadPGOptimizedLibraries', ULONG, 1),
        ('DisableTraceSessionDirectoryCompression', ULONG, 1),
        ('DisablePrefaultPages', ULONG, 1),
        ('DisableFileFlagOverlapped', ULONG, 1),
        ('DisableFileFlagSequentialScan', ULONG, 1),
        ('EnableFileFlagRandomAccess', ULONG, 1),
        ('EnableFileFlagWriteThrough', ULONG, 1),
        ('EnableWorkingSetTracing', ULONG, 1),
        ('EnablePerformanceTracing', ULONG, 1),
        ('DisableAsynchronousInitialization', ULONG, 1),
    ]
PTRACER_FLAGS = POINTER(TRACER_FLAGS)

class TRACER_RUNTIME_PARAMETERS(Structure):
    _fields_ = [
        ('GetWorkingSetChangesIntervalInMilliseconds', ULONG),
        ('GetWorkingSetChangesWindowLengthInMilliseconds', ULONG),
        ('WsWatchInfoExInitialBufferNumberOfElements', ULONG),
        ('CapturePerformanceMetricsIntervalInMilliseconds', ULONG),
        ('CapturePerformanceMetricsWindowLengthInMilliseconds', ULONG),
        ('ConcurrentAllocationsCriticalSectionSpinCount', ULONG),
        ('IntervalFramesPerSecond', ULONG),
        ('Unused', ULONG),
    ]


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
        ('NumberOfElementsPerTraceStore', USHORT),
        ('SizeOfTraceStoreStructure', USHORT),
        ('Padding', USHORT),
        ('MaximumTraceStoreId', ULONG),
        ('MaximumTraceStoreIndex', ULONG),
        ('Flags', TRACER_FLAGS),
        ('RuntimeParameters', TRACER_RUNTIME_PARAMETERS),
        ('Allocator', PALLOCATOR),
        ('TlsTracerHeapModule', HMODULE),
        ('ListEntry', LIST_ENTRY),
        ('Paths', TRACER_PATHS),
        ('TraceSessionDirectories', TRACE_SESSION_DIRECTORIES),
        ('CuDeviceOrdinal', ULONG),
        ('Padding2', ULONG),
    ]

    def trace_store_directories(self):
        base = self.Paths.BaseTraceDirectory.Buffer
        return list_directories_by_latest(base)

    def choose_trace_store_directory(self):
        base = self.Paths.BaseTraceDirectory.Buffer
        return prompt_for_directory(base)

PTRACER_CONFIG = POINTER(TRACER_CONFIG)
PPTRACER_CONFIG = POINTER(PTRACER_CONFIG)

#===============================================================================
# Function Types
#===============================================================================

INITIALIZE_TRACER_CONFIG = CFUNCTYPE(
    PTRACER_CONFIG,
    PALLOCATOR,
    PUNICODE_STRING
)

CREATE_AND_INITIALIZE_TRACER_CONFIG_AND_RTL = CFUNCTYPE(
    BOOL,
    PALLOCATOR,
    PUNICODE_STRING,
    PPTRACER_CONFIG,
    PPRTL,
)

CREATE_AND_INITIALIZE_ALLOCATOR = CFUNCTYPE(BOOL, PPALLOCATOR)
GET_OR_CREATE_GLOBAL_ALLOCATOR = CFUNCTYPE(BOOL, PPALLOCATOR)

INITIALIZE_TRACER_CONFIG.errcheck = errcheck
CREATE_AND_INITIALIZE_TRACER_CONFIG_AND_RTL.errcheck = errcheck
CREATE_AND_INITIALIZE_ALLOCATOR.errcheck = errcheck
GET_OR_CREATE_GLOBAL_ALLOCATOR.errcheck = errcheck

DEBUG_BREAK = CFUNCTYPE(None)

#===============================================================================
# Binding
#===============================================================================

TracerConfigDll = load_tracer_config_dll()

CreateAndInitializeAllocator = CREATE_AND_INITIALIZE_ALLOCATOR(
    ('CreateAndInitializeDefaultHeapAllocator', TracerConfigDll),
    ((1, 'Allocator'),),
)

GetOrCreateGlobalAllocator = GET_OR_CREATE_GLOBAL_ALLOCATOR(
    ('GetOrCreateGlobalAllocator', TracerConfigDll),
    ((1, 'Allocator'),),
)

InitializeTracerConfig = INITIALIZE_TRACER_CONFIG(
    ('InitializeTracerConfig', TracerConfigDll),
    (
        (1, 'Allocator'),
        (1, 'RegistryKey'),
    ),
)

CreateAndInitializeTracerConfigAndRtl = (
    CREATE_AND_INITIALIZE_TRACER_CONFIG_AND_RTL(
        ('CreateAndInitializeTracerConfigAndRtl', TracerConfigDll),
        (
            (1, 'Allocator'),
            (1, 'RegistryKey'),
            (1, 'TracerConfig'),
            (1, 'Rtl'),
        ),
    )
)

_DebugBreak = DEBUG_BREAK(
    ('_DebugBreak', TracerConfigDll),
)

def load_tracer_config():
    allocator = PALLOCATOR()
    CreateAndInitializeAllocator(byref(allocator))
    registry_key = create_unicode_string(TRACER_REGISTRY_KEY)

    return InitializeTracerConfig(allocator, byref(registry_key)).contents


def load_tracer_config_and_rtl():
    allocator = PALLOCATOR()
    CreateAndInitializeAllocator(byref(allocator))
    registry_key = create_unicode_string(TRACER_REGISTRY_KEY)

    rtl = PRTL()
    tracer_config = PTRACER_CONFIG()

    success = CreateAndInitializeTracerConfigAndRtl(
        allocator,
        byref(registry_key),
        byref(tracer_config),
        byref(rtl),
    )

    return (tracer_config.contents, rtl.contents)

def debugbreak():
    _DebugBreak()

# vim:set ts=8 sw=4 sts=4 tw=80 ai et                                          :
