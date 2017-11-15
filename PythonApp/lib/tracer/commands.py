#===============================================================================
# Imports
#===============================================================================
import sys

import textwrap

from .util import strip_linesep_if_present

from .command import (
    Command,
    CommandError,
)

from .invariant import (
    BoolInvariant,
    PathInvariant,
    StringInvariant,
    DirectoryInvariant,
    PositiveIntegerInvariant,
)

from .commandinvariant import (
    InvariantAwareCommand,
)

#===============================================================================
# Imports
#===============================================================================
class TracerCommand(InvariantAwareCommand):

    def prompt_for_directory(self, activity_name,
                                   success_filename,
                                   base_directory=None,
                                   directory_option_name=None):

        if self.has_parent:
            raise CommandError(
                "--directory argument needs to be provided when "
                "--parent-pid is specified"
            )

        from .path import join, isdir, isfile, basename
        from .util import is_win32, file_timestamps, yes_no_quit
        from os import listdir

        out = self._out
        conf = self.conf
        verbose = self._verbose

        success = success_filename
        base = base_directory
        if not base:
            base = conf.base_output_dir

        optname = directory_option_name
        if not optname:
            optname = '--directory'

        paths = [ join(base, p) for p in listdir(base) ]
        dirs = [ d for d in paths if isdir(d) ]
        dirs = [ d for d in dirs if not isfile(join(d, success)) ]
        latest_dirs = file_timestamps(dirs)
        if not latest_dirs:
            fmt = (
                "No suitable directories found within %s.  Try the %s "
                "option to use a directory outside of the default area."
            )
            msg = fmt % (base, optname)
            return

        verbose("Found %d output directories." % len(latest_dirs))

        ostream = self.ostream
        estream = self.estream
        istream = self.istream
        fmt = "%s %s? [y/n/q] " % (activity_name, '%s')
        errmsg = "\nSorry, I didn't get that.\n"

        found = None
        for latest_dir in latest_dirs:
            path = latest_dir.path
            name = basename(path)
            prompt = fmt % name
            while True:
                ostream.write(prompt)
                response = yes_no_quit(istream)
                if response:
                    break
                estream.write(errmsg)

            if response == 'y':
                found = path
                break
            elif response == 'q':
                out("Quitting.")
                self.returncode = 1
                return

        if not found:
            fmt = (
                "Sorry, no more directories left.  Try the %s option to use "
                "a directory outside of %s."
            )
            msg = fmt % (optname, base)
            out(msg)
            return

        return path

    def run(self):
        raise NotImplementedError()

#===============================================================================
# Commands
#===============================================================================

class VsProfileProgram(InvariantAwareCommand):
    """
    Runs the vspyprof against the given file.
    """
    _verbose_ = True

    python_file = None
    _python_file = None
    class PythonFileArg(PathInvariant):
        _help = "Path to the Python file to profile"
        _mandatory = True

    python_exe = None
    _python_exe = None
    class PythonExeArg(PathInvariant):
        _help = (
            "Path to the Python interpreter to use.  Defaults to "
            "[ptvs:python_exe], or if that's not set, whatever "
            "sys.executable is."
        )
        _mandatory = False

    program_args = None
    class ProgramArgsArg(StringInvariant):
        _help = (
            "Additional list of arguments to pass to the Python "
            "file being profiled.  (XXX: Not Yet Implemented.)"
        )
        _mandatory = False

    vspyprof_dll = None
    class VspyprofDllArg(PathInvariant):
        _help = (
            "Path to the Visual Studio Profiler DLL "
            "(defaults to [ptvs:dll] in config file)."
        )
        _mandatory = False

    use_debug_dlls = None
    class UseDebugDllsArg(BoolInvariant):
        _help = "Use the debug versions of profiler DLLs."
        _mandatory = False
        _default = False

    trace = None
    class TraceArg(BoolInvariant):
        _help = "Enable tracing (instead of profiling)."
        _mandatory = False
        _default = False

    custom_profiler_dll = None
    class CustomProfilerDllArg(PathInvariant):
        _help = (
            "Optional path to a custom profile DLL to use instead of the "
            "Visual Studio vsperf.dll that vspyprof.dll was built against. "
            "This must export C __stdcall symbols for EnterFunction, "
            "ExitFunction, NameToken and SourceLine.  (See PythonApi.cpp "
            "in PTVS for more info.)"
        )
        _mandatory = False

    pause_before_starting = None
    class PauseBeforeStartingArg(BoolInvariant):
        _help = (
            "If set to true, pauses prior to starting profiling.  This allows "
            "you to independently attach debuggers, etc."
        )
        _default = False
        _mandatory = False

    run_dir = None
    _run_dir = None
    class RunDirArg(DirectoryInvariant):
        _help = "Directory to run the profiler from."

    def run(self):
        InvariantAwareCommand.run(self)

        conf = self.conf
        dll = self.options.profiler_dll
        if not dll:
            if self.options.use_debug_dlls:
                dll = conf.ptvs_debug_dll_path
            else:
                dll = conf.ptvs_dll_path

        custdll = self.options.custom_profiler_dll
        if not custdll:
            if self.options.use_debug_dlls:
                custdll = conf.ptvs_custom_debug_dll_path
            else:
                custdll = conf.ptvs_custom_dll_path
        self._verbose("Using profiler DLL: %s" % dll)
        if custdll:
            self._verbose("Using custom profiler DLL: %s" % custdll)
        else:
            custdll = '-'
        from . import vspyprof

        from .util import chdir
        from .path import join_path
        from os.path import dirname

        import sys

        this_dir = dirname(__file__)
        runvspyprof = join_path(this_dir, 'runvspyprof.py')

        exe = self._python_exe or conf.ptvs_python_exe

        cmd = [
            exe,
            runvspyprof,
            dll,
            self._run_dir,
            custdll,
            self._python_file,
        ]

        program_args = self.options.program_args
        if program_args:
            import shlex
            args = shlex.split(program_args)
            cmd.append(args)

        if self.options.pause_before_starting:
            import msvcrt
            sys.stdout.write('Press any key to continue . . .')
            sys.stdout.flush()
            msvcrt.getch()

        import os
        env = os.environ
        if self.options.trace:
            env['VSPYPROF_TRACE'] = '1'

        env['VSPYPROF_DEBUGBREAK_ON_START'] = '1'

        import subprocess
        with chdir(this_dir):
            subprocess.call(cmd, env=env)

