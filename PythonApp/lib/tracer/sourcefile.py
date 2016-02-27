#===============================================================================
# Imports
#===============================================================================
import re

import sys

import textwrap

from collections import namedtuple

from .util import (
    strip_linesep_if_present,
    memoize,
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

#===============================================================================
# Helpers
#===============================================================================

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
    def multiline_defines(self):
        results = {}
        import pdb
        for define in self.defines:
            lines = []
            (lineno, line) = define
            if not line.endswith('\\'):
                continue

            name = line.replace('#define ', '')
            name = name[:name.find(' ')]

            lineno += 1
            line = self.lines[lineno]

            while line.endswith('\\'):
                lines.append((lineno, line))
                lineno += 1
                line = self.lines[lineno]

            results[name] = lines

        return results

    def function_definition(self, funcname, block=None):
        partial = False
        found = False
        for (lineno, line) in enumerate(self.lines):
            if not partial:
                if line.startswith(funcname):
                    partial = True
            else:
                next_line = self.lines[lineno+1]
                if next_line == '{':
                    found = True
                    break
                elif next_line.startswith('    '):
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
        for (lineno, line) in self.multiline_defines[name]:
            length = len(line)
            line = line[4:line.find(';')]
            (typedef, funcname) = line.split(' ')
            results.append(Function(lineno, length, typedef, funcname))
        return results

    def replace_blocks():
        pass

    @memoize
    def functions_from_head_macro(self, name):
        return self.multiline_defines.get(name)

class HeaderFile(SourceFile):
    pass

class CodeFile(SourceFile):
    pass


# vim:set ts=8 sw=4 sts=4 tw=80 et                                             :
