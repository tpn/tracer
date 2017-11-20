#===============================================================================
# Imports
#===============================================================================
import sys

import textwrap

from os.path import (
    join,
    abspath,
    dirname,
    basename,
    normpath,
)

from tracer.path import join_path

from tracer.util import strip_linesep_if_present

from tracer.command import (
    Command,
    CommandError,
)

from tracer.invariant import (
    BoolInvariant,
    PathInvariant,
    StringInvariant,
    DirectoryInvariant,
    PositiveIntegerInvariant,
    EnablePostMortemDebuggerInvariant,
)

from tracer.commandinvariant import (
    InvariantAwareCommand,
)

#===============================================================================
# Commands
#===============================================================================

class TestTracer(InvariantAwareCommand):
    """
    Runs the vspyprof against the given file.
    """
    _verbose_ = True

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


class SqlLocalDb(InvariantAwareCommand):
    """
    Run the SqlLocalDB executable.
    """
    _verbose_ = True

    def run(self):
        InvariantAwareCommand.run(self)
        conf = self.conf

        sqllocaldb = conf.sqllocaldb_exe

        func = getattr(sqllocaldb, self.args[0])
        self._out(func(*self.args[1:]))

class LoadRtl(InvariantAwareCommand):
    """
    Load the Rtl DLL.
    """
    rtl = None
    rtl_size = None


    def run(self):
        InvariantAwareCommand.run(self)

        from tracer import rtl, RTL
        from ctypes import create_string_buffer, sizeof, byref
        from ctypes.wintypes import ULONG

        #self.rtl = RTL()
        self.rtl_size = ULONG()

        rtl_dll = rtl(self.conf.tracer_rtl_debug_dll_path)

        success = rtl_dll.InitializeRtl(
            0,
            byref(self.rtl_size)
        )

        self.rtl = create_string_buffer(self.rtl_size.value)

        import pdb
        pdb.set_trace()

        success = rtl_dll.InitializeRtl(
            byref(self.rtl),
            byref(self.rtl_size)
        )

        if not success:
            if self.rtl_size.value != sizeof(self.rtl):
                msg = "Warning: RTL size mismatch: %d != %d\n" % (
                    self.rtl_size.value,
                    sizeof(self.rtl)
                )
                self._err(msg)
                self.rtl = create_string_buffer(
                    self.rtl_size.value
                )
                success = rtl_dll.InitializeRtl(
                    byref(self.rtl),
                    byref(self.rtl_size),
                )

            if not success:
                raise RuntimeError("InitializeRtl() failed")

        self._out("Loaded Rtl successfully.")

        self.interactive = True
        return self

class SyncRtlHeader(InvariantAwareCommand):
    """
    Compares the members of _RTLFUNCTIONS_HEAD to symbols resolved in
    LoadRtlSymbols() function and adds the necessary GetProcAddress()
    steps.  If a function pointer's typedef has a comment starting
    with `// Win ` above it, it is assumed that the function first
    appeared *after* Windows 7, and as such, failure to resolve the
    function (i.e. when running on Windows 7) isn't considered fatal.

    E.g.:

    // Win 8
    typedef ULONG (NTAPI *PRTL_NUMBER_OF_CLEAR_BITS_IN_RANGE)(
        _In_ PRTL_BITMAP BitMapHeader,
        _In_ ULONG StartingIndex,
        _In_ ULONG Length
        );

    """
    template = '        "%(funcname)s",'

    header_path = None
    class HeaderPathArg(PathInvariant):
        _arg = '-H/--header-path'
        _help = "path of the Rtl.h file [default: %default]"

        def _default(self):
            return join_path(dirname(__file__), "../../../Rtl/Rtl.h")

    source_path = None
    class SourcePathArg(PathInvariant):
        _help = "path of the Rtl.c file [default: %default]"

        def _default(self):
            return join_path(dirname(__file__), "../../../Rtl/Rtl.c")

    functions_head_macro_name = None
    class FunctionsHeadMacroNameArg(StringInvariant):
        _help = "name of the macro used to capture the functions to be sync'd"
        _default = "_RTLFUNCTIONS_HEAD"

    function_to_patch = None
    class FunctionToPatchArg(StringInvariant):
        _help = (
            "name of the C function to potentially patch in order to sync "
            "the functions in the macro with the symbols resolved via "
            "GetProcAddress() calls"
        )
        _default = "ResolveRtlFunctions"

    def run(self):
        header_path = self.options.header_path
        source_path = self.options.source_path
        macro_func_name = self.options.functions_head_macro_name
        function_to_patch = self.options.function_to_patch

        raise CommandError("Temporarily disabled until func is updated.")

        import re

        from tracer.sourcefile import SourceFile

        header = SourceFile(header_path)
        self._verbose("Loaded header: %s" % header_path)

        source = SourceFile(source_path)
        self._verbose("Loaded source: %s" % source_path)

        functions = header.functions_from_multiline_define(macro_func_name)

        first_function = functions[0]
        first_template = self.template % first_function._asdict()

        first_block_lines = first_template.splitlines()
        func = source.function_definition(function_to_patch, first_block_lines)
        if not func:
            msg = "no function named %s found in %s"
            raise CommandError(msg % (function_to_patch, source_path))

        if not func.first_block_line:
            msg = "Could not find starting template:\n%s"
            raise CommandError(msg % first_template)

        new_blocks = [ self.template % fn._asdict() for fn in functions ]

        pre_lines = source.lines[:func.first_block_line]
        post_lines = source.lines[func.last_block_line+2:]

        new_lines = pre_lines + new_blocks + post_lines + ['']

        new_text = '\n'.join(new_lines)

        with open(source_path, 'wb') as f:
            f.write(new_text)

        self._verbose("Synchronized file.")