class TestVsPyProfTraceStores(InvariantAwareCommand):
    """
    Runs the vspyprof against the given file.
    """
    _verbose_ = True

    python_file = None
    _python_file = None
    class PythonFileArg(PathInvariant):
        _help = "Path to the Python file to profile"
        _mandatory = True

    python_exe = None
    _python_exe = None
    class PythonExeArg(PathInvariant):
        _help = (
            "Path to the Python interpreter to use.  Defaults to "
            "[ptvs:python_exe], or if that's not set, whatever "
            "sys.executable is."
        )
        _mandatory = False

    program_args = None
    class ProgramArgsArg(StringInvariant):
        _help = (
            "Additional list of arguments to pass to the Python "
            "file being profiled.  (XXX: Not Yet Implemented.)"
        )
        _mandatory = False

    vspyprof_dll = None
    class VspyprofDllArg(PathInvariant):
        _help = (
            "Path to the Visual Studio Profiler DLL "
            "(defaults to [ptvs:dll] in config file)."
        )
        _mandatory = False

    use_debug_dlls = None
    class UseDebugDllsArg(BoolInvariant):
        _help = "Use the debug versions of profiler DLLs."
        _mandatory = False
        _default = False

    trace = None
    class TraceArg(BoolInvariant):
        _help = "Enable tracing (instead of profiling)."
        _mandatory = False
        _default = False

    custom_profiler_dll = None
    class CustomProfilerDllArg(PathInvariant):
        _help = (
            "Optional path to a custom profile DLL to use instead of the "
            "Visual Studio vsperf.dll that vspyprof.dll was built against. "
            "This must export C __stdcall symbols for EnterFunction, "
            "ExitFunction, NameToken and SourceLine.  (See PythonApi.cpp "
            "in PTVS for more info.)"
        )
        _mandatory = False

    pause_before_starting = None
    class PauseBeforeStartingArg(BoolInvariant):
        _help = (
            "If set to true, pauses prior to starting profiling.  This allows "
            "you to independently attach debuggers, etc."
        )
        _default = False
        _mandatory = False

    base_dir = None
    _base_dir = None
    class BaseDirArg(DirectoryInvariant):
        _help = "Base directory to pass to tracer"

    dll = None

    def run(self):
        InvariantAwareCommand.run(self)

        conf = self.conf
        dllpath = self.options.profiler_dll
        if not dllpath:
            if self.options.use_debug_dlls:
                dllpath = conf.ptvs_debug_dll_path
            else:
                dllpath = conf.ptvs_dll_path

        import ctypes
        from .wintypes import DWORD
        from .dll import pytrace

        dll = pytrace(path=dllpath)

        basedir = ctypes.c_wchar_p(self._base_dir)

        size = dll.GetTraceStoresAllocationSize()
        stores = ctypes.create_string_buffer(size)

        import pdb
        if self.options.pause_before_starting:
            dll.Debugbreak()
            import pdb
            pdb.set_trace()

        dll.InitializeTraceStores(
            basedir,
            ctypes.pointer(stores),
            ctypes.byref(size),
            ctypes.c_void_p(0),
        )

        import ipdb
        ipdb.set_trace()
        self.dll = dll

class TestTracer(InvariantAwareCommand):
    """
    Runs the vspyprof against the given file.
    """
    _verbose_ = True

    python_file = None
    _python_file = None
    class PythonFileArg(PathInvariant):
        _help = "Path to the Python file to profile"
        _mandatory = True

    python_exe = None
    _python_exe = None
    class PythonExeArg(PathInvariant):
        _help = (
            "Path to the Python interpreter to use.  Defaults to "
            "[ptvs:python_exe], or if that's not set, whatever "
            "sys.executable is."
        )
        _mandatory = False

    program_args = None
    class ProgramArgsArg(StringInvariant):
        _help = (
            "Additional list of arguments to pass to the Python "
            "file being profiled.  (XXX: Not Yet Implemented.)"
        )
        _mandatory = False

    vspyprof_dll = None
    class VspyprofDllArg(PathInvariant):
        _help = (
            "Path to the Visual Studio Profiler DLL "
            "(defaults to [ptvs:dll] in config file)."
        )
        _mandatory = False

    use_debug_dlls = None
    class UseDebugDllsArg(BoolInvariant):
        _help = "Use the debug versions of profiler DLLs."
        _mandatory = False
        _default = False

    trace = None
    class TraceArg(BoolInvariant):
        _help = "Enable tracing (instead of profiling)."
        _mandatory = False
        _default = False

    custom_profiler_dll = None
    class CustomProfilerDllArg(PathInvariant):
        _help = (
            "Optional path to a custom profile DLL to use instead of the "
            "Visual Studio vsperf.dll that vspyprof.dll was built against. "
            "This must export C __stdcall symbols for EnterFunction, "
            "ExitFunction, NameToken and SourceLine.  (See PythonApi.cpp "
            "in PTVS for more info.)"
        )
        _mandatory = False

    pause_before_starting = None
    class PauseBeforeStartingArg(BoolInvariant):
        _help = (
            "If set to true, pauses prior to starting profiling.  This allows "
            "you to independently attach debuggers, etc."
        )
        _default = False
        _mandatory = False

    base_dir = None
    _base_dir = None
    class BaseDirArg(DirectoryInvariant):
        _help = "Base directory to pass to tracer"

    dll = None

    def run(self):
        InvariantAwareCommand.run(self)

        conf = self.conf
        dllpath = self.options.profiler_dll
        if not dllpath:
            if self.options.use_debug_dlls:
                dllpath = conf.ptvs_debug_dll_path
            else:
                dllpath = conf.ptvs_dll_path

        import ctypes
        from .wintypes import DWORD
        from .dll import pytrace

        dll = pytrace(path=dllpath)

        basedir = ctypes.c_wchar_p(self._base_dir)

        size = dll.GetTraceStoresAllocationSize()
        stores = ctypes.create_string_buffer(size)

        import pdb
        if self.options.pause_before_starting:
            dll.Debugbreak()
            import pdb
            pdb.set_trace()

        dll.InitializeTraceStores(
            basedir,
            ctypes.pointer(stores),
            ctypes.byref(size),
            ctypes.c_void_p(0),
        )

        import ipdb
        ipdb.set_trace()
        self.dll = dll

class TracerDriverIoctlDeviceExtensionSize(InvariantAwareCommand):
    """
    Issues a IOCTL_TRACER_CONTROL_DEVEXT_SIZE and prints the response.
    """

    def run(self):
        from tracer.device import TracerControlDevice

        device = TracerControlDevice.create(
            conf=self.conf,
            options=self.options
        )

        with device:
            self._out(str(device.device_extension_size))


class TracerDriverIoctlReadCr3(InvariantAwareCommand):
    """
    Issues a IOCTL_TRACER_CONTROL_READ_CR3 and prints the response.
    """

    def run(self):
        from tracer.device import TracerControlDevice

        device = TracerControlDevice.create(
            conf=self.conf,
            options=self.options
        )

        with device:
            self._out(str(device.cr3))


class ListTraceSessions(InvariantAwareCommand):
    """
    Lists trace session directories in the base trace directory.
    """

    def run(self):
        from .dll.TracerConfig import load_tracer_config
        tc = load_tracer_config()

        self._out('\n'.join(tc.trace_store_directories()))

