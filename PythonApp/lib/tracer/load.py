#===============================================================================
# Imports
#===============================================================================
from tracer.dll import (
    Rtl,
    Python,
    Allocator,
    TraceStore,
    TracerConfig,
    PythonTracer,
)

from tracer.wintypes import (
    ULONG,
    PVOID,
)

from ctypes import (
    cast,
    byref,
    sizeof,
)

from tracer.dll.TraceStore import (
    TP_CALLBACK_ENVIRON,

    create_threadpool,

    InitializeThreadpoolEnvironment,
    SetThreadpoolCallbackPool
)


#===============================================================================
# Classes
#===============================================================================

class Session:
    def __init__(self, trace_dir=None):
        self.tracer_config = TracerConfig.load_tracer_config()

        tc = self.tracer_config
        if not trace_dir:
            trace_dir = tc.trace_store_directories()[0]
        self.base = trace_dir

        Rtl.bind(path=tc.Paths.RtlDllPath.Buffer)
        self.rtl = Rtl.create_and_initialize_rtl()

        TraceStore.bind(path=tc.Paths.TraceStoreDllPath.Buffer)
        self.allocator = tc.Allocator.contents
        self.threadpool = create_threadpool()
        self.tp_callback_env = TP_CALLBACK_ENVIRON()
        self.cancel_tp_callback_env = TP_CALLBACK_ENVIRON()

        InitializeThreadpoolEnvironment(self.tp_callback_env)
        SetThreadpoolCallbackPool(self.tp_callback_env, self.threadpool)

        self.flags = TraceStore.TRACE_CONTEXT_FLAGS()
        self.ctx = TraceStore.TRACE_CONTEXT()
        self.trace_stores = (
            TraceStore.create_and_initialize_readonly_trace_stores(
                self.rtl,
                self.allocator,
                self.base,
                self.tracer_config
            )
        )

    def load(self):
        success = TraceStore.InitializeReadonlyTraceContext(
            byref(self.rtl),
            byref(self.allocator),
            byref(self.tracer_config),
            byref(self.ctx),
            byref(ULONG(sizeof(TraceStore.TRACE_CONTEXT))),
            cast(0, TraceStore.PTRACE_SESSION),
            byref(self.trace_stores),
            byref(self.tp_callback_env),
            byref(self.cancel_tp_callback_env),
            byref(self.flags),
            cast(0, PVOID),
        )

    def get_functions(self):
        pft = self.trace_stores.PythonFunctionTableEntryStore
        pfa = pft.as_array
        functions = [p.Function for p in pfa]
        return functions

    def get_function_highlights(self, functions):
        return [
            {
                'MaxCallStackDepth': f.MaxCallStackDepth,
                'CallCount': f.CallCount,
                'FullName': f.PathEntry.FullName,
                'ModuleName': f.PathEntry.ModuleName,
                'ClassName': f.PathEntry.ClassName,
            } for f in functions
        ]

#===============================================================================
# Helpers
#===============================================================================

def load(trace_dir=None):
    session = Session(trace_dir)
    session.load()
    return session

# vim:set ts=8 sw=4 sts=4 tw=80 et                                             :