class SyncRtlExFunctions(InvariantAwareCommand):
    """
    Compares the members of _RTLEXFUNCTIONS_HEAD to symbols resolved in
    LoadRtlExSymbols() function and adds the necessary GetProcAddress()
    steps.
    """

    template = """\
    if (!(RtlExFunctions->%(funcname)s = (%(typedef)s)
        GetProcAddress(RtlExModule, "%(funcname)s"))) {

        OutputDebugStringA("RtlEx: failed to resolve '%(funcname)s'");
        return FALSE;
    }
"""

    header_path = None
    class HeaderPathArg(PathInvariant):
        _arg = '-H/--header-path'
        _help = "path of the Rtl.h file [default: %default]"

        def _default(self):
            return join_path(dirname(__file__), "../../../Rtl/Rtl.h")

    source_path = None
    class SourcePathArg(PathInvariant):
        _help = "path of the Rtl.c file [default: %default]"

        def _default(self):
            return join_path(dirname(__file__), "../../../Rtl/Rtl.c")

    functions_head_macro_name = None
    class FunctionsHeadMacroNameArg(StringInvariant):
        _help = "name of the macro used to capture the functions to be sync'd"
        _default = "_RTLEXFUNCTIONS_HEAD"

    function_to_patch = None
    class FunctionToPatchArg(StringInvariant):
        _help = (
            "name of the C function to potentially patch in order to sync "
            "the functions in the macro with the symbols resolved via "
            "GetProcAddress() calls"
        )
        _default = "LoadRtlExFunctions"

    def run(self):
        header_path = self.options.header_path
        source_path = self.options.source_path
        macro_func_name = self.options.functions_head_macro_name
        function_to_patch = self.options.function_to_patch

        import re

        from tracer.sourcefile import SourceFile

        header = SourceFile(header_path)
        self._verbose("Loaded header: %s" % header_path)

        source = SourceFile(source_path)
        self._verbose("Loaded source: %s" % source_path)

        functions = header.functions_from_multiline_define(macro_func_name)

        first_function = functions[0]
        first_template = self.template % first_function._asdict()

        first_block_lines = first_template.splitlines()

        func = source.function_definition(function_to_patch, first_block_lines)
        if not func:
            msg = "no function named %s found in %s"
            raise CommandError(msg % (function_to_patch, source_path))

        if not func.first_block_line:
            msg = "Could not find starting template:\n%s"
            raise CommandError(msg % first_template)

        new_blocks = [ self.template % fn._asdict() for fn in functions ]

        assert func.first_block_line
        assert func.last_block_line

        pre_lines = source.lines[:func.first_block_line]
        post_lines = source.lines[func.last_block_line+2:]

        new_lines = pre_lines + new_blocks + post_lines + ['']

        new_text = '\n'.join(new_lines)

        with open(source_path, 'wb') as f:
            f.write(new_text)

        self._verbose("Synchronized file.")