class PrintTraceSessionInfo(InvariantAwareCommand):
    def run(self):
        out = self._out
        from .dll.TracerConfig import load_tracer_config
        tc = load_tracer_config()

        #basedir = tc.choose_trace_store_directory()
        basedir = tc.trace_store_directories()[0]
        out("Selected: %s." % basedir)

        from .dll import (
            Rtl,
            Python,
            Allocator,
            TraceStore,
            TracerConfig,
        )

        Rtl.bind(path=tc.Paths.RtlDllPath.Buffer)
        rtl = Rtl.create_and_initialize_rtl()

        TraceStore.bind(path=tc.Paths.TraceStoreDllPath.Buffer)
        allocator = tc.Allocator.contents

        from .dll.TraceStore import (
            TP_CALLBACK_ENVIRON,

            create_threadpool,

            SetThreadpoolCallbackPool,
            InitializeThreadpoolEnvironment,
        )

        from ctypes import (
            cast,
            byref,
            sizeof,
        )

        from .wintypes import (
            wait,
            ULONG,
            PVOID,
        )

        from .util import timer

        t = timer(verbose=False)
        with t:
            tracer_config = TracerConfig.load_tracer_config()
            tc = tracer_config
            base = tc.trace_store_directories()[0]
            Rtl.bind(path=tc.Paths.RtlDllPath.Buffer)
            rtl = Rtl.create_and_initialize_rtl()
            TraceStore.bind(path=tc.Paths.TraceStoreDllPath.Buffer)
            allocator = tc.Allocator.contents
            threadpool = create_threadpool()
            tp_callback_env = TP_CALLBACK_ENVIRON()
            InitializeThreadpoolEnvironment(tp_callback_env)
            SetThreadpoolCallbackPool(tp_callback_env, threadpool)
            cancel_tp_callback_env = TP_CALLBACK_ENVIRON()
            InitializeThreadpoolEnvironment(cancel_tp_callback_env)
            SetThreadpoolCallbackPool(cancel_tp_callback_env, threadpool)
            flags = TraceStore.TRACE_CONTEXT_FLAGS()
            flags.InitializeAsync = True
            ctx = TraceStore.TRACE_CONTEXT()
            ctx_size = ULONG(sizeof(TraceStore.TRACE_CONTEXT))

        out("Initialized libraries and threadpool in %s." % t.fmt)

        t = timer(verbose=False)
        with t:
            ts = TraceStore.create_and_initialize_readonly_trace_stores(
                rtl,
                allocator,
                basedir,
                tc
            )

        out("Initialized readonly trace stores in %s." % t.fmt)

        t = timer(verbose=False)
        with t:
            success = TraceStore.InitializeReadonlyTraceContext(
                byref(rtl),
                byref(allocator),
                tc,
                byref(ctx),
                byref(ctx_size),
                cast(0, TraceStore.PTRACE_SESSION),
                byref(ts),
                byref(tp_callback_env),
                byref(cancel_tp_callback_env),
                byref(flags),
                cast(0, PVOID),
            )

        assert success
        out("Initialized readonly trace context in %s." % t.fmt)

        if not wait(ctx.LoadingCompleteEvent):
            raise CommandError("Context failed to load asynchronously.")

        from .util import (
            Dict,
            bytes_to_human,
            render_text_table,
        )

        totals = ts.PythonFunctionTableEntryStore.Totals.contents
        total_allocs = totals.NumberOfAllocations
        out("Total number of function table allocations: %d." % total_allocs)

        header = [
            'Trace Store',
            'Size (Bytes)',
            'Size GB/MB/KB',
            'Compressed (Bytes)',
            'Compressed GB/MB/KB',
            'Compression Ratio',
            'Space Saved %',
            '# Allocations',
            'Dropped Records',
            'Exhausted Free Memory Maps',
            'Allocations Outpaced Async Prep.',
            'Preferred Address Unavailable',
            'Access Violation During Async Prefault',
            'Blocked Allocations',
            'Suspended Allocations',
            'Elapsed Suspension Time (\xc2\xb5)'
        ]

        tsa = cast(byref(ts), TraceStore.PTRACE_STORE_ARRAY).contents

        rows = []

        fmt = lambda v: '{:,}'.format(v)
        fmtz = lambda v: fmt(v) if v else ''
        drop = lambda f, v, d: f % v if v > d else ''

        def make_row(target):

            (name, store) = target
            rows.append([
                str(name),
                fmt(store.size),
                bytes_to_human(store.size),
                fmt(store.compressed_size),
                bytes_to_human(store.compressed_size),
                drop('%0.2f', store.compression_ratio, 1.0),
                drop('%0.2f %%', store.space_saved_percent, 0),
                fmt(store.num_allocations),
                fmtz(store.dropped_records),
                fmtz(store.exhausted_free_memory_maps),
                fmtz(store.allocations_outpacing_next_memory_map_preparation),
                fmtz(store.preferred_address_unavailable),
                fmtz(store.access_violations_encountered_during_async_prefault),
                fmtz(store.blocked_allocations),
                fmtz(store.suspended_allocations),
                fmtz(store.elapsed_suspension_time),
            ])

        trace_stores = tsa._stores()
        for ((name, store), metadata_stores) in trace_stores:
            make_row((name, store))
            for metadata_store in metadata_stores:
                make_row(metadata_store)

        from itertools import chain, repeat

        k = Dict()
        k.output = self.ostream
        k.banner = 'Trace Store Stats for %s' % str(ts.BaseDirectory.Buffer)
        k.formats = lambda: chain(
            (str.ljust,),           # Name
            (str.rjust,),           # Size (Bytes)
            (str.rjust,),           # Size GB/MB/KB
            (str.rjust,),           # Compressed (Bytes)
            (str.rjust,),           # Compressed GB/MB/KB
            (str.center,),          # Compression Ratio
            (str.center,),          # Space Saved
            (str.rjust,),           # Allocations
            (str.rjust,),           # Mapping Size
            repeat(str.center,)
        )

        results_chain = chain((header,), rows)
        render_text_table(results_chain, **k)

        #import IPython
        #IPython.embed()

