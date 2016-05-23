from __future__ import print_function
import ctypes
import sys
import os

if os.environ['USERNAME'] == 'r541964':
    sys.path.insert(0, r'd:\src\tpn\lib')
    sys.path.insert(0, r'd:\src\tracer\PythonApp\lib')
    basedir = r'e:\trace2'
else:
    if os.environ['COMPUTERNAME'] == 'COUGAR':
        basedir = r'S:\trace'
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

print("Press any key to continue.")
dummy = sys.stdin.read(1)

if use_debug:
    print("using debug")
    t = tracer.Tracer.create_debug(basedir, conf)
else:
    print("not using debug")
    t = tracer.Tracer.create_release(basedir, conf)

print("Created tracer...")
print("Press any key to continue.")
dummy = sys.stdin.read(1)

t.add_module('tpn')
t.enable_memory_tracing()
t.enable_io_counters_tracing()
t.enable_handle_count_tracing()

with t:
    for i in range(1000):
        bits_table(output=null_writer)

print("Press any key to continue.")
dummy = sys.stdin.read(1)

t.finish()

print("Press any key to continue.")
dummy = sys.stdin.read(1)