class SyncDbgFunctions(InvariantAwareCommand):
    """
    Compares the members of _RTLEXFUNCTIONS_HEAD to symbols resolved in
    LoadDbgSymbols() function and adds the necessary GetProcAddress()
    steps.
    """
    _verbose_ = True

    template = """\
    if (!(Dbg->%(funcname)s = (%(typedef)s)
        GetProcAddress(DbgHelpModule, "%(funcname)s"))) {

        OutputDebugStringA("DbgHelp: failed to resolve '%(funcname)s'");
    }
"""

    header_path = None
    class HeaderPathArg(PathInvariant):
        _arg = '-H/--header-path'
        _help = "path of the Rtl.h file [default: %default]"

        def _default(self):
            return join_path(dirname(__file__), "../../../Rtl/Rtl.h")

    source_path = None
    class SourcePathArg(PathInvariant):
        _help = "path of the Rtl.c file [default: %default]"

        def _default(self):
            return join_path(dirname(__file__), "../../../Rtl/Rtl.c")

    functions_head_macro_name = None
    class FunctionsHeadMacroNameArg(StringInvariant):
        _help = "name of the macro used to capture the functions to be sync'd"
        _default = "_DBGHELP_FUNCTIONS_HEAD"

    function_to_patch = None
    class FunctionToPatchArg(StringInvariant):
        _help = (
            "name of the C function to potentially patch in order to sync "
            "the functions in the macro with the symbols resolved via "
            "GetProcAddress() calls"
        )
        _default = "LoadDbg"

    def run(self):
        header_path = self.options.header_path
        source_path = self.options.source_path
        macro_func_name = self.options.functions_head_macro_name
        function_to_patch = self.options.function_to_patch

        import re

        from tracer.sourcefile import SourceFile

        header = SourceFile(header_path)
        self._verbose("Loaded header: %s" % header_path)

        source = SourceFile(source_path)
        self._verbose("Loaded source: %s" % source_path)

        functions = header.functions_from_multiline_define(macro_func_name)

        first_function = functions[0]
        first_template = self.template % first_function._asdict()

        first_block_lines = first_template.splitlines()

        func = source.function_definition(function_to_patch, first_block_lines)
        if not func:
            msg = "no function named %s found in %s"
            raise CommandError(msg % (function_to_patch, source_path))

        if not func.first_block_line:
            msg = "Could not find starting template:\n%s"
            raise CommandError(msg % first_template)

        new_blocks = [ self.template % fn._asdict() for fn in functions ]

        assert func.first_block_line
        assert func.last_block_line

        pre_lines = source.lines[:func.first_block_line]
        post_lines = source.lines[func.last_block_line+2:]

        new_lines = pre_lines + new_blocks + post_lines + ['']

        new_text = '\n'.join(new_lines)

        with open(source_path, 'wb') as f:
            f.write(new_text)

        self._verbose("Synchronized file.")


class SyncPythonExFunctions(InvariantAwareCommand):
    """
    Compares the members of _PYTHONEXFUNCTIONS_HEAD to symbols resolved in
    LoadPythonExSymbols() function and adds the necessary GetProcAddress()
    steps.
    """

    template = """\
    if (!(PythonExFunctions->%(funcname)s = (%(typedef)s)
        GetProcAddress(PythonExModule, "%(funcname)s"))) {

        OutputDebugStringA("PythonEx: failed to resolve '%(funcname)s'");
        return FALSE;
    }
"""

    header_path = None
    class HeaderPathArg(PathInvariant):
        _arg = '-H/--header-path'
        _help = "path of the Python.h file [default: %default]"

        def _default(self):
            return join_path(dirname(__file__), "../../../Python/Python.h")

    source_path = None
    class SourcePathArg(PathInvariant):
        _help = "path of the Python.c file [default: %default]"

        def _default(self):
            return join_path(dirname(__file__), "../../../Python/Python.c")

    functions_head_macro_name = None
    class FunctionsHeadMacroNameArg(StringInvariant):
        _help = "name of the macro used to capture the functions to be sync'd"
        _default = "_PYTHONEXFUNCTIONS_HEAD"

    function_to_patch = None
    class FunctionToPatchArg(StringInvariant):
        _help = (
            "name of the C function to potentially patch in order to sync "
            "the functions in the macro with the symbols resolved via "
            "GetProcAddress() calls"
        )
        _default = "LoadPythonExFunctions"

    def run(self):
        header_path = self.options.header_path
        source_path = self.options.source_path
        macro_func_name = self.options.functions_head_macro_name
        function_to_patch = self.options.function_to_patch

        import re

        from tracer.sourcefile import SourceFile

        header = SourceFile(header_path)
        self._verbose("Loaded header: %s" % header_path)

        source = SourceFile(source_path)
        self._verbose("Loaded source: %s" % source_path)

        functions = header.functions_from_multiline_define(macro_func_name)

        first_function = functions[0]
        first_template = self.template % first_function._asdict()

        first_block_lines = first_template.splitlines()

        func = source.function_definition(function_to_patch, first_block_lines)
        if not func:
            msg = "no function named %s found in %s"
            raise CommandError(msg % (function_to_patch, source_path))

        if not func.first_block_line:
            msg = "Could not find starting template:\n%s"
            raise CommandError(msg % first_template)

        new_blocks = [ self.template % fn._asdict() for fn in functions ]

        assert func.first_block_line
        assert func.last_block_line

        pre_lines = source.lines[:func.first_block_line]
        post_lines = source.lines[func.last_block_line+2:]

        new_lines = pre_lines + new_blocks + post_lines + ['']

        new_text = '\n'.join(new_lines)

        with open(source_path, 'wb') as f:
            f.write(new_text)

        self._verbose("Synchronized file.")