class PrintAddressInfo(InvariantAwareCommand):
    def run(self):
        out = self._out
        from .dll.TracerConfig import load_tracer_config
        tc = load_tracer_config()

        #basedir = tc.choose_trace_store_directory()
        basedir = tc.trace_store_directories()[0]
        out("Selected: %s." % basedir)

        from .dll import (
            Rtl,
            Python,
            Allocator,
            TraceStore,
            TracerConfig,
        )

        Rtl.bind(path=tc.Paths.RtlDllPath.Buffer)
        rtl = Rtl.create_and_initialize_rtl()

        TraceStore.bind(path=tc.Paths.TraceStoreDllPath.Buffer)
        allocator = tc.Allocator.contents

        from .dll.TraceStore import (
            TP_CALLBACK_ENVIRON,

            create_threadpool,

            SetThreadpoolCallbackPool,
            InitializeThreadpoolEnvironment,
        )

        from ctypes import (
            cast,
            byref,
            sizeof,
        )

        from .wintypes import (
            wait,
            ULONG,
            PVOID,
        )

        from .util import timer

        t = timer(verbose=False)
        with t:
            tracer_config = TracerConfig.load_tracer_config()
            tc = tracer_config
            base = tc.trace_store_directories()[0]
            Rtl.bind(path=tc.Paths.RtlDllPath.Buffer)
            rtl = Rtl.create_and_initialize_rtl()
            TraceStore.bind(path=tc.Paths.TraceStoreDllPath.Buffer)
            allocator = tc.Allocator.contents
            threadpool = create_threadpool()
            tp_callback_env = TP_CALLBACK_ENVIRON()
            InitializeThreadpoolEnvironment(tp_callback_env)
            SetThreadpoolCallbackPool(tp_callback_env, threadpool)
            cancel_tp_callback_env = TP_CALLBACK_ENVIRON()
            InitializeThreadpoolEnvironment(cancel_tp_callback_env)
            SetThreadpoolCallbackPool(cancel_tp_callback_env, threadpool)
            flags = TraceStore.TRACE_CONTEXT_FLAGS()
            flags.InitializeAsync = True
            ctx = TraceStore.TRACE_CONTEXT()
            ctx_size = ULONG(sizeof(TraceStore.TRACE_CONTEXT))

        out("Initialized libraries and threadpool in %s." % t.fmt)

        t = timer(verbose=False)
        with t:
            ts = TraceStore.create_and_initialize_readonly_trace_stores(
                rtl,
                allocator,
                basedir,
                tc
            )

        out("Initialized readonly trace stores in %s." % t.fmt)

        t = timer(verbose=False)
        with t:
            success = TraceStore.InitializeReadonlyTraceContext(
                byref(rtl),
                byref(allocator),
                tc,
                byref(ctx),
                byref(ctx_size),
                cast(0, TraceStore.PTRACE_SESSION),
                byref(ts),
                byref(tp_callback_env),
                byref(cancel_tp_callback_env),
                byref(flags),
                cast(0, PVOID),
            )

        out("Initialized readonly trace context in %s." % t.fmt)

        if not wait(ctx.LoadingCompleteEvent):
            raise CommandError("Context failed to load asynchronously.")

        from .util import (
            Dict,
            hex_zfill,
            bytes_to_human,
            render_text_table,
        )

        totals = ts.PythonFunctionTableEntryStore.Totals.contents
        total_allocs = totals.NumberOfAllocations
        out("Total number of function table allocations: %d." % total_allocs)

        header = [
            'Trace Store',
            'Preferred Base',
            'Base Address',
            'File Offset',
            'Mapped Size',
            'Awaiting Preparation',
            'Awaiting Consumption',
            'Active',
            'Awaiting Release',
            'Mapped Sequence ID',
            'Process ID',
            'Requesting Thread ID#',
            'Fulfilling Thread ID#',
            'Requesting CPU Group#',
            'Fulfilling CPU Group#',
            'Requesting CPU Numa Node#',
            'Fulfilling CPU Numa Node#',
            'Requesting CPU Number#',
            'Fulfilling CPU Number#',
        ]

        tsa = cast(byref(ts), TraceStore.PTRACE_STORE_ARRAY).contents

        rows = []

        fmt_num = lambda v: '{:,}'.format(v)
        fmt_hex = lambda v: '0x{:02X}'.format(v)
        drop = lambda f, v, d: f % v if v > d else ''

        def make_row(target):

            (name, store) = target
            for address in store.addresses:
                rows.append([
                    str(name),
                    hex_zfill(address.PreferredBaseAddress),
                    hex_zfill(address.BaseAddress),
                    fmt_num(address.FileOffset),
                    bytes_to_human(address.MappedSize),
                    address.Elapsed.AwaitingPreparation,
                    address.Elapsed.AwaitingConsumption,
                    address.Elapsed.Active,
                    address.Elapsed.AwaitingRelease,
                    address.MappedSequenceId,
                    address.ProcessId,
                    address.RequestingThreadId,
                    address.FulfillingThreadId,
                    address.RequestingProcessor.Group,
                    address.FulfillingProcessor.Group,
                    address.RequestingProcessor.Reserved,
                    address.FulfillingProcessor.Reserved,
                    address.RequestingProcessor.Number,
                    address.FulfillingProcessor.Number,
                ])

        trace_stores = tsa._stores()

        for ((name, store), metadata_stores) in trace_stores:
            make_row((name, store))
            #for metadata_store in metadata_stores:
            #    make_row(metadata_store)


        from itertools import chain, repeat

        k = Dict()
        k.output = self.ostream
        k.banner = 'Trace Store Stats for %s' % str(ts.BaseDirectory.Buffer)
        k.formats = lambda: chain(
            (str.ljust,),           # Name
            (str.rjust,),           # Size (Bytes)
            (str.rjust,),           # Size GB/MB/KB
            (str.rjust,),           # Compressed (Bytes)
            (str.rjust,),           # Compressed GB/MB/KB
            (str.center,),          # Compression Ratio
            (str.center,),          # Space Saved
            (str.rjust,),           # Allocations
            (str.rjust,),           # Mapping Size
            repeat(str.center,)
        )

        results_chain = chain((header,), rows)
        render_text_table(results_chain, **k)

        #import IPython
        #IPython.embed()

