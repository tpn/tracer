#===============================================================================
# Imports
#===============================================================================
import re

import sys

import textwrap

from collections import namedtuple

from .util import (
    memoize,
    align_trailing_slashes,
    strip_linesep_if_present,
)

from .command import (
    Command,
    CommandError,
)

from .invariant import (
    BoolInvariant,
    PathInvariant,
    StringInvariant,
    DirectoryInvariant,
    InvariantAwareObject,
    PositiveIntegerInvariant,
)

from .commandinvariant import (
    InvariantAwareCommand,
)

#===============================================================================
# Named Tuples
#===============================================================================
Function = namedtuple(
    'Function',
    ['lineno', 'length', 'typedef', 'funcname']
)

FunctionDefinition = namedtuple(
    'FunctionDefinition', [
        'funcname',
        'first_line',
        'last_line',
        'first_block_line',
        'last_block_line',
        'last_return_line',
    ]
)

MultilineMacroDefinition = namedtuple(
    'MultilineMacroDefinition', [
        'name',
        'first_lineno',
        'last_lineno',
        'lines',
    ]
)

#===============================================================================
# Helpers
#===============================================================================
def convert_function_decls_to_funcptr_typedefs(text, prefix=None, prepend=None):
    if not prefix:
        prefix = '^BOOL\nIMAGEAPI\n'
    if not prepend:
        prepend = 'typedef\n_Check_return_\n_Success_(return != 0)'.split('\n')
    pre_lines = prefix.count('\n')
    name_line = pre_lines
    pattern = '%s.*?    \);$' % prefix
    regex = re.compile(pattern, re.MULTILINE | re.DOTALL)
    matches = regex.findall(text)
    regex = re.compile('[A-Z][^A-Z]*')
    results = []
    for match in matches:
        match_lines = match.splitlines()
        name = match_lines[name_line][:-1]
        tokens = regex.findall(name)
        upper_tokens = [ t.upper() for t in tokens ]
        cap_name = '_'.join(upper_tokens)
        func_lines = (
            prepend +
            match_lines[0:pre_lines] +
            [ '(%s)(' % cap_name ] +
            match_lines[name_line+1:] +
            [ 'typedef %s *P%s;\n' % (cap_name, cap_name) ]
        )
        func_text = '\n'.join(func_lines)
        results.append(func_text)

    return results

#===============================================================================
# Classes
#===============================================================================
class SourceFile(InvariantAwareObject):
    path = None
    _path = None
    class PathArg(PathInvariant):
        pass

    def __init__(self, path):
        InvariantAwareObject.__init__(self)
        self.path = path

    @property
    @memoize
    def data(self):
        with open(self._path, 'r') as f:
            return f.read()

    @property
    @memoize
    def lines(self):
        return self.data.splitlines()

    @property
    @memoize
    def defines(self):
        results = []
        for (lineno, line) in enumerate(self.lines):
            if line.startswith('#define'):
                results.append((lineno, line))
        return results

    @property
    @memoize
    def lines_ending_with_backslash(self):
        results = []
        for (lineno, line) in enumerate(self.lines):
            if line.endswith('\\'):
                results.append((lineno, line))
        return results

    @property
    @memoize
    def blank_lines(self):
        results = []
        for (lineno, line) in enumerate(self.lines):
            if not line or not line.replace(' ', ''):
                results.append((lineno, line))
        return results

    @property
    @memoize
    def multiline_macro_defines(self):
        results = {}

        for define in self.defines:
            lines = []
            (lineno, line) = define
            if not line.endswith('\\'):
                continue

            name = line.replace('#define ', '')
            name = name[:name.find(' ')]
            first_lineno = lineno
            lines.append(line)

            lineno += 1
            line = self.lines[lineno]

            while line.endswith('\\'):
                lines.append(line)
                lineno += 1
                line = self.lines[lineno]

            last_lineno = lineno
            lines.append(line)

            results[name] = MultilineMacroDefinition(
                name=name,
                first_lineno=first_lineno,
                last_lineno=last_lineno,
                lines=lines
            )

        return results

    def function_definition(self, funcname, block=None):
        partial = False
        found = False
        func_line = None
        for (lineno, line) in enumerate(self.lines):
            if not partial:
                if line.startswith(funcname):
                    partial = True
                    func_line = line
                    continue
            else:
                if line == '{':
                    found = True
                    break
                elif line.startswith('    '):
                    continue
                else:
                    partial = False
                    continue

        if not found:
            return None

        i = lineno-1
        prev_line = self.lines[i]
        while prev_line:
            i -= 1
            prev_line = self.lines[i]
        first_line = i

        first_block = None
        last_block = None
        last_return = None

        block_length = len(block) if block else None

        i = lineno+2
        next_line = self.lines[i]
        while next_line != '}':
            if next_line == '    }':
                last_block = i
            elif next_line.startswith('    return '):
                last_return = i

            if block:
                next_block = self.lines[i:i+block_length]
                if block == next_block:
                    if not first_block:
                        first_block = i

            i += 1
            next_line = self.lines[i]

        last_line = i

        return FunctionDefinition(
            funcname,
            first_line,
            last_line,
            first_block,
            last_block,
            last_return,
        )

    @memoize
    def functions_from_multiline_define(self, name):
        results = []
        macro = self.multiline_macro_defines[name]
        for (lineno, line) in enumerate(macro.lines):
            if line.startswith('#define'):
                continue
            length = len(line)
            line = line[4:line.find(';')]
            (typedef, funcname) = line.split()
            results.append(Function(lineno, length, typedef, funcname))
        return results


class HeaderFile(SourceFile):
    pass

class CodeFile(SourceFile):
    pass


# vim:set ts=8 sw=4 sts=4 tw=80 et                                             :