class FindMultilineMacros(InvariantAwareCommand):
    """
    Prints a list of all multi-line macros found in the incoming source file.
    """

    path = None
    class PathArg(PathInvariant):
        _help = "path of the file"

    def run(self):
        out = self._out
        options = self.options
        verbose = self._verbose

        path = options.path

        from tracer.sourcefile import SourceFile

        source = SourceFile(path)
        verbose("Loaded file: %s" % path)

        names = source.multiline_defines.keys()
        if names:
            out('\n'.join(names))

class TrailingSlashesAlign(InvariantAwareCommand):
    """
    Finds all multi-line macros and aligns trailing slashes where necessary.
    """

    path = None
    class PathArg(PathInvariant):
        _help = "path of the file"

    def run(self):
        out = self._out
        options = self.options
        verbose = self._verbose

        path = options.path

        from tracer.sourcefile import SourceFile

        source = SourceFile(path)
        orig_data = source.data
        orig_lines = source.lines

        defines = source.defines
        multiline_macro_defines = source.multiline_macro_defines

        if not multiline_macro_defines:
            return

        lines = source.lines
        dirty = False

        from tracer.util import align_trailing_slashes

        for (name, macro) in multiline_macro_defines.items():
            old_lines = macro.lines
            new_lines = align_trailing_slashes(old_lines)
            old_length = len(old_lines)
            new_length = len(new_lines)
            assert old_length == new_length, (old_length, new_length)
            if new_lines == old_lines:
                continue

            lines[macro.first_lineno:macro.last_lineno+1] = new_lines
            dirty = True
            out("Aligned trailing slashes for %s macro." % name)

        if dirty:
            with open(path, 'wb') as f:
                f.write('\n'.join(lines))
                f.write('\n')

class SyncSchema(InvariantAwareCommand):
    """
    Syncs the TraceStore/TraceStoreSqlite3Schemas.c file.
    """

    path = None
    class PathArg(PathInvariant):
        _help = "path of the file"
        _default = "TraceStore\\TraceStoreSqlite3Schemas.c"

    enable_post_mortem_debugger = None
    class EnablePostMortemDebuggerArg(EnablePostMortemDebuggerInvariant):
        pass

    def run(self):
        out = self._out
        options = self.options
        verbose = self._verbose

        path = options.path


        from tracer.sourcefile import (
            SourceFile,
            generate_sqlite3_column_func_switch_statement,
        )

        source = SourceFile(path)
        orig_data = source.data
        orig_lines = source.lines

        from copy import copy
        lines = copy(orig_lines)

        funcs = {}
        block = ['    switch (ColumnNumber) {']
        decls = source.const_string_decls
        multiline_const_string_decls = source.multiline_const_string_decls

        if not multiline_const_string_decls:
            return

        switches = {}

        dirty = False
        gen_switch_statement = generate_sqlite3_column_func_switch_statement

        for (mname, mdecl) in multiline_const_string_decls.items():
            name = mname.replace('TraceStore', '').replace('Schema', '')
            column_func_name = 'TraceStoreSqlite3%sColumn' % name
            func = source.function_definition(column_func_name, block=block)
            if not func:
                continue
            first_line = func.first_block_line
            last_line = func.last_block_line
            if not first_line:
                verbose("Skipping %s, no switch block." % name)
                continue
            assert last_line

            funcs[mname] = func

            switch_stmt_lines = gen_switch_statement(name, mdecl)

            old_lines = lines[first_line+1:last_line]
            if old_lines != switch_stmt_lines:
                out("Updating %s." % column_func_name)

                assert lines[last_line] == '    }'
                assert lines[first_line] == block[0]

                lines = (
                    lines[:first_line+1] +
                    switch_stmt_lines  +
                    lines[last_line:]
                )

                with open(path, 'wb') as f:
                    f.write('\n'.join(lines))
                    f.write('\n')

                source = SourceFile(path)
                lines = copy(source.lines)

