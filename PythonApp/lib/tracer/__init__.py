__version__ = '0.1'

#===============================================================================
# Imports
#===============================================================================
import sys
from functools import partial

import ctypes
from ctypes import *
from ctypes.wintypes import *

from .wintypes import *

from .util import NullObject

from .path import join_path

from multiprocessing import cpu_count

#===============================================================================
# Globals
#===============================================================================
TRACER = None

#===============================================================================
# Aliases
#===============================================================================
PPYTHON = PVOID
PPYTRACEFUNC = PVOID
PUSERDATA = PVOID

#===============================================================================
# Helpers
#===============================================================================

#===============================================================================
# ctypes Wrappers
#===============================================================================
class RTL(Structure):
    _fields_ = [
        ('Size', ULONG),
        ('NtdllModule', HMODULE),
        ('Kernel32Module', HMODULE),
        ('GetSystemTimePreciseAsFileTime', PVOID),
        ('NtQuerySystemTime', PVOID),
        ('RtlCharToInteger', PVOID),
        ('__C_specific_handler', PVOID),
        ('CopyToMappedMemory', PVOID),
    ]
PRTL = POINTER(RTL)
PRTL = PVOID

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
        ('CriticalSection',     CRITICAL_SECTION),
        ('FileHandle',          HANDLE),
        ('FileInfo',            FILE_STANDARD_INFO),
        ('CurrentFilePointer',  LARGE_INTEGER),
        ('MappingHandle',       HANDLE),
        ('MappingSize',         LARGE_INTEGER),
        ('BaseAddress',         PVOID),
        ('PrevAddress',         PVOID),
        ('NextAddress',         PVOID),
    ]
PTRACE_STORE_MEMORY_MAP = POINTER(TRACE_STORE_MEMORY_MAP)

class TRACE_STORE(Structure):
    _fields_ = [
        ('TraceContext', PVOID),
        ('InitialSize', LARGE_INTEGER),
        ('ExtensionSize', LARGE_INTEGER),
        ('MaximumSize', LARGE_INTEGER),
        ('DroppedRecords', ULONG),
        ('PrefaultFuturePageWork', PTP_WORK),
        ('ExtendFileWork', PTP_WORK),
        ('FileExtendedEvent', HANDLE),
        ('TraceStoreMemoryMap', TRACE_STORE_MEMORY_MAP),
        ('NextTraceStoreMemoryMap', TRACE_STORE_MEMORY_MAP),
        ('LastTraceStoreMemoryMap', TRACE_STORE_MEMORY_MAP),
        ('MetadataStore', PVOID),
        ('AllocateRecords', PVOID),
        ('s', _TRACE_STORE_METADATA),
    ]
PTRACE_STORE = POINTER(TRACE_STORE)

class TRACE_STORES_OLD(Structure):
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
PTRACE_STORES_OLD = POINTER(TRACE_STORES_OLD)

class TRACE_STORES(Structure):
    _fields_ = [
        ('Size',                USHORT),
        ('NumberOfTraceStores', USHORT),
        ('Reserved',            ULONG),
        ('Stores',              TRACE_STORE * 12),
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

    @classmethod
    def create(cls):
        trace_session = cls()
        trace_session.Size = sizeof(TRACE_SESSION)
        trace_session.SessionId = 1
        return trace_session

PTRACE_SESSION = POINTER(TRACE_SESSION)

class TRACE_CONTEXT(Structure):
    _fields_ = [
        ('Size',                            ULONG),
        ('SequenceId',                      ULONG),
        ('TraceSession',                    POINTER(TRACE_SESSION)),
        ('TraceStores',                     POINTER(TRACE_STORES)),
        ('SystemTimerFunction',             PVOID),
        ('PerformanceCounterFrequency',     LARGE_INTEGER),
        ('UserData',                        PVOID),
        ('ThreadpoolCallbackEnvironment',   PTP_CALLBACK_ENVIRON),
        ('HeapHandle',                      HANDLE),
    ]
PTRACE_CONTEXT = POINTER(TRACE_CONTEXT)
PTRACE_CONTEXT = PVOID

class PYTHON_TRACE_CONTEXT(Structure):
    _fields_ = [
        ('Size',                    ULONG),
        ('Python',                  PPYTHON),
        ('TraceContext',            PTRACE_CONTEXT),
        ('PythonTraceFunction',     PVOID),
        ('UserData',                PVOID),
        ('FunctionObject',          PVOID),
    ]
PPYTHON_TRACE_CONTEXT = POINTER(PYTHON_TRACE_CONTEXT)
PPYTHON_TRACE_CONTEXT = PVOID

class _PYTHON_TRACE_EVENT_TIME_FIELD(Union):
    _fields_ = [
        ('Timestamp',   LARGE_INTEGER),
        ('Elapsed',     LARGE_INTEGER),
    ]

class _PYTHON_TRACE_EVENT_TYPE_BITFIELD(Structure):
    _fields_ = [
        ('IsCall',          ULONG, 1),
        ('IsException',     ULONG, 1),
        ('IsLine',          ULONG, 1),
        ('IsReturn',        ULONG, 1),
        ('IsC',             ULONG, 1),
        ('IsReverseJump',   ULONG, 1),
    ]

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
        PRTL,
        PWSTR,
        PVOID,
        PDWORD,
        PDWORD,
    ]

    return dll

