#===============================================================================
# Imports
#===============================================================================
import sys
import ctypes

from .wintypes import *

#===============================================================================
# Globals/Aliases
#===============================================================================
PPYTHON = PVOID
PPYTRACEFUNC = PVOID
PUSERDATA = PVOID

#===============================================================================
# Classes
#===============================================================================
class TRACE_STORE_METADATA(Structure):
    _fields_ = [
        ('NumberOfRecords', ULARGE_INTEGER),
        ('RecordSize', LARGE_INTEGER),
    ]

PTRACE_STORE_METADATA = POINTER(TRACE_STORE_METADATA)

class _TRACE_STORE_METADATA(Union):
    _fields_ = [
        ('Metadata', TRACE_STORE_METADATA),
        ('pMetadata', PTRACE_STORE_METADATA),
    ]

class TRACE_STORE_MEMORY_MAP(Structure):
    _fields_ = [
        ('SlimReadWriteLock',   SRWLOCK),
        ('MappingHandle',       HANDLE),
        ('MappingSize',         LARGE_INTEGER),
        ('BaseAddress',         PVOID),
        ('ExtendAtAddress',     PVOID),
        ('EndAddress',          PVOID),
        ('PrevAddress',         PVOID),
        ('NextAddress',         PVOID),
    ]
PTRACE_STORE_MEMORY_MAP = POINTER(TRACE_STORE_MEMORY_MAP)

class TRACE_STORE(Structure):
    _fields_ = [
        ('TraceStores', PVOID),
        ('FileHandle', HANDLE),
        ('InitialSize', LARGE_INTEGER),
        ('ExtensionSize', LARGE_INTEGER),
        ('FileInfo', FILE_STANDARD_INFO),
        ('CriticalSection', PCRITICAL_SECTION),
        ('DroppedRecords', ULONG),
        ('MemoryMap', TRACE_STORE_MEMORY_MAP),
        ('NextMemoryMap', TRACE_STORE_MEMORY_MAP),
        ('MetadataStore', PVOID),
        ('AllocateRecords', PVOID),
        ('', _TRACE_STORE_METADATA),
    ]
PTRACE_STORE = POINTER(TRACE_STORE)

class TRACE_STORES(Structure):
    _fields_ = [
        ('Size',                USHORT),
        ('NumberOfTraceStores', USHORT),
        ('Reserved',            ULONG),
        ('Events',              TRACE_STORE),
        ('Frames',              TRACE_STORE),
        ('Modules',             TRACE_STORE),
        ('Functions',           TRACE_STORE),
        ('Exceptions',          TRACE_STORE),
        ('Lines',               TRACE_STORE),
        ('EventsMetadata',      TRACE_STORE),
        ('FramesMetadata',      TRACE_STORE),
        ('ModulesMetadata',     TRACE_STORE),
        ('FunctionsMetadata',   TRACE_STORE),
        ('ExceptionsMetadata',  TRACE_STORE),
        ('LinesMetadata',       TRACE_STORE),
    ]
PTRACE_STORES = POINTER(TRACE_STORES)

class TRACE_SESSION(Structure):
    _fields_ = [
        ('Size',            DWORD),
        ('SessionId',       LARGE_INTEGER),
        ('MachineGuid',     GUID),
        ('Sid',             PVOID),
        ('UserName',        PCWSTR),
        ('ComputerName',    PCWSTR),
        ('DomainName',      PCWSTR),
        ('SystemTime',      FILETIME),
    ]
PTRACE_SESSION = POINTER(TRACE_SESSION)

class TRACE_CONTEXT(Structure):
    _fields_ = [
        ('Size',                ULONG),
        ('SequenceId',          ULONG),
        ('TraceSession',        POINTER(TRACE_SESSION)),
        ('TraceStores',         POINTER(TRACE_STORES)),
        ('SystemTimerFunction', PVOID),
        ('UserData',            PVOID),
    ]
PTRACE_CONTEXT = POINTER(TRACE_CONTEXT)

class PYTHON_TRACE_CONTEXT(Structure):
    _fields_ = [
        ('Size',                    ULONG),
        ('Python',                  PPYTHON),
        ('TraceContext',            PTRACE_CONTEXT),
        ('PythonTraceFunction',     PVOID),
        ('UserData',                PVOID),
    ]
PPYTHON_TRACE_CONTEXT = POINTER(PYTHON_TRACE_CONTEXT)

#===============================================================================
# Functions
#===============================================================================
def vspyprof(path=None, dll=None):
    assert path or dll
    if not dll:
        dll = ctypes.PyDLL(path)

    dll.CreateProfiler.restype = c_void_p
    dll.CreateCustomProfiler.restype = c_void_p
    dll.CreateCustomProfiler.argtypes = [c_void_p, ctypes.c_void_p]
    dll.CloseThread.argtypes = [c_void_p]
    dll.CloseProfiler.argtypes = [c_void_p]
    dll.InitProfiler.argtypes = [c_void_p]
    dll.InitProfiler.restype = c_void_p

    #dll.SetTracing.argtypes = [c_void_p]
    #dll.UnsetTracing.argtypes = [c_void_p]
    #dll.IsTracing.argtypes = [c_void_p]
    #dll.IsTracing.restype = c_bool

    return dll

def pytrace(path=None, dll=None):
    assert path or dll
    dll = vspyprof(path, dll)

    dll.CreateTracer.restype = PVOID
    dll.CreateTracer.argtypes = [PVOID, PVOID]

    dll.InitializeTraceStores.restype = BOOL
    dll.InitializeTraceStores.argtypes = [
        PWSTR,
        PVOID,
        PDWORD,
        PDWORD,
    ]

    return dll

def tracer(path=None, dll=None):
    assert path or dll
    if not dll:
        dll = ctypes.PyDLL(path)

    dll.InitializeTraceStores.restype = BOOL
    dll.InitializeTraceStores.argtypes = [
        PWSTR,
        PVOID,
        PDWORD,
        PDWORD,
    ]

    dll.InitializeTraceContext.restype = BOOL
    dll.InitializeTraceContext.argtypes = [
        PTRACE_CONTEXT,
        PDWORD,
        PTRACE_SESSION,
        PTRACE_STORES,
        PVOID,
    ]

    dll.InitializeTraceSession.restype = BOOL
    dll.InitializeTraceSession.argtypes = [
        PTRACE_SESSION,
        PDWORD
    ]

    #dll.CallSystemTimer.restype = BOOL
    #dll.CallSystemTimer.argtypes = [
    #    PFILETIME,
    #    PVOID,
    #]

    return dll

def python(path=None, dll=None):
    assert path or dll
    if not dll:
        dll = ctypes.PyDLL(path)

    dll.InitializePython.restype = BOOL
    dll.InitializePython.argtypes = [
        HMODULE,
        PVOID,
        PDWORD
    ]

    return dll

def pythontracer(path=None, dll=None):
    assert path or dll
    if not dll:
        dll = ctypes.PyDLL(path)

    dll.InitializePythonTraceContext.restype = BOOL
    dll.InitializePythonTraceContext.argtypes = [
        PPYTHON_TRACE_CONTEXT,
        PULONG,
        PPYTHON,
        PTRACE_CONTEXT,
        PPYTRACEFUNC,
        PUSERDATA
    ]

    dll.StartTracing.restype = BOOL
    dll.StartTracing.argtypes = [ PPYTHON_TRACE_CONTEXT, ]

    dll.StopTracing.restype = BOOL
    dll.StopTracing.argtypes = [ PPYTHON_TRACE_CONTEXT, ]

    return dll

# vim:set ts=8 sw=4 sts=4 tw=80 et                                             :
