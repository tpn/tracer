from __future__ import print_function
import ctypes
import sys
import os

os.environ['COMPUTERNAME']

if os.environ['USERNAME'] == 'r541964':
    sys.path.insert(0, r'c:\users\r541964\home\src\tpn\lib')
    sys.path.insert(0, r'c:\users\r541964\home\src\tracer\PythonApp\lib')
    basedir = r'e:\trace2'
else:
    if os.environ['COMPUTERNAME'] == 'COUGAR':
        basedir = r'S:\Data'
    else:
        basedir = r'C:\Users\Trent\Home\data'
    sys.path.insert(0, r'c:\users\trent\home\src\tpn\lib')
    sys.path.insert(0, r'c:\users\trent\home\src\tracer\PythonApp\lib')

import tpn

#reload(tpn)

from tpn.path import join_path

import tpn.wintypes
reload(tpn.wintypes)
from tpn.wintypes import *

trace_events_dat_path = join_path(basedir, 'trace_events.dat')
trace_events_dat_metadata_path = ''.join((trace_events_dat_path, ':metadata'))

import re

from tpn.convert import (
    convert_windows_typedef_to_python_ctypes_structure,
    TYPEDEF_FILE_STANDARD_INFO,
    PYTHON_CTYPES_FILE_STANDARD_INFO_FORMAT
)

from tpn.util import bits_table, NullObject
null_writer = NullObject()

import tracer.config
conf = tracer.config.Config()
conf.load()

conf.tracer_pythontracer_debug_dll_path

os.path.exists(conf.tracer_pythontracer_debug_dll_path)

import tracer
reload(tracer)

use_debug = True

#print("Press any key to continue.")
#dummy = sys.stdin.read(1)

if use_debug:
    t = tracer.Tracer.create_debug(basedir, conf)
else:
    t = tracer.Tracer.create_release(basedir, conf)

#print("Press any key to continue.")
#dummy = sys.stdin.read(1)

with t:
    bits_table(output=null_writer)