class SyncTraceStoreIndexHeader(InvariantAwareCommand):
    """
    Finds all multi-line macros and aligns trailing slashes where necessary.
    """
    _shortname_ = 'tsi'

    path = None
    _path = None
    class PathArg(PathInvariant):
        _help = "path of the TraceStoreIndex.h file [default: %default]"

        def _default(self):
            return join_path(
                dirname(__file__),
                "../../../Tracer/TraceStoreIndex.h"
            )

    def run(self):
        out = self._out
        options = self.options
        verbose = self._verbose

        path = self._path

        from tracer.sourcefile import SourceFile

        source = SourceFile(path)
        orig_data = source.data
        orig_lines = source.lines

        lines = source.lines
        dirty = False

        new_lines = []

        count = 0
        digits = 1
        for line in lines:
            parts = line.split(' ')
            if digits == 1:
                parts[-1] = str(count)
            else:
                assert digits == 2, digits
                parts = parts[:-2]
                parts.append(str(count))

            new_line = ' '.join(parts)
            if new_line != line:
                dirty = True

            new_lines.append(new_line)

            count += 1
            if digits == 1 and count == 10:
                digits = 2

        if dirty:
            with open(path, 'wb') as f:
                f.write('\n'.join(new_lines))
                f.write('\n')
            out("Updated header.")

class ReplaceUuid(InvariantAwareCommand):
    """
    Replaces UUIDs in a file with new ones.
    """

    path = None
    _path = None
    class PathArg(PathInvariant):
        _help = "path of file to replace UUIDs in"

    def run(self):
        out = self._out
        options = self.options
        verbose = self._verbose

        path = self._path

        import re
        from uuid import uuid4
        import linecache

        regex = re.compile(r'[0-9a-f]{8}(?:-[0-9a-f]{4}){4}[0-9a-f]{8}', re.I)

        with open(path, 'r') as f:
            text = f.read()

        uuid_map = {
            src_uuid: str(uuid4()).upper()
                for src_uuid in regex.findall(text)
        }

        for (old_uuid, new_uuid) in uuid_map.items():
            out("Replacing %s -> %s." % (old_uuid, new_uuid))
            text = text.replace(old_uuid, new_uuid)

        if 'vcxproj' in path:
            newline = '\r\n'
        else:
            newline = '\n'

        with open(path, 'w') as f:
            f.write(text)

class CreateNewProjectFromExisting(InvariantAwareCommand):
    """
    Creates a new Visual Studio project from a project in an existing directory.
    """
    _shortname_ = 'np'

    src = None
    class SrcArg(StringInvariant):
        _help = "Source directory to copy."

    dst = None
    class DstArg(StringInvariant):
        _help = "Destination directory."

    def run(self):
        out = self._out
        options = self.options
        verbose = self._verbose

        src = self.src
        dst = self.dst

        from os import mkdir, listdir
        from os.path import isdir

        from tracer.path import join_path

        if not isdir(dst):
            mkdir(dst)

        src_vcxproj = '%s/%s.vcxproj' % (src, src)
        dst_vcxproj = '%s/%s.vcxproj' % (dst, dst)

        endings = (
            '.c',
            '.h',
            '.asm',
            '.inc',
            '.vcxproj',
            '.vcxproj.filters',
        )

        src_files = [
            '%s/%s' % (src, path)
                for path in listdir(src)
                    if path.endswith(endings)
        ]

        dst_files = {
            src_path: src_path.replace(src, dst)
                for src_path in src_files
        }

        import ipdb
        ipdb.set_trace()


        import re
        from uuid import uuid4
        import linecache

        regex = re.compile(r'[0-9a-f]{8}(?:-[0-9a-f]{4}){4}[0-9a-f]{8}', re.I)

        dst_path

        lines = linecache.getlines(filename)
        num_lines = len(lines)

        i = 0
        while i < num_lines:
            line = lines[i]

class ParseCdbDumpTypes(InvariantAwareCommand):
    """
    Parses the output of cdb-output.txt.
    """

    enable_post_mortem_debugger = None
    class EnablePostMortemDebuggerArg(EnablePostMortemDebuggerInvariant):
        pass

    def run(self):
        from tracer.path import join_path
        from tracerdev.config import ROOT_DIR
        path = join_path(ROOT_DIR, 'cdb-output.txt')

        with open(path, 'r') as f:
            text = f.read()

        from tracer.dbgeng import Struct
        structs = Struct.load_all_from_text(text)

        import IPython
        IPython.embed()




# vim:set ts=8 sw=4 sts=4 tw=80 et                                             :