class PrintAddressRangeInfo(InvariantAwareCommand):
    def run(self):
        out = self._out
        from .dll.TracerConfig import load_tracer_config
        tc = load_tracer_config()

        #basedir = tc.choose_trace_store_directory()
        basedir = tc.trace_store_directories()[0]
        out("Selected: %s." % basedir)

        from .dll import (
            Rtl,
            Python,
            Allocator,
            TraceStore,
            TracerConfig,
        )

        Rtl.bind(path=tc.Paths.RtlDllPath.Buffer)
        rtl = Rtl.create_and_initialize_rtl()

        TraceStore.bind(path=tc.Paths.TraceStoreDllPath.Buffer)
        allocator = tc.Allocator.contents

        from .dll.TraceStore import (
            TP_CALLBACK_ENVIRON,

            create_threadpool,

            SetThreadpoolCallbackPool,
            InitializeThreadpoolEnvironment,
        )

        from ctypes import (
            cast,
            byref,
            sizeof,
        )

        from .wintypes import (
            wait,
            ULONG,
            PVOID,
        )

        from .util import timer

        t = timer(verbose=False)
        with t:
            tracer_config = TracerConfig.load_tracer_config()
            tc = tracer_config
            base = tc.trace_store_directories()[0]
            Rtl.bind(path=tc.Paths.RtlDllPath.Buffer)
            rtl = Rtl.create_and_initialize_rtl()
            TraceStore.bind(path=tc.Paths.TraceStoreDllPath.Buffer)
            allocator = tc.Allocator.contents
            threadpool = create_threadpool()
            tp_callback_env = TP_CALLBACK_ENVIRON()
            InitializeThreadpoolEnvironment(tp_callback_env)
            SetThreadpoolCallbackPool(tp_callback_env, threadpool)
            cancel_tp_callback_env = TP_CALLBACK_ENVIRON()
            InitializeThreadpoolEnvironment(cancel_tp_callback_env)
            SetThreadpoolCallbackPool(cancel_tp_callback_env, threadpool)
            flags = TraceStore.TRACE_CONTEXT_FLAGS()
            flags.InitializeAsync = True
            ctx = TraceStore.TRACE_CONTEXT()
            ctx_size = ULONG(sizeof(TraceStore.TRACE_CONTEXT))

        out("Initialized libraries and threadpool in %s." % t.fmt)

        t = timer(verbose=False)
        with t:
            ts = TraceStore.create_and_initialize_readonly_trace_stores(
                rtl,
                allocator,
                basedir,
                tc
            )

        out("Initialized readonly trace stores in %s." % t.fmt)

        t = timer(verbose=False)
        with t:
            success = TraceStore.InitializeReadonlyTraceContext(
                byref(rtl),
                byref(allocator),
                tc,
                byref(ctx),
                byref(ctx_size),
                cast(0, TraceStore.PTRACE_SESSION),
                byref(ts),
                byref(tp_callback_env),
                byref(cancel_tp_callback_env),
                byref(flags),
                cast(0, PVOID),
            )

        out("Initialized readonly trace context in %s." % t.fmt)

        if not wait(ctx.LoadingCompleteEvent):
            raise CommandError("Context failed to load asynchronously.")

        from .util import (
            Dict,
            bin_zfill,
            hex_zfill,
            bytes_to_human,
            render_text_table,
        )

        totals = ts.PythonFunctionTableEntryStore.Totals.contents
        total_allocs = totals.NumberOfAllocations
        out("Total number of function table allocations: %d." % total_allocs)

        header = [
            'Trace Store',
            'Preferred Base Address',
            'Actual Base Address',
            'End Address',
            'Preferred Address Bit Counts',
            'Actual Address Bit Counts',
            'Mapped Size',
            'Valid From',
            'Valid To',
        ]

        tsa = cast(byref(ts), TraceStore.PTRACE_STORE_ARRAY).contents

        rows = []

        fmt_num = lambda v: '{:,}'.format(v)
        fmt_hex = lambda v: '0x{:02X}'.format(v)
        drop = lambda f, v, d: f % v if v > d else ''

        def make_row(target):

            (name, store) = target
            for address_range in store.address_ranges:
                row = [
                    str(name),
                    hex_zfill(address_range.PreferredBaseAddress),
                    hex_zfill(address_range.ActualBaseAddress),
                    hex_zfill(address_range.EndAddress),
                    repr(address_range.BitCounts.Preferred),
                    repr(address_range.BitCounts.Actual),
                    bytes_to_human(address_range.MappedSize),
                    address_range.valid_from,
                    address_range.valid_to,
                ]
                rows.append(row)

        trace_stores = tsa._stores()

        for ((name, store), metadata_stores) in trace_stores:
            make_row((name, store))
            #for metadata_store in metadata_stores:
            #    make_row(metadata_store)


        from itertools import chain, repeat

        k = Dict()
        k.output = self.ostream
        k.banner = (
            'Trace Store Address Ranges for %s' % str(ts.BaseDirectory.Buffer)
        )

        k.formats = lambda: chain(
            (str.ljust,),           # Name
            (str.rjust,),           # Preferred Base
            (str.rjust,),           # Actual Base
            (str.rjust,),           # End
            (str.ljust,),           # BitCounts: Preferred
            (str.ljust,),           # BitCounts: Actual
            (str.rjust,),           # Mapped Size
            (str.rjust,),           # Valid From
            (str.rjust,),           # Valid To
        )

        results_chain = chain((header,), rows)
        render_text_table(results_chain, **k)

