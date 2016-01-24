"""
Starts profiling, expected to start with normal program
to start as first argument and directory to run from as
the second argument.
"""

import sys

if sys.platform == 'cli':
    print('Python profiling is not supported on IronPython, press enter to exit...')
    raw_input()
    sys.exit(1)

import os
import vspyprof

# arguments are path to profiling DLL, working dir, custom profile DLL or -,
# normal arguments which should include a filename to execute

# change to directory we expected to start from
python_file = None
sys.argv.pop(0)
profdll = sys.argv.pop(0)
run_dir = sys.argv.pop(0)
custdll = sys.argv.pop(0)
if custdll == '-':
    custdll = None
if custdll and not custdll.endswith('.dll'):
    python_file = custdll
    custdll = None
if not python_file:
    python_file = sys.argv.pop(0)

# fix sys.path to be our real starting dir, not this one
sys.path.insert(0, run_dir)

# set file appropriately, fix up sys.argv...
__file__ = python_file
sys.argv.insert(0, python_file)

# remove all state we imported
del sys, os

# and start profiling
try:
    vspyprof.profile(
        __file__,
        globals(),
        locals(),
        profdll,
        custprofdllname=custdll,
    )
except SystemExit:
    import sys, msvcrt, os
    if sys.exc_info()[1].code:
        env_var = 'VSPYPROF_WAIT_ON_ABNORMAL_EXIT'
    else:
        env_var = 'VSPYPROF_WAIT_ON_NORMAL_EXIT'
    if env_var in os.environ:
        sys.stdout.write('Press any key to continue . . .')
        sys.stdout.flush()
        msvcrt.getch()
except:
    import sys, msvcrt, os, traceback
    if 'VSPYPROF_WAIT_ON_ABNORMAL_EXIT' in os.environ:
        traceback.print_exc()
        sys.stdout.write('Press any key to continue . . .')
        sys.stdout.flush()
        msvcrt.getch()
    else:
        raise
else:
    import sys, msvcrt, os
    if 'VSPYPROF_WAIT_ON_NORMAL_EXIT' in os.environ:
        sys.stdout.write('Press any key to continue . . .')
        sys.stdout.flush()
        msvcrt.getch()
