#===============================================================================
# Imports
#===============================================================================

import re

from tempfile import NamedTemporaryFile

from .path import join_path

from .util import (
    try_remove_file,
    ProcessWrapper,
)

from .config import DEBUG_DIR

import subprocess

#===============================================================================
# Globals/Aliases
#===============================================================================

CDB_TEMPLATE = """\
bp ModuleLoader!MaybeBreak
g
.logopen %s
%s
.logclose
q"""

#===============================================================================
# Helpers
#===============================================================================

def run_cdb_commands(lines):

    commands_file = NamedTemporaryFile(suffix='_commands', delete=False)
    output_file = commands_file.name.replace('_commands', '_output')

    commands = CDB_TEMPLATE % (output_file, '\n'.join(lines))
    commands_file.write(commands)
    commands_file.close()

    module_loader_exe = join_path(DEBUG_DIR, 'ModuleLoader.exe')
    cmd = 'cdb -cf %s %s' % (commands_file.name, module_loader_exe)

    result = os.system(cmd)

    with open(output_file, 'r') as f:
        output = f.read()

    try_remove_file(commands_file)
    try_remove_file(output_file)

    return output


class ModuleLoaderExe(object):

    @classmethod
    def run_single(cls, command):

        text = '\n'.join([
            'bp ModuleLoader!MaybeBreak',
            'g',
            command,
            'q',
        ])

        with NamedTemporaryFile(delete=False) as f:
            path = f.name
            f.write(text)

        module_loader_exe = join_path(DEBUG_DIR, 'ModuleLoader.exe')
        commandline = 'cdb -cf %s %s' % (path, module_loader_exe)

        output = subprocess.check_output(commandline)

        try_remove_file(path)

        splits = re.split('^0:000> ', output, flags=re.MULTILINE)
        split = ''.join(('0:000> ', splits[-2]))

        return split

def run_single(command):
    return ModuleLoaderExe.run_single(command)


# vim:set ts=8 sw=4 sts=4 tw=80 et                                             :