class PrintWorkingSetInfo(InvariantAwareCommand):
    def run(self):
        out = self._out
        from .dll.TracerConfig import load_tracer_config
        tc = load_tracer_config()

        #basedir = tc.choose_trace_store_directory()
        basedir = tc.trace_store_directories()[0]
        out("Selected: %s." % basedir)

        from .dll import (
            Rtl,
            Python,
            Allocator,
            TraceStore,
            TracerConfig,
        )

        Rtl.bind(path=tc.Paths.RtlDllPath.Buffer)
        rtl = Rtl.create_and_initialize_rtl()

        TraceStore.bind(path=tc.Paths.TraceStoreDllPath.Buffer)
        allocator = tc.Allocator.contents

        from .dll.TraceStore import (
            TP_CALLBACK_ENVIRON,

            create_threadpool,

            SetThreadpoolCallbackPool,
            InitializeThreadpoolEnvironment,
        )

        from ctypes import (
            cast,
            byref,
            sizeof,
        )

        from .wintypes import (
            wait,
            ULONG,
            PVOID,
        )

        from .util import timer

        t = timer(verbose=False)
        with t:
            tracer_config = TracerConfig.load_tracer_config()
            tc = tracer_config
            base = tc.trace_store_directories()[0]
            Rtl.bind(path=tc.Paths.RtlDllPath.Buffer)
            rtl = Rtl.create_and_initialize_rtl()
            TraceStore.bind(path=tc.Paths.TraceStoreDllPath.Buffer)
            allocator = tc.Allocator.contents
            threadpool = create_threadpool()
            tp_callback_env = TP_CALLBACK_ENVIRON()
            InitializeThreadpoolEnvironment(tp_callback_env)
            SetThreadpoolCallbackPool(tp_callback_env, threadpool)
            cancel_tp_callback_env = TP_CALLBACK_ENVIRON()
            InitializeThreadpoolEnvironment(cancel_tp_callback_env)
            SetThreadpoolCallbackPool(cancel_tp_callback_env, threadpool)
            flags = TraceStore.TRACE_CONTEXT_FLAGS()
            flags.InitializeAsync = True
            ctx = TraceStore.TRACE_CONTEXT()
            ctx_size = ULONG(sizeof(TraceStore.TRACE_CONTEXT))

        out("Initialized libraries and threadpool in %s." % t.fmt)

        t = timer(verbose=False)
        with t:
            ts = TraceStore.create_and_initialize_readonly_trace_stores(
                rtl,
                allocator,
                basedir,
                tc
            )

        out("Initialized readonly trace stores in %s." % t.fmt)

        t = timer(verbose=False)
        with t:
            success = TraceStore.InitializeReadonlyTraceContext(
                byref(rtl),
                byref(allocator),
                tc,
                byref(ctx),
                byref(ctx_size),
                cast(0, TraceStore.PTRACE_SESSION),
                byref(ts),
                byref(tp_callback_env),
                byref(cancel_tp_callback_env),
                byref(flags),
                cast(0, PVOID),
            )

        out("Initialized readonly trace context in %s." % t.fmt)

        if not wait(ctx.LoadingCompleteEvent):
            raise CommandError("Context failed to load asynchronously.")

        from .util import (
            Dict,
            bin_zfill,
            hex_zfill,
            bytes_to_human,
            render_text_table,
        )

        totals = ts.PythonFunctionTableEntryStore.Totals.contents
        total_allocs = totals.NumberOfAllocations
        out("Total number of function table allocations: %d." % total_allocs)

        header = [
            'Timestamp',
            'Faulting IP',
            'Faulting Address',
            'Faulting Thread ID',
            'NUMA Node',
            'Protection',
            'Valid?',
            'Locked?',
            'Large Page?',
            'Bad?',
            'Shared?',
            'Share Count',
        ]

        watch_info_ex_store = ts.WsWatchInfoExStore
        working_set_ex_info_store = ts.WsWorkingSetExInfoStore
        allocation_timestamp_store = ts.WsWatchInfoExAllocationTimestampStore

        watch_info_ex_arrays = list(
            watch_info_ex_store.readonly_arrays
        )

        working_set_ex_info_arrays = list(
            working_set_ex_info_store.readonly_arrays
        )

        from collections import defaultdict
        lengths = defaultdict(list)

        assert len(watch_info_ex_arrays) == 1
        assert len(working_set_ex_info_arrays) == 1

        watch_info_ex_array = watch_info_ex_arrays[0]
        working_set_ex_info_array = working_set_ex_info_arrays[0]

        array_len = len(watch_info_ex_array)

        assert array_len == len(working_set_ex_info_array)

        allocations = ts.WsWatchInfoExAllocationStore.as_array
        timestamps = allocation_timestamp_store.as_array

        assert len(allocations) == len(timestamps), (
               len(allocations) == len(timestamps)
        )

        allocs = [
            (a.NumberOfRecords, b.Timestamp)
                for (a, b) in zip(allocations, timestamps)
        ]

        yes_or_drop = lambda v: 'Y' if v else ''
        fmt_num = lambda v: '{:,}'.format(v)
        fmt_hex = lambda v: '0x{:02X}'.format(v)
        fmt_or_drop = lambda f, v, d: f % v if v > d else ''
        drop_zero = lambda v: str(v) if v else ''

        ix = 0
        base_ix = 0
        rows = []

        from .wintypes import MEMORY_PROTECTION

        from tqdm import tqdm
        progressbar = tqdm(total=array_len)

        for (num_records, timestamp) in allocs:

            for i in range(num_records):
                progressbar.update(1)
                ix = base_ix = i
                watch_info_ex = watch_info_ex_array[ix]
                working_set_ex_info = working_set_ex_info_array[ix]

                wi = watch_info_ex
                bi = wi.BasicInfo
                va = working_set_ex_info.VirtualAttributes

                row = [
                    timestamp,
                    hex_zfill(bi.FaultingPc),
                    hex_zfill(bi.FaultingVa),
                    wi.FaultingThreadId,
                    va.Node,
                    MEMORY_PROTECTION._format(va.Win32Protection),
                    yes_or_drop(va.Valid),
                    yes_or_drop(va.Locked),
                    yes_or_drop(va.LargePage),
                    yes_or_drop(va.Bad),
                    yes_or_drop(va.Shared),
                    drop_zero(va.ShareCount),
                ]
                rows.append(row)

            base_ix += num_records

        from itertools import chain, repeat

        k = Dict()
        k.output = self.ostream
        k.banner = (
            'Trace Store Working Set Information for %s' % (
                str(ts.BaseDirectory.Buffer)
            )
        )

        k.formats = lambda: chain(
            (str.ljust,),           # Name
            (str.rjust,),           # Faulting IP
            (str.rjust,),           # Faulting Address
            (str.rjust,),           # Faulting Thread ID
            (str.ljust,),           # Flags
            (str.ljust,),           # NUMA Node
            (str.center,),          # Valid?
            (str.center,),          # Locked?
            (str.center,),          # Large Page?
            (str.center,),          # Bad?
            (str.center,),          # Shared?
            (str.center,),          # Share Count
            (str.center,),          # Protection

        )

        results_chain = chain((header,), rows)
        render_text_table(results_chain, **k)

class PrintNames(InvariantAwareCommand):
    def run(self):
        out = self._out
        from .dll.TracerConfig import load_tracer_config
        tc = load_tracer_config()

        #basedir = tc.choose_trace_store_directory()
        basedir = tc.trace_store_directories()[0]
        out("Selected: %s." % basedir)

        from .dll import (
            Rtl,
            Python,
            Allocator,
            TraceStore,
            TracerConfig,
        )

        from .dll.TraceStore import (
            TP_CALLBACK_ENVIRON,

            create_threadpool,

            SetThreadpoolCallbackPool,
            InitializeThreadpoolEnvironment,
        )

        from ctypes import (
            cast,
            byref,
            sizeof,
        )

        from .wintypes import (
            wait,
            ULONG,
            PVOID,
        )

        from .util import timer

        t = timer(verbose=False)
        with t:
            tracer_config = TracerConfig.load_tracer_config()
            tc = tracer_config
            base = tc.trace_store_directories()[0]
            Rtl.bind(path=tc.Paths.RtlDllPath.Buffer)
            rtl = Rtl.create_and_initialize_rtl()
            TraceStore.bind(path=tc.Paths.TraceStoreDllPath.Buffer)
            allocator = tc.Allocator.contents
            threadpool = create_threadpool()
            tp_callback_env = TP_CALLBACK_ENVIRON()
            InitializeThreadpoolEnvironment(tp_callback_env)
            SetThreadpoolCallbackPool(tp_callback_env, threadpool)
            cancel_tp_callback_env = TP_CALLBACK_ENVIRON()
            InitializeThreadpoolEnvironment(cancel_tp_callback_env)
            SetThreadpoolCallbackPool(cancel_tp_callback_env, threadpool)
            flags = TraceStore.TRACE_CONTEXT_FLAGS()
            flags.InitializeAsync = True
            ctx = TraceStore.TRACE_CONTEXT()
            ctx_size = ULONG(sizeof(TraceStore.TRACE_CONTEXT))

        out("Initialized libraries and threadpool in %s." % t.fmt)

        t = timer(verbose=False)
        with t:
            ts = TraceStore.create_and_initialize_readonly_trace_stores(
                rtl,
                allocator,
                basedir,
                tc
            )

        out("Initialized readonly trace stores in %s." % t.fmt)

        t = timer(verbose=False)
        with t:
            success = TraceStore.InitializeReadonlyTraceContext(
                byref(rtl),
                byref(allocator),
                tc,
                byref(ctx),
                byref(ctx_size),
                cast(0, TraceStore.PTRACE_SESSION),
                byref(ts),
                byref(tp_callback_env),
                byref(cancel_tp_callback_env),
                byref(flags),
                cast(0, PVOID),
            )

        out("Initialized readonly trace context in %s." % t.fmt)

        if not wait(ctx.LoadingCompleteEvent):
            raise CommandError("Context failed to load asynchronously.")

        from .util import (
            Dict,
            timer,
            bytes_to_human,
            render_text_table,
        )

        totals = ts.PythonFunctionTableEntryStore.Totals.contents
        total_allocs = totals.NumberOfAllocations
        out("Total number of function table allocations: %d." % total_allocs)

        header = [
            'Full Name',
            'Name',
            'Module Name',
            'Class Name',
            'Path',
        ]

        funcs = ts.PythonFunctionTableEntryStore.get_valid_functions()

        rows = [
            (f.fullname,
             f.name,
             f.modulename,
             f.classname,
             f.path) for f in (f.Function for f in funcs)
        ]

        #import IPython
        #IPython.embed()

        k = Dict()
        k.output = self.ostream
        k.banner = 'Trace Store Names for %s' % str(ts.BaseDirectory.Buffer)
        k.formats = lambda: chain(
            (str.ljust,),           # Path
            (str.ljust,),           # Full Name
            (str.ljust,),           # Module Name
            (str.ljust,),           # Class Name
            (str.ljust,),           # Name
        )

        from itertools import chain

        results_chain = chain((header,), rows)
        render_text_table(results_chain, **k)

