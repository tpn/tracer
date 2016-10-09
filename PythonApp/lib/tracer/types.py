#===============================================================================
# Imports
#===============================================================================
import sys
import ctypes

from ctypes import *
from ctypes.wintypes import *
from .wintypes import *

#===============================================================================
# Globals/Aliases
#===============================================================================

#===============================================================================
# Enums
#===============================================================================

class PYTHON_PATH_ENTRY_TYPE(Structure):
    _fields_ = [
        ('IsModuleDirectory', ULONG, 1),
        ('IsNonModuleDirectory', ULONG, 1),
        ('IsFile', ULONG, 1),
        ('IsClass', ULONG, 1),
        ('IsFunction', ULONG, 1),
        ('IsBuiltin', ULONG, 1),
        ('IsValid', ULONG, 1),
    ]

#===============================================================================
# Classes
#===============================================================================

# Forward definitions.

class PYTHON_PATH_TABLE_ENTRY(Structure):
    pass
PPYTHON_PATH_TABLE_ENTRY = POINTER(PYTHON_PATH_TABLE_ENTRY)

class PYTHON_PATH_TABLE(Structure):
    pass
PPYTHON_PATH_TABLE = POINTER(PYTHON_PATH_TABLE)

class _PYTHON_PATH_TABLE_ENTRY(Structure):
    pass

class PYTHON_PATH_TABLE_ENTRY(Structure):
    pass
PPYTHON_PATH_TABLE_ENTRY = POINTER(PYTHON_PATH_TABLE_ENTRY)

class PYTHON_FUNCTION(Structure):
    pass
PPYTHON_FUNCTION = POINTER(PYTHON_FUNCTION)

class PYTHON_FUNCTION_TABLE_ENTRY(Structure):
    pass
PPYTHON_FUNCTION_TABLE_ENTRY = POINTER(PYTHON_FUNCTION)

class PYTHON_FUNCTION_TABLE(Structure):
    _fields_ = [
        ('GenericTable', RTL_GENERIC_TABLE),
    ]
PPYTHON_FUNCTION_TABLE = POINTER(PYTHON_FUNCTION_TABLE)

class PYTHON_TRACE_EVENT(Structure):
    pass
PPYTHON_TRACE_EVENT = POINTER(PYTHON_TRACE_EVENT)

_PYTHON_PATH_TABLE_ENTRY._fields_ = [
    ('NodeTypeCode', SHORT),
    ('PrefixNameLength', SHORT),
    ('PathEntryType', PYTHON_PATH_ENTRY_TYPE),
    ('NextPrefixTree', PPYTHON_PATH_TABLE_ENTRY),
    ('Links', RTL_SPLAY_LINKS),
    ('Prefix', PSTRING),
]

PYTHON_PATH_TABLE_ENTRY._fields_ = [
    ('PrefixTableEntry', _PYTHON_PATH_TABLE_ENTRY),
    ('Path', STRING),
    ('FullName', STRING),
    ('ModuleName', STRING),
    ('Name', STRING),
    ('ClassName', STRING),
]

PYTHON_FUNCTION._fields_ = [
    ('PathEntry', PYTHON_PATH_TABLE_ENTRY),
    ('ParentPathEntry', PPYTHON_PATH_TABLE_ENTRY),
    ('CodeObject', PVOID),
    ('LineNumbersBitmap', PVOID),
    ('Histogram', PVOID),
    ('CodeLineNumbers', PUSHORT),
    ('ReferenceCount', ULONG),
    ('CodeObjectHash', LONG),
    ('FunctionHash', ULONG),
    ('Unused1', ULONG),
    ('FirstLineNumber', USHORT),
    ('NumberOfLines', USHORT),
    ('NumberOfCodeLines', USHORT),
    ('SizeOfByteCode', USHORT),
    ('Unused2', ULONGLONG),
    ('Unused3', ULONGLONG),
    ('Unused4', ULONGLONG),
]

PYTHON_FUNCTION_TABLE_ENTRY._fields_ = [
    ('Links', RTL_SPLAY_LINKS),
    ('ListEntry', LIST_ENTRY),
    ('Function', PYTHON_FUNCTION),
]
SizeOfPythonFunctionTableEntry = sizeof(PYTHON_FUNCTION_TABLE_ENTRY)

PYTHON_TRACE_EVENT._fields_ = [
    ('Timestamp', ULONGLONG),
    ('Function', PPYTHON_FUNCTION),

    ('WorkingSetSize', ULONGLONG),
    ('PageFaultCount', ULONGLONG),
    ('CommittedSize', ULONGLONG),

    ('ReadTransferCount', ULONGLONG),
    ('WriteTransferCount', ULONGLONG),

    ('HandleCount', ULONG),
    ('ThreadId', ULONG),
    ('ElapsedMicroseconds', ULONG),

    ('WorkingSetDelta', LONG),
    ('CommittedDelta', LONG),
    ('ReadTransferDelta', ULONG),
    ('WriteTransferDelta', ULONG),

    ('CodeObjectHash', ULONG),
    ('FunctionHash', ULONG),

    ('PathHash', LONG),
    ('FullNameHash', LONG),
    ('ModuleNameHash', LONG),
    ('ClassNameHash', LONG),
    ('NameHash', LONG),

    ('Flags', ULONG),

    ('HandleDelta', SHORT),
    ('PageFaultDelta', USHORT),

    ('LineNumber', USHORT),
    ('FirstLineNumber', USHORT),
    ('NumberOfLines', USHORT),
    ('NumberOfCodeLines', USHORT),
]

#===============================================================================
# Functions
#===============================================================================


# vim:set ts=8 sw=4 sts=4 tw=80 et                                             :
