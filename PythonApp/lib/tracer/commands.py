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




# vim:set ts=8 sw=4 sts=4 tw=80 et                                             :