class PrintPathTableEntries(InvariantAwareCommand):
    def run(self):
        out = self._out
        from .dll.TracerConfig import load_tracer_config
        tc = load_tracer_config()

        #basedir = tc.choose_trace_store_directory()
        basedir = tc.trace_store_directories()[0]
        out("Selected: %s." % basedir)

        from .dll import (
            Rtl,
            Python,
            Allocator,
            TraceStore,
            TracerConfig,
        )

        from .dll.TraceStore import (
            TP_CALLBACK_ENVIRON,

            create_threadpool,

            SetThreadpoolCallbackPool,
            InitializeThreadpoolEnvironment,
        )

        from ctypes import (
            cast,
            byref,
            sizeof,
        )

        from .wintypes import (
            wait,
            ULONG,
            PVOID,
        )

        from .util import timer

        t = timer(verbose=False)
        with t:
            tracer_config = TracerConfig.load_tracer_config()
            tc = tracer_config
            base = tc.trace_store_directories()[0]
            Rtl.bind(path=tc.Paths.RtlDllPath.Buffer)
            rtl = Rtl.create_and_initialize_rtl()
            TraceStore.bind(path=tc.Paths.TraceStoreDllPath.Buffer)
            allocator = tc.Allocator.contents
            threadpool = create_threadpool()
            tp_callback_env = TP_CALLBACK_ENVIRON()
            InitializeThreadpoolEnvironment(tp_callback_env)
            SetThreadpoolCallbackPool(tp_callback_env, threadpool)
            cancel_tp_callback_env = TP_CALLBACK_ENVIRON()
            InitializeThreadpoolEnvironment(cancel_tp_callback_env)
            SetThreadpoolCallbackPool(cancel_tp_callback_env, threadpool)
            flags = TraceStore.TRACE_CONTEXT_FLAGS()
            flags.InitializeAsync = True
            ctx = TraceStore.TRACE_CONTEXT()
            ctx_size = ULONG(sizeof(TraceStore.TRACE_CONTEXT))

        out("Initialized libraries and threadpool in %s." % t.fmt)

        t = timer(verbose=False)
        with t:
            ts = TraceStore.create_and_initialize_readonly_trace_stores(
                rtl,
                allocator,
                basedir,
                tc
            )

        out("Initialized readonly trace stores in %s." % t.fmt)

        t = timer(verbose=False)
        with t:
            success = TraceStore.InitializeReadonlyTraceContext(
                byref(rtl),
                byref(allocator),
                tc,
                byref(ctx),
                byref(ctx_size),
                cast(0, TraceStore.PTRACE_SESSION),
                byref(ts),
                byref(tp_callback_env),
                byref(cancel_tp_callback_env),
                byref(flags),
                cast(0, PVOID),
            )

        out("Initialized readonly trace context in %s." % t.fmt)

        if not wait(ctx.LoadingCompleteEvent):
            raise CommandError("Context failed to load asynchronously.")

        from .util import (
            Dict,
            timer,
            bytes_to_human,
            render_text_table,
        )

        totals = ts.PathTableEntryStore.Totals.contents
        total_allocs = totals.NumberOfAllocations
        out("Total number of path table entry allocations: %d." % total_allocs)

        header = [
            'Path',
            'Type',
            'Full Name',
            'Name',
            'Module Name',
            'Class Name',
        ]

        #entries = ts.PathTableEntryStore.entries
        entries = [ e for e in ts.PathTableEntryStore.readonly_arrays ]
        assert len(entries) == 1, len(entries)
        entries = entries[0]

        def get_type(e):
            pt = e.PrefixTableEntry.PathEntryType
            if pt.IsModuleDirectory:
                return 'Module Directory'
            elif pt.IsNonModuleDirectory:
                return 'Non-module Directory'
            elif pt.IsFile:
                return 'File'
            else:
                return 'Unknown?'

        rows = [
            (e.path,
             get_type(e),
             e.fullname,
             e.name,
             e.modulename,
             e.classname) for e in entries
        ]

        #import IPython
        #IPython.embed()

        k = Dict()
        k.output = self.ostream
        k.banner = (
            'Trace Store Path Table Entries for %s' % (
                str(ts.BaseDirectory.Buffer)
            )
        )
        k.formats = lambda: chain(
            (str.ljust,),           # Path
            (str.ljust,),           # Full Name
            (str.ljust,),           # Module Name
            (str.ljust,),           # Class Name
            (str.ljust,),           # Name
        )

        from itertools import chain

        results_chain = chain((header,), rows)
        render_text_table(results_chain, **k)