def rtl(path=None, dll=None):
    assert path or dll
    if not dll:
        dll = ctypes.PyDLL(path)

    dll.InitializeRtl.restype = BOOL
    dll.InitializeRtl.argtypes = [
        PVOID,
        PULONG,
    ]

    return dll

def tracer(path=None, dll=None):
    assert path or dll
    if not dll:
        dll = ctypes.PyDLL(path)

    dll.InitializeTraceStores.restype = BOOL
    dll.InitializeTraceStores.argtypes = [
        PVOID, #PRTL,
        PWSTR,
        PVOID,
        PDWORD,
        PDWORD,
        PVOID,  # Readonly, should be BOOL
    ]

    dll.InitializeTraceContext.restype = BOOL
    dll.InitializeTraceContext.argtypes = [
        PVOID, #PRTL,
        PVOID, #PTRACE_CONTEXT,
        PDWORD,
        PVOID, #PTRACE_SESSION,
        PVOID, #PTRACE_STORES,
        PVOID,
        PVOID,
    ]

    dll.InitializeTraceSession.restype = BOOL
    dll.InitializeTraceSession.argtypes = [
        PVOID, #PRTL,
        PVOID, #PTRACE_SESSION,
        PDWORD
    ]

    dll.SubmitTraceStoreFileExtensionThreadpoolWork.restype = None
    dll.SubmitTraceStoreFileExtensionThreadpoolWork.argtypes = [ PTRACE_STORE, ]

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
        PVOID,      # Rtl
        HMODULE,    # PythonModule,
        PVOID,      # Python
        PULONG,     # SizeOfPython
    ]

    dll.InitializePythonRuntimeTables.restype = BOOL
    dll.InitializePythonRuntimeTables.argtypes = [
        PVOID,  # Python
        PVOID,  # AllocationRoutine
        PVOID,  # AllocationContext
        PVOID,  # FreeRoutine
        PVOID,  # FreeContext
    ]

    return dll

def pythontracer(path=None, dll=None):
    assert path or dll
    if not dll:
        dll = ctypes.PyDLL(path)

    dll.InitializePythonTraceContext.restype = BOOL
    dll.InitializePythonTraceContext.argtypes = [
        PVOID, #PRTL,
        PVOID, #PPYTHON_TRACE_CONTEXT,
        PULONG,
        PPYTHON,
        PTRACE_CONTEXT,
        PPYTRACEFUNC,
        PUSERDATA
    ]

    dll.AddFunction.restype = BOOL
    #dll.AddFunction.argtypes = [ PVOID, ]

    dll.AddModuleName.restype = BOOL
    dll.AddModuleName.argtypes = [ PVOID, py_object ]

    dll.EnableMemoryTracing.restype = VOID
    dll.EnableMemoryTracing.argtypes = [ PPYTHON_TRACE_CONTEXT, ]

    dll.DisableMemoryTracing.restype = VOID
    dll.DisableMemoryTracing.argtypes = [ PPYTHON_TRACE_CONTEXT, ]

    dll.StartTracing.restype = BOOL
    dll.StartTracing.argtypes = [ PPYTHON_TRACE_CONTEXT, ]

    dll.StopTracing.restype = BOOL
    dll.StopTracing.argtypes = [ PPYTHON_TRACE_CONTEXT, ]

    return dll

def sqlite3(path=None, dll=None):
    assert path or dll
    if not dll:
        try:
            dll = ctypes.PyDLL(path)
        except:
            dll = None

    return dll

def tracersqlite(path=None, dll=None):
    assert path or dll
    if not dll:
        dll = ctypes.PyDLL(path)

    return dll

#===============================================================================
# Decorators
#===============================================================================
class trace:
    def __init__(self, func):
        self.func = func

    def __get__(self, obj, objtype=None):
        if obj is None:
            return self.func
        return partial(self, obj)

    def __call__(self, *args, **kw):
        global TRACER
        tracer = TRACER
        if not tracer:
            tracer = NullObject()

        tracer.add_function(self.func)
        tracer.start()
        result = self.func(*args, **kwds)
        tracer.stop()
        return result

