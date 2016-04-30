import sys
from os.path import (
    join,
    abspath,
    dirname,
    normpath,
)

libdir = abspath(normpath(join(dirname(__file__), 'lib')))

sys.path.insert(0, libdir)

from tracer.cli import main as cli_main
cli_main(program_name='tracerdev', library_name='tracerdev')