class EmbeddedPythonSession(InvariantAwareCommand):
    def run(self):
        out = self._out
        from .dll.TracerConfig import load_tracer_config
        tc = load_tracer_config()

        #basedir = tc.choose_trace_store_directory()
        basedir = tc.trace_store_directories()[0]
        out("Selected: %s." % basedir)

        from .dll import (
            Rtl,
            Python,
            Allocator,
            TraceStore,
            TracerConfig,
        )

        from .dll.TraceStore import (
            TP_CALLBACK_ENVIRON,

            create_threadpool,

            SetThreadpoolCallbackPool,
            InitializeThreadpoolEnvironment,
        )

        from ctypes import (
            cast,
            byref,
            sizeof,
        )

        from .wintypes import (
            wait,
            ULONG,
            PVOID,
        )

        from .util import timer

        t = timer(verbose=False)
        with t:
            tracer_config = TracerConfig.load_tracer_config()
            tc = tracer_config
            base = tc.trace_store_directories()[0]
            Rtl.bind(path=tc.Paths.RtlDllPath.Buffer)
            rtl = Rtl.create_and_initialize_rtl()
            TraceStore.bind(path=tc.Paths.TraceStoreDllPath.Buffer)
            allocator = tc.Allocator.contents
            threadpool = create_threadpool()
            tp_callback_env = TP_CALLBACK_ENVIRON()
            InitializeThreadpoolEnvironment(tp_callback_env)
            SetThreadpoolCallbackPool(tp_callback_env, threadpool)
            cancel_tp_callback_env = TP_CALLBACK_ENVIRON()
            InitializeThreadpoolEnvironment(cancel_tp_callback_env)
            SetThreadpoolCallbackPool(cancel_tp_callback_env, threadpool)
            flags = TraceStore.TRACE_CONTEXT_FLAGS()
            flags.InitializeAsync = True
            ctx = TraceStore.TRACE_CONTEXT()
            ctx_size = ULONG(sizeof(TraceStore.TRACE_CONTEXT))

        out("Initialized libraries and threadpool in %s." % t.fmt)

        import os
        out("Process ID: %d" % os.getpid())
        import IPython
        IPython.embed()

        return

        t = timer(verbose=False)
        with t:
            ts = TraceStore.create_and_initialize_readonly_trace_stores(
                rtl,
                allocator,
                basedir,
                tc
            )

        out("Initialized readonly trace stores in %s." % t.fmt)

        t = timer(verbose=False)
        with t:
            success = TraceStore.InitializeReadonlyTraceContext(
                byref(rtl),
                byref(allocator),
                tc,
                byref(ctx),
                byref(ctx_size),
                cast(0, TraceStore.PTRACE_SESSION),
                byref(ts),
                byref(tp_callback_env),
                byref(cancel_tp_callback_env),
                byref(flags),
                cast(0, PVOID),
            )

        out("Initialized readonly trace context in %s." % t.fmt)

        if not wait(ctx.LoadingCompleteEvent):
            raise CommandError("Context failed to load asynchronously.")

        import os
        out("Process ID: %d" % os.getpid())
        import IPython
        IPython.embed()

class LoadTrace(InvariantAwareCommand):
    def run(self):
        out = self._out
        from .dll.TracerConfig import load_tracer_config
        tc = load_tracer_config()

        basedir = tc.choose_trace_store_directory()
        out("Selected: %s." % basedir)

        from .dll import (
            Rtl,
            Python,
            Allocator,
            TraceStore,
            TracerConfig,
        )

        from .dll.TraceStore import (
            TP_CALLBACK_ENVIRON,

            create_threadpool,

            SetThreadpoolCallbackPool,
            InitializeThreadpoolEnvironment,
        )

        from ctypes import (
            cast,
            byref,
            sizeof,
        )

        from .wintypes import (
            wait,
            ULONG,
            PVOID,
        )

        from .util import timer

        t = timer(verbose=False)
        with t:
            tracer_config = TracerConfig.load_tracer_config()
            tc = tracer_config
            base = tc.trace_store_directories()[0]
            Rtl.bind(path=tc.Paths.RtlDllPath.Buffer)
            rtl = Rtl.create_and_initialize_rtl()
            TraceStore.bind(path=tc.Paths.TraceStoreDllPath.Buffer)
            allocator = tc.Allocator.contents
            threadpool = create_threadpool()
            tp_callback_env = TP_CALLBACK_ENVIRON()
            InitializeThreadpoolEnvironment(tp_callback_env)
            SetThreadpoolCallbackPool(tp_callback_env, threadpool)
            cancel_tp_callback_env = TP_CALLBACK_ENVIRON()
            InitializeThreadpoolEnvironment(cancel_tp_callback_env)
            SetThreadpoolCallbackPool(cancel_tp_callback_env, threadpool)
            flags = TraceStore.TRACE_CONTEXT_FLAGS()
            flags.InitializeAsync = True
            ctx = TraceStore.TRACE_CONTEXT()
            ctx_size = ULONG(sizeof(TraceStore.TRACE_CONTEXT))

        out("Initialized libraries and threadpool in %s." % t.fmt)

        t = timer(verbose=False)
        with t:
            ts = TraceStore.create_and_initialize_readonly_trace_stores(
                rtl,
                allocator,
                basedir,
                tc
            )

        out("Initialized readonly trace stores in %s." % t.fmt)

        t = timer(verbose=False)
        with t:
            success = TraceStore.InitializeReadonlyTraceContext(
                byref(rtl),
                byref(allocator),
                tc,
                byref(ctx),
                byref(ctx_size),
                cast(0, TraceStore.PTRACE_SESSION),
                byref(ts),
                byref(tp_callback_env),
                byref(cancel_tp_callback_env),
                byref(flags),
                cast(0, PVOID),
            )

        out("Initialized readonly trace context in %s." % t.fmt)

        if not wait(ctx.LoadingCompleteEvent):
            raise CommandError("Context failed to load asynchronously.")

        from .util import (
            Dict,
            timer,
            bytes_to_human,
            render_text_table,
        )

        totals = ts.PythonFunctionTableEntryStore.Totals.contents
        total_allocs = totals.NumberOfAllocations
        out("Total number of function table allocations: %d." % total_allocs)

        address_ranges = ts.PythonFunctionTableEntryStore.address_ranges

        import numpy as np
        import pandas as pd


        events = list(ts.EventStore.readonly_arrays)[0]
        event_array = np.ctypeslib.as_array(events, shape=(len(events),))

        event_series = pd.Series(event_array)

        freq = pd.value_counts(event_series)

        def make_uniq():
            uniq = np.unique(
                event_array,
                return_index=True,
                return_counts=True,
                return_inverse=True
            )

        funcs = ts.PythonFunctionTableEntryStore.get_valid_functions()

        #events = [ a for a in ts.EventStore.arrays ][0]

        for i in range(len(funcs)):
            f = funcs[i].Function

            name = f.PathEntry.FullName
            if not name.Length:
                out("[%d]: skipping." % i)
                continue
            buf = name.Buffer[:name.Length]
            out("[%d]: Full Name: %s" % (i, buf))

        import IPython
        IPython.embed()




# vim:set ts=8 sw=4 sts=4 tw=80 et                                             :