#===============================================================================
# Classes
#===============================================================================
class TracerError(BaseException):
    pass

class Tracer:
    def __init__(self,
                 basedir,
                 tracer_dll_path,
                 tracer_rtl_dll_path,
                 tracer_sqlite3_dll_path,
                 tracer_tracersqlite_dll_path,
                 tracer_python_dll_path,
                 tracer_pythontracer_dll_path,
                 threadpool=None,
                 threadpool_callback_environment=None):

        self.basedir = basedir
        self.system_dll = sys.dllhandle

        self.tracer_sqlitedb_path = join_path(basedir, "trace.db")

        self.tracer_dll_path = tracer_dll_path
        self.tracer_rtl_dll_path = tracer_rtl_dll_path
        self.tracer_sqlite3_dll_path = tracer_sqlite3_dll_path
        self.tracer_tracersqlite_dll_path = tracer_tracersqlite_dll_path
        self.tracer_python_dll_path = tracer_python_dll_path
        self.tracer_pythontracer_dll_path = tracer_pythontracer_dll_path

        self.tracer_dll = tracer(self.tracer_dll_path)
        self.tracer_rtl_dll = rtl(self.tracer_rtl_dll_path)
        self.tracer_sqlite3_dll = sqlite3(self.tracer_sqlite3_dll_path)
        self.tracer_python_dll = python(self.tracer_python_dll_path)
        self.tracer_pythontracer_dll = (
            pythontracer(self.tracer_pythontracer_dll_path)
        )

        # Rtl needs to be initialized first.
        self.rtl = RTL()
        self.rtl_size = ULONG(sizeof(self.rtl))
        success = self.tracer_rtl_dll.InitializeRtl(
            byref(self.rtl),
            byref(self.rtl_size)
        )

        if not success:
            if self.rtl_size.value != sizeof(self.rtl):
                msg = "Warning: RTL size mismatch: %d != %d\n" % (
                    self.rtl_size.value,
                    sizeof(self.rtl)
                )
                sys.stderr.write(msg)
                self.rtl = create_string_buffer(
                    self.rtl_size.value
                )
                success = self.tracer_rtl_dll.InitializeRtl(
                    byref(self.rtl),
                    byref(self.rtl_size),
                )

            if not success:
                raise TracerError("InitializeRtl() failed")

        # The Python structure is complex; we haven't written a ctypes.Structure
        # wrapper for it yet.  So, for now, we use a raw buffer instead.
        self.python_size = ULONG()
        # Get the size required for the structure first.
        self.tracer_python_dll.InitializePython(
            None,
            None,
            None,
            byref(self.python_size),
        )
        self.python = create_string_buffer(self.python_size.value)
        success = self.tracer_python_dll.InitializePython(
            self.rtl,
            sys.dllhandle,
            byref(self.python),
            byref(self.python_size),
        )
        if not success:
            raise TracerError("InitializePython() failed")

        self.trace_session = TRACE_SESSION.create()

        self.trace_stores = TRACE_STORES()
        self.trace_stores_size = ULONG(sizeof(self.trace_stores))

        success = self.tracer_dll.InitializeTraceStores(
            self.rtl,
            self.basedir,
            byref(self.trace_stores),
            byref(self.trace_stores_size),
            None,
            None,   # readonly
        )
        if not success:
            if self.trace_stores_size.value != sizeof(self.trace_stores):
                msg = "Warning: TRACE_STORES size mismatch: %d != %d\n" % (
                    self.trace_stores_size.value,
                    sizeof(self.trace_stores)
                )
                sys.stderr.write(msg)
                self.trace_stores = create_string_buffer(
                    self.trace_stores_size.value
                )
                success = self.tracer_dll.InitializeTraceStores(
                    self.rtl,
                    self.basedir,
                    byref(self.trace_stores),
                    byref(self.trace_stores_size),
                    None,
                    None,   # readonly
                )

            if not success:
                raise TracerError("InitializeTraceStores() failed")

        if not threadpool:
            threadpool = kernel32.CreateThreadpool(None)
            if not threadpool:
                raise TracerError("CreateThreadpool() failed")
            num_cpus = cpu_count()
            kernel32.SetThreadpoolThreadMinimum(threadpool, num_cpus)
            kernel32.SetThreadpoolThreadMaximum(threadpool, num_cpus)

        self.threadpool = threadpool

        if not threadpool_callback_environment:
            threadpool_callback_environment = TP_CALLBACK_ENVIRON()
            InitializeThreadpoolEnvironment(threadpool_callback_environment)
            SetThreadpoolCallbackPool(
                threadpool_callback_environment,
                threadpool
            )

        self.threadpool_callback_environment = threadpool_callback_environment

        self.trace_context = TRACE_CONTEXT()
        self.trace_context_size = ULONG(sizeof(TRACE_CONTEXT))
        success = self.tracer_dll.InitializeTraceContext(
            byref(self.rtl),
            byref(self.trace_context),
            byref(self.trace_context_size),
            byref(self.trace_session),
            byref(self.trace_stores),
            byref(self.threadpool_callback_environment),
            None,
            None,
        )
        if not success:
            if self.trace_context_size.value != sizeof(self.trace_context):
                msg = "TRACE_CONTEXT size mismatch: %d != %d\n" % (
                    self.trace_context_size.value,
                    sizeof(self.trace_context)
                )
                sys.stderr.write(msg)
                self.trace_context = create_string_buffer(
                    self.trace_context_size.value
                )
                success = self.tracer_dll.InitializeTraceContext(
                    byref(self.rtl),
                    byref(self.trace_context),
                    byref(self.trace_context_size),
                    byref(self.trace_session),
                    byref(self.trace_stores),
                    byref(self.threadpool_callback_environment),
                    None,
                    None,
                )

            if not success:
                kernel32.CloseThreadpool(self.threadpool)
                self.threadpool = None
                raise TracerError("InitializeTraceContext() failed")

        self.python_trace_context_size = ULONG(sizeof(PYTHON_TRACE_CONTEXT))
        self.tracer_pythontracer_dll.InitializePythonTraceContext(
            None,
            None,
            byref(self.python_trace_context_size),
            None,
            None,
            None,
            None,
        )

        self.python_trace_context = create_string_buffer(
            self.python_trace_context_size.value
        )
        success = self.tracer_pythontracer_dll.InitializePythonTraceContext(
            byref(self.rtl),
            byref(self.python_trace_context),
            byref(self.python_trace_context_size),
            byref(self.python),
            byref(self.trace_context),
            None,
            None,
        )
        if not success:
            raise TracerError("InitializePythonTraceContext() failed")

        global TRACER
        TRACER = self

    @classmethod
    def create_debug(cls, basedir, conf=None):
        if not conf:
            from .config import get_or_create_config
            conf = get_or_create_config()

        return cls(
            basedir,
            conf.tracer_debug_dll_path,
            conf.tracer_rtl_debug_dll_path,
            conf.tracer_sqlite3_debug_dll_path,
            conf.tracer_tracersqlite_debug_dll_path,
            conf.tracer_python_debug_dll_path,
            conf.tracer_pythontracer_debug_dll_path,
        )

    @classmethod
    def create_release(cls, basedir, conf=None):
        if not conf:
            from .config import get_or_create_config
            conf = get_or_create_config()

        return cls(
            basedir,
            conf.tracer_dll_path,
            conf.tracer_rtl_dll_path,
            conf.tracer_sqlite3_dll_path,
            conf.tracer_tracersqlite_dll_path,
            conf.tracer_python_dll_path,
            conf.tracer_pythontracer_dll_path,
        )

    def enable_memory_tracing(self):
        dll = self.tracer_pythontracer_dll
        dll.EnableMemoryTracing(self.python_trace_context)

    def disable_memory_tracing(self):
        dll = self.tracer_pythontracer_dll
        dll.DisableMemoryTracing(self.python_trace_context)

    def add_function(self, func):
        dll = self.tracer_pythontracer_dll
        if not dll.AddFunction(self.python_trace_context, func):
            raise TracerError("AddFunction() failed")

    def add_module(self, module_name):
        name = module_name.replace('.', '\\')
        dll = self.tracer_pythontracer_dll
        arg = py_object(name)
        if not dll.AddModuleName(self.python_trace_context, arg):
            raise TracerError("AddModuleName() failed")

    def start(self):
        dll = self.tracer_pythontracer_dll
        if not dll.StartTracing(self.python_trace_context):
            raise TracerError("StartTracing() failed")

    def stop(self):
        dll = self.tracer_pythontracer_dll
        if not dll.StopTracing(self.python_trace_context):
            raise TracerError("StopTracing() failed")

    def start_profiling(self):
        dll = self.tracer_pythontracer_dll
        if not dll.StartProfiling(self.python_trace_context):
            raise TracerError("StartProfiling() failed")

    def stop_profiling(self):
        dll = self.tracer_pythontracer_dll
        if not dll.StopProfiling(self.python_trace_context):
            raise TracerError("StopProfiling() failed")

    def close_trace_stores(self):
        self.tracer_dll.CloseTraceStores(byref(self.trace_stores))

    def save(self):
        dll = self.tracer_pythontracer_dll_path

    def finish(self):
        self.save()
        self.close_trace_stores()

    def __enter__(self):
        self.start()
        return self

    def __exit__(self, *exc_info):
        self.stop()

# vim:set ts=8 sw=4 sts=4 tw=80 ai et                                          :
